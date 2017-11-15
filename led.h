
#include <avr/io.h>

#define ROWS 16


void led_init(void);
void led_set(uint8_t x, uint8_t y, uint8_t value);
void l2led();

extern volatile uint8_t led_tick,led_phase,led_button;
extern uint8_t  l[];
extern uint8_t  l_port[][4][2];

struct line {
	uint8_t port;
	uint8_t bit;
};
