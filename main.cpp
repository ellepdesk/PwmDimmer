/*
    Led dimmer
    Author: P.S. van der Heide
    Target: Atmel Tiny25

    Behavior
    read potmeter on ADC3 and adjust brightness accordingly
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include "bitaccess.h"
#include <util/delay_basic.h>

//enum state
//{
//    OFF,
//    OFF_BUTTON_PRESSED,
//    ON,
//    ON_BUTTON_PRESSED,
//};

//static bool countUp = false;//
//static uint16_t tick_counter = 0;
//static state s = OFF;
static volatile uint16_t adcValue = 1023;

//only for wake-up
EMPTY_INTERRUPT(PCINT0_vect);
ISR(TIM1_COMPA_vect)
{
    //stop timer
    clrbit(GTCCR,0);
}

ISR(ADC_vect)
{
    uint16_t low, high, value;
    low = ADCL;     //ADCL must be read first
    high = ADCH;
    value = high * 256 + low;

    // IIR filter
    // use 15/16ths of old value and 1/16th of the new value
    const uint16_t factor_16 = 15;
    adcValue = ((adcValue * factor_16) + (value * (16 - factor_16))) / 16;
}

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

void setupAdc()
{
    setbit(DIDR0, ADC3D);
    ADMUX = 0b00000011;  // vcc as vref, align right, select ADC3
    ADCSRA = 0b11101111; // enable adc, external trigger, start conversion, enable interrupt, 128x prescaler
                         // trigger source = 0, continous conversion
}

void sleep(uint8_t period = 0)
{
    if (period > 0)
    {
        OCR1A = period;
        setupSleepTimer();
    }
    sleep_cpu();
}

int main(void)
{
    setbit(DDRB,DDB0);
    setbit(DDRB,DDB2);

    //enable pin-change interrupt in PB4, using internal pull-up
    setbit(PORTB,PORTB4);
    setbit(GIMSK, 5);
    setbit(PCMSK, PCINT4);
    OCR0A = 0xFF;
    sei();
    setupAdc();
    sleep_enable();
    set_sleep_mode(SLEEP_MODE_IDLE);
    while (true)
    {
        // wait for adc interrupt
        sleep();
        if (adcValue > 768)
        {
            deSetupPwm();
        }
        else if (adcValue < 512)
        {
            setupPwm();
            OCR0A = 255 - (adcValue/2);
        }
        else
        {
            setupPwm();
            OCR0A = 0;
        }

    }
    return 0;
}

