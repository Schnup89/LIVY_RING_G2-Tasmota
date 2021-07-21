/*
  xsns_13_ina219.ino - INA219 Current Sensor support for Tasmota

  Copyright (C) 2021  Stefan Bode and Theo Arends

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
#ifdef USE_MCP4725
/*********************************************************************************************\
 * INA219 - Low voltage (max 32V!) Current sensor
 *
 * Source: Adafruit Industries
 *
 * I2C Address: 0x40, 0x41 0x44 or 0x45
\*********************************************************************************************/

#define XSNS_91                                 91
#define XI2C_14                                 14        // See I2CDEVICES.md

#define MCP4725_ADDRESS1                         (0x60)    // 1000000 (A0+A1=GND)
#define MCP4725_ADDRESS2                         (0x62)    // 1000000 (A0=Vcc, A1=GND)
#define MCP4725_ADDRESS3                         (0x64)    // 1000000 (A0=GND, A1=Vcc)
#define MCP4725_ADDRESS4                         (0x66)    // 1000000 (A0+A1=Vcc)

#define MCP4725_CMD_WRITEDAC                     (0x40)



/********************************************************************************************/


bool MCP4725_Detect(void)
{
  if (I2cActive(MCP4725_ADDRESS1)) { return false; }
  I2cSetActiveFound(MCP4725_ADDRESS1,"MCP4725");
  return true;
  
}

void MCP4725_SetVoltage(uint16_t output) //0..4095
{
  uint8_t packet[2];
  packet[0] = output / 16;        // Upper data bits (D11.D10.D9.D8.D7.D6.D5.D4)
  packet[1] = (output % 16) << 4; // Lower data bits (D3.D2.D1.D0.x.x.x.x)
  uint16_t writep= 256U*packet[1]+packet[0];
  if (I2cWrite16(MCP4725_ADDRESS1,MCP4725_CMD_WRITEDAC,writep)){
    AddLog(LOG_LEVEL_DEBUG,"MCP4725: SetVoltageOK V: %d", output);
  }
}


bool MCP4725_Command(void)
{
  bool serviced = true;
  uint8_t paramcount = 0;
  if (XdrvMailbox.data_len > 0) {
    paramcount=1;
  } else {
    serviced = false;
    return serviced;
  }
  char argument[XdrvMailbox.data_len];
  UpperCase(XdrvMailbox.data,XdrvMailbox.data);
  if (!strcmp(ArgV(argument, 1),"SETV")) {
    uint16_t setval = atoi(ArgV(argument, 2));
    if ((setval >= 0) && (setval <= 4095)) {
        MCP4725_SetVoltage(setval);
        serviced = true;
        return serviced;
      }
    serviced = false;
    return serviced;
  }
  return false;
}



/*********************************************************************************************\
 * Interface
\*********************************************************************************************/

bool Xsns91(uint8_t function)
{
  if (!I2cEnabled(XI2C_14)) { return false; }
  bool result = false;
  switch (function) {
    case FUNC_INIT:
      result = MCP4725_Detect();
      break;
    case FUNC_COMMAND_SENSOR:
      if (XSNS_91 == XdrvMailbox.index) {
        result = MCP4725_Command();
      }
      break;
  }
  return result;
}

#endif  // USE_INA219
#endif  // USE_I2C