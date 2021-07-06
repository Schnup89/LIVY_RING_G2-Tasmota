# Vorwort
Sobald ihr den ESP flashed, gibt es kein zurück auf den Ursprungszustand, außer Ihr sichert euch evtl. den Flash (habe ich weder gemacht noch getestet).
Ich übernehme keine Haftung für irgendwas ;)
Bitte entfernt für das Testen die drei LiPo Batterien!

!! Es gibt keine Garantie dass alle Sensoren in Tasmota implementiert werden können !! 

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

~~ Was ihr braucht ~~
- Einen USB TTL Adapter oder ein anderes Gerät mit TTL USB RX TX Schnittstelle
- Tasmota Binary livyringg2tasmo.bin aus diesem Repository
- Tasmota "Flash-Files" https://github.com/arendst/Tasmota/tree/firmware/firmware/tasmota32/ESP32_needed_files
- Flasher Linux https://github.com/espressif/esptool 
- oder Flasher Windows (Müsst ihr probieren, hier die originale Anleitung: https://tasmota.github.io/docs/ESP32/#flashing)

~~ Vorbereitung ~~  
livyringg2tasmo.bin und alle vier flash Files in den selben Ordner wie das Flash-Tool ablegen.
Ich habe für das Flashen die Kabel direkt an die Pins des ESP-Chip rangehalten, mit etwas Geduld hat es dann funktioniert ;)

~~ Flash-Modus aktivieren ~~
- gpio0 mit GND verbinden
- ESP32 starten (USB Kabel verbinden)
- Led bleibt dauerhaft "grün"
- gpio0 Verbindung zu GND trennen
- gpio1 mit dem TTL-Modul RX verbinden
- gpio3 mit dem TTL-Modul TX verbinden
- Kommando ausführen: 
```
esptool.py --chip esp32 --port COM4 --baud 921600 --before default_reset --after hard_reset write_flash -z --flash_mode dout --flash_freq 40m --flash_size detect 0x1000 bootloader_dout_40m.bin 0x8000 partitions.bin 0xe000 boot_app0.bin 0x10000 livyringg2tasmo.bin
```
- Wenn der Flasher erfolg vermeldet hat, den ESP neu starten und es sollte ein Tasmota WLAN für die weitere Einrichtung erscheinen.


## gpios and sensor

~~ :heavy_check_mark: Buttons ~~  
gpio35  -  RESET  
 
~~ :heavy_division_sign: Motion (PIR) PYQ 1548/7660 ~~   
gpio32  -  serial IN       OUTPUT  
gpio2   -  DirectLink      INPUT  
gpio??  -  Power? 3,3v ON/OFF mit Transmitter?

~~ :heavy_check_mark: LED ~~   
gpio21  -  RED LED Inverted     	Relay1  
gpio22  -  BLUE LED Inverted 	    Relay2  
gpio4   -  GREEN LED Inverted	    Relay3  

~~ :heavy_division_sign: PIEZO ~~   
gpio16  -  Funktion nicht bekannt, Piezo Knackt beim schalten von low auf high  

~~ :heavy_division_sign: GAS SENSOR CCS801 ~~  
gpio18  -  HeaterPIN CSS811 oder CSS801   

~~ :heavy_division_sign: Chip: 1099 a40AG ~~   
gpio13  -  Funktion unbekannt

~~ :heavy_division_sign: i2c LiPO Spannungsanzeige [LC709203F] ~~  
gpio32  -  low power alarm  

~~ :heavy_check_mark: i2c RTC Clock (MCP7940M)  
i2c-pins

~~ :heavy_check_mark: i2C ~~  
gpio14 - SDA   
gpio12 - SCL  
  
Found Devices:  
{"I2CScan":"Device(s) found at 0x0b 0x6f"}  
0x0b = LC709203F (LiPo-SPannungsanzeige)  
0x6f = MCP7940M (RTC Clock)
