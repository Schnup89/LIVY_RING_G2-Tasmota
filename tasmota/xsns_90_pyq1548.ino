/*
  xsns_90_pyq_1548_7660.ino - PIR PYQ 1548/7660 support for Tasmota

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

#ifdef USE_PYQ1548


/*********************************************************************************************\
 * PYQ 1548/7659 - Sensor PIR (Passive Infrarot)
 *
 * Code for PYQ 1548/7659 Sensor:
 * References:
 * - https://github.com/renebohne/esp32-bigclown-pir-module/blob/master/src/main.cpp
\*********************************************************************************************/

#define XSNS_90                   90
#define PYQ1548_CoolDown          5  //Reading (seconds to display movement)
#define PYQ1548_ReActivate        30 //Reactivate (write config) device after On/OFF using Relay
    //see https://media.digikey.com/pdf/Data%20Sheets/Excelitas%20PDFs/PYQ1648-7052.pdf
    // | 8bit sensitivity | 4bit blind time | 2bit pulse counter | 2bit window time | 2bit operatin mode | 2bit filter source | 5bit reserved |
    //00011000 0100 01 10 10 00 10000
#define PYQ1548_HexConf           0x00304D10


bool bmove = 0;
uint8_t coold = PYQ1548_CoolDown;
uint16_t confwrite = PYQ1548_ReActivate;
bool pyq1548_found = 0;

void writeregval(int pin1,unsigned long regval){
  int i;
  int _pin1=pin1;
  unsigned long _regval=regval;
  unsigned char nextbit;
  unsigned long regmask = 0x1000000;
  pinMode(_pin1,OUTPUT);
  digitalWrite(_pin1,LOW);
  for(i=0; i < 25; i++) {
    nextbit = (_regval&regmask)!=0;
    regmask >>=1;
    digitalWrite(_pin1,LOW);
    digitalWrite(_pin1,HIGH);

    if(nextbit) {
      digitalWrite(_pin1,HIGH);
    }
    else{
      digitalWrite(_pin1,LOW);
    }
    delayMicroseconds(100);
  }
  digitalWrite(_pin1,LOW);
  delayMicroseconds(600);
}

bool PYQ1548Init(void)
{
  if (PinUsed(GPIO_PYQ_PIR_DL) && PinUsed(GPIO_PYQ_PIR_SER))  // Only start, if the pins are configured
  {
    pinMode(Pin(GPIO_PYQ_PIR_SER), OUTPUT);   //Serial IN Interface
    pinMode(Pin(GPIO_PYQ_PIR_DL), INPUT);     //Direct Link Interface

    writeregval(Pin(GPIO_PYQ_PIR_SER),PYQ1548_HexConf);
    pyq1548_found = 1;
    return true;
  } else {
    AddLog(LOG_LEVEL_DEBUG, PSTR("PYQ: No PIN Configured!")); 
    return false;
  }
}

void PYQ1548Reading(void) {
  if (PinUsed(GPIO_PYQ_PIR_DL) && PinUsed(GPIO_PYQ_PIR_SER) && pyq1548_found)  // Only start, if the pins are configured
  {
    if(digitalRead(Pin(GPIO_PYQ_PIR_DL))) {
      if (!bmove)  {
        bmove = 1;
        MqttPublishSensor();
      }
      coold = PYQ1548_CoolDown;
      AddLog(LOG_LEVEL_DEBUG, PSTR("PYQ: MOVEMENT!"));   
      pinMode(Pin(GPIO_PYQ_PIR_DL),OUTPUT);
      digitalWrite(Pin(GPIO_PYQ_PIR_DL),LOW);
      delayMicroseconds(100);
      pinMode(Pin(GPIO_PYQ_PIR_DL),INPUT);        
    }else{
      if (bmove == 1 && coold >= 0 ) {
        coold--;
      }
      if (coold <= 0 && bmove == 1 ) {
        bmove = 0;
        MqttPublishSensor();
      }
    }

    if (confwrite <= 0){
      writeregval(Pin(GPIO_PYQ_PIR_SER),PYQ1548_HexConf);
      confwrite = PYQ1548_ReActivate;
    }else{
      confwrite--;
    }
    return;
  }
  return;
}

void PYQ1548Show(bool json)
{
  if (PinUsed(GPIO_PYQ_PIR_DL) && PinUsed(GPIO_PYQ_PIR_SER) && pyq1548_found) {
    if (json) {
      ResponseAppend_P(PSTR(",\"PYQ\":{\"Movement\": %s}"), bmove?"true":"false");
  #ifdef USE_WEBSERVER
    } else {
      WSContentSend_PD("{s}Movement {m}%s {e}", bmove?"Moved!":"Nothing");
  #endif  // USE_WEBSERVER
    }
  }
  return;
}

/*********************************************************************************************\
 * Interface
\*********************************************************************************************/

bool Xsns90(uint8_t function)
{
  bool result = false;
  switch (function) {
    case FUNC_INIT:
      result = PYQ1548Init();
      break;
    case FUNC_EVERY_SECOND:
      PYQ1548Reading();
      result = true;
      break;
    case FUNC_JSON_APPEND:
      PYQ1548Show(1);
      break;
#ifdef USE_WEBSERVER
    case FUNC_WEB_SENSOR:
      PYQ1548Show(0);
      break;
#endif  // USE_WEBSERVER
    }
  return result;
}

#endif  // USE_PYQ1548

