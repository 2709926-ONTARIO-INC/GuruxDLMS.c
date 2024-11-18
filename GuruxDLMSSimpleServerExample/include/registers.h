#ifndef REGISTER_H__
#define REGISTER_H__

#include <stdint.h>

// Functions to add the KIGG register
int addVoltageL1(void);

// Functions to get the KIGG registers' values
uint16_t readVoltageL1Value();

// Functions to set the KIGG registers' values
void writeVoltageL1Value(uint16_t value);

#endif
