/*
 Copyright (C) 2011 J. Coliz <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

#include "atmega328.h"
#include <string.h>

/* ============================================== */
#define IF_SERIAL_DEBUG(x) x
#define printf_P printf
#define strlen_P strlen
#define PRIPSTR "%s"
#define PSTR(x) x

/* ============================================== */
class HardwarePlatform {
public:
	void initIO();
	void csn(uint8_t value);
	void ce(uint8_t value);
	void initSPI();
	uint8_t spiTransfer(uint8_t tx_);
	void delayMicroseconds(uint64_t micros);
	void delayMilliseconds(uint64_t milisec);
};

