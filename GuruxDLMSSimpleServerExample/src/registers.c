#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <stdio.h>

#include "../cJSON/include/cJSON.h"

#include "../include/registers.h"
#include "../../development/include/gxmem.h"
#include "../../development/include/dlmssettings.h"
#include "../../development/include/variant.h"
#include "../../development/include/cosem.h"

// Include the getter for enableGarbageValues
extern bool isGarbageValuesEnabled();

// Constants for garbage value counters
#define MIN_GARBAGE_COUNT 4
#define MAX_GARBAGE_COUNT 7

// Garbage value arrays for each variable
uint16_t voltageL1GarbageValues[] = {500, 999, 1100};
uint16_t voltageL2GarbageValues[] = {600, 888, 1200};
uint16_t voltageL3GarbageValues[] = {550, 1000, 1300};
uint16_t currentL1GarbageValues[] = {50, 99, 110};
uint16_t currentL2GarbageValues[] = {60, 98, 120};
uint16_t currentL3GarbageValues[] = {55, 100, 130};
uint16_t frequencyGarbageValues[] = {0, 999, 110};
float powerFactorL1GarbageValues[] = {-100, 21, 200};
float powerFactorL2GarbageValues[] = {-90, 21, 210};
float powerFactorL3GarbageValues[] = {-80, 21, 220};
uint32_t blockEnergyKWhImportGarbageValues[] = {999999, 1234567, 2147483647};
uint32_t blockEnergyKVAhLagGarbageValues[] = {999999, 2345678, 1987654321};
uint32_t blockEnergyKVAhLeadGarbageValues[] = {888888, 3456789, 1098765432};
uint32_t blockEnergyKVAhImportGarbageValues[] = {777777, 4567890, 987654321};
uint32_t cumulativeEnergyKWhImportGarbageValues[] = {9999999, 9876543, 7654321};
uint32_t cumulativeEnergyKVAhImportGarbageValues[] = {8888888, 8765432, 6543210};

// Define the KIGG register objects globally.
gxRegister voltageL1, voltageL2, voltageL3;
gxRegister currentL1, currentL2, currentL3;
gxRegister frequency;
gxRegister powerFactorL1, powerFactorL2, powerFactorL3;
gxRegister blockEnergyKWhImport, blockEnergyKVAhLag, blockEnergyKVAhLead, blockEnergyKVAhImport;
gxRegister cumulativeEnergyKWhImport, cumulativeEnergyKVAhImport;

// Define variables to store the KIGG registers' values
static uint16_t voltageL1Value = 0, voltageL2Value = 0, voltageL3Value = 0;
static uint16_t currentL1Value = 0, currentL2Value = 0, currentL3Value = 0;
static uint16_t frequencyValue = 0;
static float powerFactorL1Value = 0.0, powerFactorL2Value = 0.0, powerFactorL3Value = 0.0;
static uint32_t blockEnergyKWhImportValue = 0, blockEnergyKVAhLagValue = 0, blockEnergyKVAhLeadValue = 0, blockEnergyKVAhImportValue = 0;
static uint32_t cumulativeEnergyKWhImportValue = 0, cumulativeEnergyKVAhImportValue = 0;

// Garbage counters for each variable
int voltageL1Counter = 0, voltageL2Counter = 0, voltageL3Counter = 0;
int currentL1Counter = 0, currentL2Counter = 0, currentL3Counter = 0;
int frequencyCounter = 0;
int powerFactorL1Counter = 0, powerFactorL2Counter = 0, powerFactorL3Counter = 0;
int blockEnergyKWhImportCounter = 0, blockEnergyKVAhLagCounter = 0, blockEnergyKVAhLeadCounter = 0, blockEnergyKVAhImportCounter = 0;
int cumulativeEnergyKWhImportCounter = 0, cumulativeEnergyKVAhImportCounter = 0;

// Helper to reset a counter
int resetCounter()
{
    return MIN_GARBAGE_COUNT + rand() % (MAX_GARBAGE_COUNT - MIN_GARBAGE_COUNT + 1);
}

// Function to add the voltageL1 register to the DLMS server
int addVoltageL1()
{
    int ret;
    const unsigned char ln[6] = { 1, 0, 32, 7, 0, 255 };  // Example LN for voltageL1

    // Initialize the voltageL1 register object
    if ((ret = INIT_OBJECT(voltageL1, DLMS_OBJECT_TYPE_REGISTER, ln)) == 0)
    {
        voltageL1Value = 115;
        GX_UINT16_BYREF(voltageL1.value, voltageL1Value);
        // Set additional properties  
        voltageL1.scaler = 3;
        voltageL1.unit = 35;   
    }
    
    return ret;
}

// Function to set the voltageL1 registers' value
void writeVoltageL1Value(uint16_t value) 
{
    voltageL1Value = value;
}

// Function to get the voltageL1 registers' value with garbage value injection
uint16_t readVoltageL1Value()
{
    if (isGarbageValuesEnabled())
    {
        if (voltageL1Counter == 0)
        {
            // Select a random garbage value
            printf("Meter sending garbage value for voltage L1.\n");
            voltageL1Value = voltageL1GarbageValues[rand() % (sizeof(voltageL1GarbageValues) / sizeof(voltageL1GarbageValues[0]))];
            // Reset the counter
            voltageL1Counter = resetCounter();
        }
        else
        {
            voltageL1Value = 115 + (rand() % 11 - 5); // Normal value
            voltageL1Counter--;
        }
    }
    else
    {
        voltageL1Value = 115 + (rand() % 11 - 5); // Normal value
    }
    return voltageL1Value;
}

// Function to add the voltageL2 register to the DLMS server
int addVoltageL2()
{
    int ret;
    const unsigned char ln[6] = { 1, 0, 52, 7, 0, 255 };  // LN for voltageL2

    if ((ret = INIT_OBJECT(voltageL2, DLMS_OBJECT_TYPE_REGISTER, ln)) == 0)
    {
        voltageL2Value = 112;
        GX_UINT16_BYREF(voltageL2.value, voltageL2Value);
        voltageL2.scaler = -3;
        voltageL2.unit = 35;
    }

    return ret;
}

// Function to set the voltageL2 registers' value
void writeVoltageL2Value(uint16_t value)
{
    voltageL2Value = value;
}

// Function to get the voltageL2 registers' value with garbage value injection
uint16_t readVoltageL2Value()
{
    if (isGarbageValuesEnabled())
    {
        if (voltageL2Counter == 0)
        {
            // Select a random garbage value
            printf("Meter sending garbage value for voltage L2\n");
            voltageL2Value = voltageL2GarbageValues[rand() % (sizeof(voltageL2GarbageValues) / sizeof(voltageL2GarbageValues[0]))];
            // Reset the counter
            voltageL2Counter = resetCounter();
        }
        else
        {
            voltageL2Value = 112 + (rand() % 11 - 5); // Normal value
            voltageL2Counter--;
        }
    }
    else
    {
        voltageL2Value = 112 + (rand() % 11 - 5); // Normal value
    }
    return voltageL2Value;
}

// Function to add the voltageL3 register to the DLMS server
int addVoltageL3()
{
    int ret;
    const unsigned char ln[6] = { 1, 0, 72, 7, 0, 255 };  // LN for voltageL3

    if ((ret = INIT_OBJECT(voltageL3, DLMS_OBJECT_TYPE_REGISTER, ln)) == 0)
    {
        voltageL3Value = 117;
        GX_UINT16_BYREF(voltageL3.value, voltageL3Value);
        voltageL3.scaler = 0;
        voltageL3.unit = 35;
    }

    return ret;
}

// Function to set the voltageL3 registers' value
void writeVoltageL3Value(uint16_t value)
{
    voltageL3Value = value;
}

// Function to get the voltageL3 registers' value with garbage value injection
uint16_t readVoltageL3Value()
{
    if (isGarbageValuesEnabled())
    {
        if (voltageL3Counter == 0)
        {
            // Select a random garbage value
            printf("Meter sending garbage value for voltage L3\n");
            voltageL3Value = voltageL3GarbageValues[rand() % (sizeof(voltageL3GarbageValues) / sizeof(voltageL3GarbageValues[0]))];
            // Reset the counter
            voltageL3Counter = resetCounter();
        }
        else
        {
            voltageL3Value = 117 + (rand() % 11 - 5); // Normal value
            voltageL3Counter--;
        }
    }
    else
    {
        voltageL3Value = 117 + (rand() % 11 - 5); // Normal value
    }
    return voltageL3Value;
}

// Function to add the currentL1 register to the DLMS server
int addCurrentL1()
{
    int ret;
    const unsigned char ln[6] = { 1, 0, 31, 7, 0, 255 };  // LN for currentL1

    if ((ret = INIT_OBJECT(currentL1, DLMS_OBJECT_TYPE_REGISTER, ln)) == 0)
    {
        currentL1Value = 10;
        GX_UINT16_BYREF(currentL1.value, currentL1Value);
        currentL1.scaler = 3;
        currentL1.unit = 33;
    }

    return ret;
}

// Function to set the currentL1 registers' value
void writeCurrentL1Value(uint16_t value)
{
    currentL1Value = value;
}

// Function to get the currentL1 registers' value with garbage value injection
uint16_t readCurrentL1Value()
{
    if (isGarbageValuesEnabled())
    {
        if (currentL1Counter == 0)
        {
            // Select a random garbage value
            printf("Meter sending garbage value for current L1\n");
            currentL1Value = currentL1GarbageValues[rand() % (sizeof(currentL1GarbageValues) / sizeof(currentL1GarbageValues[0]))];
            // Reset the counter
            currentL1Counter = resetCounter();
        }
        else
        {
            currentL1Value = 10 + (rand() % 3 - 1); // Normal value
            currentL1Counter--;
        }
    }
    else
    {
        currentL1Value = 10 + (rand() % 3 - 1); // Normal value
    }
    return currentL1Value;
}

// Function to add the currentL2 register to the DLMS server
int addCurrentL2()
{
    int ret;
    const unsigned char ln[6] = { 1, 0, 51, 7, 0, 255 };  // LN for currentL2

    if ((ret = INIT_OBJECT(currentL2, DLMS_OBJECT_TYPE_REGISTER, ln)) == 0)
    {
        currentL2Value = 10;
        GX_UINT16_BYREF(currentL2.value, currentL2Value);
        currentL2.scaler = 3;
        currentL2.unit = 33;
    }

    return ret;
}

// Function to set the currentL2 registers' value
void writeCurrentL2Value(uint16_t value)
{
    currentL2Value = value;
}

// Function to get the currentL2 registers' value with garbage value injection
uint16_t readCurrentL2Value()
{
    if (isGarbageValuesEnabled())
    {
        if (currentL2Counter == 0)
        {
            // Select a random garbage value
            printf("Meter sending garbage value for current L2\n");
            currentL2Value = currentL2GarbageValues[rand() % (sizeof(currentL2GarbageValues) / sizeof(currentL2GarbageValues[0]))];
            // Reset the counter
            currentL2Counter = resetCounter();
        }
        else
        {
            currentL2Value = 10 + (rand() % 3 - 1); // Normal value
            currentL2Counter--;
        }
    }
    else
    {
        currentL2Value = 10 + (rand() % 3 - 1); // Normal value
    }
    return currentL2Value;
}

// Function to add the currentL3 register to the DLMS server
int addCurrentL3()
{
    int ret;
    const unsigned char ln[6] = { 1, 0, 71, 7, 0, 255 };  // LN for currentL3

    if ((ret = INIT_OBJECT(currentL3, DLMS_OBJECT_TYPE_REGISTER, ln)) == 0)
    {
        currentL3Value = 10;
        GX_UINT16_BYREF(currentL3.value, currentL3Value);
        currentL3.scaler = 3;
        currentL3.unit = 33;
    }

    return ret;
}

// Function to set the currentL3 registers' value
void writeCurrentL3Value(uint16_t value)
{
    currentL3Value = value;
}

// Function to get the currentL3 registers' value with garbage value injection
uint16_t readCurrentL3Value()
{
    if (isGarbageValuesEnabled())
    {
        if (currentL3Counter == 0)
        {
            // Select a random garbage value
            printf("Meter sending garbage value for current L3\n");
            currentL3Value = currentL3GarbageValues[rand() % (sizeof(currentL3GarbageValues) / sizeof(currentL3GarbageValues[0]))];
            // Reset the counter
            currentL3Counter = resetCounter();
        }
        else
        {
            currentL3Value = 10 + (rand() % 3 - 1); // Normal value
            currentL3Counter--;
        }
    }
    else
    {
        currentL3Value = 10 + (rand() % 3 - 1); // Normal value
    }
    return currentL3Value;
}

// Function to add the frequency register to the DLMS server
int addFrequency()
{
    int ret;
    const unsigned char ln[6] = { 1, 0, 14, 7, 0, 255 };  // LN for frequency

    if ((ret = INIT_OBJECT(frequency, DLMS_OBJECT_TYPE_REGISTER, ln)) == 0)
    {
        frequencyValue = 50;
        GX_UINT16_BYREF(frequency.value, frequencyValue);
        frequency.scaler = 3;
        frequency.unit = 44;
    }

    return ret;
}

// Function to set the frequency registers' value
void writeFrequencyValue(uint16_t value)
{
    frequencyValue = value;
}

// Function to get the frequency registers' value with garbage value injection
uint16_t readFrequencyValue()
{
    if (isGarbageValuesEnabled())
    {
        if (frequencyCounter == 0)
        {
            // Select a random garbage value
            printf("Meter sending garbage value for frequency\n");
            frequencyValue = frequencyGarbageValues[rand() % (sizeof(frequencyGarbageValues) / sizeof(frequencyGarbageValues[0]))];
            // Reset the counter
            frequencyCounter = resetCounter();
        }
        else
        {
            frequencyValue = 50 + (rand() % 3 - 1); // Normal value
            frequencyCounter--;
        }
    }
    else
    {
        frequencyValue = 50 + (rand() % 3 - 1); // Normal value
    }
    return frequencyValue;
}

// Function to add the powerFactorL1 register to the DLMS server
int addPowerFactorL1()
{
    int ret;
    const unsigned char ln[6] = { 1, 0, 33, 7, 0, 255 };  // LN for powerFactorL1

    if ((ret = INIT_OBJECT(powerFactorL1, DLMS_OBJECT_TYPE_REGISTER, ln)) == 0)
    {
        powerFactorL1Value = 1;
        GX_FLOAT_BYREF(powerFactorL1.value, powerFactorL1Value);
        powerFactorL1.scaler = 0;
        powerFactorL1.unit = 27;  
    }

    return ret;
}

// Function to set the powerFactorL1 registers' value
void writePowerFactorL1Value(int16_t value)
{
    powerFactorL1Value = value;
}

// Function to get the powerFactorL1 registers' value with garbage value injection
float readPowerFactorL1Value()
{
    if (isGarbageValuesEnabled())
    {
        if (powerFactorL1Counter == 0)
        {
            // Select a random garbage value
            printf("Meter sending garbage value for power factor L1\n");
            powerFactorL1Value = powerFactorL1GarbageValues[rand() % (sizeof(powerFactorL1GarbageValues) / sizeof(powerFactorL1GarbageValues[0]))];
            // Reset the counter
            powerFactorL1Counter = resetCounter();
        }
        else
        {
            powerFactorL1Value = 0.8f + ((float)rand() / RAND_MAX) * 0.19f; // Normal value
            powerFactorL1Counter--;
        }
    }
    else
    {
        powerFactorL1Value = 0.8f + ((float)rand() / RAND_MAX) * 0.19f; // Normal value
    }
    return powerFactorL1Value;
}

// Function to add Power Factor L2 register to the DLMS server
int addPowerFactorL2()
{
    int ret;
    const unsigned char ln[6] = { 1, 0, 53, 7, 0, 255 };  // LN for powerFactorL2

    if ((ret = INIT_OBJECT(powerFactorL2, DLMS_OBJECT_TYPE_REGISTER, ln)) == 0)
    {
        powerFactorL2Value = 1; 
        GX_FLOAT_BYREF(powerFactorL2.value, powerFactorL2Value);
        powerFactorL2.scaler = 0;
        powerFactorL2.unit = 40;  
    }

    return ret;
}

// Function to set Power Factor L2 register's value
void writePowerFactorL2Value(int16_t value)
{
    powerFactorL2Value = value;
}

// Function to get the powerFactorL2 registers' value with garbage value injection
float readPowerFactorL2Value()
{
    if (isGarbageValuesEnabled())
    {
        if (powerFactorL2Counter == 0)
        {
            // Select a random garbage value
            printf("Meter sending garbage value for power factor L2\n");
            powerFactorL2Value = powerFactorL2GarbageValues[rand() % (sizeof(powerFactorL2GarbageValues) / sizeof(powerFactorL2GarbageValues[0]))];
            // Reset the counter
            powerFactorL2Counter = resetCounter();
        }
        else
        {
            powerFactorL2Value = 0.8f + ((float)rand() / RAND_MAX) * 0.19f; // Normal value
            powerFactorL2Counter--;
        }
    }
    else
    {
        powerFactorL2Value = 0.8f + ((float)rand() / RAND_MAX) * 0.19f; // Normal value
    }
    return powerFactorL2Value;
}

// Function to add Power Factor L3 register to the DLMS server
int addPowerFactorL3()
{
    int ret;
    const unsigned char ln[6] = { 1, 0, 73, 7, 0, 255 };  // LN for powerFactorL3

    if ((ret = INIT_OBJECT(powerFactorL3, DLMS_OBJECT_TYPE_REGISTER, ln)) == 0)
    {
        powerFactorL3Value = 1; 
        GX_FLOAT_BYREF(powerFactorL3.value, powerFactorL3Value);
        powerFactorL3.scaler = 0;
        powerFactorL3.unit = 40;  
    }

    return ret;
}

// Function to set Power Factor L3 register's value
void writePowerFactorL3Value(int16_t value)
{
    powerFactorL3Value = value;
}

// Function to get the powerFactorL3 registers' value with garbage value injection
float readPowerFactorL3Value()
{
    if (isGarbageValuesEnabled())
    {
        if (powerFactorL3Counter == 0)
        {
            // Select a random garbage value
            printf("Meter sending garbage value for power factor L3\n");
            powerFactorL3Value = powerFactorL3GarbageValues[rand() % (sizeof(powerFactorL3GarbageValues) / sizeof(powerFactorL3GarbageValues[0]))];
            // Reset the counter
            powerFactorL3Counter = resetCounter();
        }
        else
        {
            powerFactorL3Value = 0.8f + ((float)rand() / RAND_MAX) * 0.19f; // Normal value
            powerFactorL3Counter--;
        }
    }
    else
    {
        powerFactorL3Value = 0.8f + ((float)rand() / RAND_MAX) * 0.19f; // Normal value
    }
    return powerFactorL3Value;
}

// Function to add Block Energy (kWh Import) register to the DLMS server
int addBlockEnergyKWhImport()
{
    int ret;
    const unsigned char ln[6] = { 1, 0, 1, 29, 0, 255 };  // LN for Block Energy KWh Import

    if ((ret = INIT_OBJECT(blockEnergyKWhImport, DLMS_OBJECT_TYPE_REGISTER, ln)) == 0)
    {
        blockEnergyKWhImportValue = 1000; 
        GX_UINT32_BYREF(blockEnergyKWhImport.value, blockEnergyKWhImportValue);
        blockEnergyKWhImport.scaler = 3;
        blockEnergyKWhImport.unit = 30;  
    }

    return ret;
}

// Function to set Block Energy (kWh Import) register's value
void writeBlockEnergyKWhImportValue(uint32_t value)
{
    blockEnergyKWhImportValue = value;
}

// Function to get Block Energy (kWh Import) register's value with garbage value injection
uint32_t readBlockEnergyKWhImportValue()
{
    if (isGarbageValuesEnabled())
    {
        if (blockEnergyKWhImportCounter == 0)
        {
            // Select a random garbage value
            printf("Meter sending garbage value for block energy kWh import\n");
            blockEnergyKWhImportValue = blockEnergyKWhImportGarbageValues[rand() % (sizeof(blockEnergyKWhImportGarbageValues) / sizeof(blockEnergyKWhImportGarbageValues[0]))];
            // Reset the counter
            blockEnergyKWhImportCounter = resetCounter();
        }
        else
        {
            blockEnergyKWhImportValue = 1000 + (rand() % 3 - 1); // Normal value
            blockEnergyKWhImportCounter--;
        }
    }
    else
    {
        blockEnergyKWhImportValue = 1000 + (rand() % 3 - 1); // Normal value
    }
    return blockEnergyKWhImportValue;
}

// Function to add Block Energy (kVAh Lag) register to the DLMS server
int addBlockEnergyKVAhLag()
{
    int ret;
    const unsigned char ln[6] = { 1, 0, 5, 29, 0, 255 };  // LN for Block Energy KVAh Lag

    if ((ret = INIT_OBJECT(blockEnergyKVAhLag, DLMS_OBJECT_TYPE_REGISTER, ln)) == 0)
    {
        blockEnergyKVAhLagValue = 1000; 
        GX_UINT32_BYREF(blockEnergyKVAhLag.value, blockEnergyKVAhLagValue);
        blockEnergyKVAhLag.scaler = 3;
        blockEnergyKVAhLag.unit = 30;  
    }

    return ret;
}

// Function to set Block Energy (kVAh Lag) register's value
void writeBlockEnergyKVAhLagValue(uint32_t value)
{
    blockEnergyKVAhLagValue = value;
}

// Function to get Block Energy (kVAh Lag) register's value with garbage value injection
uint32_t readBlockEnergyKVAhLagValue()
{
    if (isGarbageValuesEnabled())
    {
        if (blockEnergyKVAhLagCounter == 0)
        {
            // Select a random garbage value
            printf("Meter sending garbage value for block energy kVAh lag\n");
            blockEnergyKVAhLagValue = blockEnergyKVAhLagGarbageValues[rand() % (sizeof(blockEnergyKVAhLagGarbageValues) / sizeof(blockEnergyKVAhLagGarbageValues[0]))];
            // Reset the counter
            blockEnergyKVAhLagCounter = resetCounter();
        }
        else
        {
            blockEnergyKVAhLagValue = 1000 + (rand() % 3 - 1); // Normal value
            blockEnergyKVAhLagCounter--;
        }
    }
    else
    {
        blockEnergyKVAhLagValue = 1000 + (rand() % 3 - 1); // Normal value
    }
    return blockEnergyKVAhLagValue;
}

// Function to add Block Energy (kVAh Lead) register to the DLMS server
int addBlockEnergyKVAhLead()
{
    int ret;
    const unsigned char ln[6] = { 1, 0, 8, 29, 0, 255 };  // LN for Block Energy KVAh Lead

    if ((ret = INIT_OBJECT(blockEnergyKVAhLead, DLMS_OBJECT_TYPE_REGISTER, ln)) == 0)
    {
        blockEnergyKVAhLeadValue = 1000; 
        GX_UINT32_BYREF(blockEnergyKVAhLead.value, blockEnergyKVAhLeadValue);
        blockEnergyKVAhLead.scaler = 3;
        blockEnergyKVAhLead.unit = 30;  
    }

    return ret;
}

// Function to set Block Energy (kVAh Lead) register's value
void writeBlockEnergyKVAhLeadValue(uint32_t value)
{
    blockEnergyKVAhLeadValue = value;
}

// Function to get Block Energy (kVAh Lead) register's value with garbage value injection
uint32_t readBlockEnergyKVAhLeadValue()
{
    if (isGarbageValuesEnabled())
    {
        if (blockEnergyKVAhLeadCounter == 0)
        {
            // Select a random garbage value
            printf("Meter sending garbage value for block energy kVAh lead\n");
            blockEnergyKVAhLeadValue = blockEnergyKVAhLeadGarbageValues[rand() % (sizeof(blockEnergyKVAhLeadGarbageValues) / sizeof(blockEnergyKVAhLeadGarbageValues[0]))];
            // Reset the counter
            blockEnergyKVAhLeadCounter = resetCounter();
        }
        else
        {
            blockEnergyKVAhLeadValue = 1000 + (rand() % 3 - 1); // Normal value
            blockEnergyKVAhLeadCounter--;
        }
    }
    else
    {
        blockEnergyKVAhLeadValue = 1000 + (rand() % 3 - 1); // Normal value
    }
    return blockEnergyKVAhLeadValue;
}

// Function to add Block Energy (kVAh Import) register to the DLMS server
int addBlockEnergyKVAhImport()
{
    int ret;
    const unsigned char ln[6] = { 1, 0, 9, 29, 0, 255 };  // LN for Block Energy KVAh Import

    if ((ret = INIT_OBJECT(blockEnergyKVAhImport, DLMS_OBJECT_TYPE_REGISTER, ln)) == 0)
    {
        blockEnergyKVAhImportValue = 1000; 
        GX_UINT32_BYREF(blockEnergyKVAhImport.value, blockEnergyKVAhImportValue);
        blockEnergyKVAhImport.scaler = 3;
        blockEnergyKVAhImport.unit = 30;  
    }

    return ret;
}

// Function to set Block Energy (kVAh Import) register's value
void writeBlockEnergyKVAhImportValue(uint32_t value)
{
    blockEnergyKVAhImportValue = value;
}

// Function to get Block Energy (kVAh Import) register's value with garbage value injection
uint32_t readBlockEnergyKVAhImportValue()
{
    if (isGarbageValuesEnabled())
    {
        if (blockEnergyKVAhImportCounter == 0)
        {
            // Select a random garbage value
            printf("Meter sending garbage value for block energy kVAh import\n");
            blockEnergyKVAhImportValue = blockEnergyKVAhImportGarbageValues[rand() % (sizeof(blockEnergyKVAhImportGarbageValues) / sizeof(blockEnergyKVAhImportGarbageValues[0]))];
            // Reset the counter
            blockEnergyKVAhImportCounter = resetCounter();
        }
        else
        {
            blockEnergyKVAhImportValue = 1000 + (rand() % 3 - 1); // Normal value
            blockEnergyKVAhImportCounter--;
        }
    }
    else
    {
        blockEnergyKVAhImportValue = 1000 + (rand() % 3 - 1); // Normal value
    }
    return blockEnergyKVAhImportValue;
}

// Function to add Cumulative Energy (kWh Import) register to the DLMS server
int addCumulativeEnergyKWhImport()
{
    int ret;
    const unsigned char ln[6] = { 1, 0, 1, 8, 0, 255 };  // LN for Cumulative Energy KWh Import

    if ((ret = INIT_OBJECT(cumulativeEnergyKWhImport, DLMS_OBJECT_TYPE_REGISTER, ln)) == 0)
    {
        cumulativeEnergyKWhImportValue = 1000; 
        GX_UINT32_BYREF(cumulativeEnergyKWhImport.value, cumulativeEnergyKWhImportValue);
        cumulativeEnergyKWhImport.scaler = 3;
        cumulativeEnergyKWhImport.unit = 30;  
    }

    return ret;
}

// Function to set Cumulative Energy (kWh Import) register's value
void writeCumulativeEnergyKWhImportValue(uint32_t value)
{
    cumulativeEnergyKWhImportValue = value;
}

// Function to get Cumulative Energy (kWh Import) register's value with garbage value injection
uint32_t readCumulativeEnergyKWhImportValue()
{
    if (isGarbageValuesEnabled())
    {
        if (cumulativeEnergyKWhImportCounter == 0)
        {
            // Select a random garbage value
            printf("Meter sending garbage value for cumulative energy kWh import\n");
            cumulativeEnergyKWhImportValue = cumulativeEnergyKWhImportGarbageValues[rand() % (sizeof(cumulativeEnergyKWhImportGarbageValues) / sizeof(cumulativeEnergyKWhImportGarbageValues[0]))];
            // Reset the counter
            cumulativeEnergyKWhImportCounter = resetCounter();
        }
        else
        {
            cumulativeEnergyKWhImportValue = 1000 + (rand() % 3 - 1); // Normal value
            cumulativeEnergyKWhImportCounter--;
        }
    }
    else
    {
        cumulativeEnergyKWhImportValue = 1000 + (rand() % 3 - 1); // Normal value
    }
    return cumulativeEnergyKWhImportValue;
}

// Function to add Cumulative Energy (kVAh Import) register to the DLMS server
int addCumulativeEnergyKVAhImport()
{
    int ret;
    const unsigned char ln[6] = { 1, 0, 2, 8, 0, 255 };  // LN for Cumulative Energy KVAh Import

    if ((ret = INIT_OBJECT(cumulativeEnergyKVAhImport, DLMS_OBJECT_TYPE_REGISTER, ln)) == 0)
    {
        cumulativeEnergyKVAhImportValue = 1000; 
        GX_UINT32_BYREF(cumulativeEnergyKVAhImport.value, cumulativeEnergyKVAhImportValue);
        cumulativeEnergyKVAhImport.scaler = 3;
        cumulativeEnergyKVAhImport.unit = 30;  
    }

    return ret;
}

// Function to set Cumulative Energy (kVAh Import) register's value
void writeCumulativeEnergyKVAhImportValue(uint32_t value)
{
    cumulativeEnergyKVAhImportValue = value;
}

// Function to get Cumulative Energy (kVAh Import) register's value with garbage value injection
uint32_t readCumulativeEnergyKVAhImportValue()
{
    if (isGarbageValuesEnabled())
    {
        if (cumulativeEnergyKVAhImportCounter == 0)
        {
            // Select a random garbage value
            printf("Meter sending garbage value for cumulative energy kVAh import\n");
            cumulativeEnergyKVAhImportValue = cumulativeEnergyKVAhImportGarbageValues[rand() % (sizeof(cumulativeEnergyKVAhImportGarbageValues) / sizeof(cumulativeEnergyKVAhImportGarbageValues[0]))];
            // Reset the counter
            cumulativeEnergyKVAhImportCounter = resetCounter();
        }
        else
        {
            cumulativeEnergyKVAhImportValue = 1000 + (rand() % 3 - 1); // Normal value
            cumulativeEnergyKVAhImportCounter--;
        }
    }
    else
    {
        cumulativeEnergyKVAhImportValue = 1000 + (rand() % 3 - 1); // Normal value
    }
    return cumulativeEnergyKVAhImportValue;
}

// Initialize counters
void initializeCounters()
{
    srand(time(NULL));
    voltageL1Counter = resetCounter();
    voltageL2Counter = resetCounter();
    voltageL3Counter = resetCounter();
    currentL1Counter = resetCounter();
    currentL2Counter = resetCounter();
    currentL3Counter = resetCounter();
    frequencyCounter = resetCounter();
    powerFactorL1Counter = resetCounter();
    powerFactorL2Counter = resetCounter();
    powerFactorL3Counter = resetCounter();
    blockEnergyKWhImportCounter = resetCounter();
    blockEnergyKVAhLagCounter = resetCounter();
    blockEnergyKVAhLeadCounter = resetCounter();
    blockEnergyKVAhImportCounter = resetCounter();
    cumulativeEnergyKWhImportCounter = resetCounter();
    cumulativeEnergyKVAhImportCounter = resetCounter();
}

// Initialize limits
bool initializeLimits(const char* filePath)
{
    // Open the file for reading
    FILE* file = fopen(filePath, "r");
    if (file == NULL)
    {
        printf("Failed to open configuration file: %s\n", filePath);
        perror("Error");
        return false;
    }

    // Determine file size
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);

    // Allocate memory for the file contents
    char* buffer = (char*)malloc(fileSize + 1);
    if (buffer == NULL)
    {
        printf("Failed to allocate memory for configuration file.\n");
        fclose(file);
        return false;
    }

    // Read the file into the buffer
    fread(buffer, 1, fileSize, file);
    buffer[fileSize] = '\0'; // Null-terminate the string
    fclose(file);

    // Parse the JSON contents
    cJSON* json = cJSON_Parse(buffer);
    free(buffer); // Free the buffer as it's no longer needed

    if (json == NULL)
    {
        printf("Failed to parse JSON from the configuration file.\n");
        const char* errorPtr = cJSON_GetErrorPtr();
        if (errorPtr != NULL)
        {
            printf("Error before: %s\n", errorPtr);
        }
        return false;
    }

    // Print the JSON in a pretty format
    char* jsonString = cJSON_Print(json);
    if (jsonString == NULL)
    {
        printf("Failed to print JSON content.\n");
    }
    else
    {
        printf("Contents of the configuration file:\n%s\n", jsonString);
        free(jsonString); // Free the printed JSON string
    }

    // Cleanup the JSON object
    cJSON_Delete(json);
    return true;
}
