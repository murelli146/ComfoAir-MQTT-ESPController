//comfoair.cpp

#include "comfoair.h"  // Importiere die Header-Datei

State currentState = IDLE;
//extern int ABL_0, ABL_1, ABL_2, ZUL_0, ZUL_1, ZUL_2, ABL_3, ZUL_3;

const int RESPONSE_BUFFER_SIZE = 100;  // Beispielgröße
char responseBuffer[RESPONSE_BUFFER_SIZE];

// Schritt 1: Senden des Befehls
void sendCommand(HardwareSerial& serial, const char* command) {
  char buffer[65];
  uint8_t byteArray[65];
  int byteArraySize;

  sprintf(buffer, "07F0%s%02X070F", command, calculateChecksum(command));
  const char* hexData = buffer;

//  DEBUG_PRINT("Befehl gesendet: ");
//  DEBUG_PRINT(buffer);
snprintf(debugMsg, sizeof(debugMsg), "CA_ST1 Befehl gesendet: %s", buffer);
DEBUG_PRINT(debugMsg);

  packHStar(hexData, byteArray, byteArraySize);
  for (int i = 0; i < byteArraySize; i++) {
    serial.write(byteArray[i]);
  }

  currentState = WAITING_FOR_ACK;
}

// Schritt 2: Überprüfen auf verfügbare Daten
void checkForResponse(HardwareSerial& serial) {
  static int index = 0;

  if (serial.available()) {
    char c = serial.read();
    if (index < RESPONSE_BUFFER_SIZE - 3) {
      byteToHexStr(c, &responseBuffer[index]);
      index += 2;
    }
  }

  responseBuffer[index] = '\0';  // Null-Terminator hinzufügen

  switch (currentState) {
    case WAITING_FOR_ACK:
      if (strncmp(responseBuffer, "07F3", 4) == 0) {
        currentState = CHECKING_DATA;
      } else if (index >= 4) {
        DEBUG_PRINT("CA_ST2 Kein ACK empfangen");
        currentState = IDLE;
        index = 0;  // Index zurücksetzen
      }
      break;

    case CHECKING_DATA:
      if (!serial.available()) {
        if (index > 4) {
          // Weitere Daten vorhanden
          currentState = RECEIVING_DATA;
        } else {
          // Nur ACK empfangen, keine weiteren Daten
          DEBUG_PRINT("CA_ST2 Nur ACK empfangen, keine weiteren Daten");
          currentState = IDLE;
          index = 0;  // Index zurücksetzen
          // ACK senden
          sendAck(serial);
        }
      }
      break;

    case RECEIVING_DATA:
//      DEBUG_PRINT("Empfangene Daten: ");
//      DEBUG_PRINT(responseBuffer);
snprintf(debugMsg, sizeof(debugMsg), "CA_ST2 Empfangene Daten: %s", responseBuffer);
DEBUG_PRINT(debugMsg);

      processResponse(serial, responseBuffer);
      currentState = IDLE;
      index = 0;  // Index zurücksetzen
      break;

    default:
      // Keine Aktion erforderlich
      break;
  }
}

// Schritt 3: Verarbeiten der Antwort
void processResponse(HardwareSerial& serial, char* responseBuffer) {

  // Entfernen von Startbytes und ACK "07F307F0"
  if (strncmp(responseBuffer, "07F307F0", 8) == 0) {
    memmove(responseBuffer, responseBuffer + 8, strlen(responseBuffer) - 7);
  }

  // Entfernen von Endbytes "070F"
  if (strlen(responseBuffer) > 6) {
    // Extrahiere die Länge der Daten aus dem dritten Byte der Antwort
    char lengthHex[3] = { responseBuffer[4], responseBuffer[5], '\0' };
    int dataLength = strtol(lengthHex, NULL, 16);

    // Berechne die erwartete Gesamtlänge der Antwort
    // Befehl (2) + Länge (2) + Daten (dataLength * 2) + Checksumme (2) + Endbytes (4)
    int expectedLength = 2 + 2 + dataLength * 2 + 2 + 4;

    // Überprüfe, ob die tatsächliche Länge des Antwort-Strings mit der erwarteten Länge übereinstimmt
    if (strlen(responseBuffer) >= expectedLength) {
      // Überprüfe, ob die Endbytes vorhanden sind
      char* endPtr = strstr(responseBuffer + expectedLength - 6, "070F");
      if (endPtr != nullptr) {
        *endPtr = '\0';  // Setze das Ende des Strings vor den Endbytes
      }
    }
    // ACK an ComfoAir
    sendAck(serial);
  }

  // Entfernen doppelter "07"
  char* double07Ptr = strstr(responseBuffer, "0707");
  while (double07Ptr != nullptr) {
    memmove(double07Ptr, double07Ptr + 2, strlen(double07Ptr) - 1);
    double07Ptr = strstr(responseBuffer, "0707");
//    DEBUG_PRINT("Nach Entfernen von doppelte 07: ");
//    DEBUG_PRINT(responseBuffer);
snprintf(debugMsg, sizeof(debugMsg), "CA_ST3 Empfangene Daten: %s", responseBuffer);
DEBUG_PRINT(debugMsg);

  }

  // Checksumme überprüfen und entfernen
  int len = strlen(responseBuffer);
  if (len >= 2) {
    char checksum[3] = { responseBuffer[len - 2], responseBuffer[len - 1], '\0' };
    responseBuffer[len - 2] = '\0';  // Entferne die Checksumme aus dem String

    uint8_t calculatedChecksum = calculateChecksum(responseBuffer);
    char calculatedChecksumStr[3];
    sprintf(calculatedChecksumStr, "%02X", calculatedChecksum);

    if (strcmp(calculatedChecksumStr, checksum) != 0) {
      DEBUG_PRINT("CA_ST3 Checksummenfehler");
      return;
    }
  }

  publishValues(responseBuffer);
}

// Schritt 4: MQTT publish
void publishValues(const char* responseBuffer) {
  StaticJsonDocument<512> doc;  // Umstellung auf JSON (doc)
  char buffer[512];             // Umstellung auf  JSON  (doc)
  // Erstelle einen String für das Thema und reserviere Speicher
  String reciv = responseBuffer;
  reciv.reserve(65);  // Angenommene maximale Länge des Empfangs


  // Temperaturen verarbeiten
  if (reciv.startsWith("00D209")) {
    float b1 = (hexToDec(reciv.substring(6, 8)) / 2.0) - 20;
    float b2 = (hexToDec(reciv.substring(8, 10)) / 2.0) - 20;
    float b3 = (hexToDec(reciv.substring(10, 12)) / 2.0) - 20;
    float b4 = (hexToDec(reciv.substring(12, 14)) / 2.0) - 20;
    float b5 = (hexToDec(reciv.substring(14, 16)) / 2.0) - 20;
    float b7 = (hexToDec(reciv.substring(18, 20)) / 2.0) - 20;

    /* Umstellung auf JSON (doc)
    client.publish("ComfoAir/status/temperatur/Soll", String(b1).c_str());
    client.publish("ComfoAir/status/temperatur/Aussenluft", String(b2).c_str());
    client.publish("ComfoAir/status/temperatur/Zuluft", String(b3).c_str());
    client.publish("ComfoAir/status/temperatur/Abluft", String(b4).c_str());
    client.publish("ComfoAir/status/temperatur/Fortluft", String(b5).c_str());
    client.publish("ComfoAir/status/temperatur/Erdwärme", String(b7).c_str());
	  */

    doc["Soll"] = b1;
    doc["Aussenluft"] = b2;
    doc["Zuluft"] = b3;
    doc["Abluft"] = b4;
    doc["Fortluft"] = b5;
    doc["Erdwärme"] = b7;

    size_t len = serializeJson(doc, buffer, sizeof(buffer));
    client.publish("ComfoAir/status/temperatur", buffer, len);
    doc.clear();  // JSON-Dokument für den nächsten Abschnitt leeren
  }

  // Vorheizregister Status
  if (reciv.startsWith("00E206")) {
    int b1 = hexToDec(reciv.substring(6, 8));
    int b2 = hexToDec(reciv.substring(8, 10));
    int b3 = hexToDec(reciv.substring(10, 12));
    int b4_5 = hexToDec(reciv.substring(12, 16));
    int b6 = hexToDec(reciv.substring(16, 18));

    /* Umstellung auf JSON (doc)
    client.publish("ComfoAir/status/vorheizregister/Frostklappe", String(b1).c_str());
    client.publish("ComfoAir/status/vorheizregister/Frostschutz", String(b2).c_str());
    client.publish("ComfoAir/status/vorheizregister/Vorheizung", String(b3).c_str());
    client.publish("ComfoAir/status/vorheizregister/FrostMinuten", String(b4_5).c_str());
    client.publish("ComfoAir/status/vorheizregister/Frostsicherheit", String(b6).c_str());
    */

    doc["Frostklappe"] = b1;
    doc["Frostschutz"] = b2;
    doc["Vorheizung"] = b3;
    doc["FrostMinuten"] = b4_5;
    doc["Frostsicherheit"] = b6;

    size_t len = serializeJson(doc, buffer, sizeof(buffer));
    client.publish("ComfoAir/status/vorhzgregister", buffer, len);
    doc.clear();  // JSON-Dokument für den nächsten Abschnitt leeren
  }

  // Motoren
  if (reciv.startsWith("000C06")) {
    int b1 = hexToDec(reciv.substring(6, 8));
    int b2 = hexToDec(reciv.substring(8, 10));
    int b3_4 = 1875000 / hexToDec(reciv.substring(10, 14));
    int b5_6 = 1875000 / hexToDec(reciv.substring(14, 18));

    /* Umstellung auf JSON (doc)
    client.publish("ComfoAir/status/motor/Zuluft", String(b1).c_str());
    client.publish("ComfoAir/status/motor/Abluft", String(b2).c_str());
    client.publish("ComfoAir/status/motor/Zuluft_RPM", String(b3_4).c_str());
    client.publish("ComfoAir/status/motor/Abluft_RPM", String(b5_6).c_str());
    */

    doc["Zuluft"] = b1;
    doc["Abluft"] = b2;
    doc["Zuluft_RPM"] = b3_4;
    doc["Abluft_RPM"] = b5_6;

    size_t len = serializeJson(doc, buffer, sizeof(buffer));
    client.publish("ComfoAir/status/motor", buffer, len);
    doc.clear();  // JSON-Dokument für den nächsten Abschnitt leeren
  }

  //Ventilationsstufen
  if (reciv.startsWith("00CE0E")) {
    ABL_0 = hexToDec(reciv.substring(6, 8));
    ABL_1 = hexToDec(reciv.substring(8, 10));
    ABL_2 = hexToDec(reciv.substring(10, 12));
    ZUL_0 = hexToDec(reciv.substring(12, 14));
    ZUL_1 = hexToDec(reciv.substring(14, 16));
    ZUL_2 = hexToDec(reciv.substring(16, 18));
    int b7 = hexToDec(reciv.substring(18, 20));
    int b8 = hexToDec(reciv.substring(20, 22));
    STUFE = hexToDec(reciv.substring(22, 24));
    int b10 = hexToDec(reciv.substring(24, 26));
    ABL_3 = hexToDec(reciv.substring(26, 28));
    ZUL_3 = hexToDec(reciv.substring(28, 30));
    int b13 = hexToDec(reciv.substring(30, 32));
    int b14 = hexToDec(reciv.substring(32, 34));

    /* Umstellung auf JSON (doc)
    client.publish("ComfoAir/status/stufe/ABL_0", String(ABL_0).c_str());
    client.publish("ComfoAir/status/stufe/ABL_1", String(ABL_1).c_str());
    client.publish("ComfoAir/status/stufe/ABL_2", String(ABL_2).c_str());
    client.publish("ComfoAir/status/stufe/ZUL_0", String(ZUL_0).c_str());
    client.publish("ComfoAir/status/stufe/ZUL_1", String(ZUL_1).c_str());
    client.publish("ComfoAir/status/stufe/ZUL_2", String(ZUL_2).c_str());
    client.publish("ComfoAir/status/stufe/ABL_IST", String(b7).c_str());
    client.publish("ComfoAir/status/stufe/ZUL_IST", String(b8).c_str());
    client.publish("ComfoAir/status/stufe/STUFE", String(STUFE - 1).c_str());  //Mapping der Stufe (eins versetzt)
    client.publish("ComfoAir/status/stufe/vent_abl", String(b10).c_str());
    client.publish("ComfoAir/status/stufe/ABL_3", String(ABL_3).c_str());
    client.publish("ComfoAir/status/stufe/ZUL_3", String(ZUL_3).c_str());
    client.publish("ComfoAir/status/stufe/Byte_13", String(b13).c_str());
    client.publish("ComfoAir/status/stufe/Byte_14", String(b14).c_str());
    */

    doc["ABL_0"] = ABL_0;
    doc["ABL_1"] = ABL_1;
    doc["ABL_2"] = ABL_2;
    doc["ZUL_0"] = ZUL_0;
    doc["ZUL_1"] = ZUL_1;
    doc["ZUL_2"] = ZUL_2;
    doc["ABL_IST"] = b7;
    doc["ZUL_IST"] = b8;
    doc["STUFE"] = STUFE - 1;  //Mapping der Stufe (eins versetzt)
    doc["vent_abl"] = b10;
    doc["ABL_3"] = ABL_3;
    doc["ZUL_3"] = ZUL_3;
    doc["Byte_13"] = b13;
    doc["Byte_14"] = b14;

    size_t len = serializeJson(doc, buffer, sizeof(buffer));
    client.publish("ComfoAir/status/stufe", buffer, len);
    doc.clear();  // JSON-Dokument für den nächsten Abschnitt leeren
  }

  //Bypass
  if (reciv.startsWith("000E04")) {
    int b1 = hexToDec(reciv.substring(6, 8));

    client.publish("ComfoAir/status/Bypass", String(b1).c_str());
  }

  //Betriebsstunden
  if (reciv.startsWith("00DE14")) {
    int b1_3 = hexToDec(reciv.substring(6, 12));
    int b4_6 = hexToDec(reciv.substring(12, 18));
    int b7_9 = hexToDec(reciv.substring(18, 24));
    int b10_11 = hexToDec(reciv.substring(24, 28));
    int b12_13 = hexToDec(reciv.substring(28, 32));
    int b14_15 = hexToDec(reciv.substring(32, 36));
    int b16_17 = hexToDec(reciv.substring(36, 40));
    int b18_20 = hexToDec(reciv.substring(40, 46));

    /* Umstellung auf JSON (doc)
    client.publish("ComfoAir/status/betriebs_h/Stufe_0", String(b1_3).c_str());
    client.publish("ComfoAir/status/betriebs_h/Stufe_1", String(b4_6).c_str());
    client.publish("ComfoAir/status/betriebs_h/Stufe_2", String(b7_9).c_str());
    client.publish("ComfoAir/status/betriebs_h/Frost", String(b10_11).c_str());
    client.publish("ComfoAir/status/betriebs_h/Vorheizung", String(b12_13).c_str());
    client.publish("ComfoAir/status/betriebs_h/Bypass", String(b14_15).c_str());
    client.publish("ComfoAir/status/betriebs_h/Filter", String(b16_17).c_str());
    client.publish("ComfoAir/status/betriebs_h/Stufe_3", String(b18_20).c_str());
	*/

    doc["Stufe_0"] = b1_3;
    doc["Stufe_1"] = b4_6;
    doc["Stufe_2"] = b7_9;
    doc["Frost"] = b10_11;
    doc["Vorheizung"] = b12_13;
    doc["Bypass"] = b14_15;
    doc["Filter"] = b16_17;
    doc["Stufe_3"] = b18_20;

    size_t len = serializeJson(doc, buffer, sizeof(buffer));
    client.publish("ComfoAir/status/betriebs_h", buffer, len);
    doc.clear();  // JSON-Dokument für den nächsten Abschnitt leeren
  }

  //Störungen
  if (reciv.startsWith("00DA11")) {
    String fehlerAlo = reciv.substring(6, 8);
    String fehlerAhi = reciv.substring(30, 32);
    String fehlerE = reciv.substring(8, 10);
    String fehlerFilter = reciv.substring(22, 24);
    String fehlerEA = reciv.substring(24, 26);

    int aloIndex = findFirstSetBit(hexToBinary(fehlerAlo));
    int ahiIndex = findFirstSetBit(hexToBinary(fehlerAhi)) + 8;
    int eIndex = findFirstSetBit(hexToBinary(fehlerE));
    int eaIndex = findFirstSetBit(hexToBinary(fehlerEA));

    String aktFC = "";
    if (aloIndex > 0) aktFC = "A" + String(aloIndex);
    else if (ahiIndex > 8) aktFC = "A" + String(ahiIndex);
    else if (eIndex > 0) aktFC = "E" + String(eIndex);
    else if (eaIndex > 0) aktFC = "EA" + String(eaIndex);

    int filterFlag = hexToDec(fehlerFilter) > 0 ? 1 : 0;
    String statText = aktFC == "" ? "Aktuell kein Fehler" : "Fehler: " + aktFC;
    statText += filterFlag == 0 ? " - Filter nicht voll" : " - Filter voll";

    /* Umstellung auf JSON (doc)
    client.publish("ComfoAir/status/error/FehlerCode", String(aktFC).c_str());
    client.publish("ComfoAir/status/error/Filter", String(filterFlag).c_str());
    client.publish("ComfoAir/status/error/Text", String(statText).c_str());
	*/

    doc["FehlerCode"] = aktFC;
    doc["Filter"] = filterFlag;
    doc["Text"] = statText;

    size_t len = serializeJson(doc, buffer, sizeof(buffer));
    client.publish("ComfoAir/status/error", buffer, len);
    doc.clear();  // JSON-Dokument für den nächsten Abschnitt leeren
  }
  //Firmware
  if (reciv.startsWith("006A0D")) {
    String firmwareVersion = String(hexToDec(reciv.substring(6, 8))) + "." + String(hexToDec(reciv.substring(8, 10))) + "." + String(hexToDec(reciv.substring(10, 12)));
    String deviceNamehex = reciv.substring(12, 32);
    ;
    std::string deviceNameHexStd = deviceNamehex.c_str();
    std::string deviceNameStd = hexToAscii(deviceNameHexStd);
    const char* deviceNameCStr = deviceNameStd.c_str();
    String deviceNameArduinoString = String(deviceNameCStr);

    client.publish("ComfoAir/status/Firmware_Version", String(firmwareVersion).c_str());
    client.publish("ComfoAir/status/Geraete_Name", deviceNameArduinoString.c_str());
  }
}

void sendAck(HardwareSerial& serial) {
  uint8_t ackArray[2];
  int ackArraySize;
  packHStar("07F3", ackArray, ackArraySize);
  for (int i = 0; i < ackArraySize; i++) {
    serial.write(ackArray[i]);
  }
}

uint8_t calculateChecksum(const char* data) {
  uint8_t checksum = 173;  // Startwert für die Checksumme

  while (*data) {
    char hexPair[3] = { *data, *(data + 1), '\0' };
    checksum += strtol(hexPair, NULL, 16);
    data += 2;
  }

  return checksum & 0xFF;
}

void packHStar(const char* hexString, uint8_t* byteArray, int& byteArraySize) {
  byteArraySize = 0;
  int length = strlen(hexString);
  for (int i = 0; i < length; i += 2) {  //2
    char byteString[3] = { hexString[i], hexString[i + 1], '\0' };
    byteArray[byteArraySize++] = static_cast<uint8_t>(strtol(byteString, nullptr, 16));
  }
}

// Hilfsfunktion, um ein Byte in einen Hex-String zu konvertieren
void byteToHexStr(char byte, char* hexStr) {
  sprintf(hexStr, "%02X", byte);
}
//Helferlein für publishValues
long hexToDec(String hexString) {
  return strtol(hexString.c_str(), NULL, 16);
}

int findFirstSetBit(String binaryString) {
  for (int i = 0; i < binaryString.length(); i++) {
    if (binaryString.charAt(i) == '1') {
      return i + 1;
    }
  }
  return 0;
}

String hexToBinary(String hexString) {
  String binaryString = "";
  for (char& c : hexString) {
    switch (c) {
      case '0': binaryString += "0000"; break;
      case '1': binaryString += "0001"; break;
      case '2': binaryString += "0010"; break;
      case '3': binaryString += "0011"; break;
      case '4': binaryString += "0100"; break;
      case '5': binaryString += "0101"; break;
      case '6': binaryString += "0110"; break;
      case '7': binaryString += "0111"; break;
      case '8': binaryString += "1000"; break;
      case '9': binaryString += "1001"; break;
      case 'A':
      case 'a': binaryString += "1010"; break;
      case 'B':
      case 'b': binaryString += "1011"; break;
      case 'C':
      case 'c': binaryString += "1100"; break;
      case 'D':
      case 'd': binaryString += "1101"; break;
      case 'E':
      case 'e': binaryString += "1110"; break;
      case 'F':
      case 'f': binaryString += "1111"; break;
      default:
        // Fehlerbehandlung für ungültige Hexadezimalzeichen
        binaryString += "????";
        break;
    }
  }
  return binaryString;
}

std::string hexToAscii(const std::string& hexStr) {
  std::string asciiStr;
  for (size_t i = 0; i < hexStr.length(); i += 2) {
    std::string byteStr = hexStr.substr(i, 2);
    char byte = static_cast<char>(strtol(byteStr.c_str(), nullptr, 16));
    asciiStr += byte;
  }
  return asciiStr;
}