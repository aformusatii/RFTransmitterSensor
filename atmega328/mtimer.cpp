/********************************************************************************
Includes
********************************************************************************/

#include "mtimer.h"

volatile uint32_t timer1_ovf_count = 0;

/**
 * Initialize timer.
 */
void initTimer() {
    TCCR1B = (1<<CS12)|(0<<CS11)|(1<<CS10);
	_on(TOIE1, TIMSK1);
}

void incrementOvf() {
	timer1_ovf_count++;
}

/**
 * Returns elapsed time in milliseconds starting with given startTime (in cpu cicles).
 * Figure valid for clkI/O/1024 (From prescaler) and clk = 8 Mhz
 */
uint32_t getElapsedMilliseconds(uint64_t startTime) {
	uint64_t endTime = getCurrentTimeCicles();
	uint64_t diff = (endTime - startTime);

	return (uint32_t) ((diff * 10000) / 78125);
}

/**
 * Returns cpu cicles needed to reach given period of seconds.
 */
uint64_t convertSecondsToCicles(uint16_t value) {
	return (value == 0) ? 0: getCurrentTimeCicles() + ((value * 78125) / 10);
}

/**
 * Returns current cpu cicles counted so far by Timer 1 (48 bits - 7812.5 per second, max value ~1142 years, should be enough).
 * Figure valid for clkI/O/1024 (From prescaler) and clk = 8 Mhz
 */
uint64_t getCurrentTimeCicles() {
	uint32_t highValue = timer1_ovf_count;
	uint16_t lowValue = TCNT1;

	if (lowValue < 4) {
		highValue = timer1_ovf_count;
	}

	return _to_uint64(highValue, lowValue);
}
