/*
    Bit accessors
    Author: P.S. van der Heide

    convenience functions for bit manipulation
    set, clear, complement, read, and write
*/

#pragma once
#include "stdint.h"
#include "stdbool.h"

inline void setbit(volatile uint8_t& address, uint8_t bit)
{
    address |= (1<<bit);
}

inline void clrbit(volatile uint8_t& address, uint8_t bit)
{
    address &= ~(1<<bit);
}

inline void cplbit(volatile uint8_t& address, uint8_t bit)
{
    address ^= (1<<bit);
}

inline bool readbit(volatile uint8_t& address, uint8_t bit)
{
    return (address & (1<<bit));
}

inline void writebit(volatile uint8_t& address, uint8_t bit, bool value)
{
    value ? setbit(address, value) : clrbit(address, value);
}
