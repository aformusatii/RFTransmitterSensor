/* DHT library

MIT license
written by Adafruit Industries
*/
#include "../dht/DHT.h"

DHT::DHT(uint8_t type) {
  _type = type;
  _maxcycles = microsecondsToClockCycles(1000);  // 1 millisecond timeout for
                                                 // reading pulses from DHT sensor.
}

void DHT::begin(void) {
  // set up the pins!
  _in(DHT_PORT, DHT_D_REG); // set port as input
  // Using this value makes sure that millis() - lastreadtime will be
  // >= MIN_INTERVAL right away. Note that this assignment wraps around,
  // but so will the subtraction.
  debug_print("Max clock cycles: %ld", _maxcycles);
  _out(PC1, DHT_D_REG);
  _off(PC1, PORTC);
}

//boolean S == Scale.  True == Fahrenheit; False == Celcius
double DHT::getTemperature(bool S) {
	double f = 0;

	switch (_type) {
	case DHT11:
		f = data[2];
		if (S) {
			f = convertCtoF(f);
		}
		break;

	case DHT22:
	case DHT21:
		f = data[2] & 0x7F;
		f *= 256;
		f += data[3];
		f *= 0.1;
		if (data[2] & 0x80) {
			f *= -1;
		}
		if (S) {
			f = convertCtoF(f);
		}
		break;
	}

	return f;
}

double DHT::convertCtoF(double c) {
  return c * 1.8 + 32;
}

double DHT::convertFtoC(double f) {
  return (f - 32) * 0.55555;
}

double DHT::getHumidity() {
	double f = 0;
	switch (_type) {
	case DHT11:
		f = data[0];
		break;
	case DHT22:
	case DHT21:
		f = data[0];
		f *= 256;
		f += data[1];
		f *= 0.1;
		break;
	}
	return f;
}

//boolean isFahrenheit: True == Fahrenheit; False == Celcius
double DHT::computeHeatIndex(double temperature, double percentHumidity, bool isFahrenheit) {
  // Using both Rothfusz and Steadman's equations
  // http://www.wpc.ncep.noaa.gov/html/heatindex_equation.shtml
  double hi;

  if (!isFahrenheit)
    temperature = convertCtoF(temperature);

  hi = 0.5 * (temperature + 61.0 + ((temperature - 68.0) * 1.2) + (percentHumidity * 0.094));

  if (hi > 79) {
    hi = -42.379 +
             2.04901523 * temperature +
            10.14333127 * percentHumidity +
            -0.22475541 * temperature*percentHumidity +
            -0.00683783 * pow(temperature, 2) +
            -0.05481717 * pow(percentHumidity, 2) +
             0.00122874 * pow(temperature, 2) * percentHumidity +
             0.00085282 * temperature*pow(percentHumidity, 2) +
            -0.00000199 * pow(temperature, 2) * pow(percentHumidity, 2);

    if((percentHumidity < 13) && (temperature >= 80.0) && (temperature <= 112.0))
      hi -= ((13.0 - percentHumidity) * 0.25) * sqrt((17.0 - abs(temperature - 95.0)) * 0.05882);

    else if((percentHumidity > 85.0) && (temperature >= 80.0) && (temperature <= 87.0))
      hi += ((percentHumidity - 85.0) * 0.1) * ((87.0 - temperature) * 0.2);
  }

  return isFahrenheit ? hi : convertFtoC(hi);
}

bool DHT::read() {
  // Reset 40 bits of received data to zero.
  data[0] = data[1] = data[2] = data[3] = data[4] = 0;

  // Send start signal.  See DHT datasheet for full signal diagram:
  //   http://www.adafruit.com/datasheets/Digital%20humidity%20and%20temperature%20sensor%20AM2302.pdf

  // Go into high impedance state to let pull-up raise data line level and
  // start the reading process.
  _delay_us(250);

  // First set data line low for 5 milliseconds.
  _out(DHT_PORT, DHT_D_REG); // configure port as output
  _off(DHT_PORT, DHT_OUT_REG); // set port to low
  _delay_ms(5);

  uint32_t cycles[80];
  {
    // Turn off interrupts temporarily because the next sections are timing critical
    // and we don't want any interruptions.
    cli();

    // End the start signal by setting data line high for 40 microseconds.
    _on(DHT_PORT, DHT_OUT_REG); // set port to high
    _delay_us(40);

    // Now start reading the data line to get the value from the DHT sensor.
    _in(DHT_PORT, DHT_D_REG); // set port as input
    _delay_us(10);  // Delay a bit to let sensor pull data line low.

    // First expect a low signal for ~80 microseconds followed by a high signal
    // for ~80 microseconds again.
    if (expectPulse(false) == 0) {
    	debug_print("Timeout waiting for start signal low pulse.");
      return false;
    }
    if (expectPulse(true) == 0) {
    	debug_print("Timeout waiting for start signal high pulse.");
      return false;
    }

    // Now read the 40 bits sent by the sensor.  Each bit is sent as a 50
    // microsecond low pulse followed by a variable length high pulse.  If the
    // high pulse is ~28 microseconds then it's a 0 and if it's ~70 microseconds
    // then it's a 1.  We measure the cycle count of the initial 50us low pulse
    // and use that to compare to the cycle count of the high pulse to determine
    // if the bit is a 0 (high state cycle count < low state cycle count), or a
    // 1 (high state cycle count > low state cycle count). Note that for speed all
    // the pulses are read into a array and then examined in a later step.
    for (int i=0; i<80; i+=2) {
      _on(PC1, PORTC);
      cycles[i]   = expectPulse(false);
      _off(PC1, PORTC);
      cycles[i+1] = expectPulse(true);
    }

    // Enable interruptions
    sei();
  } // Timing critical code is now complete.

  // Inspect pulses and determine which ones are 0 (high state cycle count < low
  // state cycle count), or 1 (high state cycle count > low state cycle count).
  for (int i=0; i<40; ++i) {
    uint32_t lowCycles  = cycles[2*i];
    uint32_t highCycles = cycles[2*i+1];
    if ((lowCycles == 0) || (highCycles == 0)) {
        debug_print("i=%d", i);
    	debug_print("Timeout waiting for pulse.");
    	return false;
    }
    data[i/8] <<= 1;
    // Now compare the low and high cycle times to see if the bit is a 0 or 1.
    if (highCycles > lowCycles) {
      // High cycles are greater than 50us low cycle count, must be a 1.
      data[i/8] |= 1;
    }
    // Else high cycles are less than (or equal to, a weird case) the 50us low
    // cycle count so this must be a zero.  Nothing needs to be changed in the
    // stored data.
  }

  /*debug_print("Received:");
  debug_print("data[0]=%d", data[0]);
  debug_print("data[1]=%d", data[1]);
  debug_print("data[2]=%d", data[2]);
  debug_print("data[3]=%d", data[3]);
  debug_print("data[4]=%d", data[4]);
  debug_print("checksum=%d", (data[0] + data[1] + data[2] + data[3]) & 0xFF);*/

  // Check we read 40 bits and that the checksum matches.
  if (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
    return true;
  }
  else {
	  debug_print("Checksum failure!");
    return false;
  }
}

// Expect the signal line to be at the specified level for a period of time and
// return a count of loop cycles spent at that level (this cycle count can be
// used to compare the relative time of two pulses).  If more than a millisecond
// ellapses without the level changing then the call fails with a 0 response.
// This is adapted from Arduino's pulseInLong function (which is only available
// in the very latest IDE versions):
//   https://github.com/arduino/Arduino/blob/master/hardware/arduino/avr/cores/arduino/wiring_pulse.c
uint32_t DHT::expectPulse(bool level) {
  uint32_t count = 0;
  // On AVR platforms use direct GPIO port access as it's much faster and better
  // for catching pulses that are 10's of microseconds in length:
  if (level) {
	while (GET_REG1_FLAG(DHT_IN_REG, DHT_PORT)) {
	  if (count++ >= _maxcycles) {
		return 0; // Exceeded timeout, fail.
	  }
	}
  } else {
	while ((GET_REG1_FLAG(DHT_IN_REG, DHT_PORT)) == 0) {
	  if (count++ >= _maxcycles) {
		return 0; // Exceeded timeout, fail.
	  }
	}
  }

  return count;
}
