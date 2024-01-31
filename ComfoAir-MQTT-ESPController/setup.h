// setup.h

#ifndef SETUP_H
#define SETUP_H

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>

extern const char* ssid;
extern const char* password;

extern const char* mqtt_Server;
extern const int mqttPort;
extern const char* mqttUser;      // Optional
extern const char* mqttPassword;  // Optional
extern const char* maintopic;
extern PubSubClient client;

extern const char* OTA_HOSTNAME;
extern const char* OTA_PASSWORD;

void setup_wifi();  // Deklaration der setupFunction
void reconnect();
void setupOTA();
void setup_mqtt();

#endif
