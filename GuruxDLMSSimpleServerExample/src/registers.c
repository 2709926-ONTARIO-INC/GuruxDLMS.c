#include <stdlib.h>

#include "../include/registers.h"
#include "../../development/include/gxmem.h"
#include "../../development/include/dlmssettings.h"
#include "../../development/include/variant.h"
#include "../../development/include/cosem.h"

// Define the KIGG register objects globally.
gxRegister voltageL1;

// Define variables to store the KIGG registers' values
static uint16_t voltageL1Value = 0;

// Function to add the voltageL1 register to the DLMS server
int addVoltageL1()
{
    int ret;
    const unsigned char ln[6] = { 1, 0, 32, 7, 0, 255 };  // Example LN for voltageL1

    // Initialize the voltageL1 register object
    if ((ret = INIT_OBJECT(voltageL1, DLMS_OBJECT_TYPE_REGISTER, ln)) == 0)
    {
        voltageL1Value = 230 + (rand() % 11 - 5);
        GX_UINT16_BYREF(voltageL1.value, voltageL1Value);
        // Set additional properties  
        voltageL1.unit = 35;   
    }
    
    return ret;
}

void writeVoltageL1Value(uint16_t value) 
{
    voltageL1Value = value;
}

uint16_t readVoltageL1Value()
{
    return voltageL1Value;
}
