#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h> 
#include "led.h"

#define BIT(x,b) ((x & (1<<b)) >> b)

void tick(void);

int main(void) {    
    // setup system
    led_init();
    sei();

    for (uint8_t i=0;i<20;i++) {
        tick();
    }
    for (uint8_t i=0;i<20;i++) {
        tick();
    }
    
    int8_t x = 2;
    int8_t y = 0;
    uint8_t rb_pressed = 0;
    uint8_t lb_pressed = 0;
    uint8_t rot_pressed = 0;
    uint16_t timer = 0;

    // set starting tetromino to T
    uint16_t current_block = 0b0000000001100110;    
    uint8_t next_block = 1;

    while(1) {
        // power down
        if (led_button) {
            // immediately blank display
            led_init(); 
            while (led_button);
            tick();
            tick();
            cli();
            // disable timer 1 compare A interrupt
            TIMSK=0x00;     
            DDRA=0x00;
            DDRB=0x00;
            DDRC=0x00;
            DDRD=0x00;
            DDRE=0x00;
            PORTA=0x00;
            PORTB=0x00;
            PORTC=0x00;
            PORTD=0x04;
            PORTE=0x00;
            // select low level for INT0
            MCUCR&=~0x03;   
            // clear pending interrupts
            GIFR=0x00;      
            // enable INT0
            GICR=0x40;      
            sei();
            MCUCR|=0x20;    
            // enable sleep
            asm volatile("sleep\n");
        }
        // game loop
        if (1) {
            if(timer % 256 == 0) {
                // check right button input
                uint8_t button_right = PIND & 0x01; 
                if (button_right == 0x00) {
                    if(rb_pressed == 0) {
                        uint8_t can_move = 1;
                        // check that block doesn't go out of play area
                        for(int i = 0; i < 16; i++) {
                            if(BIT(current_block, i) > 0) {
                                l[16*y+x+(i%4 + (i/4)*16)] = 0;
                                if(x+(i%4)+1 > 7) {
                                    can_move = 0;
                                }
                            }
                        }
                        if(can_move) {
                            x++;
                            rb_pressed = 1;
                        }
                    }
                }
                else {
                    rb_pressed = 0;
                }
                // check left button input
                uint8_t button_left = PIND & 0x02; 
                if (button_left == 0x00) {
                    if(lb_pressed == 0) {
                        uint8_t can_move = 1;
                        // check that block doesn't go out of play area
                        for(int i = 0; i < 16; i++) {
                            if(BIT(current_block, i) > 0) {
                                l[16*y+x+(i%4 + (i/4)*16)] = 0;
                                if(x+(i%4)-1 < 0) {
                                    can_move = 0;
                                }
                            }
                        }
                        if(can_move) {
                            x--;
                            lb_pressed = 1;
                        }
                    }
                }
                else {
                    lb_pressed = 0;
                }
                // check rotate button input
                uint8_t button_rotate = PINB & 0x04; 
                if (button_rotate == 0x00) {
                    if(rot_pressed == 0) {
                        if(current_block != 0b0000000001100110) {
                            // remove block from play area
                            for(int i = 0; i < 16; i++) {
                                if(BIT(current_block, i) > 0) {
                                    l[16*y+x+(i%4 + (i/4)*16)] = 0;
                                }
                            }   
                            uint8_t can_move = 1;                   
                            uint8_t prev_block = current_block;

                            // T shape
                            if(current_block == 0b0000000001110010) {
                                current_block = 0b0000001001100010;
                            }
                            else if(current_block == 0b0000001001100010) { 
                                current_block = 0b0000001001110000;
                            }
                            else if(current_block == 0b0000001001110000) {
                                current_block = 0b0000001000110010;
                            }
                            else if(current_block == 0b0000001000110010) {
                                current_block = 0b0000000001110010;
                            }
                            // I shape
                            else if(current_block == 0b0000000011110000) {
                                current_block = 0b0010001000100010;
                            }
                            else if(current_block == 0b0010001000100010) {
                                current_block = 0b0000000011110000;
                            }
                            // Z shape
                            else if(current_block == 0b0000000011000110) {
                                current_block = 0b0000010011001000;
                            }
                            else if(current_block == 0b0000010011001000) {
                                current_block = 0b0000000011000110;
                            }
                            // S shape
                            else if(current_block == 0b0000000001101100) {
                                current_block = 0b0000100011000100;
                            }
                            else if(current_block == 0b0000100011000100) {
                                current_block = 0b0000000001101100;
                            }
                            // L shape
                            else if(current_block == 0b0000000011101000) {
                                current_block = 0b0000110001000100;
                            }
                            else if(current_block == 0b0000110001000100) {
                                current_block = 0b0000001011100000 ;
                            }
                            else if(current_block == 0b0000001011100000) {
                                current_block = 0b0000010001000110;
                            }
                            else if(current_block == 0b0000010001000110) {
                                current_block = 0b0000000011101000;
                            }
                            // J shape
                            else if(current_block == 0b0000000011100010) {
                                current_block = 0b0000010001001100;
                            }
                            else if(current_block == 0b0000010001001100) {
                                current_block = 0b0000100011100000 ;
                            }
                            else if(current_block == 0b0000100011100000) {
                                current_block = 0b0000011001000100;
                            }
                            else if(current_block == 0b0000011001000100) {
                                current_block = 0b0000000011100010;
                            }
                            // check if rotation results in illegal move
                            for(int i = 0; i < 16; i++) {
                                if(BIT(current_block, i) > 0) {
                                    // hits another block       
                                    if(l[16*(y+1)+x+(i%4 + (i/4)*16)]==9) {
                                        can_move = 0;
                                    }       
                                    // goes out of play area
                                    if(y + (i/4) + 1>15) {
                                        can_move = 0;
                                    }
                                    // out of left side
                                    if(x+(i%4)-1 < 0) {
                                        can_move = 0;
                                    }
                                    // out of right side
                                    if(x+(i%4)+1 > 7) {
                                        can_move = 0;
                                    }
                                }
                            }
                            // restore old block if it's an illegal move
                            if(can_move == 0) {
                                current_block = prev_block;
                            }           
                            // set block back into play area
                            for(int i = 0; i < 16; i++) {
                                if(BIT(current_block, i) > 0) {
                                    l[16*y+x+(i%4 + (i/4)*16)] = 9;
                                }
                            }                                                       
                        }
                        rot_pressed = 1;                
                    }
                }
                else {
                    rot_pressed = 0;
                }
            }
            // update game logic after about a second
            if(timer % 2048 == 0) {
                // first remove the block from play area so that it doesn't 
                // collide with itself
                for(int i = 0; i < 16; i++) {
                    if(BIT(current_block, i) > 0) {
                        l[16*y+x+(i%4 + (i/4)*16)] = 0;
                    }
                }
                // check if we need to spawn new block
                uint8_t cant_move = 0;
                for(int i = 0; i < 16; i++) {
                    if(BIT(current_block, i) > 0) {
                        // hits another block       
                        if(l[16*(y+1)+x+(i%4 + (i/4)*16)]==9) {
                            cant_move = 1;
                        }       
                        // goes out of play area
                        if(y + (i/4) + 1>15) {
                            cant_move = 1;
                        }
                    }
                }
                // spawn new block if can't move
                if(cant_move) {
                    // set old block in place
                    for(int i = 0; i < 16; i++) {
                        if(BIT(current_block, i) > 0) {
                            l[16*y+x+(i%4 + (i/4)*16)] = 9;
                        }
                    }
                    // and then spawn new one
                    x = 2;
                    y = 0;

                    // set new block
                    switch(next_block++) {
                        case 0:
                            current_block = 0b0000000001100110;
                            break;
                        case 1:
                            current_block = 0b0000000001110010;
                            break;
                        case 2:
                            current_block = 0b0000000011110000;
                            break;
                        case 3:
                            current_block = 0b0000000011000110;
                            break;
                        case 4:
                            current_block = 0b0000000001101100;
                            break;
                        case 5:
                            current_block = 0b0000000011101000;
                            break;
                        case 6:
                            current_block = 0b0000000011100010;
                            break;
                    }
                    // check for full rows
                    for(int r = 0; r<16; r++) {
                        uint8_t full_row = 1;
                        for(int i = 0; i<8; i++) {
                            if(l[r*16+i] == 0) {
                                full_row = 0;
                                break;
                            }
                        }
                        // delete it if there's one and move the ones above down
                        if(full_row) {
                            uint8_t n = r;
                            while(n-1 >= 0) {
                                for(int i = 0; i<8; i++) {
                                    l[r*16+i] = l[(r-1)*16+i];
                                }
                                n--;
                            }
                        }
                    }
                }
                // otherwise move block down
                else {
                    for(int i = 0; i < 16; i++) {
                        if(BIT(current_block, i) > 0) {
                            l[16*y+x+(i%4 + (i/4)*16)] = 0;
                        }
                    }                   
                    y++;
                }                       
            }
            timer++;      

            // set led lights   
            for(int i = 0; i < 16; i++) {
                if(BIT(current_block, i) > 0) {
                    l[16*y+x+(i%4 + (i/4)*16)]=9;
                }
            }  
            // turn led lights on
            // run autoanimation only every 4th tick
            if (led_tick&0x03) {
                continue; 
            }
		
            uint8_t i=0;
            do {
                uint8_t a_tmp=l[i];
                // automatic animation command for this led
                if (a_tmp&0x10) {
                    uint8_t a_c,a_d;
                    a_d=a_tmp&0x0f;
                    a_c=a_tmp&0xf0;
                    if (a_c&0x20) {
                        a_d++;
                        if (a_d&0xf0) {
                            a_d=0x0f;
                            if (a_c&0x40) {
                               a_c^=0x20;
                            }
                            else {
                               a_c&=~0x10;
                            }
                        }
                    }
                    else {
                        a_d--;
                        if (a_d&0xf0) {
                            a_d=0x00;
                            if (a_c&0x40) {
                               a_c^=0x20;
                            }
                            else {
                               a_c&=~0x10;
                            }
                        }
                    }
                    l[i]=a_c|a_d;
                }
                i++;
            }
            while (i!=0); 
            // loop all 256 values
            l2led();
        }
        tick(); 
    }
}

void tick(void)
{
    uint8_t tmp;
    tmp=led_tick;
    while (tmp==led_tick);
}
