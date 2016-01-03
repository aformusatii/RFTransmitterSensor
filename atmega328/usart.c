/********************************************************************************
Includes
********************************************************************************/
#include "usart.h"
#include <string.h>

/********************************************************************************
Internal Function Prototypes
********************************************************************************/
void parse_usart_cmd();

/********************************************************************************
Global Variables
********************************************************************************/
static FILE mystdout = FDEV_SETUP_STREAM(usart_putchar_printf, NULL, _FDEV_SETUP_WRITE);

volatile uint8_t usart_reg1_flags = 0;
volatile uint8_t usart_cmd_buffer[255];
volatile uint8_t usart_cmd_buffer_count = 0;

void usart_init() {
    // Set baud rate
    UBRR0H = (uint8_t)((MYUBRR)>>8);
    UBRR0L = (uint8_t)(MYUBRR);

    // Enable receiver and transmitter and Interrupt on receive complete
    UCSR0B = (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0);

    // Set frame format: 8data, 2stop bit, Odd Parity
    UCSR0C = (1<<UPM01)|(1<<UPM00)|(1<<USBS0)|(1<<UCSZ01)|(1<<UCSZ00);

    // setup our stdio stream
    stdout = &mystdout;
}

void usart_putchar(char data) {
    // Wait for empty transmit buffer
    while ( !(UCSR0A & (_BV(UDRE0))) );

    // Start transmission
    UDR0 = data;
}

char usart_getchar(void) {
    // Wait for incoming data
    while ( !(UCSR0A & (_BV(RXC0))) );

    // Return the data
    return UDR0;
}

unsigned char usart_kbhit(void) {
    //return nonzero if char waiting polled version
    unsigned char b;
    b=0;
    if(UCSR0A & (1<<RXC0)) b=1;
    return b;
}

void usart_pstr(char *s) {
    // loop through entire string
    while (*s) {
        usart_putchar(*s);
        s++;
    }
}

// this function is called by printf as a stream handler
int usart_putchar_printf(char var, FILE *stream) {
    // translate \n to \r for br@y++ terminal
    if (var == '\n') usart_putchar('\r');
    usart_putchar(var);
    return 0;
}

void handle_usart_interrupt() {
	uint8_t usart_data = UDR0;

	switch (usart_data) {
		case 13:
			SET_REG1_FLAG(usart_reg1_flags, UART_CMD_RECEIVED);
			break;

		case 127:
			SET_REG1_FLAG(usart_reg1_flags, UNSUPPORTED_CMD_RECEIVED);
			break;

		default:
			usart_putchar(usart_data);
			usart_cmd_buffer[usart_cmd_buffer_count++] = usart_data;
	}
}

void usart_check_loop() {

	/**
	 * --------> USART Unsupported command received.
	 */
	if (GET_REG1_FLAG(usart_reg1_flags, UNSUPPORTED_CMD_RECEIVED)) {
		printf("<BACKSPACE NOT SUPPORTED>");
		printf(CONSOLE_PREFIX);
		usart_cmd_buffer_count = 0;
		CLR_REG1_FLAG(usart_reg1_flags, UNSUPPORTED_CMD_RECEIVED);
	}

	/**
	 * --------> USART Command received.
	 */
	if (GET_REG1_FLAG(usart_reg1_flags, UART_CMD_RECEIVED)) {
		parse_usart_cmd();
		printf(CONSOLE_PREFIX);
		usart_cmd_buffer_count = 0;
		CLR_REG1_FLAG(usart_reg1_flags, UART_CMD_RECEIVED);
	}
}

/**
 * Processes the Command received via USART interface.
 */
void parse_usart_cmd() {
	if (usart_cmd_buffer_count > 0) {
		uint8_t sub_usart_cmd_buffer[usart_cmd_buffer_count + 1];
		memcpy(sub_usart_cmd_buffer, (uint8_t*) usart_cmd_buffer, usart_cmd_buffer_count);
		sub_usart_cmd_buffer[usart_cmd_buffer_count] = 0;

		char* pch;
		pch = strtok((char *) sub_usart_cmd_buffer, " ");

		char* cmd = NULL;
		char* cmd_args = NULL;

		if (pch != NULL) {
			cmd = pch;
		}

		pch = strtok(NULL, " ");
		if (pch != NULL) {
			cmd_args = pch;
		}

		handle_usart_cmd(cmd, cmd_args);
	}
}
