//ComfoAir-MQTT-ESPController.ino

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>

#include "setup.h"
#include "comfoair.h"
#include "config.h"

// OTA Firmwareupdate
const char* OTA_HOSTNAME = "ComfoAir350";
const char* OTA_PASSWORD = "1234";

// WLAN
const char* ssid = my_ssid;          //const char* ssid = "ihre_ssid";  !!!
const char* password = my_password;  //const char* password = "ihr_Passwort"; !!!

// MQTT
const char* mqtt_Server = my_mqtt_Server;  //const char* mqtt_Server = "192.168.0.77"; !!!
const int mqttPort = 1883;
const char* mqttUser = "Ihr_MQTT_Benutzername";  // Optional
const char* mqttPassword = "Ihr_MQTT_Passwort";  // Optional
const char* maintopic = "ComfoAir/";
WiFiClient espClient;
PubSubClient client(espClient);

// ComfoAir
unsigned long COMMAND_INTERVAL = 40000;  // Zeit zwischen den Befehlen für Zyklisch senden
unsigned long lastCommandTime = 0;
unsigned long lastSendTime = millis();                              // Verzögerung nach commandSend
unsigned long requestProcessingStartTime = 0;                       //Zeitmessung Abarbeitung requests[]
int ABL_0, ABL_1, ABL_2, ZUL_0, ZUL_1, ZUL_2, ABL_3, ZUL_3, STUFE;  // globale Variablen für Stufensetup

const char* requests[] = {
  "00DD00",  // Betriebsstunden
  "00D100",  // Temperaturen
  "00CD00",  // Stufen
  "000B00",  // Motoren
  "000D00",  // Bypass
  "00D900",  // Fehler
  "00E100",  // Vorheizung
  "006900",  // Geräteinfo
};

int currentRequestIndex = 0;
const int numRequests = sizeof(requests) / sizeof(requests[0]);

// Maximale Größe der Queue
#define MAX_QUEUE_SIZE 10

// Queue-Struktur
struct CommandQueue {
  String commands[MAX_QUEUE_SIZE];
  int front = 0;
  int rear = -1;
  int count = 0;

  // Füge einen Befehl zur Queue hinzu
  void enqueue(String cmd) {
    if (count < MAX_QUEUE_SIZE) {
      rear = (rear + 1) % MAX_QUEUE_SIZE;
      commands[rear] = cmd;
      count++;
    }
  }

  // Entferne einen Befehl aus der Queue
  String dequeue() {
    String cmd = "";
    if (count > 0) {
      cmd = commands[front];
      front = (front + 1) % MAX_QUEUE_SIZE;
      count--;
    }
    return cmd;
  }

  // Überprüfe, ob die Queue leer ist
  bool isEmpty() {
    return count == 0;
  }
};

// Globale Instanz der Queue
CommandQueue commandQueue;

void setup() {
  setupOTA();
  Serial.begin(9600);
  DEBUG_INIT(115200);
  setup_wifi();
  setup_mqtt();

  // Abonnieren der Topics
  client.subscribe("ComfoAir/cmd/#");

  commandQueue.enqueue("00CD00");  // Globale Variablen setzen für Stufensetup
}

// Logik Stoßlüftung
bool stosslueftungAktiv = false;
int aktuelleStufe = 0;
int nachlaufzeit = 1200;  // in Sekunden
unsigned long stosslueftungStartzeit = 0;

void loop() {

  ArduinoOTA.handle();
  // MQTT
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Logiken
  // Stoßlüftung
  if (stosslueftungAktiv && millis() - stosslueftungStartzeit > nachlaufzeit * 1000) {
    stosslueftungAktiv = false;
    commandQueue.enqueue("0099010" + String(aktuelleStufe));  // Zurück zur ursprünglichen Stufe
    commandQueue.enqueue("00CD00");
  }



  //_______ComfoAir_________________________________________________
  // Überprüfen, ob seit dem letzten gesendeten Befehl 200 ms vergangen sind
  if (currentState == SENDING_COMMAND && millis() - lastSendTime > 200) {
    currentState = WAITING_FOR_ACK;
  }

  // Überprüfen, ob eine Antwort von der ComfoAir vorliegt
  if (currentState == WAITING_FOR_ACK || currentState == RECEIVING_DATA || currentState == CHECKING_DATA) {
    checkForResponse(Serial);
  }

  // Befehle senden, wenn im IDLE-Zustand
  if (currentState == IDLE) {
    if (!commandQueue.isEmpty()) {
      // Priorisiere MQTT Befehle
      String command = commandQueue.dequeue();
      sendCommand(Serial, command.c_str());
      lastSendTime = millis();
      currentState = SENDING_COMMAND;
    } else if (millis() - lastCommandTime > COMMAND_INTERVAL) {
      if (currentRequestIndex == 0) {           //Zeitmessung für requests[]
        requestProcessingStartTime = millis();  // Startzeit speichern
      }
      // Nächsten Befehl aus dem Array senden
      sendCommand(Serial, requests[currentRequestIndex]);
      lastSendTime = millis();
      currentState = SENDING_COMMAND;

      // Zum nächsten Befehl wechseln oder zurücksetzen
      currentRequestIndex = (currentRequestIndex + 1) % numRequests;
      lastCommandTime = millis();

      //Zeitmessung für requests[]
      if (currentRequestIndex == 0) {
        unsigned long processingTime = millis() - requestProcessingStartTime;
        client.publish("ComfoAir/status/sendezyklus", String(processingTime / 1000).c_str());
        DEBUG_PRINT("Gesamte Abarbeitungszeit für requests[]: ");
        DEBUG_PRINT(processingTime);
        DEBUG_PRINTLN(" ms");
      }
    }
  }
}

void callback(char* topic, byte* message, unsigned int length) {
  // Erstelle einen String für das Thema und reserviere Speicher
  String topicStr = String(topic);
  topicStr.reserve(100);  // Angenommene maximale Länge des Themas

  // Erstelle einen String für die Nachricht und reserviere Speicher
  String messageStr;
  messageStr.reserve(100);  // Angenommene maximale Länge der Nachricht
                            // Konvertiere die Nachricht von byte* zu String
  for (int i = 0; i < length; i++) {
    messageStr += (char)message[i];
  }

  // Erstelle einen String und reserviere ausreichend Speicher
  String daten;
  daten.reserve(100);  // Reserviere Speicher für die maximale Länge des Strings

  DEBUG_PRINT("Topic: ");
  DEBUG_PRINTLN(topicStr);

  // Verarbeite das Thema und die Nachricht
  if (topicStr == "ComfoAir/cmd/stufe") {
    int value = messageStr.toInt();
    if (value >= 0 && value <= 3) {
      daten = "0099010";
      daten += (value + 1);  //Mapping der Stufe (eins versetzt)

      commandQueue.enqueue(daten);
      commandQueue.enqueue("00CD00");
    }
  }

  if (topicStr == "ComfoAir/cmd/soll") {
    float value = messageStr.toFloat();  // Konvertiere den String in einen float
    if (value >= 12.0 && value <= 28.0) {
      value = round(value * 2) / 2.0;  // Runde auf die nächste halbe Zahl

      // Berechne den Hex-Wert und erstelle den Datenstring
      daten = String(static_cast<int>((value + 20) * 2), HEX);
      daten.toUpperCase();  // Konvertiere den String in Großbuchstaben
      daten = "00D301" + daten;

      // Füge den Befehl zur Warteschlange hinzu
      commandQueue.enqueue(daten);
      commandQueue.enqueue("00D100");  // Lese den aktuellen Status nach dem Senden des Befehls
    }
  }

  if (topicStr == "ComfoAir/cmd/filterreset") {
    commandQueue.enqueue("00DB0400000001");
    commandQueue.enqueue("00D900");
  }

  if (topicStr == "ComfoAir/cmd/errorreset") {
    commandQueue.enqueue("00DB0401000000");
    commandQueue.enqueue("00D900");
  }

  if (topicStr == "ComfoAir/cmd/firmwareinfo") {
    commandQueue.enqueue("006900");
  }

  if (topicStr.startsWith("ComfoAir/cmd/stufensetup")) {
    bool stufeGeaendert = false;

    if (topicStr == "ComfoAir/cmd/stufensetup/ABL_0") {
      int value = messageStr.toInt();
      if (value >= 15 && value <= 97 && value != ABL_0) {
        ABL_0 = value;
        stufeGeaendert = true;
      }
    } else if (topicStr == "ComfoAir/cmd/stufensetup/ABL_1") {
      int value = messageStr.toInt();
      if (value >= 16 && value <= 98 && value != ABL_1) {
        ABL_1 = value;
        stufeGeaendert = true;
      }
    } else if (topicStr == "ComfoAir/cmd/stufensetup/ABL_2") {
      int value = messageStr.toInt();
      if (value >= 17 && value <= 99 && value != ABL_2) {
        ABL_2 = value;
        stufeGeaendert = true;
      }
    } else if (topicStr == "ComfoAir/cmd/stufensetup/ZUL_0") {
      int value = messageStr.toInt();
      if (value >= 15 && value <= 97 && value != ZUL_0) {
        ZUL_0 = value;
        stufeGeaendert = true;
      }
    } else if (topicStr == "ComfoAir/cmd/stufensetup/ZUL_1") {
      int value = messageStr.toInt();
      if (value >= 16 && value <= 98 && value != ZUL_1) {
        ZUL_1 = value;
        stufeGeaendert = true;
      }
    } else if (topicStr == "ComfoAir/cmd/stufensetup/ZUL_2") {
      int value = messageStr.toInt();
      if (value >= 17 && value <= 99 && value != ZUL_2) {
        ZUL_2 = value;
        stufeGeaendert = true;
      }
    } else if (topicStr == "ComfoAir/cmd/stufensetup/ABL_3") {
      int value = messageStr.toInt();
      if (value >= 18 && value <= 100 && value != ABL_3) {
        ABL_3 = value;
        stufeGeaendert = true;
      }
    } else if (topicStr == "ComfoAir/cmd/stufensetup/ZUL_3") {
      int value = messageStr.toInt();
      if (value >= 18 && value <= 100 && value != ZUL_3) {
        ZUL_3 = value;
        stufeGeaendert = true;
      }
    }

    if (stufeGeaendert) {
      char befehl[24];
      sprintf(befehl, "00CF09%02X%02X%02X%02X%02X%02X%02X%02X00", ABL_0, ABL_1, ABL_2, ZUL_0, ZUL_1, ZUL_2, ABL_3, ZUL_3);
      DEBUG_PRINT("Stufensetup Befehl: ");
      DEBUG_PRINTLN(befehl);

      commandQueue.enqueue(befehl);
      commandQueue.enqueue("00CD00");
    }
  }

  if (topicStr == "ComfoAir/cmd/sendezyklus") {
    int value = messageStr.toInt();
    if (value >= 60 && value <= 600) {
      COMMAND_INTERVAL = value * 1000 / (numRequests - 1);
      DEBUG_PRINT("Neues COMMAND_INTERVAL: ");
      DEBUG_PRINTLN(numRequests - 1);
    }
  }

  if (topicStr == "ComfoAir/cmd/stosslueftung") {
    if (messageStr == "1") {
      if (!stosslueftungAktiv) {
        aktuelleStufe = STUFE;
      }
      stosslueftungAktiv = true;
      commandQueue.enqueue("00990104");  // Stufe 3 setzen
      commandQueue.enqueue("00CD00");
      stosslueftungStartzeit = millis();
    } else if (messageStr == "0") {
      stosslueftungAktiv = false;
      commandQueue.enqueue("0099010" + String(aktuelleStufe));  // Zurück zur ursprünglichen Stufe
      commandQueue.enqueue("00CD00");
    }
  }

  if (topicStr == "ComfoAir/cmd/nachlaufzeit") {
    nachlaufzeit = messageStr.toInt();
  }
}
