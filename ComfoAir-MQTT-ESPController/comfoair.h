// comfoair.h

#ifndef COMFOAIR_H
#define COMFOAIR_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <string>

#include "setup.h"

extern char responseBuffer[];
enum State {
  IDLE,
  SENDING_COMMAND,
  WAITING_FOR_ACK,
  CHECKING_DATA,
  RECEIVING_DATA,
  PROCESSING_DATA
};

extern unsigned long lastSendTime;  // Verzögerung nach commandSend
extern bool isWaitingForResponse;   // Verzögerung nach commandSend
extern bool isReadingResponse;      // Verzögerung nach commandSend
extern State currentState;
extern int ABL_0, ABL_1, ABL_2, ZUL_0, ZUL_1, ZUL_2, ABL_3, ZUL_3, STUFE;

// Speicherzuweisung Debug Nachrichten
extern char debugMsg[128];

/*
Info: Wegen Speicherproblemen wurden die Funktionen auf C_Strings umgebaut.
      Die Funktion publishValues mit Hilfsfunktionen verwendet strings mit reserviertem Speicher.      

DoTo: > publishValues beobachten und bei Speicherproblemen auf C_Strings umbauen.
*/

// ComfoAir Funktionen (string vermieden)
void sendCommand(HardwareSerial& serial, const char* command);                  // Schritt 1: Senden des Befehls
void checkForResponse(HardwareSerial& serial);                                  // Schritt 2: Überprüfen auf verfügbare Daten
void processResponse(HardwareSerial& serial, char* response);                   // Schritt 3: Verarbeiten der Antwort
void publishValues(const char* responseBuffer);                                 //Schritt 4: Status Datenpunkte auf MQTT senden.
void sendAck(HardwareSerial& serial);                                           // ACK senden
uint8_t calculateChecksum(const char* data);                                    // Berechnung Checksumme
void packHStar(const char* hexString, uint8_t* byteArray, int& byteArraySize);  //String zum Versenden auf RS232 aufbereiten.
void byteToHexStr(char byte, char* hexStr);                                     // Wandelt ein Byte in einen Hex C_String

//Hilfsfunktionen für publishValues (string verwendet, Speicher reserviert)
long hexToDec(String hexString);
int findFirstSetBit(String binaryString);
String hexToBinary(String hexString);
std::string hexToAscii(const std::string& hexStr);

#endif