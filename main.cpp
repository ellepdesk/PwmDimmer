/*
 */

#include <avr/io.h>
#include "bitaccess.h"
#include <util/delay_basic.h>

void SetupPwm()
{
    //Set up timer for fast-pwm
    TCCR0A = 0b10000011;
    TCCR0B = 0b00000001;
    GTCCR  = 0b00000001;
}

void DeSetupPwm()
{
    //Set up timer for fast-pwm
    GTCCR  = 0b00000000;
    TCCR0A = 0b00000000;
    TCCR0B = 0b00000000;
}

int main(void)
{

    setbit(DDRB,DDB0);
    setbit(DDRB,DDB3);
    const uint16_t delay = 0x3FF;

    while (1)
    {
        uint8_t i = 0;
        SetupPwm();
        //Slow raise, 0..63
        for (i = 0 ; i < 128 ; i++)
        {
            OCR0A = i / 2;
            cplbit(PORTB, PB3);
            _delay_loop_2(delay);
        }
        //Fast raise 64..255
        for (i = 64 ; i < 255 ; i++)
        {
            OCR0A = i;
            cplbit(PORTB, PB3);
            _delay_loop_2(delay);
        }
        //Keep high
        for (i = 0 ; i < 128 ; i++)
        {
            cplbit(PORTB, PB3);
            _delay_loop_2(delay);
        }

        //Fast fall 255..64
        for (i = 0 ; i < 196 ; i++)
        {
            OCR0A = 255 - i;
            cplbit(PORTB, PB3);
            _delay_loop_2(delay);
        }
        //Slow fall 63..0
        for (i = 0 ; i < 128 ; i++)
        {
            OCR0A = 64 - (i/2);
            cplbit(PORTB, PB3);
            _delay_loop_2(delay);
        }
        //Keep low
        DeSetupPwm();
        clrbit(PORTB,PB0);
        for (i = 0 ; i < 128 ; i++)
        {
            cplbit(PORTB, PB3);
            _delay_loop_2(delay);
        }
    }

    return 0;
}
