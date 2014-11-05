#pragma once
#include "stdint.h"
#include "stdbool.h"

inline void setbit(uint8_t* address, uint8_t bit)
{
    *address |= (1<<bit);
}

inline void clrbit(uint8_t* address, uint8_t bit)
{
    *address &= ~(1<<bit);
}

inline void cplbit(uint8_t* address, uint8_t bit)
{
    *address ^= (1<<bit);
}

inline bool readbit(uint8_t* address, uint8_t bit)
{
    return (*address & (1<<bit));
}

inline void writebit(uint8_t* address, uint8_t bit, bool value)
{
    value ? setbit(address, value) : clrbit(address, value);
}
