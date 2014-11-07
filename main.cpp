/*
    Led dimmer
    Author: P.S. van der Heide
    Target: Atmel Tiny25

    Behavior
    Upon power on the output will be OFF, stored brightness will be MAX

    Led off:
    When the button is pressed shortly, the output will be turned ON using the stored brightness
    When the button is held longer the output will be turned on, with brightness set to min.

    Led on:
    Pressing the button shortly will turn the led off, storing the brightness
    Pressing the button longer, the brightness will be adjusted.
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

void setupPwm()
{
    //Set up timer for fast-pwm
    TCCR0A = 0b10000011;
    TCCR0B = 0b00000001;
    GTCCR  = 0b00000001;
}

void deSetupPwm()
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

bool pollButton()
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
    else
    {
        sleep();
    }
    return toReturn;
}

void brightnessAdjust()
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

void handleState(bool button)
{
    switch (s)
    {
        case OFF:
        case ON:
        if (button)
        {
            tick_counter = 0;
            s = (s == ON) ? ON_BUTTON_PRESSED : OFF_BUTTON_PRESSED;
        }
        break;

        case OFF_BUTTON_PRESSED:
        tick_counter++;
        //long press -> on at MIN
        if (tick_counter > 20)
        {
            OCR0A = 0x0;
            countUp = true;
            setupPwm();
            s = ON;
        }
        //short press -> on
        if (! button)
        {
            setupPwm();
            s = ON;
        }
        break;

        case ON_BUTTON_PRESSED:
        tick_counter++;
        if (tick_counter < 20)
        {
            //short press -> off
            if (! button)
            {
                s = OFF;
                deSetupPwm();
            }
        }
        else
        {
            //long press -> change brightness
            if (! button)
            {
                //on release change direction
                s = ON;
                countUp = !countUp;
            }
            else
            {
                brightnessAdjust();
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
        bool button = pollButton();
        handleState(button);
    }
    return 0;
}

