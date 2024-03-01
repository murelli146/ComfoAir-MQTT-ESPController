
# ComfoAir-MQTT-ESPController

![Hardware](./pics/Hardware.png)

## Über das Projekt
Dieses Projekt bietet eine Lösung zur systemunabhängigen Verwendung zur Steuerung der Comfoair-Lüftungsanlage (Zehnder Comfoair CA350/550, Stork und baugleiche). Es verwendet das MQTT-Protokoll in Verbindung mit einem ESP-Mikrocontroller, um eine flexible Steuerung und Überwachung der Comfoair-Funktionen zu ermöglichen.

### Hauptfunktionen

- **Lüftersteuerung:** Ermöglicht das Einstellen der Lüfterstufen und der Komforttemperatur.
- **Stoßlüftungslogik:** Integrierte Logik für die Stoßlüftung mit Nachlaufzeit, aktivierbar über ein MQTT-Objekt.
- **Statusüberwachung:** Sendet Statusobjekte wie Temperaturen, Vorheizregister, Motoren, Ventilationsstufen, Bypass-Status, Betriebsstunden, Störungsinformationen und Geräteinformationen in einem einstellbaren Sendezyklus.
- **Stufensetup:** die Konfiguration der einzelnen Lüftereinstellung ist über MQTT möglich.

### Schnelleinrichtung

1. **Arduino IDE:** Öffne `ComfoAir-MQTT-ESPController.ino` in der Arduino IDE.
2. **Konfiguration:** Gib SSID, Passwort und die IP-Adresse des MQTT-Brokers an.
3. **Flashen:** Lade den Code auf den ESP-Mikrocontroller.
4. **Hardware-Anschluss:** Verbinde die Hardware gemäß dem bereitgestellten Schaltplan mit dem RJ45-Port der Comfoair.
5. **Fertig:** Dein System ist einsatzbereit!

Mit diesem Projekt kannst du deine Comfoair-Lüftungsanlage einfach und effizient steuern und überwachen.

ACHTUNG! Grundsätzlich gilt: Nutzung auf eigenes Risiko!!!

## Technische Details
- **Mikrocontroller**: ESP8266
- **Schnittstellenkonverter**: MAX3232
- **Spannungswandler**: 12V auf 3,3V
- **Kommunikation**: RJ45 Kabel
- **Entwicklungsumgebung**: Arduino IDE

![Schaltplan](./pics/Schaltplan.png)

## Installation und Setup
### Voraussetzungen
- Ein ESP8266 Mikrocontroller
- Ein USB-Kabel, um den ESP8266 mit Ihrem Computer zu verbinden
- Die neueste Version der Arduino IDE (herunterladbar von [Arduino Website](https://www.arduino.cc/en/software))

### Schritt 1: Arduino IDE einrichten
1. **Installieren Sie die Arduino IDE**: Laden Sie die Software von der offiziellen Website herunter und installieren Sie sie auf Ihrem Computer.

2. **ESP8266 Board-Manager hinzufügen**:
   - Starten Sie die Arduino IDE.
   - Gehen Sie zu `Datei` > `Voreinstellungen`.
   - Fügen Sie in das Feld "Zusätzliche Boardverwalter-URLs" folgende URL ein: `http://arduino.esp8266.com/stable/package_esp8266com_index.json`.
   - Klicken Sie auf "OK".

3. **ESP8266 Boards installieren**:
   - Gehen Sie zu `Werkzeuge` > `Board` > `Boards Manager...`.
   - Suchen Sie nach "ESP8266" und installieren Sie das Paket.

### Schritt 2: PubSubClient Bibliothek installieren
1. In der Arduino IDE, gehen Sie zu `Sketch` > `Bibliothek einbinden` > `Bibliotheken verwalten...`.
2. Geben Sie in das Suchfeld "PubSubClient" ein und installieren Sie die Bibliothek.

### Schritt 3: ESP8266 mit der Arduino IDE konfigurieren
1. Schließen Sie den ESP8266 über das USB-Kabel an Ihren Computer an.
2. Wählen Sie in der Arduino IDE unter `Werkzeuge` > `Board` Ihr ESP8266-Board aus (z.B. "Generic ESP8266 Module").
3. Wählen Sie den richtigen COM-Port unter `Werkzeuge` > `Port`.

### Schritt 4: Code schreiben und hochladen
1. **Code schreiben**: Schreiben Sie Ihren Code oder öffnen Sie Ihr Projekt in der Arduino IDE.
2. **Konfigurieren Sie WLAN und MQTT-Einstellungen**: Stellen Sie sicher, dass Sie die SSID und das Passwort Ihres WLANs sowie die MQTT-Serverdetails in Ihrem Code korrekt eingetragen haben.
3. **Code hochladen**:
   - Klicken Sie auf den Pfeil-Button in der oberen linken Ecke der Arduino IDE, um den Code zu kompilieren und auf den ESP8266 hochzuladen.
   - Warten Sie, bis der Upload-Prozess abgeschlossen ist. In der Konsole unten in der IDE wird eine Erfolgsmeldung angezeigt.

### Schritt 5: Testen
- Nachdem der Code erfolgreich hochgeladen wurde, können Sie den ESP8266 testen, indem Sie ihn in Ihrem Netzwerk betreiben und überprüfen, ob er die erwarteten MQTT-Nachrichten sendet und empfängt.

### Hinweise
- Stellen Sie sicher, dass Sie während des Hochladens des Codes keine anderen Geräte (wie Sensoren oder Aktoren) an den ESP8266 angeschlossen haben, da dies manchmal zu Problemen beim Hochladen führen kann.
- Wenn Sie Probleme beim Hochladen haben, versuchen Sie, den Boot-Modus des ESP8266 zu ändern, indem Sie bestimmte Pins (z.B. GPIO0) auf GND ziehen, abhängig von Ihrem spezifischen Board.

## Code-Übersicht
Der Code (`ComfoAir-MQTT-ESPController.ino`) beinhaltet die Logik für die MQTT-Kommunikation und die Steuerung der ComfoAir-Anlage. Hauptfunktionen umfassen:
- OTA-Firmware-Updates
- WLAN-Verbindung
- MQTT-Client-Konfiguration
- Senden und Empfangen von Befehlen an die ComfoAir-Anlage

## MQTT-Datenpunkte

### MQTT Subscribe Topics
- `ComfoAir/cmd/stufe`: Setzt die Lüftungsstufe (0-3).
- `ComfoAir/cmd/soll`: Setzt die Solltemperatur (12.0 - 28.0 °C).
- `ComfoAir/cmd/filterreset`: Setzt den Filterzähler zurück.
- `ComfoAir/cmd/errorreset`: Setzt den Fehlerzähler zurück.
- `ComfoAir/cmd/firmwareinfo`: Fordert Firmware-Informationen an.
- `ComfoAir/cmd/stufensetup/ABL_0`: Setzt individuelle Einstellungen für jede Lüftungsstufe.
- `ComfoAir/cmd/stufensetup/ABL_1`
- `ComfoAir/cmd/stufensetup/ABL_2`
- `ComfoAir/cmd/stufensetup/ZUL_0`
- `ComfoAir/cmd/stufensetup/ZUL_1`
- `ComfoAir/cmd/stufensetup/ZUL_2`
- `ComfoAir/cmd/stufensetup/ABL_3`
- `ComfoAir/cmd/stufensetup/ZUL_3`
- `ComfoAir/cmd/sendezyklus`: Setzt das Intervall für den automatischen Sendevorgang (in Sekunden).
- `ComfoAir/cmd/stosslueftung`: Aktiviert oder deaktiviert die Stoßlüftung.
- `ComfoAir/cmd/nachlaufzeit`: Setzt die Nachlaufzeit für die Stoßlüftung (in Sekunden).

### MQTT Publish Topics

#### Temperatur-Status
- `ComfoAir/status/temperatur
```json
{
    "Abluft": 19.5,
    "Aussenluft": 9.5,
    "Erdwärme": 0,
    "Fortluft": 11.5,
    "Soll": 24.5,
    "Zuluft": 19
}

#### Vorheizregister-Status
- `ComfoAir/status/vorhzgregister`
```json
{
    "Frostklappe": 0,
    "FrostMinuten": 0,
    "Frostschutz": 0,
    "Frostsicherheit": 3,
    "Vorheizung": 0
}

#### Motoren-Status
- `ComfoAir/status/motor`
```json
{
    "Abluft": 35,
    "Abluft_RPM": 1164,
    "Zuluft": 30,
    "Zuluft_RPM": 1015
}

#### Ventilationsstufen-Status
- `ComfoAir/status/stufe`
```json
{
    "ABL_0": 20,
    "ABL_1": 35,
    "ABL_2": 50,
    "ABL_3": 70,
    "ABL_IST": 35,
    "STUFE": 1,
    "vent_abl": 1,
    "ZUL_0": 15,
    "ZUL_1": 30,
    "ZUL_2": 45,
    "ZUL_3": 65,
    "ZUL_IST": 30
    "Byte_13": 0,
    "Byte_14": 0,
}

#### Bypass-Status
- `ComfoAir/status/Bypass`: Status des Bypass-Ventils.

#### Betriebsstunden-Status
- `ComfoAir/status/betriebs_h`
```json
{
    "Bypass": 14749,
    "Filter": 3330,
    "Frost": 0,
    "Stufe_0": 4791,
    "Stufe_1": 64168,
    "Stufe_2": 2128,
    "Stufe_3": 12196,
    "Vorheizung": 0
}

#### Störungs-Status
- `ComfoAir/status/error`
```json
{
    "FehlerCode": "",
    "Filter": 0,
    "Text": "Aktuell kein Fehler - Filter nicht voll"
}

#### Firmware-Informationen
- `ComfoAir/status/Firmware_Version`: Firmware-Version des Geräts.
- `ComfoAir/status/Geraete_Name`: Name des Geräts.

#### Sendezyklus-Status
- `ComfoAir/status/sendezyklus`: Dauer des aktuellen Sendezyklus in Sekunden.

## Verwendung
ToDo

## Dokumentation und Ressourcen
- [Protokollbeschreibung ComfoAir](http://www.see-solutions.de/sonstiges/Protokollbeschreibung_ComfoAir.pdf)
- [Bedienungsanleitung ComfoAir](https://www.oeko-therm-systems.at/media/docs/bedienungsanleitungen/zehnder-bedienungsanleitung-ca350.pdf)

## Sicherheitshinweise
ACHTUNG! Grundsätzlich gilt: Nutzung auf eigenes Risiko!!!

## Beitragen und Support
ToDo

## Entwicklungsboard

![Entwicklungsboard](./pics/Entwicklungsboard.png)
