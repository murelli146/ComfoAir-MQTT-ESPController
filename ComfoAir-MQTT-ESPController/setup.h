// setup.h

#ifndef SETUP_H
#define SETUP_H

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>

//#define DEBUG 1 // Setzen Sie dies auf 1, um Debugging zu aktivieren, oder auf 0, um es zu deaktivieren

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

// Speicherzuweisung Debug Nachrichten
extern char debugMsg[128];
extern bool debugEnabled;  // Globale Variable für den Debug-Status

void setup_wifi();  // Deklaration der setupFunction
void checkWiFi(); //25.5.24
void reconnect();
void setupOTA();
void setup_mqtt();

void callback(char* topic, byte* message, unsigned int length); // von ComfoAir-MQTT-ESPController.ino 

void mqttDebugPrint(const char* message);
void mqttDebugPrint(String message);
void mqttDebugPrint(int message);
void mqttDebugPrint(unsigned long message);
void mqttDebugPrint(IPAddress message);
void mqttDebugPrintf(const char* format, ...);

#define DEBUG_PRINT(x) if (debugEnabled) mqttDebugPrint(x)
#define DEBUG_PRINT_HEX(x) if (debugEnabled) { char buf[10]; snprintf(buf, sizeof(buf), "%X", x); mqttDebugPrint(buf); }
#define DEBUG_PRINT_BIN(x) if (debugEnabled) { char buf[10]; snprintf(buf, sizeof(buf), "%B", x); mqttDebugPrint(buf); }
#define DEBUG_PRINTF(...) if (debugEnabled) mqttDebugPrintf(__VA_ARGS__)


/*
#if DEBUG
#define DEBUG_INIT(speed)
#define DEBUG_PRINT(x) mqttDebugPrint(x)
#define DEBUG_PRINT_HEX(x) mqttDebugPrint(String(x, HEX))
#define DEBUG_PRINT_BIN(x) mqttDebugPrint(String(x, BIN))
#define DEBUG_PRINTF(...) mqttDebugPrintf(__VA_ARGS__)
#else
#define DEBUG_INIT(speed)
#define DEBUG_PRINT(x)
#define DEBUG_PRINT_HEX(x)
#define DEBUG_PRINT_BIN(x)
#define DEBUG_PRINTF(...)
#endif


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
*/
#endif
