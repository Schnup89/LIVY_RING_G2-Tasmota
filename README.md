# Vorwort
Sobald ihr den ESP flashed, gibt es kein zurück auf den Ursprungszustand, außer Ihr sichert euch evtl. den Flash (habe ich weder gemacht noch getestet).
Ich übernehme keine Haftung für irgendwas ;)
Bitte entfernt für das Testen die drei LiPo Batterien!


PIN-Layout ESP32-WROOM-32X
https://tasmota.github.io/docs/Pinouts/#esp32-wroom-32x


# Was funktioniert aktuell?
- LED Steurung (Rot,Grün,Blau) [Sichtbar am PIR Sensor] 
- Der Reset-Knopf wird beim drücken erkannt
- i2c Verbindung, aber für die Chips gibt es ~ noch ~ keine Tasmota Module


## Den Ring öffnen
- Plastikschutzfolio an der Unterseite des "RING" entfernen
- Vier Kreuzschlitzschrauben entfernen (am besten die Löcher mit einem Schranubenzieher ertasten)
- Ring entfernen


## Flashen
!!!! Batterien entfernen !!!!

Ihr braucht:
- Einen USB TTL Adapter oder ein anderes Gerät mit TTL USB RX TX Schnittstelle
- Tasmota Binary ***Link kommt noch***
- Flasher ***Link kommt noch***

~~ Flash-Modus aktivieren ~~
- gpio0 mit GND verbinden
- ESP32 starten (USB Kabel verbinden)
- Led bleibt dauerhaft "grün"
- gpio0 Verbindung zu GND trennen
- gpio1 mit dem TTL-Modul RX verbinden
- gpio3 mit dem TTL-Modul TX verbinden
- ***tbd flashen erklären***

## gpios and sensor

~~ Buttons ~~  
gpio35  -  RESET  
 
~~ Motion (PIR) ~~   
gpio32  -  serial IN       OUTPUT  
gpio2   -  DirectLink      INPUT  

~~ LED ~~   
gpio21  -  RED LED Inverted     	Relay1  
gpio22  -  BLUE LED Inverted 	    Relay2  
gpio4   -  GREEN LED Inverted	    Relay3  

~~ PIEZO ~~   
gpio16  -  Funktion nicht bekannt, Piezo Knackt beim schalten von low auf high  

~~ ?i2c? GAS SENSOR ~~  
gpio18  -  HeaterPIN CSS811 oder CSS801   

~~ i2c LiPO Spannungsanzeige [LC709203F] ~~  
gpio32  -  low power alarm  

~~ i2C ~~  
gpio14 - SDA   
gpio12 - SCL  
Found Devices:  
{"I2CScan":"Device(s) found at 0x0b 0x6f"}  
0x5f = LC709203F  