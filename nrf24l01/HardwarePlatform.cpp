#include "HardwarePlatform.h"

/* ======================================================= */
void HardwarePlatform::initIO() {
	setup_io();
}

void HardwarePlatform::initSPI() {
	setup_spi();
}

void HardwarePlatform::csn(uint8_t value) {
	setCSN(value);
}

void HardwarePlatform::ce(uint8_t value) {
	setCE(value);
}

uint8_t HardwarePlatform::spiTransfer(uint8_t tx_) {
	return transfer_spi(tx_);
}

void HardwarePlatform::delayMicroseconds(uint64_t micros) {
	_delay_us(micros);
}

void HardwarePlatform::delayMilliseconds(uint64_t milisec) {
	_delay_ms(milisec);
}
