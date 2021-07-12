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

~~ Flashen ~~
- gpio0 mit GND verbinden
- ESP32 starten (USB Kabel verbinden)
- Led bleibt dauerhaft "grün" <- Flashmodus aktiv
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
 
~~ :heavy_check_mark: Motion (PIR) PYQ 1548/7660 ~~   
gpio32  -  serial IN       OUTPUT  
gpio2   -  DirectLink      INPUT  
gpio27  -  Power 3,3v Sensor ON/OFF   **[RELAY 4]**
Tasmota -  Neue xsns Lib überarbeiten, evtl. GIT Push

~~ :heavy_check_mark: LED ~~   
gpio21  -  RED LED Inverted     	**[RELAY 1]**  
gpio22  -  BLUE LED Inverted 	   **[RELAY 2]**  
gpio4   -  GREEN LED Inverted	   **[RELAY 3]**   

~~ :heavy_check_mark: PIEZO ~~   
gpio16  -  Funktioniert als PWM Output

~~ :heavy_division_sign: GAS SENSOR CCS801 ~~  
gpio18  -  Heater PIN   
gpio33  -  Power 3,3v Sensor ON/OFF   **[RELAY 5]**  

~~ :heavy_division_sign: Mikrofon Chip: 1099 a40AG ~~   
gpio13  -  Funktion unbekannt [kein Widerstand wohl ein Input]  
gpio15  -  Funktion unbekannt [kein Widerstand wohl ein Input]  
Mikrofon: https://www.mouser.de/datasheet/2/218/pk0641ht4h-1-rev-a-1519664.pdf  
Chip: ???? 1099 a40AG  

~~ :heavy_check_mark: HDC1080 Temperatur und Luftfeuchtigkeit ~~   
i2c  -  Gruppe1  
  
~~ :heavy_division_sign: LiPO Spannungsanzeige [LC709203F] ~~  
i2c  -  Gruppe2  
gpio32? -  low power alarm  
Tasmota -  Neue xsns Lib überarbeiten, evtl. GIT Push

~~ :heavy_check_mark: RTC Clock (MCP7940M)  
i2c  -  Gruppe2  

~~ :heavy_check_mark: i2c GRUPPE 2 ~~  
gpio14  -  SDA    
gpio12  -  SCL   
Found Devices:  
{"I2CScan":"Device(s) found at 0x0b 0x6f"}  
0x0b = LC709203F (LiPo-SPannungsanzeige)  
0x6f = MCP7940M (RTC Clock)

~~ :heavy_division_sign: i2c GRUPPE 1 ~~   
gpio19  -  SDA  
gpio18  -  SCL   
Found Devices:  
{"I2CScan":"Device(s) found at 0x40 0x48 0x60"}  
0x40 = HDC1080 Temp&Feuchtigkeit  
0x48 = ADS1115 Analog zu DigitalWandler... Gas Sensor? Gpio33 An/Aus  
0x60 = Unbekannt  

