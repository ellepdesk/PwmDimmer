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
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include "bitaccess.h"
#include <util/delay_basic.h>

//only for wake-up
EMPTY_INTERRUPT(PCINT0_vect);
ISR(TIM1_COMPA_vect)
{
    //stop timer
    clrbit(GTCCR,0);
}

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

void setupSleepTimer()
{
    TCCR1 = 0b0000111; // 1MHz/1024 about 1ms/tick
    setbit(TIMSK,6);   // enable interrupt on ocr1a
    setbit(GTCCR,1);   // start timer
}

void setupPwm()
{
    //Set up timer for fast-pwm
    TCCR0A = 0b10000011;
    TCCR0B = 0b00000001;
    setbit(GTCCR,0);
}

void deSetupPwm()
{
    //Clear timer registers
    clrbit(GTCCR,0);
    TCCR0A = 0b00000000;
    TCCR0B = 0b00000000;
}

void sleep(uint8_t period = 0)
{
    if (period > 0)
    {
        OCR1A = period;
        setupSleepTimer();
    }
    sleep_cpu();
    //when value > 0
    //  load timer 1 with value
    //  start timer
    //go to low-power mode
    //will return on either button or timeout
    //const uint16_t delay = 0xFFF;
    //_delay_loop_2(delay);
}

bool pollButton()
{
    bool toReturn = false;
    if (!readbit(PINB, PINB4))
    {
        //de-bounce
        sleep(100);
        if (!readbit(PINB, PINB4))
        {
            toReturn = true;
        }
    }
    else
    {
        sleep(100);
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

bool handleState(bool button)
{
    bool busy = false;
    switch (s)
    {
        case OFF:
        case ON:
        if (button)
        {
            tick_counter = 0;
            busy = true;
            s = (s == ON) ? ON_BUTTON_PRESSED : OFF_BUTTON_PRESSED;
        }
        break;

        case OFF_BUTTON_PRESSED:
        busy = true;
        tick_counter++;
        //long press -> on at MIN
        if (tick_counter > 10)
        {
            OCR0A = 0x0;
            countUp = true;
            setupPwm();
            s = ON_BUTTON_PRESSED;
        }
        //short press -> on
        if (! button)
        {
            setupPwm();
            s = ON;
        }
        break;

        case ON_BUTTON_PRESSED:
        busy = true;
        tick_counter++;
        if (tick_counter < 10)
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
            else if (tick_counter > 20)
            {
                brightnessAdjust();
            }
        }
        break;
    }
    return busy;
}

int main(void)
{
    setbit(DDRB,DDB0);
    setbit(DDRB,DDB3);

    //enable pin-change interrupt in PB4, using internal pull-up
    setbit(PORTB,PORTB4);
    setbit(GIMSK, 5);
    setbit(PCMSK, PCINT4);
    OCR0A = 0xFF;
    sei();
    sleep_enable();
    set_sleep_mode(SLEEP_MODE_IDLE);
    while (true)
    {
        bool button = pollButton();
        if (!handleState(button))
        {
            sleep(0);
            //allow deep sleep
            //wake up by pin change
        }
    }
    return 0;
}

