// setup.cpp

#include "setup.h"  // Importiere die Header-Datei

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  DEBUG_PRINTLN();
  DEBUG_PRINT("Connecting to ");
  DEBUG_PRINTLN(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    DEBUG_PRINT(".");
  }

  randomSeed(micros());

  DEBUG_PRINTLN("");
  DEBUG_PRINTLN("WiFi connected");
  DEBUG_PRINTLN("IP address: ");
  DEBUG_PRINTLN(WiFi.localIP());
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
    DEBUG_PRINTLN("Starte OTA-Update " + type);
  });
  ArduinoOTA.onEnd([]() {
    DEBUG_PRINTLN("\nEnde");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    DEBUG_PRINTF("Fortschritt: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    DEBUG_PRINTF("Fehler[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      DEBUG_PRINTLN("Authentifizierungsfehler");
    } else if (error == OTA_BEGIN_ERROR) {
      DEBUG_PRINTLN("Beginn-Fehler");
    } else if (error == OTA_CONNECT_ERROR) {
      DEBUG_PRINTLN("Verbindungsfehler");
    } else if (error == OTA_RECEIVE_ERROR) {
      DEBUG_PRINTLN("Empfangsfehler");
    } else if (error == OTA_END_ERROR) {
      DEBUG_PRINTLN("Ende-Fehler");
    }
  });
  ArduinoOTA.begin();
}

void setup_mqtt() {
  client.setServer(mqtt_Server, mqttPort);
  client.setCallback(callback);

  // Verbinden mit MQTT Broker
  while (!client.connected()) {
    DEBUG_PRINTLN("Verbindung mit MQTT Broker...");
    if (client.connect("ESP8266_ComfoAir", mqttUser, mqttPassword)) {
      DEBUG_PRINTLN("MQTT Broker Verbunden");
    } else {
      DEBUG_PRINTLN("Fehler, rc=" + String(client.state()) + " Versuche es in 5 Sekunden erneut");
      delay(5000);
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    DEBUG_PRINTLN("Attempting MQTT connection...");
    if (client.connect("ESP8266Client", mqttUser, mqttPassword)) {
      DEBUG_PRINTLN("connected");
      // Hier können Sie Ihre MQTT-Abonnements hinzufügen
      client.subscribe("ComfoAir/cmd/#");
      break;  // Verbindung erfolgreich, Schleife verlassen
    } else {
      DEBUG_PRINT("failed, rc=");
      DEBUG_PRINT(client.state());
      DEBUG_PRINTLN(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
