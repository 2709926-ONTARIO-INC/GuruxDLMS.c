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
#define MIN_GARBAGE_COUNT   4
#define MAX_GARBAGE_COUNT   7
#define SIZE                50

// Garbage value arrays for each variable
uint16_t voltageL1GarbageValues[] = {500000, 999000, 1100000};
uint16_t voltageL2GarbageValues[] = {600000, 888000, 1200000};
uint16_t voltageL3GarbageValues[] = {550000, 100000, 1300000};
uint16_t currentL1GarbageValues[] = {500000, 990000, 1100000};
uint16_t currentL2GarbageValues[] = {600000, 980000, 1200000};
uint16_t currentL3GarbageValues[] = {550000, 100000, 1300000};
uint16_t frequencyGarbageValues[] = {555000, 999000, 1100000};
float powerFactorL1GarbageValues[] = {800000.0, 210000.0, 200000.0};
float powerFactorL2GarbageValues[] = {900000.0, 210000.0, 210000.0};
float powerFactorL3GarbageValues[] = {800000.0, 210000.0, 220000.0};
uint32_t blockEnergyKWhImportGarbageValues[] = {99999, 12345, 214745};
uint32_t blockEnergyKVAhLagGarbageValues[] = {99995, 23455, 198765};
uint32_t blockEnergyKVAhLeadGarbageValues[] = {88885, 34565, 109875};
uint32_t blockEnergyKVAhImportGarbageValues[] = {77775, 45675, 987655};
uint32_t cumulativeEnergyKWhImportGarbageValues[] = {99995, 98765, 765435};
uint32_t cumulativeEnergyKVAhImportGarbageValues[] = {88885, 87655, 6543265};

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

// Define variables for upper and lower limits
static uint16_t voltageL1ValueMin = 50000, voltageL1ValueMax = 60000;
static uint16_t voltageL2ValueMin = 50000, voltageL2ValueMax = 60000;
static uint16_t voltageL3ValueMin = 50000, voltageL3ValueMax = 60000;

static uint16_t currentL1ValueMin = 1, currentL1ValueMax = 10000;
static uint16_t currentL2ValueMin = 1, currentL2ValueMax = 10000;
static uint16_t currentL3ValueMin = 1, currentL3ValueMax = 10000;

static uint16_t frequencyValueMin = 45000, frequencyValueMax = 55000;

static float powerFactorL1ValueMin = 800.0, powerFactorL1ValueMax = 1000.0;
static float powerFactorL2ValueMin = 800.0, powerFactorL2ValueMax = 1000.0;
static float powerFactorL3ValueMin = 800.0, powerFactorL3ValueMax = 1000.0;

static uint32_t blockEnergyKWhImportValueMin = 100, blockEnergyKWhImportValueMax = 1000;
static uint32_t blockEnergyKVAhLagValueMin = 100, blockEnergyKVAhLagValueMax = 1000;
static uint32_t blockEnergyKVAhLeadValueMin = 100, blockEnergyKVAhLeadValueMax = 1000;
static uint32_t blockEnergyKVAhImportValueMin = 100, blockEnergyKVAhImportValueMax = 1000;

static uint32_t cumulativeEnergyKWhImportValueMin = 450, cumulativeEnergyKWhImportValueMax = 500;
static uint32_t cumulativeEnergyKVAhImportValueMin = 500, cumulativeEnergyKVAhImportValueMax = 600;

// Garbage counters for each variable
static int voltageL1Counter = 0, voltageL2Counter = 0, voltageL3Counter = 0;
static int currentL1Counter = 0, currentL2Counter = 0, currentL3Counter = 0;
static int frequencyCounter = 0;
static int powerFactorL1Counter = 0, powerFactorL2Counter = 0, powerFactorL3Counter = 0;
static int blockEnergyKWhImportCounter = 0, blockEnergyKVAhLagCounter = 0, blockEnergyKVAhLeadCounter = 0, blockEnergyKVAhImportCounter = 0;
static int cumulativeEnergyKWhImportCounter = 0, cumulativeEnergyKVAhImportCounter = 0;

static char* current_timestamp = NULL;

// Helper to reset a counter
int resetCounter()
{
    return MIN_GARBAGE_COUNT + rand() % (MAX_GARBAGE_COUNT - MIN_GARBAGE_COUNT + 1);
}

// Function to get the current formatted timestamp
char* getFormattedTimestamp()
{
    static char formattedTime[SIZE + 20]; // Allocate enough space for prepending "Timestamp (GMT): "
    time_t t;
    struct tm *tmp;
    char MY_TIME[SIZE];

    // Get the current time
    time(&t);
    // Convert to local time
    tmp = localtime(&t);
    // Format the date and time
    strftime(MY_TIME, sizeof(MY_TIME), "%x - %I:%M%p", tmp);
    // Prepend "Timestamp (GMT): " to the formatted time
    snprintf(formattedTime, sizeof(formattedTime), "Timestamp (GMT): %s", MY_TIME);

    return formattedTime;
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
        voltageL1.scaler = -3;
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
            voltageL1Value = voltageL1GarbageValues[rand() % (sizeof(voltageL1GarbageValues) / sizeof(voltageL1GarbageValues[0]))];
            current_timestamp = getFormattedTimestamp();
            printf("%s -> Meter sending garbage value %u for voltage L1.\n", current_timestamp, voltageL1Value);
            // Reset the counter
            voltageL1Counter = resetCounter();
        }
        else
        {
            voltageL1Value = voltageL1ValueMin + rand() % (voltageL1ValueMax - voltageL1ValueMin + 1); // Normal value
            voltageL1Counter--;
        }
    }
    else
    {
        voltageL1Value = voltageL1ValueMin + rand() % (voltageL1ValueMax - voltageL1ValueMin + 1); // Normal value
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
            voltageL2Value = voltageL2GarbageValues[rand() % (sizeof(voltageL2GarbageValues) / sizeof(voltageL2GarbageValues[0]))];
            current_timestamp = getFormattedTimestamp();
            printf("%s -> Meter sending garbage value %u for voltage L2.\n", current_timestamp, voltageL2Value);
            // Reset the counter
            voltageL2Counter = resetCounter();
        }
        else
        {
            voltageL2Value = voltageL2ValueMin + rand() % (voltageL2ValueMax - voltageL2ValueMin + 1); // Normal value
            voltageL2Counter--;
        }
    }
    else
    {
        voltageL2Value = voltageL2ValueMin + rand() % (voltageL2ValueMax - voltageL2ValueMin + 1); // Normal value
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
        voltageL3.scaler = -3;
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
            voltageL3Value = voltageL3GarbageValues[rand() % (sizeof(voltageL3GarbageValues) / sizeof(voltageL3GarbageValues[0]))];
            current_timestamp = getFormattedTimestamp();
            printf("%s -> Meter sending garbage value %u for voltage L3.\n", current_timestamp, voltageL3Value);
            // Reset the counter
            voltageL3Counter = resetCounter();
        }
        else
        {
            voltageL3Value = voltageL3ValueMin + rand() % (voltageL3ValueMax - voltageL3ValueMin + 1); // Normal value
            voltageL3Counter--;
        }
    }
    else
    {
        voltageL3Value = voltageL3ValueMin + rand() % (voltageL3ValueMax - voltageL3ValueMin + 1); // Normal value
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
        currentL1.scaler = -5;
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
            currentL1Value = currentL1GarbageValues[rand() % (sizeof(currentL1GarbageValues) / sizeof(currentL1GarbageValues[0]))];
            current_timestamp = getFormattedTimestamp();
            printf("%s -> Meter sending garbage value %u for current L1.\n", current_timestamp, currentL1Value);
            // Reset the counter
            currentL1Counter = resetCounter();
        }
        else
        {
            currentL1Value = currentL1ValueMin + rand() % (currentL1ValueMax - currentL1ValueMin + 1); // Normal value
            currentL1Counter--;
        }
    }
    else
    {
        currentL1Value = currentL1ValueMin + rand() % (currentL1ValueMax - currentL1ValueMin + 1); // Normal value
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
        currentL2.scaler = -5;
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
            currentL2Value = currentL2GarbageValues[rand() % (sizeof(currentL2GarbageValues) / sizeof(currentL2GarbageValues[0]))];
            current_timestamp = getFormattedTimestamp();
            printf("%s -> Meter sending garbage value %u for current L2.\n", current_timestamp, currentL2Value);
            // Reset the counter
            currentL2Counter = resetCounter();
        }
        else
        {
            currentL2Value = currentL2ValueMin + rand() % (currentL2ValueMax - currentL2ValueMin + 1); // Normal value
            currentL2Counter--;
        }
    }
    else
    {
        currentL2Value = currentL2ValueMin + rand() % (currentL2ValueMax - currentL2ValueMin + 1); // Normal value
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
        currentL3.scaler = -5;
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
            currentL3Value = currentL3GarbageValues[rand() % (sizeof(currentL3GarbageValues) / sizeof(currentL3GarbageValues[0]))];
            current_timestamp = getFormattedTimestamp();
            printf("%s -> Meter sending garbage value %u for current L3.\n", current_timestamp, currentL3Value);
            // Reset the counter
            currentL3Counter = resetCounter();
        }
        else
        {
            currentL3Value = currentL3ValueMin + rand() % (currentL3ValueMax - currentL3ValueMin + 1); // Normal value
            currentL3Counter--;
        }
    }
    else
    {
        currentL3Value = currentL3ValueMin + rand() % (currentL3ValueMax - currentL3ValueMin + 1); // Normal value
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
        frequency.scaler = -3;
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
            frequencyValue = frequencyGarbageValues[rand() % (sizeof(frequencyGarbageValues) / sizeof(frequencyGarbageValues[0]))];
            current_timestamp = getFormattedTimestamp();
            printf("%s -> Meter sending garbage value %u for frequency.\n", current_timestamp, frequencyValue);
            // Reset the counter
            frequencyCounter = resetCounter();
        }
        else
        {
            frequencyValue = frequencyValueMin + rand() % (frequencyValueMax - frequencyValueMin + 1); // Normal value
            frequencyCounter--;
        }
    }
    else
    {
        frequencyValue = frequencyValueMin + rand() % (frequencyValueMax - frequencyValueMin + 1); // Normal value
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
        powerFactorL1.scaler = -3;
        powerFactorL1.unit = 0;  
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
            powerFactorL1Value = powerFactorL1GarbageValues[rand() % (sizeof(powerFactorL1GarbageValues) / sizeof(powerFactorL1GarbageValues[0]))];
            current_timestamp = getFormattedTimestamp();
            printf("%s -> Meter sending garbage value %f for power factor L1.\n", current_timestamp, powerFactorL1Value);
            // Reset the counter
            powerFactorL1Counter = resetCounter();
        }
        else
        {
            powerFactorL1Value = powerFactorL1ValueMin + ((float)rand() / RAND_MAX) * (powerFactorL1ValueMax - powerFactorL1ValueMin); // Normal value
            powerFactorL1Counter--;
        }
    }
    else
    {
        powerFactorL1Value = powerFactorL1ValueMin + ((float)rand() / RAND_MAX) * (powerFactorL1ValueMax - powerFactorL1ValueMin); // Normal value // Normal value
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
        powerFactorL2.scaler = -3;
        powerFactorL2.unit = 0;  
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
            powerFactorL2Value = powerFactorL2GarbageValues[rand() % (sizeof(powerFactorL2GarbageValues) / sizeof(powerFactorL2GarbageValues[0]))];
            current_timestamp = getFormattedTimestamp();
            printf("%s -> Meter sending garbage value %f for power factor L2.\n", current_timestamp, powerFactorL2Value);
            // Reset the counter
            powerFactorL2Counter = resetCounter();
        }
        else
        {
            powerFactorL2Value = powerFactorL2ValueMin + ((float)rand() / RAND_MAX) * (powerFactorL2ValueMax - powerFactorL2ValueMin); // Normal value
            powerFactorL2Counter--;
        }
    }
    else
    {
        powerFactorL2Value = powerFactorL2ValueMin + ((float)rand() / RAND_MAX) * (powerFactorL2ValueMax - powerFactorL2ValueMin); // Normal value
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
        powerFactorL3.scaler = -3;
        powerFactorL3.unit = 0;  
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
            powerFactorL3Value = powerFactorL3GarbageValues[rand() % (sizeof(powerFactorL3GarbageValues) / sizeof(powerFactorL3GarbageValues[0]))];
            current_timestamp = getFormattedTimestamp();
            printf("%s -> Meter sending garbage value %f for power factor L3.\n", current_timestamp, powerFactorL3Value);
            // Reset the counter
            powerFactorL3Counter = resetCounter();
        }
        else
        {
            powerFactorL3Value = powerFactorL3ValueMin + ((float)rand() / RAND_MAX) * (powerFactorL3ValueMax - powerFactorL3ValueMin); // Normal value
            powerFactorL3Counter--;
        }
    }
    else
    {
        powerFactorL3Value = powerFactorL3ValueMin + ((float)rand() / RAND_MAX) * (powerFactorL3ValueMax - powerFactorL3ValueMin); // Normal value
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
        blockEnergyKWhImport.scaler = 2;
        blockEnergyKWhImport.unit = 32;  
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
            blockEnergyKWhImportValue = blockEnergyKWhImportGarbageValues[rand() % (sizeof(blockEnergyKWhImportGarbageValues) / sizeof(blockEnergyKWhImportGarbageValues[0]))];
            current_timestamp = getFormattedTimestamp();
            printf("%s -> Meter sending garbage value %u for block energy kWh import.\n", current_timestamp, blockEnergyKWhImportValue);
            // Reset the counter
            blockEnergyKWhImportCounter = resetCounter();
        }
        else
        {
            blockEnergyKWhImportValue = blockEnergyKWhImportValueMin + rand() % (blockEnergyKWhImportValueMax - blockEnergyKWhImportValueMin + 1); // Normal value
            blockEnergyKWhImportCounter--;
        }
    }
    else
    {
        blockEnergyKWhImportValue = blockEnergyKWhImportValueMin + rand() % (blockEnergyKWhImportValueMax - blockEnergyKWhImportValueMin + 1); // Normal value
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
        blockEnergyKVAhLag.scaler = 2;
        blockEnergyKVAhLag.unit = 32;  
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
            blockEnergyKVAhLagValue = blockEnergyKVAhLagGarbageValues[rand() % (sizeof(blockEnergyKVAhLagGarbageValues) / sizeof(blockEnergyKVAhLagGarbageValues[0]))];
            current_timestamp = getFormattedTimestamp();
            printf("%s -> Meter sending garbage value %u for block energy block energy kVAh lag.\n", current_timestamp, blockEnergyKVAhLagValue);
            // Reset the counter
            blockEnergyKVAhLagCounter = resetCounter();
        }
        else
        {
            blockEnergyKVAhLagValue = blockEnergyKVAhLagValueMin + rand() % (blockEnergyKVAhLagValueMax - blockEnergyKVAhLagValueMin + 1); // Normal value
            blockEnergyKVAhLagCounter--;
        }
    }
    else
    {
        blockEnergyKVAhLagValue = blockEnergyKVAhLagValueMin + rand() % (blockEnergyKVAhLagValueMax - blockEnergyKVAhLagValueMin + 1); // Normal value
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
        blockEnergyKVAhLead.scaler = 2;
        blockEnergyKVAhLead.unit = 32;  
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
            blockEnergyKVAhLeadValue = blockEnergyKVAhLeadGarbageValues[rand() % (sizeof(blockEnergyKVAhLeadGarbageValues) / sizeof(blockEnergyKVAhLeadGarbageValues[0]))];
            current_timestamp = getFormattedTimestamp();
            printf("%s -> Meter sending garbage value %u for block energy block energy kVAh lead.\n", current_timestamp, blockEnergyKVAhLeadValue);
            // Reset the counter
            blockEnergyKVAhLeadCounter = resetCounter();
        }
        else
        {
            blockEnergyKVAhLeadValue = blockEnergyKVAhLeadValueMin + rand() % (blockEnergyKVAhLeadValueMax - blockEnergyKVAhLeadValueMin + 1); // Normal value
            blockEnergyKVAhLeadCounter--;
        }
    }
    else
    {
        blockEnergyKVAhLeadValue = blockEnergyKVAhLeadValueMin + rand() % (blockEnergyKVAhLeadValueMax - blockEnergyKVAhLeadValueMin + 1); // Normal value
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
        blockEnergyKVAhImport.scaler = 2;
        blockEnergyKVAhImport.unit = 32;  
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
            blockEnergyKVAhImportValue = blockEnergyKVAhImportGarbageValues[rand() % (sizeof(blockEnergyKVAhImportGarbageValues) / sizeof(blockEnergyKVAhImportGarbageValues[0]))];
            current_timestamp = getFormattedTimestamp();
            printf("%s -> Meter sending garbage value %u for block energy block energy kVAh import.\n", current_timestamp, blockEnergyKVAhImportValue);
            // Reset the counter
            blockEnergyKVAhImportCounter = resetCounter();
        }
        else
        {
            blockEnergyKVAhImportValue = blockEnergyKVAhImportValueMin + rand() % (blockEnergyKVAhImportValueMax - blockEnergyKVAhImportValueMin + 1); // Normal value
            blockEnergyKVAhImportCounter--;
        }
    }
    else
    {
        blockEnergyKVAhImportValue = blockEnergyKVAhImportValueMin + rand() % (blockEnergyKVAhImportValueMax - blockEnergyKVAhImportValueMin + 1); // Normal value
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
        cumulativeEnergyKWhImport.scaler = 2;
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
            cumulativeEnergyKWhImportValue = cumulativeEnergyKWhImportGarbageValues[rand() % (sizeof(cumulativeEnergyKWhImportGarbageValues) / sizeof(cumulativeEnergyKWhImportGarbageValues[0]))];
            current_timestamp = getFormattedTimestamp();
            printf("%s -> Meter sending garbage value %u for cumulative energy kWh import.\n", current_timestamp, cumulativeEnergyKWhImportValue);
            // Reset the counter
            cumulativeEnergyKWhImportCounter = resetCounter();
        }
        else
        {
            cumulativeEnergyKWhImportValue = cumulativeEnergyKWhImportValueMin + rand() % (cumulativeEnergyKWhImportValueMax - cumulativeEnergyKWhImportValueMin + 1); // Normal value
            cumulativeEnergyKWhImportCounter--;
        }
    }
    else
    {
        cumulativeEnergyKWhImportValue = cumulativeEnergyKWhImportValueMin + rand() % (cumulativeEnergyKWhImportValueMax - cumulativeEnergyKWhImportValueMin + 1); // Normal value
    }
    return cumulativeEnergyKWhImportValue;
}

// Function to add Cumulative Energy (kVAh Import) register to the DLMS server
int addCumulativeEnergyKVAhImport()
{
    int ret;
    const unsigned char ln[6] = { 1, 0, 9, 8, 0, 255 };  // LN for Cumulative Energy KVAh Import

    if ((ret = INIT_OBJECT(cumulativeEnergyKVAhImport, DLMS_OBJECT_TYPE_REGISTER, ln)) == 0)
    {
        cumulativeEnergyKVAhImportValue = 1000; 
        GX_UINT32_BYREF(cumulativeEnergyKVAhImport.value, cumulativeEnergyKVAhImportValue);
        cumulativeEnergyKVAhImport.scaler = 2;
        cumulativeEnergyKVAhImport.unit = 31;  
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
            cumulativeEnergyKVAhImportValue = cumulativeEnergyKVAhImportGarbageValues[rand() % (sizeof(cumulativeEnergyKVAhImportGarbageValues) / sizeof(cumulativeEnergyKVAhImportGarbageValues[0]))];
            current_timestamp = getFormattedTimestamp();
            printf("%s -> Meter sending garbage value %u for cumulative energy kVAh import.\n", current_timestamp, cumulativeEnergyKVAhImportValue);
            // Reset the counter
            cumulativeEnergyKVAhImportCounter = resetCounter();
        }
        else
        {
            cumulativeEnergyKVAhImportValue = cumulativeEnergyKVAhImportValueMin + rand() % (cumulativeEnergyKVAhImportValueMax - cumulativeEnergyKVAhImportValueMin + 1); // Normal value
            cumulativeEnergyKVAhImportCounter--;
        }
    }
    else
    {
        cumulativeEnergyKVAhImportValue = cumulativeEnergyKVAhImportValueMin + rand() % (cumulativeEnergyKVAhImportValueMax - cumulativeEnergyKVAhImportValueMin + 1); // Normal value
    }
    return cumulativeEnergyKVAhImportValue;
}

// Initialize counters
void initializeCounters(void)
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
