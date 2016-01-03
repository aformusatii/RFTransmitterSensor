/********************************************************************************
Includes
********************************************************************************/
#include "atmega328.h"

/********************************************************************************
	Global Variables
********************************************************************************/
volatile uint64_t startTime = 0;

/* ======================================================= */
// Set up a memory regions to access GPIO
void setup_io()
{
	_out(DDB1, DDRB); // CSN
	_out(DDB5, DDRB); // SCK
	_out(DDB3, DDRB); // MOSI
	 _in(DDB4, DDRB); // MISO
	_out(DDB2, DDRB); // CE
} // setup_io

/* ======================================================= */
// Set up SPI interface
void setup_spi()
{
	/* Enable SPI, Master, set clock rate fck/16 */
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);
} // setup_spi

/* ======================================================= */
void setCSN(uint8_t value)
{
	if (value) {
		_on(SPI_CSN, PORTB);
	} else {
		_off(SPI_CSN, PORTB);
	}
}

/* ======================================================= */
void setCE(uint8_t value)
{
	if (value) {
		_on(SPI_CE, PORTB);
	} else {
		_off(SPI_CE, PORTB);
	}
}

/* ======================================================= */
// SPI transfer
uint8_t transfer_spi(uint8_t tx_)
{
	/* Start transmission */
	SPDR = tx_;
	/* Wait for transmission complete */
	while(!(SPSR & (1<<SPIF)));
	/* Return data register */
	return SPDR;
} // transfer_spi

/* ======================================================= */
void __start_timer()
{
	startTime = getCurrentTimeCicles();
}

/* ======================================================= */
uint16_t __millis()
{
	return (uint16_t) getElapsedMilliseconds(startTime);
}

