/********************************************************************************
	Includes
********************************************************************************/
#include <avr/io.h>

/********************************************************************************
	Macros and Defines
********************************************************************************/
#define _NOP() do { __asm__ __volatile__ ("nop"); } while (0)

#define _in(bit,port)	port &= ~(1 << bit)
#define _out(bit,port)	port |= (1 << bit)
#define _on(bit,port)	port |= (1 << bit)
#define _off(bit,port)	port &= ~(1 << bit)

#define _to_uint64(x,y) ((uint64_t) x << 16) | y

#define GET_REG1_FLAG(reg, bit) (1 << bit) & reg
#define SET_REG1_FLAG(reg, bit) reg |= (1 << bit)
#define CLR_REG1_FLAG(reg, bit) reg &= ~(1 << bit)

#define _to_uint64(x,y) ((uint64_t) x << 16) | y

#define _between(actual,expected) (actual > (expected - 8)) & (actual < (expected + 8))

#define CONSOLE_DEBUG 0

#if CONSOLE_DEBUG == 1
#define debug_print(...) \
			printf(__VA_ARGS__)
#else
#define debug_print(...) \
			do {} while (0)
#endif
