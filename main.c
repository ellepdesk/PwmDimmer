/*
 */

#include <avr/io.h>
//#include "bitaccess.h"
#include <util/delay_basic.h>

int main(void)
{

    // Insert code
    DDRB |= 1<<DDB0;
    DDRB |= 1<<DDB3;
    PORTB |= 1<<PB0;

    TCCR0A = 0b10000011;
    TCCR0B = 0b00000001;

    GTCCR  = 0b00000001;


    while (1)
    {
        uint8_t i = 0;
        for (i = 0 ; i < 128 ; i++)
        {
            OCR0A = i / 2;
            PORTB ^= 1 << PB3;
            _delay_loop_2(0xFFF);
        }
        for (i = 64 ; i < 255 ; i++)
        {
            OCR0A = i;
            PORTB ^= 1 << PB3;
            _delay_loop_2(0xFFF);
        }
        for (i = 0 ; i < 128 ; i++)
        {
            OCR0A = 255 - i/2;
            PORTB ^= 1 << PB3;
            _delay_loop_2(0xFFF);
        }
        for (i = 0 ; i < 255 ; i++)
        {
            OCR0A = 255 - i;
            PORTB ^= 1 << PB3;
            _delay_loop_2(0xFFF);
        }
    }

    return 0;
}
