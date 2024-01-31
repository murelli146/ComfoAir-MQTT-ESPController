// setup.cpp

#include "setup.h"  // Importiere die Header-Datei

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial1.println();
  Serial1.print("Connecting to ");
  Serial1.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial1.print(".");
  }

  randomSeed(micros());

  Serial1.println("");
  Serial1.println("WiFi connected");
  Serial1.println("IP address: ");
  Serial1.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial1.println("Attempting MQTT connection...");
    if (client.connect("ESP8266Client", mqttUser, mqttPassword)) {
      Serial1.println("connected");
      // Hier können Sie Ihre MQTT-Abonnements hinzufügen
      // client.subscribe("your/topic");
      break;  // Verbindung erfolgreich, Schleife verlassen
    } else {
      Serial1.print("failed, rc=");
      Serial1.print(client.state());
      Serial1.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setupOTA() {
  ArduinoOTA.setHostname(OTA_HOSTNAME);  // Setzen Sie einen eindeutigen Namen
  ArduinoOTA.setPassword(OTA_PASSWORD);  // Setzen Sie hier Ihr Passwort
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "Sketch";
    } else {  // U_SPIFFS
      type = "Dateisystem";
    }
    // Hinweis: Wenn das Dateisystem aktualisiert wird, sollten alle
    // Dateisystemoperationen hier beendet werden.
    Serial1.println("Starte OTA-Update " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial1.println("\nEnde");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial1.printf("Fortschritt: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial1.printf("Fehler[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial1.println("Authentifizierungsfehler");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial1.println("Beginn-Fehler");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial1.println("Verbindungsfehler");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial1.println("Empfangsfehler");
    } else if (error == OTA_END_ERROR) {
      Serial1.println("Ende-Fehler");
    }
  });
  ArduinoOTA.begin();
}

void setup_mqtt() {
  client.setServer(mqtt_Server, mqttPort);

  // Verbinden mit MQTT Broker
  while (!client.connected()) {
    Serial1.println("Verbindung mit MQTT Broker...");
    if (client.connect("ESP8266Client", mqttUser, mqttPassword)) {
      Serial1.println("MQTT Broker Verbunden");
    } else {
      Serial1.println("Fehler, rc=" + String(client.state()) + " Versuche es in 5 Sekunden erneut");
      delay(5000);
    }
  }
}