/*
  xsns_89_lc709203f.ino - LC709203F Fuel Gauge for LiPo's support for Tasmota

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifdef USE_I2C
#ifdef USE_LC709203F
/*********************************************************************************************\
 * lc7090203f - LC709203F is a Fuel Gauge for a single lithium ion/polymer battery
 *
 * Source: https://github.com/adafruit/Adafruit_LC709203F
 * Source: https://github.com/boverby/lc709203f
 *
 * I2C Address: 0x5f
 * 
 * Tasmota I2CRead/I2CWrite cannot be used because lack of crc support
\*********************************************************************************************/

#define XSNS_89                                 89
#define XI2C_61                                 61       // See I2CDEVICES.md

#define LC709203F_ADDRESS                      (0x0B)    // HEX Device Addr

/*********************************************************************************************\
 * LC709203F Definitions 
\*********************************************************************************************/
#define LC709203F_CMD_THERMISTORB              (0x06)    // Read/write thermistor B
#define LC709203F_CMD_INITRSOC                 (0x07)    // Initialize RSOC calculation
#define LC709203F_CMD_CELLTEMPERATURE          (0x08)    // Read/write batt temperature
#define LC709203F_CMD_CELLVOLTAGE              (0x09)    // Read batt voltage
#define LC709203F_CMD_CURDIR                   (0x0A)    // Read Current Direction Charge/Discharge
#define LC709203F_CMD_APA                      (0x0B)    // Adjustment Pack Application
#define LC709203F_CMD_RSOC                     (0x0D)    // Read state of charge
#define LC709203F_CMD_CELLITE                  (0x0F)    // Read batt indicator to empty
#define LC709203F_CMD_ICVERSION                (0x11)    // Read IC version
#define LC709203F_CMD_BATTPROF                 (0x12)    // Set the battery profile
#define LC709203F_CMD_ALARMRSOC                (0x13)    // Alarm on percent threshold
#define LC709203F_CMD_ALARMVOLT                (0x14)    // Alarm on voltage threshold
#define LC709203F_CMD_POWERMODE                (0x15)    // Sets sleep/power mode
#define LC709203F_CMD_STATUSBIT                (0x16)    // Temperature obtaining method
#define LC709203F_CMD_PARAMETER                (0x1A)    // Batt profile code

float lc709203f_voltage = -1;
float lc709203f_percent = -1;
float lc709203f_charge = -1;
float lc709203f_temperature = -1;
bool lc709203f_found = 0;
#ifdef ESP32
uint8_t lc709203f_bus = -1;
#endif


/*********************************************************************************************\
 * Helper-Functions
\*********************************************************************************************/
static uint8_t crc8(uint8_t *data, int len) 
{
  const uint8_t POLYNOMIAL(0x07);
  uint8_t crc(0x00);
  for (int j = len; j; --j) {
    crc ^= *data++;

    for (int i = 8; i; --i) {
      crc = (crc & 0x80) ? (crc << 1) ^ POLYNOMIAL : (crc << 1);
    }
  }
  return crc;
}


/*********************************************************************************************\
 * Main-Functions
\*********************************************************************************************/
bool lc709203fWriteI2C(uint8_t regAddress, uint16_t data)
{
  #ifdef ESP32
  TwoWire & myWire = (lc709203f_bus == 0) ? Wire : Wire1;
  #else
    TwoWire & myWire = Wire;
  #endif
  uint8_t crcArray[5];                // Setup array to hold bytes to send including CRC-8
  crcArray[0] = 0x16;
  crcArray[1] = regAddress;
  crcArray[2] = lowByte(data);
  crcArray[3] = highByte(data);
  crcArray[4] = crc8( crcArray, 4 );  // Calculate crc of preceding four bytes and place in crcArray[4]
  myWire.beginTransmission(LC709203F_ADDRESS);     // Device address
  myWire.write(regAddress);           // Register address
  myWire.write(crcArray[2]);          // low byte
  myWire.write(crcArray[3]);          // high byte
  myWire.write(crcArray[4]);          // Send crc8 
  if (myWire.endTransmission() != 0) {
    DEBUG_SENSOR_LOG("LC709203F: Failed to write Register: 0x%x", regAddress);
    return false;
  }
  return true;
}

int16_t lc709203fReadI2C(uint8_t regAddress)
{
  #ifdef ESP32
  TwoWire & myWire = (lc709203f_bus == 0) ? Wire : Wire1;
  #else
    TwoWire & myWire = Wire;
  #endif
  int16_t data = 0;
  myWire.beginTransmission(LC709203F_ADDRESS);
  myWire.write(regAddress);
  myWire.endTransmission(false);
  myWire.requestFrom(LC709203F_ADDRESS, 2);
  uint8_t lowByteData = myWire.read();
  uint8_t highByteData = myWire.read();
  data = word(highByteData, lowByteData);
  return( data );
}

float lc709203fGetCellVoltage_V()
{
  float mV = (float)lc709203fReadI2C(LC709203F_CMD_CELLVOLTAGE) / 1000.0;
  return  mV;
}

float lc709203fGetCellPercent()
{
  // Get battery state in percent (0-100%)
  return lc709203fReadI2C(LC709203F_CMD_CELLITE) / 10;
}

float lc709203fGetCellTemperature()
{
  // Get battery thermistor temperature
  return lc709203fReadI2C(LC709203F_CMD_CELLTEMPERATURE) / 100;
}

float lc709203fGetCharge()
{
  // Get RSOC Charge
  return lc709203fReadI2C(LC709203F_CMD_RSOC);
}

bool lc709203fRead(void)
{
  lc709203f_voltage = (float)lc709203fGetCellVoltage_V();
  lc709203f_percent = (float)lc709203fGetCellPercent();
  lc709203f_temperature = (float)lc709203fGetCellTemperature();
  lc709203f_charge = (float)lc709203fGetCharge();
  return true;
}


/*********************************************************************************************\
 * Interface-Functions
\*********************************************************************************************/
bool lc709203fSetCalibration()
{
  delayMicroseconds(10000);
  
  // Write PowerMode -> Operate
  if (!lc709203fWriteI2C(LC709203F_CMD_POWERMODE, 0x0001)) return false; 
  //Write (APA) BatteryPackSize
  #ifdef USE_LC709203F_BatteryPackSize
    if (!lc709203fWriteI2C(LC709203F_CMD_APA, USE_LC709203F_BatteryPackSize))  return false;
  #else
    if (!lc709203fWriteI2C(LC709203F_CMD_APA, 0x2D))  return false;
  #endif
  // Write BatteryProfile -> https://www.onsemi.com/pdf/datasheet/lc709203f-d.pdf Page 13
  #ifdef USE_LC709203F_BatteryProfile
    if (!lc709203fWriteI2C(LC709203F_CMD_BATTPROF, USE_LC709203F_BatteryProfile)) return false; 
  #else
    if (!lc709203fWriteI2C(LC709203F_CMD_BATTPROF, 0x0))  return false;
  #endif
  // Write ThermistorMode ON 
  if (!lc709203fWriteI2C(LC709203F_CMD_STATUSBIT, 0x0001))  return false;
  // Write Thermistor B-Constant
  #ifdef USE_LC709203F_ThermistorB
    if (!lc709203fWriteI2C(LC709203F_CMD_THERMISTORB , USE_LC709203F_ThermistorB))  return false;
  #else
    if (!lc709203fWriteI2C(LC709203F_CMD_THERMISTORB, 0x0D34))  return false;
  #endif
  //Write Alarmpercent if defined
  #ifdef USE_LC709203F_ALARMPERCENT
    if (!lc709203fWriteI2C(LC709203F_CMD_ALARMRSOC, USE_LC709203F_ALARMPERCENT)) return false;
  #endif
  return true;      
}

bool lc709203fWakeUp(){

}

void lc709203fDetect(void)
{
#ifdef ESP32
  for (uint8_t checkbus = 0; checkbus < MAX_I2C; checkbus++) {
    if (I2cSetDevice(LC709203F_ADDRESS,checkbus)) { 
      bus = checkbus;
      if (lc709203fSetCalibration()) {
        I2cSetActiveFound(LC709203F_ADDRESS, "LC709203F", bus);
        lc709203f_found = true;
        return;
      }
    }
  }  
#else
  if (I2cSetDevice(LC709203F_ADDRESS)) {
    if (lc709203fSetCalibration()) {
      I2cSetActiveFound(LC709203F_ADDRESS, "LC709203F");
      lc709203f_found = true;
      return;
    }
  }
#endif
}

void lc709203fEverySecond(void)
{
  lc709203fRead();
}

#ifdef USE_WEBSERVER
const char HTTP_SNS_lc709203f_DATA[] PROGMEM =
  "{s}%s " D_VOLTAGE "{m}%s " D_UNIT_VOLT "{e}"
  "{s}%s Cell Remain {m}%s " D_UNIT_PERCENT "{e}"
  "{s}%s " D_CELSIUS "{m}%s " D_UNIT_CELSIUS "{e}"
  "{s}%s RSOC Charge  {m}%s " D_UNIT_PERCENT "{e}";
#endif  // USE_WEBSERVER

void lc709203fShow(bool json)
{
  char voltage[16];
  dtostrfd(lc709203f_voltage, 2, voltage);
  char percent[16];
  dtostrfd(lc709203f_percent, 1, percent); 
  char temperature[16];
  dtostrfd(lc709203f_temperature, Settings->flag2.temperature_resolution, temperature);
  char charge[16];
  dtostrfd(lc709203f_charge, 1, charge);

  if (json) {
    ResponseAppend_P(PSTR(",\"LC709203F\":{\"" D_JSON_VOLTAGE "\":%s,\"Cell_Remain\":%s,\"RSOC Charge\":%s}"), voltage, percent, charge);
#ifdef USE_WEBSERVER
    } else {
      WSContentSend_PD(HTTP_SNS_lc709203f_DATA, "LC709203F", voltage, "LC709203F", percent, "LC709203F", temperature, "LC709203F", charge);
#endif  // USE_WEBSERVER
  }
}


/*********************************************************************************************\
 * Interface
\*********************************************************************************************/
bool Xsns89(uint8_t function)
{
  if (!I2cEnabled(XI2C_61)) { return false; }

  bool result = false;

  if (FUNC_INIT == function) {
    lc709203fDetect();
  }
  else if (lc709203f_found) {
    switch (function) {
    case FUNC_EVERY_SECOND:
      lc709203fEverySecond();
      break;
    case FUNC_JSON_APPEND:
      lc709203fShow(1);
      break;
  #ifdef USE_WEBSERVER
      case FUNC_WEB_SENSOR:
        lc709203fShow(0);
        break;
  #endif  // USE_WEBSERVER
    }
  }
  return result;
}

#endif  // USE_lc709203f
#endif  // USE_I2C
