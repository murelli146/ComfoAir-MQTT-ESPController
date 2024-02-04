// setup.h

#ifndef SETUP_H
#define SETUP_H

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>

#define DEBUG 1 // Setzen Sie dies auf 1, um Debugging zu aktivieren, oder auf 0, um es zu deaktivieren

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

void callback(char* topic, byte* message, unsigned int length); // von ComfoAir-MQTT-ESPController.ino 

#if DEBUG
#define DEBUG_INIT(speed) Serial1.begin(speed)
#define DEBUG_PRINT(x) Serial1.print(x)
#define DEBUG_PRINTLN(x) Serial1.println(x)
#define DEBUG_PRINT_HEX(x) Serial1.print(x, HEX)
#define DEBUG_PRINTLN_HEX(x) Serial1.println(x, HEX)
#define DEBUG_PRINT_BIN(x) Serial1.print(x, BIN)
#define DEBUG_PRINTLN_BIN(x) Serial1.println(x, BIN)
#define DEBUG_PRINTF(...) Serial1.printf(__VA_ARGS__)
#else
#define DEBUG_INIT(speed)
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINT_HEX(x)
#define DEBUG_PRINTLN_HEX(x)
#define DEBUG_PRINT_BIN(x)
#define DEBUG_PRINTLN_BIN(x)
#define DEBUG_PRINTF(...)
#endif

#endif
