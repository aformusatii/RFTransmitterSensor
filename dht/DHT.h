/* DHT library

MIT license
written by Adafruit Industries
*/
#ifndef DHT_H
#define DHT_H

#include <avr/interrupt.h>
#include <util/delay.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "../common/util.h"

// Define types of sensors.
#define DHT11 11
#define DHT22 22
#define DHT21 21
#define AM2301 21

#define DHT_PORT    PC0
#define DHT_D_REG   DDRC
#define DHT_OUT_REG PORTC
#define DHT_IN_REG  PINC

class DHT {
  public:
   DHT(uint8_t type);
   void begin(void);
   double getTemperature(bool S=false);
   double convertCtoF(double);
   double convertFtoC(double);
   double computeHeatIndex(double temperature, double percentHumidity, bool isFahrenheit=true);
   double getHumidity();
   bool read();

 private:
  uint8_t data[5];
  uint8_t _type;
  uint32_t _maxcycles;

  uint32_t expectPulse(bool level);

};

#endif
