/********************************************************************************
	Includes
********************************************************************************/

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <util/delay.h>

#include "../nrf24l01/RF24.h"
#include "../atmega328/mtimer.h"
#include "../common/util.h"
#include "../dht/dht.h"

extern "C" {
#include "../ds18x20/ds18x20lib.h"
#include "../atmega328/usart.h"
}

/********************************************************************************
	Macros and Defines
********************************************************************************/


/********************************************************************************
	Function Prototypes
********************************************************************************/
void initTimer2();
void readAndSendTemperature();

/********************************************************************************
	Global Variables
********************************************************************************/
volatile uint16_t timer2_count = 3600;

RF24 radio;
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };
DHT dht(DHT22);

/********************************************************************************
	Interrupt Service
********************************************************************************/
ISR(USART_RX_vect)
{
	handle_usart_interrupt();
}

ISR(TIMER1_OVF_vect)
{
	incrementOvf();
}

ISR(INT0_vect)
{
	//printf("\n%i", index++);
}

ISR(TIMER2_OVF_vect)
{
	_NOP();
}

/********************************************************************************
	Main
********************************************************************************/
int main(void) {
    // initialize code
	usart_init();

    // enable interrupts
    sei();

    // Init Timer 1
    initTimer();

    // Init Timer 2
    initTimer2();

    // Configure Sleep Mode - Power-save
    SMCR = (0<<SM2)|(1<<SM1)|(1<<SM0)|(0<<SE);

	// Output initialization log
    printf("Start...");
    printf(CONSOLE_PREFIX);

    radio.begin();
    radio.setRetries(15,15);
    radio.setPayloadSize(8);
    radio.setPALevel(RF24_PA_MAX);
    radio.setChannel(110);

    radio.openWritingPipe(pipes[0]);
    radio.openReadingPipe(1,pipes[1]);

    radio.printDetails();

    _delay_ms(10);
    radio.powerDown();
    _delay_ms(10);

    ds1820_init(DS1820_pin);

    dht.begin();

	// main loop
    while (1) {
    	// main usart loop
    	//usart_check_loop();

    	// Sleep mode to save battery, Timer 2 will wake up once each 8 seconds
		sleep_mode();

		// each hour send the data
		if (++timer2_count >= 450) {
			timer2_count = 0;
			readAndSendTemperature();
		}
    }
}

/********************************************************************************
	Functions
********************************************************************************/
void initTimer2() {
    // Init timer2
    //Disable timer2 interrupts
    TIMSK2  = 0;
    //Enable asynchronous mode
    ASSR  = (1<<AS2);
    //set initial counter value
    TCNT2=0;
    //set prescaller 1024
    TCCR2B = (1<<CS22)|(1<<CS21)|(1<<CS00);
    //wait for registers update
    while (ASSR & ((1<<TCN2UB)|(1<<TCR2BUB)));
    //clear interrupt flags
    TIFR2  = (1<<TOV2);
    //enable TOV2 interrupt
    TIMSK2  = (1<<TOIE2);
}

void readAndSendTemperature() {
	if (dht.read()) {
		double h = dht.getHumidity() * 10.00f;
		int16_t h_int = (int16_t) h;

	    // split humidity in two bytes
	    uint8_t h_low = (uint8_t) h_int;
	    uint8_t h_high = (uint8_t) (h_int>>8);

	    debug_print("h_high=%d", h_high);
	    debug_print("h_low=%d", h_low);

		double t = dht.getTemperature() * 10.00f;
	    int16_t t_int = (int16_t) t;

	    // split temperature in two bytes
	    uint8_t t_low = (uint8_t) t_int;
	    uint8_t t_high = (uint8_t) (t_int>>8);

	    debug_print("t_high=%d", t_high);
	    debug_print("t_low=%d", t_low);

	    // Send data to server via RF link
	    radio.powerUp();
	    _delay_ms(50);

	    // Send temperature via NRF24L01 transceiver
		uint8_t data1[] = {100, 1, 1, t_high, t_low};
		radio.write(data1, 5);

		_delay_ms(10);

		// need to flush tx buffer, fixed the issue with packet shift...
		radio.stopListening();

		_delay_ms(200);

	    // Send humidity via NRF24L01 transceiver
		uint8_t data2[] = {100, 1, 2, h_high, h_low};
		radio.write(data2, 5);

		_delay_ms(10);

		// need to flush tx buffer, fixed the issue with packet shift...
		radio.stopListening();
	    radio.powerDown();

		// Wait a little before going to sleep again
	    _delay_ms(10);
	}
}

void readAndSendTemperatureOld() {
	// read temperature from DS1820 sensor
    float temp = ds1820_read_temp(DS1820_pin);
    temp *= 10.00f;

    // take integer part of temperature
    int16_t temp_int = (int16_t) temp;

    // split temperature in two bytes
    uint8_t temp_low = (uint8_t) temp_int;
    uint8_t temp_high = (uint8_t) (temp_int>>8);

    debug_print("temp_int=%d", temp_int);
    debug_print("temp_high=%d", temp_high);
    debug_print("temp_low=%d", temp_low);

    //int16_t temp_int_rec = (int16_t) (((temp_high & 0x00FF) << 8) | (temp_low & 0x00FF));
    //debug_print("temp_int_rec=%d", temp_int_rec);

    radio.powerUp();
    _delay_ms(50);

    // Send temperature via NRF24L01 transceiver
	uint8_t data[] = {100, 1, temp_high, temp_low};
	radio.write(data, 4);

	_delay_ms(10);
	// need to flush tx buffer, fixed the issue with packet shift...
	radio.stopListening();
    radio.powerDown();

	// Wait a little before going to sleep again
    _delay_ms(10);
}

void handle_usart_cmd(char *cmd, char *args) {
	if (strcmp(cmd, "test") == 0) {
		printf("\n TEST [%s]", args);
	}

	if (strcmp(cmd, "read") == 0) {
		if (dht.read()) {
			// Reading temperature or humidity takes about 250 milliseconds!
			// Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
			double h = dht.getHumidity();
			// Read temperature as Celsius (the default)
			double t = dht.getTemperature();
			// Read temperature as Fahrenheit (isFahrenheit = true)
			double f = dht.getTemperature(true);

			// Compute heat index in Fahrenheit (the default)
			double hif = dht.computeHeatIndex(f, h);
			// Compute heat index in Celsius (isFahreheit = false)
			double hic = dht.computeHeatIndex(t, h, false);

			debug_print("Humidity: %f", h);
			debug_print("Temperature: %f *C, %f *F", t, f);
			debug_print("Heat index: %f *C, %f *F", hic, hif);
		} else {
			debug_print("ERROR reading data");
		}
	}

	if (strcmp(cmd, "read2") == 0) {
		double temp = (double) ds1820_read_temp(DS1820_pin);
		debug_print("temp=%f", temp);
	}

	if (strcmp(cmd, "send") == 0) {
		readAndSendTemperature();
	}
}
