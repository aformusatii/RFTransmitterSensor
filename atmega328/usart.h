
#ifndef USART_H_
#define USART_H_

/********************************************************************************
Includes
********************************************************************************/
#include <stdio.h>
#include <stdbool.h>
#include "../common/util.h"

/********************************************************************************
Macros and Defines
********************************************************************************/
#define BAUD 4800
#define MYUBRR F_CPU/16/BAUD-1

#define CONSOLE_PREFIX "\natmega328>"

#define UART_CMD_RECEIVED  0
#define UNSUPPORTED_CMD_RECEIVED  1

/********************************************************************************
Function Prototypes
********************************************************************************/
void usart_init();
char usart_getchar( void );
void usart_putchar( char data );
void usart_pstr (char *s);
unsigned char usart_kbhit(void);
int usart_putchar_printf(char var, FILE *stream);

void handle_usart_interrupt();
void usart_check_loop();
void handle_usart_cmd(char *cmd, char *arg);

#endif /* USART_H_ */
