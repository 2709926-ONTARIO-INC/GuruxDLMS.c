#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

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
uint32_t voltageL1GarbageValues[] = {500000, 999000, 1100000};
uint32_t voltageL2GarbageValues[] = {600000, 888000, 1200000};
uint32_t voltageL3GarbageValues[] = {5500000, 6000000, 70000000};
uint32_t currentL1GarbageValues[] = {5000000, 9900000, 11000000};
uint32_t currentL2GarbageValues[] = {6000000, 9800000, 12000000};
uint32_t currentL3GarbageValues[] = {5500000, 1000000, 13000000};
uint32_t frequencyGarbageValues[] = {555000, 999000, 1100000};
float powerFactorL1GarbageValues[] = {800000.0, 210000.0, 200000.0};
float powerFactorL2GarbageValues[] = {900000.0, 210000.0, 210000.0};
float powerFactorL3GarbageValues[] = {800000.0, 210000.0, 220000.0};
uint32_t blockEnergyKWhImportGarbageValues[] = {99999, 12345, 214745};
uint32_t blockEnergyKVAhLagGarbageValues[] = {99995, 23455, 198765};
uint32_t blockEnergyKVAhLeadGarbageValues[] = {88885, 34565, 109875};
uint32_t blockEnergyKVAhImportGarbageValues[] = {77775, 45675, 987655};
uint32_t cumulativeEnergyKWhImportGarbageValues[] = {90, 98, 76};
uint32_t cumulativeEnergyKVAhImportGarbageValues[] = {88, 87, 65};


// Define the KIGG register objects globally.
gxRegister voltageL1, voltageL2, voltageL3;
gxRegister currentL1, currentL2, currentL3;
gxRegister frequency;
gxRegister powerFactorL1, powerFactorL2, powerFactorL3;
gxRegister blockEnergyKWhImport, blockEnergyKVAhLag, blockEnergyKVAhLead, blockEnergyKVAhImport;
gxRegister cumulativeEnergyKWhImport, cumulativeEnergyKVAhImport;

// Defin the objects to store KIGG register's averages
gxRegister voltageL1Average, voltageL2Average, voltageL3Average;
gxRegister currentL1Average, currentL2Average, currentL3Average;


// Define variables to store the KIGG registers' values
static uint32_t voltageL1Value = 0, voltageL2Value = 0, voltageL3Value = 0;
static uint32_t currentL1Value = 0, currentL2Value = 0, currentL3Value = 0;
static uint32_t frequencyValue = 0;
static float powerFactorL1Value = 0.0, powerFactorL2Value = 0.0, powerFactorL3Value = 0.0;
static uint32_t blockEnergyKWhImportValue = 0, blockEnergyKVAhLagValue = 0, blockEnergyKVAhLeadValue = 0, blockEnergyKVAhImportValue = 0;
static uint32_t cumulativeEnergyKWhImportValue = 0, cumulativeEnergyKVAhImportValue = 0;

// Define variables to store the KIGG registers' averages
uint32_t voltageL1AverageValue = 0, voltageL2AverageValue = 0, voltageL3AverageValue = 0;
uint32_t currentL1AverageValue = 0, currentL2AverageValue = 0, currentL3AverageValue = 0;


// Define variables for upper and lower limits
static uint32_t voltageL1ValueMin = 50000, voltageL1ValueMax = 60000;
static uint32_t voltageL2ValueMin = 50000, voltageL2ValueMax = 60000;
static uint32_t voltageL3ValueMin = 50000, voltageL3ValueMax = 60000;

static uint32_t currentL1ValueMin = 1, currentL1ValueMax = 10000;
static uint32_t currentL2ValueMin = 1, currentL2ValueMax = 10000;
static uint32_t currentL3ValueMin = 1, currentL3ValueMax = 10000;

static uint32_t frequencyValueMin = 45000, frequencyValueMax = 55000;

static float powerFactorL1ValueMin = 800.0, powerFactorL1ValueMax = 1000.0;
static float powerFactorL2ValueMin = 800.0, powerFactorL2ValueMax = 1000.0;
static float powerFactorL3ValueMin = 800.0, powerFactorL3ValueMax = 1000.0;

static uint32_t blockEnergyKWhImportValueMin = 100, blockEnergyKWhImportValueMax = 1000;
static uint32_t blockEnergyKVAhLagValueMin = 100, blockEnergyKVAhLagValueMax = 1000;
static uint32_t blockEnergyKVAhLeadValueMin = 100, blockEnergyKVAhLeadValueMax = 1000;
static uint32_t blockEnergyKVAhImportValueMin = 100, blockEnergyKVAhImportValueMax = 1000;

// static uint32_t cumulativeEnergyKWhImportValueMin = 450, cumulativeEnergyKWhImportValueMax = 500;
// static uint32_t cumulativeEnergyKVAhImportValueMin = 500, cumulativeEnergyKVAhImportValueMax = 600;


// Garbage counters for each variable
static int voltageL1Counter = 0, voltageL2Counter = 0, voltageL3Counter = 0;
static int currentL1Counter = 0, currentL2Counter = 0, currentL3Counter = 0;
static int frequencyCounter = 0;
static int powerFactorL1Counter = 0, powerFactorL2Counter = 0, powerFactorL3Counter = 0;
static int blockEnergyKWhImportCounter = 0, blockEnergyKVAhLagCounter = 0, blockEnergyKVAhLeadCounter = 0, blockEnergyKVAhImportCounter = 0;
static int cumulativeEnergyKWhImportCounter = 0, cumulativeEnergyKVAhImportCounter = 0;

// Average value counters for each variable
static int voltageL1AverageCounter = 0, voltageL2AverageCounter = 0, voltageL3AverageCounter = 0;
static int currentL1AverageCounter = 0, currentL2AverageCounter = 0, currentL3AverageCounter = 0;


static char* current_timestamp = NULL;


// Variables to keep track of if the cumulative energy values are read once
static bool isCumulativeEnergyKWhImportReadOnce = false;
static bool isCumulativeEnergyKVAhImportReadOnce = false;


// Variable to track if garbage values were read once 
static bool isCumulativeEnergyKWhImportGarbageValueSent = false;
static bool isCumulativeEnergyKVAhImportGarbageValueSent = false;


// Variable to store the last cumulative energy values when garbage injection is enabled
static uint32_t lastCumulativeEnergyKWhImportValue = 0;
static uint32_t lastCumulativeEnergyKVAhImportValue = 0;

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

uint32_t calculateAverageUint32(uint32_t average, uint32_t value, int count) 
{
    int new_average = ((average * (count - 1) + value) / (count));
    return new_average;
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
        GX_UINT32_BYREF(voltageL1.value, voltageL1Value);
        // Set additional properties  
        voltageL1.scaler = -3;
        voltageL1.unit = 35;   
    }
    
    return ret;
}

// Function to set the voltageL1 registers' value
void writeVoltageL1Value(uint32_t value) 
{
    voltageL1Value = value;
}

// Function to get the voltageL1 registers' value with garbage value injection
uint32_t readVoltageL1Value()
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
            voltageL1AverageCounter++; // Increment the average counter to calculate new average
            voltageL1AverageValue = calculateAverageUint32(voltageL1AverageValue, voltageL1Value, voltageL1AverageCounter) / 1000;
        }
    }
    else
    {
        voltageL1Value = voltageL1ValueMin + rand() % (voltageL1ValueMax - voltageL1ValueMin + 1); // Normal value
        voltageL1AverageCounter++; // Increment the average counter to calculate new average
        voltageL1AverageValue = calculateAverageUint32(voltageL1AverageValue, voltageL1Value, voltageL1AverageCounter) / 1000;
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
        GX_UINT32_BYREF(voltageL2.value, voltageL2Value);
        voltageL2.scaler = -3;
        voltageL2.unit = 35;
    }

    return ret;
}

// Function to set the voltageL2 registers' value
void writeVoltageL2Value(uint32_t value)
{
    voltageL2Value = value;
}

// Function to get the voltageL2 registers' value with garbage value injection
uint32_t readVoltageL2Value()
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
            voltageL2AverageCounter++; // Increment the average counter to calculate new average
            voltageL2AverageValue = calculateAverageUint32(voltageL2AverageValue, voltageL2Value, voltageL2AverageCounter) / 1000;
        }
    }
    else
    {
        voltageL2Value = voltageL2ValueMin + rand() % (voltageL2ValueMax - voltageL2ValueMin + 1); // Normal value
        voltageL2AverageCounter++; // Increment the average counter to calculate new average
        voltageL2AverageValue = calculateAverageUint32(voltageL2AverageValue, voltageL2Value, voltageL2AverageCounter) / 1000;
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
        GX_UINT32_BYREF(voltageL3.value, voltageL3Value);
        voltageL3.scaler = -3;
        voltageL3.unit = 35;
    }

    return ret;
}

// Function to set the voltageL3 registers' value
void writeVoltageL3Value(uint32_t value)
{
    voltageL3Value = value;
}

// Function to get the voltageL3 registers' value with garbage value injection
uint32_t readVoltageL3Value()
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
            voltageL3AverageCounter++; // Increment the average counter to calculate new average
            voltageL3AverageValue = calculateAverageUint32(voltageL3AverageValue, voltageL3Value, voltageL3AverageCounter) / 1000;
        }
    }
    else
    {
        voltageL3Value = voltageL3ValueMin + rand() % (voltageL3ValueMax - voltageL3ValueMin + 1); // Normal value
        voltageL3AverageCounter++; // Increment the average counter to calculate new average
        voltageL3AverageValue = calculateAverageUint32(voltageL3AverageValue, voltageL3Value, voltageL3AverageCounter)/ 1000;
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
        GX_UINT32_BYREF(currentL1.value, currentL1Value);
        currentL1.scaler = -5;
        currentL1.unit = 33;
    }

    return ret;
}

// Function to set the currentL1 registers' value
void writeCurrentL1Value(uint32_t value)
{
    currentL1Value = value;
}

// Function to get the currentL1 registers' value with garbage value injection
uint32_t readCurrentL1Value()
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
            currentL1AverageCounter++; // Increment the average counter to calculate new average
            currentL1AverageValue = calculateAverageUint32(currentL1AverageValue, currentL1Value, currentL1AverageCounter) / 100000;
        }
    }
    else
    {
        currentL1Value = currentL1ValueMin + rand() % (currentL1ValueMax - currentL1ValueMin + 1); // Normal value
        currentL1AverageCounter++; // Increment the average counter to calculate new average
        currentL1AverageValue = calculateAverageUint32(currentL1AverageValue, currentL1Value, currentL1AverageCounter) / 100000;
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
        GX_UINT32_BYREF(currentL2.value, currentL2Value);
        currentL2.scaler = -5;
        currentL2.unit = 33;
    }

    return ret;
}

// Function to set the currentL2 registers' value
void writeCurrentL2Value(uint32_t value)
{
    currentL2Value = value;
}

// Function to get the currentL2 registers' value with garbage value injection
uint32_t readCurrentL2Value()
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
            currentL2AverageCounter++; // Increment the average counter to calculate new average
            currentL2AverageValue = calculateAverageUint32(currentL2AverageValue, currentL2Value, currentL2AverageCounter) / 100000;
        }
    }
    else
    {
        currentL2Value = currentL2ValueMin + rand() % (currentL2ValueMax - currentL2ValueMin + 1); // Normal value
        currentL2AverageCounter++; // Increment the average counter to calculate new average
        currentL2AverageValue = calculateAverageUint32(currentL2AverageValue, currentL2Value, currentL2AverageCounter) / 100000;
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
        GX_UINT32_BYREF(currentL3.value, currentL3Value);
        currentL3.scaler = -5;
        currentL3.unit = 33;
    }

    return ret;
}

// Function to set the currentL3 registers' value
void writeCurrentL3Value(uint32_t value)
{
    currentL3Value = value;
}

// Function to get the currentL3 registers' value with garbage value injection
uint32_t readCurrentL3Value()
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
            currentL3AverageCounter++; // Increment the average counter to calculate new average
            currentL3AverageValue = calculateAverageUint32(currentL3AverageValue, currentL3Value, currentL3AverageCounter)/ 100000;
        }
    }
    else
    {
        currentL3Value = currentL3ValueMin + rand() % (currentL3ValueMax - currentL3ValueMin + 1); // Normal value
        currentL3AverageCounter++; // Increment the average counter to calculate new average
        currentL3AverageValue = calculateAverageUint32(currentL3AverageValue, currentL3Value, currentL3AverageCounter) / 100000;
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
        GX_UINT32_BYREF(frequency.value, frequencyValue);
        frequency.scaler = -3;
        frequency.unit = 44;
    }

    return ret;
}

// Function to set the frequency registers' value
void writeFrequencyValue(uint32_t value)
{
    frequencyValue = value;
}

// Function to get the frequency registers' value with garbage value injection
uint32_t readFrequencyValue()
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
void writePowerFactorL1Value(float value)
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
void writePowerFactorL2Value(float value)
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
void writePowerFactorL3Value(float value)
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
        cumulativeEnergyKWhImportValue = (uint32_t)(
                                        (voltageL1Value * currentL1Value * cos(powerFactorL1Value)) +
                                        (voltageL2Value * currentL2Value * cos(powerFactorL2Value)) +
                                        (voltageL3Value * currentL3Value * cos(powerFactorL3Value)));
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
            lastCumulativeEnergyKWhImportValue = cumulativeEnergyKWhImportValue;
            cumulativeEnergyKWhImportValue = cumulativeEnergyKWhImportGarbageValues[rand() % (sizeof(cumulativeEnergyKWhImportGarbageValues) / sizeof(cumulativeEnergyKWhImportGarbageValues[0]))];
            current_timestamp = getFormattedTimestamp();
            printf("%s -> Meter sending garbage value %u for cumulative energy kWh import.\n", current_timestamp, cumulativeEnergyKWhImportValue);
            // Reset the counter
            cumulativeEnergyKWhImportCounter = resetCounter();
            isCumulativeEnergyKWhImportGarbageValueSent = true;
            return cumulativeEnergyKWhImportValue;
        }
        else
        {
            // Check if garbage value was sent, then restore the previous value
            if (isCumulativeEnergyKWhImportGarbageValueSent)
            {
                isCumulativeEnergyKWhImportGarbageValueSent = false;
                cumulativeEnergyKWhImportValue = lastCumulativeEnergyKWhImportValue;
            }

            if (!isCumulativeEnergyKWhImportReadOnce) 
            {
                isCumulativeEnergyKWhImportReadOnce = true;
            }
            else
            {
                // Increment the value randomly by 1, 2, or 3
                cumulativeEnergyKWhImportValue += (rand() % 3) + 1;
            }
            // cumulativeEnergyKWhImportValue = cumulativeEnergyKWhImportValueMin + rand() % (cumulativeEnergyKWhImportValueMax - cumulativeEnergyKWhImportValueMin + 1); // Normal value
            cumulativeEnergyKWhImportCounter--;
        }
    }
    else
    {
        if (!isCumulativeEnergyKWhImportReadOnce) 
        {
            isCumulativeEnergyKWhImportReadOnce = true;
        }
        else
        {
            // Increment the value randomly by 1, 2, or 3
            cumulativeEnergyKWhImportValue += (rand() % 3) + 1;
        }
        // cumulativeEnergyKWhImportValue = cumulativeEnergyKWhImportValueMin + rand() % (cumulativeEnergyKWhImportValueMax - cumulativeEnergyKWhImportValueMin + 1); // Normal value
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
        cumulativeEnergyKVAhImportValue = (voltageL1Value * currentL1Value) +
                                          (voltageL2Value * currentL2Value) +
                                          (voltageL3Value * currentL3Value); 
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
            lastCumulativeEnergyKVAhImportValue = cumulativeEnergyKVAhImportValue;
            cumulativeEnergyKVAhImportValue = cumulativeEnergyKVAhImportGarbageValues[rand() % (sizeof(cumulativeEnergyKVAhImportGarbageValues) / sizeof(cumulativeEnergyKVAhImportGarbageValues[0]))];
            current_timestamp = getFormattedTimestamp();
            printf("%s -> Meter sending garbage value %u for cumulative energy kVAh import.\n", current_timestamp, cumulativeEnergyKVAhImportValue);
            // Reset the counter
            cumulativeEnergyKVAhImportCounter = resetCounter();
            isCumulativeEnergyKVAhImportGarbageValueSent = true;
            return cumulativeEnergyKVAhImportValue;
        }
        else
        {
            // Check if garbage value was sent, then restore the previous value
            if (isCumulativeEnergyKVAhImportGarbageValueSent)
            {
                isCumulativeEnergyKVAhImportGarbageValueSent = false;
                cumulativeEnergyKVAhImportValue = lastCumulativeEnergyKVAhImportValue;
            }

            if (!isCumulativeEnergyKVAhImportReadOnce) 
            {
                isCumulativeEnergyKVAhImportReadOnce = true;
            }
            else 
            {
                // Increment the value randomly by 1, 2, or 3
                cumulativeEnergyKVAhImportValue += (rand() % 3) + 1;
            } 
            // cumulativeEnergyKVAhImportValue = cumulativeEnergyKVAhImportValueMin + rand() % (cumulativeEnergyKVAhImportValueMax - cumulativeEnergyKVAhImportValueMin + 1); // Normal value
            cumulativeEnergyKVAhImportCounter--;
        }
    }
    else
    {
        if (!isCumulativeEnergyKVAhImportReadOnce) 
        {
            isCumulativeEnergyKVAhImportReadOnce = true;
        }
        else
        {
            // Increment the value randomly by 1, 2, or 3
            cumulativeEnergyKVAhImportValue += (rand() % 3) + 1;
        }
        // cumulativeEnergyKVAhImportValue = cumulativeEnergyKVAhImportValueMin + rand() % (cumulativeEnergyKVAhImportValueMax - cumulativeEnergyKVAhImportValueMin + 1); // Normal value
    }
    return cumulativeEnergyKVAhImportValue;
}

// Function to add the voltageL1Average register to the DLMS server
int addVoltageL1Average()
{
    int ret;
    const unsigned char ln[6] = { 1, 0, 32, 27, 0, 255 };  // Example LN for voltageL1 average

    // Initialize the voltageL1 average register object
    if ((ret = INIT_OBJECT(voltageL1Average, DLMS_OBJECT_TYPE_REGISTER, ln)) == 0)
    {
        GX_UINT32_BYREF(voltageL1Average.value, voltageL1AverageValue);
        // Set additional properties  
        voltageL1Average.scaler = -3;
        voltageL1Average.unit = 35;   
    }
    
    return ret;
}

// Function to get the voltageL1Average registers' value
uint32_t readVoltageL1AverageValue()
{
    return voltageL1AverageValue;
}

// Function to add the voltageL1Average register to the DLMS server
int addVoltageL2Average()
{
    int ret;
    const unsigned char ln[6] = { 1, 0, 52, 27, 0, 255 };  // Example LN for voltageL2 average

    // Initialize the voltageL2 average register object
    if ((ret = INIT_OBJECT(voltageL2Average, DLMS_OBJECT_TYPE_REGISTER, ln)) == 0)
    {
        GX_UINT32_BYREF(voltageL2Average.value, voltageL2AverageValue);
        // Set additional properties  
        voltageL2Average.scaler = -3;
        voltageL2Average.unit = 35;   
    }
    
    return ret;
}

// Function to get the voltageL2Average registers' value 
uint32_t readVoltageL2AverageValue()
{
    return voltageL2AverageValue;
}

// Function to add the voltageL3Average register to the DLMS server
int addVoltageL3Average()
{
    int ret;
    const unsigned char ln[6] = { 1, 0, 72, 27, 0, 255 };  // Example LN for voltageL3 average

    // Initialize the voltageL3 average register object
    if ((ret = INIT_OBJECT(voltageL3Average, DLMS_OBJECT_TYPE_REGISTER, ln)) == 0)
    {
        GX_UINT32_BYREF(voltageL3Average.value, voltageL3AverageValue);
        // Set additional properties  
        voltageL3Average.scaler = -3;
        voltageL3Average.unit = 35;   
    }
    
    return ret;
}

// Function to get the voltageL3Average registers' value 
uint32_t readVoltageL3AverageValue()
{
    return voltageL3AverageValue;
}

// Function to add the currentL1Average register to the DLMS server
int addCurrentL1Average()
{
    int ret;
    const unsigned char ln[6] = { 1, 0, 31, 27, 0, 255 };  // Example LN for currentL1 average

    // Initialize the currentL1 average register object
    if ((ret = INIT_OBJECT(currentL1Average, DLMS_OBJECT_TYPE_REGISTER, ln)) == 0)
    {
        GX_UINT32_BYREF(currentL1Average.value, currentL1AverageValue);
        // Set additional properties  
        currentL1Average.scaler = -5;
        currentL1Average.unit = 35;   
    }
    
    return ret;
}

// Function to get the currentL1Average register's value
uint32_t readCurrentL1AverageValue()
{
    return currentL1AverageValue;
}

// Function to add the currentL2Average register to the DLMS server
int addCurrentL2Average()
{
    int ret;
    const unsigned char ln[6] = { 1, 0, 51, 27, 0, 255 };  // Example LN for currentL2 average

    // Initialize the currentL2 average register object
    if ((ret = INIT_OBJECT(currentL2Average, DLMS_OBJECT_TYPE_REGISTER, ln)) == 0)
    {
        GX_UINT32_BYREF(currentL2Average.value, currentL2AverageValue);
        // Set additional properties  
        currentL2Average.scaler = -5;
        currentL2Average.unit = 35;   
    }
    
    return ret;
}

// Function to get the currentL2Average register's value
uint32_t readCurrentL2AverageValue()
{
    return currentL2AverageValue;
}

// Function to add the currentL3Average register to the DLMS server
int addCurrentL3Average()
{
    int ret;
    const unsigned char ln[6] = { 1, 0, 71, 27, 0, 255 };  // Example LN for currentL3 average

    // Initialize the currentL3 average register object
    if ((ret = INIT_OBJECT(currentL3Average, DLMS_OBJECT_TYPE_REGISTER, ln)) == 0)
    {
        GX_UINT32_BYREF(currentL3Average.value, currentL3AverageValue);
        // Set additional properties  
        currentL3Average.scaler = -5;
        currentL3Average.unit = 35;   
    }
    
    return ret;
}

// Function to get the currentL3Average register's value
uint32_t readCurrentL3AverageValue()
{
    return currentL3AverageValue;
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

// Set upper and lower limits for all registers' values
bool setRegisterLimits(const char* filePath)
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

    // Parse and store limits
    cJSON* voltageLimits = cJSON_GetObjectItem(json, "voltage_limits");
    if (voltageLimits)
    {
        voltageL1ValueMin = cJSON_GetObjectItem(cJSON_GetObjectItem(voltageLimits, "L1"), "lower_limit")->valueint;
        voltageL1ValueMax = cJSON_GetObjectItem(cJSON_GetObjectItem(voltageLimits, "L1"), "upper_limit")->valueint;
        voltageL2ValueMin = cJSON_GetObjectItem(cJSON_GetObjectItem(voltageLimits, "L2"), "lower_limit")->valueint;
        voltageL2ValueMax = cJSON_GetObjectItem(cJSON_GetObjectItem(voltageLimits, "L2"), "upper_limit")->valueint;
        voltageL3ValueMin = cJSON_GetObjectItem(cJSON_GetObjectItem(voltageLimits, "L3"), "lower_limit")->valueint;
        voltageL3ValueMax = cJSON_GetObjectItem(cJSON_GetObjectItem(voltageLimits, "L3"), "upper_limit")->valueint;
    }

    cJSON* currentLimits = cJSON_GetObjectItem(json, "current_limits");
    if (currentLimits)
    {
        currentL1ValueMin = cJSON_GetObjectItem(cJSON_GetObjectItem(currentLimits, "L1"), "lower_limit")->valueint;
        currentL1ValueMax = cJSON_GetObjectItem(cJSON_GetObjectItem(currentLimits, "L1"), "upper_limit")->valueint;
        currentL2ValueMin = cJSON_GetObjectItem(cJSON_GetObjectItem(currentLimits, "L2"), "lower_limit")->valueint;
        currentL2ValueMax = cJSON_GetObjectItem(cJSON_GetObjectItem(currentLimits, "L2"), "upper_limit")->valueint;
        currentL3ValueMin = cJSON_GetObjectItem(cJSON_GetObjectItem(currentLimits, "L3"), "lower_limit")->valueint;
        currentL3ValueMax = cJSON_GetObjectItem(cJSON_GetObjectItem(currentLimits, "L3"), "upper_limit")->valueint;
    }

    cJSON* frequencyLimits = cJSON_GetObjectItem(json, "frequency_limits");
    if (frequencyLimits)
    {
        frequencyValueMin = cJSON_GetObjectItem(frequencyLimits, "lower_limit")->valueint; // Convert to integer
        frequencyValueMax = cJSON_GetObjectItem(frequencyLimits, "upper_limit")->valueint; // Convert to integer
    }

    cJSON* powerFactorLimits = cJSON_GetObjectItem(json, "power_factor_limits");
    if (powerFactorLimits)
    {
        powerFactorL1ValueMin = cJSON_GetObjectItem(cJSON_GetObjectItem(powerFactorLimits, "L1"), "lower_limit")->valuedouble;
        powerFactorL1ValueMax = cJSON_GetObjectItem(cJSON_GetObjectItem(powerFactorLimits, "L1"), "upper_limit")->valuedouble;
        powerFactorL2ValueMin = cJSON_GetObjectItem(cJSON_GetObjectItem(powerFactorLimits, "L2"), "lower_limit")->valuedouble;
        powerFactorL2ValueMax = cJSON_GetObjectItem(cJSON_GetObjectItem(powerFactorLimits, "L2"), "upper_limit")->valuedouble;
        powerFactorL3ValueMin = cJSON_GetObjectItem(cJSON_GetObjectItem(powerFactorLimits, "L3"), "lower_limit")->valuedouble;
        powerFactorL3ValueMax = cJSON_GetObjectItem(cJSON_GetObjectItem(powerFactorLimits, "L3"), "upper_limit")->valuedouble;
    }

    cJSON* blockEnergyLimits = cJSON_GetObjectItem(json, "block_energy_limits");
    if (blockEnergyLimits)
    {
        blockEnergyKWhImportValueMin = cJSON_GetObjectItem(cJSON_GetObjectItem(blockEnergyLimits, "kWh_import"), "lower_limit")->valueint;
        blockEnergyKWhImportValueMax = cJSON_GetObjectItem(cJSON_GetObjectItem(blockEnergyLimits, "kWh_import"), "upper_limit")->valueint;
        blockEnergyKVAhLagValueMin = cJSON_GetObjectItem(cJSON_GetObjectItem(blockEnergyLimits, "kVAh_lag"), "lower_limit")->valueint;
        blockEnergyKVAhLagValueMax = cJSON_GetObjectItem(cJSON_GetObjectItem(blockEnergyLimits, "kVAh_lag"), "upper_limit")->valueint;
        blockEnergyKVAhLeadValueMin = cJSON_GetObjectItem(cJSON_GetObjectItem(blockEnergyLimits, "kVAh_lead"), "lower_limit")->valueint;
        blockEnergyKVAhLeadValueMax = cJSON_GetObjectItem(cJSON_GetObjectItem(blockEnergyLimits, "kVAh_lead"), "upper_limit")->valueint;
        blockEnergyKVAhImportValueMin = cJSON_GetObjectItem(cJSON_GetObjectItem(blockEnergyLimits, "kVAh_import"), "lower_limit")->valueint;
        blockEnergyKVAhImportValueMax = cJSON_GetObjectItem(cJSON_GetObjectItem(blockEnergyLimits, "kVAh_import"), "upper_limit")->valueint;
    }

    // cJSON* cumulativeEnergyLimits = cJSON_GetObjectItem(json, "cumulative_energy_limits");
    // if (cumulativeEnergyLimits)
    // {
    //     cumulativeEnergyKWhImportValueMin = cJSON_GetObjectItem(cJSON_GetObjectItem(cumulativeEnergyLimits, "kWh_import"), "lower_limit")->valueint;
    //     cumulativeEnergyKWhImportValueMax = cJSON_GetObjectItem(cJSON_GetObjectItem(cumulativeEnergyLimits, "kWh_import"), "upper_limit")->valueint;
    //     cumulativeEnergyKVAhImportValueMin = cJSON_GetObjectItem(cJSON_GetObjectItem(cumulativeEnergyLimits, "kVAh_import"), "lower_limit")->valueint;
    //     cumulativeEnergyKVAhImportValueMax = cJSON_GetObjectItem(cJSON_GetObjectItem(cumulativeEnergyLimits, "kVAh_import"), "upper_limit")->valueint;
    // }

    // Cleanup the JSON object
    cJSON_Delete(json);
    return true;
}
