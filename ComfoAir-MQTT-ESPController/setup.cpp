// setup.cpp

#include "setup.h"  // Importiere die Header-Datei

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
//  DEBUG_PRINT("Connecting to ");
//  DEBUG_PRINT(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
//    DEBUG_PRINT(".");
  }

  randomSeed(micros());

//  DEBUG_PRINT("");
//  DEBUG_PRINT("WiFi connected");
//  DEBUG_PRINT("IP address: ");
//  DEBUG_PRINT(WiFi.localIP());
snprintf(debugMsg, sizeof(debugMsg), "WiFi IP address: %s", WiFi.localIP());
DEBUG_PRINT(debugMsg);

}

void checkWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    setup_wifi();
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
    DEBUG_PRINT("OTA Starte OTA-Update " + type);
  });
  ArduinoOTA.onEnd([]() {
    DEBUG_PRINT("OTA Ende");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    DEBUG_PRINTF("OTA Fortschritt: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    DEBUG_PRINTF("OTA Fehler[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      DEBUG_PRINT("OTA Authentifizierungsfehler");
    } else if (error == OTA_BEGIN_ERROR) {
      DEBUG_PRINT("OTA Beginn-Fehler");
    } else if (error == OTA_CONNECT_ERROR) {
      DEBUG_PRINT("OTA Verbindungsfehler");
    } else if (error == OTA_RECEIVE_ERROR) {
      DEBUG_PRINT("OTA Empfangsfehler");
    } else if (error == OTA_END_ERROR) {
      DEBUG_PRINT("OTA Ende-Fehler");
    }
  });
  ArduinoOTA.begin();
}

void setup_mqtt() {
  client.setServer(mqtt_Server, mqttPort);
  client.setCallback(callback);

  // Verbinden mit MQTT Broker
  while (!client.connected()) {
//    DEBUG_PRINT("Verbindung mit MQTT Broker...");
    if (client.connect("ESP8266_ComfoAir", mqttUser, mqttPassword, "ComfoAir/LWT", 0, true, "offline")) {
      DEBUG_PRINT("MQTT Broker Verbunden");
      client.publish("ComfoAir/LWT", "online", true);
    } else {
//      DEBUG_PRINT("Fehler, rc=" + String(client.state()) + " Versuche es in 5 Sekunden erneut");
      delay(5000);
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
//    DEBUG_PRINT("Attempting MQTT connection...");
    if (client.connect("ESP8266_ComfoAir", mqttUser, mqttPassword, "ComfoAir/LWT", 0, true, "offline")) {
      DEBUG_PRINT("MQTT RC Broker Verbunden");
      client.publish("ComfoAir/LWT", "online", true);
      // Hier können Sie Ihre MQTT-Abonnements hinzufügen
      client.subscribe("ComfoAir/cmd/#");
      break;  // Verbindung erfolgreich, Schleife verlassen
    } else {
//      DEBUG_PRINT("failed, rc=");
//      DEBUG_PRINT(client.state());
//      DEBUG_PRINT(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// MQTT-Debug-Funktionen
void mqttDebugPrint(const char* message) {
  client.publish("ComfoAir/debug", message);
}

void mqttDebugPrint(String message) {
  client.publish("ComfoAir/debug", message.c_str());
}

void mqttDebugPrint(int message) {
  client.publish("ComfoAir/debug", String(message).c_str());
}

void mqttDebugPrint(unsigned long message) {
  client.publish("ComfoAir/debug", String(message).c_str());
}

void mqttDebugPrint(IPAddress message) {
  client.publish("ComfoAir/debug", message.toString().c_str());
}

void mqttDebugPrintf(const char* format, ...) {
  char buffer[256];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  client.publish("ComfoAir/debug", buffer);
}
