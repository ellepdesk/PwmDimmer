/*
    Led dimmer
    Author: P.S. van der Heide
    Target: Atmel Tiny25

    Behavior
    Upon power on the output will be OFF, stored brightness will be MAX
    When the button is pressed shortly, the output will be turned ON using the stored brightness
    When the button is pressed longer the output will be turned on, with brightness set to min.
    When the led is on, pressing the button shortly will turn the led off, storing the brightness
    When the button is pressed longer, the brightness will be adjusted.
    The direction of the adjustment (brighter/dimmer) will be toggled on each long press
    When the button is held, the adjustment will limit at MIN or MAX, depending on the direction
 */

#include <avr/io.h>
#include "bitaccess.h"
#include <util/delay_basic.h>

enum state
{
    OFF,
    OFF_BUTTON_PRESSED,
    ON,
    ON_BUTTON_PRESSED,
};

static bool countUp = false;
static uint16_t tick_counter = 0;
static state s = OFF;

void SetupPwm()
{
    //Set up timer for fast-pwm
    TCCR0A = 0b10000011;
    TCCR0B = 0b00000001;
    GTCCR  = 0b00000001;
}

void DeSetupPwm()
{
    //Clear timer registers
    GTCCR  = 0b00000000;
    TCCR0A = 0b00000000;
    TCCR0B = 0b00000000;
}

void sleep()
{
    const uint16_t delay = 0xFFF;
    _delay_loop_2(delay);
}

bool button_pressed()
{
    bool toReturn = false;
    if (!readbit(PINB, PINB4))
    {
        //de-bounce
        sleep();
        if (!readbit(PINB, PINB4))
        {
            toReturn = true;
        }
    }
    return toReturn;
}

void adjust_brightness()
{
    if (countUp)
    {
        if (OCR0A < 255)
            OCR0A++;
    }
    else
    {
        if (OCR0A > 0)
            OCR0A--;
    }
}

void handle_state()
{
    switch (s)
    {
        case OFF:
        case ON:
        if (button_pressed())
        {
            tick_counter = 0;
            s = (s == ON) ? ON_BUTTON_PRESSED : OFF_BUTTON_PRESSED;
        }
        break;

        case OFF_BUTTON_PRESSED:
        {
        tick_counter++;
        if (tick_counter > 20)
        {
            OCR0A = 0x0;
            countUp = true;
            SetupPwm();
            s = ON;
        }
        if (! button_pressed())
        {
            SetupPwm();
            s = ON;
        }
        }
        break;

        case ON_BUTTON_PRESSED:
        tick_counter++;
        if (tick_counter < 20)
        {
            //short press -> off
            if (! button_pressed())
            {
                s = OFF;
                DeSetupPwm();
            }
        }
        else
        {
            //long press -> change brightness
            if (! button_pressed())
            {
                //on release change direction
                s = ON;
                countUp = !countUp;
            }
            else
            {
                adjust_brightness();
            }
        }
        break;
    }
}

int main(void)
{
    setbit(DDRB,DDB0);
    setbit(DDRB,DDB3);
    setbit(PORTB,PORTB4);
    OCR0A = 0xFF;
    while (true)
    {
        handle_state();
        sleep();
    }
    return 0;
}

