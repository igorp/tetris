#include <avr/io.h>
#include <avr/interrupt.h>

#include "led.h"


uint8_t  l[ROWS*ROWS]; // analog brightness values 0 - 15

// first A2 A1 A0 B4 B3 B2 B1 B0 second: D7 D6 D5 D4 D3 D2 D1 D0
uint8_t  l_port[ROWS][4][2]; // actually there are 3 ports (A, B and D) to update, but this packs and aligns the data

volatile uint8_t led_row=0,led_phase=0,led_button=0;
volatile uint8_t led_tick=0;

uint8_t l_order[16]={1,3,5,7,9,11,13,15,0,2,4,14,12,10,8,6,};

struct line X[ROWS]={{0,7},
				     {1,7},
				     {0,6},
				     {1,6},
				     {0,5},
				     {1,5},
				     {0,0},
				     {1,4},
				     {0,1},
				     {1,3},
				     {0,2},
				     {1,2},
				     {0,3},
				     {1,1},
				     {0,4},
				     {1,0}};

struct line Y[ROWS]={{(uint16_t)&PORTA,3},
					 {(uint16_t)&PORTA,4},
					 {(uint16_t)&PORTA,5},
					 {(uint16_t)&PORTA,6},
					 {(uint16_t)&PORTA,7},
					 {(uint16_t)&PORTE,0},
					 {(uint16_t)&PORTE,1},
					 {(uint16_t)&PORTE,2},
					 {(uint16_t)&PORTC,7},
					 {(uint16_t)&PORTC,6},
					 {(uint16_t)&PORTC,5},
					 {(uint16_t)&PORTC,4},
					 {(uint16_t)&PORTC,3},
					 {(uint16_t)&PORTC,2},
					 {(uint16_t)&PORTC,1},
					 {(uint16_t)&PORTC,0}};

void led_init(void)
{
	// clear sleep enable - select powerdown as later sleep mode
	MCUCR&=~0x20;
	MCUCR|=0x10;
	MCUCSR&=~0x20;
	EMCUCR&=~0x80;
	GICR=0x00;		// disable INT0

	for (uint8_t i=0;i<ROWS;i++)
	   {
	   for (uint8_t j=0;j<4;j++)
	      {
	   	  l_port[i][j][0]=0xff; // A and B
	   	  l_port[i][j][1]=0xff; // D
		  }
	   }
	for (uint16_t i=0;i<ROWS*ROWS;i++) l[i]=0;

	// set data direction for matrix driving pins to output
	DDRA=0xff;
	DDRB=0x1f;
	DDRC=0xff;
	DDRD=0xff;
	DDRE=0x07;

	// disable all - columns 1, rows 0
	PORTA=0x07;
	PORTB=0x1f;
	PORTC=0x00;
	PORTD=0xff;
	PORTE=0x00;

	// set timer interrupt
	TCCR1A=0x00; // CTC mode
	TCCR1B=0x08; // no clock yet
	TCNT1=0;     // clear counter (not really necessary after reset)
	OCR1A=256;   // 
	TIMSK=0x40;  // enable compare A interrupt
	TIFR=0x40;	 // clear possible pending flag (not really necessary, but nice)
	TCCR1B=0x09; // start - full speed
}

void led_set(uint8_t x, uint8_t y, uint8_t value)
{
	uint8_t port;
	uint8_t bit;
	port=X[x].port;
	bit=1<<X[x].bit;
	if (value&0x01)
	   {
	   l_port[y][0][port]&=~bit;
	   }
	else
	   {
	   l_port[y][0][port]|=bit;
	   }
	if (value&0x02)
	   {
	   l_port[y][1][port]&=~bit;
	   }
	else
	   {
	   l_port[y][1][port]|=bit;
	   }
	if (value&0x04)
	   {
	   l_port[y][2][port]&=~bit;
	   }
	else
	   {
	   l_port[y][2][port]|=bit;
	   }
	if (value&0x08)
	   {
	   l_port[y][3][port]&=~bit;
	   }
	else
	   {
	   l_port[y][3][port]|=bit;
	   }
}

void l2led() {
	uint8_t i,j,tmp;
	uint16_t tmp0,tmp1,tmp2,tmp3;
	for(j=0;j<16;j++)
	   {
	   tmp0=0;tmp1=0;tmp2=0;tmp3=0;
	   for(i=0;i<16;i++)
	      {
		  tmp=l[j*16+l_order[i]];
		  tmp0=(tmp0<<1)|((tmp&0x01) ? 0 : 1);
		  tmp1=(tmp1<<1)|((tmp&0x02) ? 0 : 1);
		  tmp2=(tmp2<<1)|((tmp&0x04) ? 0 : 1);
		  tmp3=(tmp3<<1)|((tmp&0x08) ? 0 : 1);
		  }
	   *(uint16_t *)&l_port[j][0][0]=tmp0;
	   *(uint16_t *)&l_port[j][1][0]=tmp1;
	   *(uint16_t *)&l_port[j][2][0]=tmp2;
	   *(uint16_t *)&l_port[j][3][0]=tmp3;
	   }
}

// led update interrupt at variable rate for 4 scans per about 2KHz
ISR(TIMER1_COMPA_vect,ISR_NAKED)
{
	// enter interrupt
	asm volatile ( 
	"push r16\n\t"
	"in r16,__SREG__\n\t"
	"push r16\n\t"
	"push r17\n\t"
	"push r30\n\t"
	"push r31\n\t"
	"clr r17\n\t"
	::);

	// increment row, update phase and ms
	asm volatile (
	"lds r16,led_row\n\t"
	"inc r16\n\t"
	"andi r16,0x0f\n\t"
	"sts led_row,r16\n\t"
	"brne done\n\t"
	"lds r16,led_phase\n\t"
	"inc r16\n\t"
	"andi r16,0x03\n\t"
	"sts led_phase,r16\n\t"
	"brne tick_ready\n\t"
	"lds r16,led_tick\n\t"
	"inc r16\n\t"
	"sts led_tick,r16\n\t"
	"clr r16\n\t"
	"tick_ready:\n\t"
	"sec\n\t"
	"clockloop:\n\t"
	"rol r17\n\t"
	"dec r16\n\t"
	"brpl clockloop\n\t"
	"out %0,r17\n\t"
	"clr r17\n\t"
	"out %1,r17\n\t"
	"done:\n\t"
	:
	:"I" (_SFR_IO_ADDR(OCR1AH)),
	"I" (_SFR_IO_ADDR(OCR1AL))
	);

	// disable all columns and rows
	asm volatile (
	"ldi r16,0x07\n\t" "out %0,r16\n\t"
	"ldi r16,0x1f\n\t" "out %1,r16\n\t"
	"ldi r16,0x00\n\t" "out %2,r16\n\t"
	"ldi r16,0xff\n\t" "out %3,r16\n\t"
	"ldi r16,0x00\n\t" "out %4,r16\n\t"
	:
	:"I" (_SFR_IO_ADDR(PORTA)),
	"I" (_SFR_IO_ADDR(PORTB)),
	"I" (_SFR_IO_ADDR(PORTC)),
	"I" (_SFR_IO_ADDR(PORTD)),
	"I" (_SFR_IO_ADDR(PORTE))
	);

	// test button
	asm volatile (
	"in r16,%0\n\t"
	"com r16\n\t"
	"andi r16,0x04\n\t"
	"sts led_button,r16\n\t"
	"brne return\n\t"
	:
	:"I" (_SFR_IO_ADDR(PIND))
	);
	
	// first A2 A1 A0 B4 B3 B2 B1 B0 second: D7 D6 D5 D4 D3 D2 D1 D0
	// update X-driving port bits from a pre-calculated table
	asm volatile (
	"lds r16,led_row\n\t"
	"lsl r16\n\t"
	"lsl r16\n\t"
	"lsl r16\n\t"
	"add r30,r16\n\t"
	"adc r31,r17\n\t"
	"lds r16,led_phase\n\t"
	"lsl r16\n\t"
	"add r30,r16\n\t"
	"adc r31,r17\n\t"
	"ld r16,Z\n\t"
	"swap r16\n\t"
	"lsr r16\n\t"
	"andi r16,0x07\n\t"
	"out %1,r16\n\t"
	"ld r16,Z+\n\t"
	"andi r16,0x1f\n\t"
	"out %2,r16\n\t"
	"ld r16,Z\n\t"
	"out %3,r16\n\t"
	:
	: "z" ((uint8_t*) &l_port[0][0][0]),
	  "I" (_SFR_IO_ADDR(PORTA)),
	  "I" (_SFR_IO_ADDR(PORTB)),
	  "I" (_SFR_IO_ADDR(PORTD))
	);

	// turn on line bit (y)
	asm volatile (
	"lds r16,led_row\n\t"
	"lsl r16\n\t"
	"add r30,r16\n\t"
	"adc r31,r17\n\t"
	"ld r16,Z+\n\t"
	"ld r17,Z+\n\t"
	"mov r30,r16\n\t"
	"clr r31\n\t"
	"clr r16\n\t"
	"sec\n\t"
	"shiftloop:\n\t"
	"rol r16\n\t"
	"dec r17\n\t"
	"brpl shiftloop\n\t"
	"ld r17,Z\n\t"
	"or r17,r16\n\t"
	"st Z,r17\n\t"
	:
	: "z" ((uint8_t*) &Y[0])
	);

	// return from interrupt
	asm volatile (
	"return:\n\t"
	"pop r31\n\t"
	"pop r30\n\t"
	"pop r17\n\t"
	"pop r16\n\t"
	"out __SREG__,r16\n\t"
	"pop r16\n\t"
	"reti\n\t"
	::);
}

