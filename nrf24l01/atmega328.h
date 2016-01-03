/********************************************************************************
Includes
********************************************************************************/
#include <stdio.h>
#include <util/delay.h>

#include "../common/util.h"
#include "../atmega328/mtimer.h"

/********************************************************************************
Macros and Defines
********************************************************************************/
#define SPI_CSN PORTB1
#define SPI_CE  PORTB2

/* =========== SPI and GPIO function ============ */
void setup_io();
void setup_spi();
void setCSN(uint8_t value);
void setCE(uint8_t value);
uint8_t transfer_spi(uint8_t tx_);
void __start_timer();
uint16_t __millis();
