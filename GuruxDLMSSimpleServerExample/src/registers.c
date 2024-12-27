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
uint32_t powerFactorL1GarbageValues[] = {800000, 210000, 200000};
uint32_t powerFactorL2GarbageValues[] = {900000, 210000, 210000};
uint32_t powerFactorL3GarbageValues[] = {800000, 210000, 220000};
uint32_t blockEnergyKWhImportGarbageValues[] = {99999, 12345, 214745};
uint32_t blockEnergyKWhExportGarbageValues[] = {99999, 12345, 214745};
uint32_t blockEnergyKVAhLagGarbageValues[] = {99995, 23455, 198765};
uint32_t blockEnergyKVAhLeadGarbageValues[] = {88885, 34565, 109875};
uint32_t blockEnergyKVAhImportGarbageValues[] = {77775, 45675, 987655};
uint32_t cumulativeEnergyKWhImportGarbageValues[] = {90, 98, 76};
uint32_t cumulativeEnergyKWhExportGarbageValues[] = {90, 98, 76};
uint32_t cumulativeEnergyKVAhLagGarbageValues[] = {88, 87, 65};
uint32_t cumulativeEnergyKVAhLeadGarbageValues[] = {88, 87, 65};
uint32_t cumulativeEnergyKVAhImportGarbageValues[] = {88, 87, 65};

// Garbage value arrays for each variable of single phase meter
uint32_t neutralCurrentGarbageValues[] = {5000000, 9900000, 11000000};
uint32_t activePowerGarbageValues[] = {5000000, 9900000, 11000000};
uint32_t apparentPowerGarbageValues[] = {5000000, 9900000, 11000000};
uint32_t signedPowerFactorGarbageValues[] = {5000000, 9900000, 11000000};


// Define the KIGG register objects globally.
gxRegister voltageL1, voltageL2, voltageL3;
gxRegister currentL1, currentL2, currentL3;
gxRegister frequency;
gxRegister powerFactorL1, powerFactorL2, powerFactorL3;
gxRegister blockEnergyKWhImport, blockEnergyKWhExport, blockEnergyKVAhLag, blockEnergyKVAhLead, blockEnergyKVAhImport;
gxRegister cumulativeEnergyKWhImport, cumulativeEnergyKVAhLag, cumulativeEnergyKVAhLead, cumulativeEnergyKVAhImport, cumulativeEnergyKWhExport;

// Define the objects to store KIGG register's averages
gxRegister voltageL1Average, voltageL2Average, voltageL3Average;
gxRegister currentL1Average, currentL2Average, currentL3Average;


// Define individual objects for nameplate profile
gxData meterSerialNumber, manufacturerName, firmwareVersion, meterType, meterCategory;
gxData currentRating, ctr, ptr, yearOfManufacture;


// Define the KIGG register objects for single phase meter globally.
gxRegister neutralCurrent;
gxRegister activePower, apparentPower;
gxRegister signedPowerFactor;


// Define variables to store the KIGG registers' values
static uint32_t voltageL1Value = 0, voltageL2Value = 0, voltageL3Value = 0;
static uint32_t currentL1Value = 0, currentL2Value = 0, currentL3Value = 0;
static uint32_t frequencyValue = 0;
static uint32_t powerFactorL1Value = 0, powerFactorL2Value = 0, powerFactorL3Value = 0;
static uint32_t blockEnergyKWhImportValue = 0, blockEnergyKWhExportValue = 0, blockEnergyKVAhLagValue = 0, blockEnergyKVAhLeadValue = 0, blockEnergyKVAhImportValue = 0;
static uint32_t cumulativeEnergyKWhImportValue = 0, cumulativeEnergyKVAhLagValue = 0, cumulativeEnergyKVAhLeadValue = 0, cumulativeEnergyKVAhImportValue = 0, cumulativeEnergyKWhExportValue = 0;

// Define variables to store the KIGG registers' averages
uint32_t voltageL1AverageValue = 0, voltageL2AverageValue = 0, voltageL3AverageValue = 0;
uint32_t currentL1AverageValue = 0, currentL2AverageValue = 0, currentL3AverageValue = 0;


// Define variables to store the KIGG registers' values for single phase meter
static uint32_t neutralCurrentValue = 0;
static uint32_t activePowerValue = 0, apparentPowerValue = 0;
static uint32_t signedPowerFactorValue = 0;


// Define variables for upper and lower limits
static uint32_t voltageL1ValueMin = 108000, voltageL1ValueMax = 112000;
static uint32_t voltageL2ValueMin = 108000, voltageL2ValueMax = 112000;
static uint32_t voltageL3ValueMin = 108000, voltageL3ValueMax = 112000;

static uint32_t currentL1ValueMin = 0, currentL1ValueMax = 1000000;
static uint32_t currentL2ValueMin = 0, currentL2ValueMax = 1000000;
static uint32_t currentL3ValueMin = 0, currentL3ValueMax = 1000000;

static uint32_t frequencyValueMin = 49800, frequencyValueMax = 50200;

static uint32_t powerFactorL1ValueMin = 100, powerFactorL1ValueMax = 990;
static uint32_t powerFactorL2ValueMin = 100, powerFactorL2ValueMax = 990;
static uint32_t powerFactorL3ValueMin = 100, powerFactorL3ValueMax = 990;

static uint32_t blockEnergyKWhImportValueMin = 0, blockEnergyKWhImportValueMax = 10000;
static uint32_t blockEnergyKWhExportValueMin = 0, blockEnergyKWhExportValueMax = 10000;
static uint32_t blockEnergyKVAhLagValueMin = 0, blockEnergyKVAhLagValueMax = 10000;
static uint32_t blockEnergyKVAhLeadValueMin = 0, blockEnergyKVAhLeadValueMax = 10000;
static uint32_t blockEnergyKVAhImportValueMin = 0, blockEnergyKVAhImportValueMax = 10000;

// static uint32_t cumulativeEnergyKWhImportValueMin = 450, cumulativeEnergyKWhImportValueMax = 500;
// static uint32_t cumulativeEnergyKVAhImportValueMin = 500, cumulativeEnergyKVAhImportValueMax = 600;


// Define variables for upper and lower limits of single phase meter registers
static uint32_t neutralCurrentValueMin = 0, neutralCurrentValueMax = 1000000;
static uint32_t activePowerValueMin = 0, activePowerValueMax = 1000000;
static uint32_t apparentPowerValueMin = 0, apparentPowerValueMax = 1000000;
static uint32_t signedPowerFactorValueMin = 100, signedPowerFactorValueMax = 990;


static char meterSerialNumberValue[64U] = "X0000000";
static const char* manufacturerNameValue = "SECURE METERS LTD.";
static const char* firmwareVersionValue = "M1XXG04";
static uint8_t meterTypeValue = 2;
static const char* meterCategoryValue = "C1";
static const char* currentRatingValue = "5 - 10 A";
static uint8_t ctrValue = 12;
static uint8_t ptrValue = 100;
static uint16_t yearOfManufactureValue = 2019;              // Fixed value for the register


// Garbage counters for each variable
static int voltageL1Counter = 0, voltageL2Counter = 0, voltageL3Counter = 0;
static int currentL1Counter = 0, currentL2Counter = 0, currentL3Counter = 0;
static int frequencyCounter = 0;
static int powerFactorL1Counter = 0, powerFactorL2Counter = 0, powerFactorL3Counter = 0;
static int blockEnergyKWhImportCounter = 0, blockEnergyKWhExportCounter = 0, blockEnergyKVAhLagCounter = 0, blockEnergyKVAhLeadCounter = 0, blockEnergyKVAhImportCounter = 0;
static int cumulativeEnergyKWhImportCounter = 0, cumulativeEnergyKVAhLagCounter = 0, cumulativeEnergyKVAhLeadCounter = 0, cumulativeEnergyKVAhImportCounter = 0, cumulativeEnergyKWhExportCounter = 0;


// Garbage counters for each variable of single phase meter
static int neutralCurrentCounter = 0;
static int activePowerCounter = 0, apparentPowerCounter = 0;
static int signedPowerFactorCounter = 0;


// Average value counters for each variable
static int voltageL1AverageCounter = 0, voltageL2AverageCounter = 0, voltageL3AverageCounter = 0;
static int currentL1AverageCounter = 0, currentL2AverageCounter = 0, currentL3AverageCounter = 0;


static char* current_timestamp = NULL;


// Variables to keep track of if the cumulative energy values are read once
static bool isCumulativeEnergyKWhImportReadOnce = false;
static bool isCumulativeEnergyKWhExportReadOnce = false;
static bool isCumulativeEnergyKVAhLagReadOnce = false;
static bool isCumulativeEnergyKVAhLeadReadOnce = false;
static bool isCumulativeEnergyKVAhImportReadOnce = false;


// Variable to track if garbage values were read once 
static bool isCumulativeEnergyKWhImportGarbageValueSent = false;
static bool isCumulativeEnergyKWhExportGarbageValueSent = false;
static bool isCumulativeEnergyKVAhLagGarbageValueSent = false;
static bool isCumulativeEnergyKVAhLeadGarbageValueSent = false;
static bool isCumulativeEnergyKVAhImportGarbageValueSent = false;


// Variable to store the last cumulative energy values when garbage injection is enabled
static uint32_t lastCumulativeEnergyKWhImportValue = 0;
static uint32_t lastCumulativeEnergyKWhExportValue = 0;
static uint32_t lastCumulativeEnergyKVAhLagValue = 0;
static uint32_t lastCumulativeEnergyKVAhLeadValue = 0;
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
    uint32_t new_average = ((average * (count - 1) + value) / (count));
    return new_average;
}   

// Function to add the voltageL1 register to the DLMS server
int addVoltageL1()
{
    int ret;

#ifdef SINGLE_PHASE
    const unsigned char ln[6] = { 1, 0, 12, 7, 0, 255 };  // Example LN for voltageL1
#elif defined(THREE_PHASE)
    const unsigned char ln[6] = { 1, 0, 32, 7, 0, 255 };  // Example LN for voltageL1
#endif

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
            voltageL1AverageValue = calculateAverageUint32(voltageL1AverageValue * 1000, voltageL1Value, voltageL1AverageCounter) / 1000;
        }
    }
    else
    {
        voltageL1Value = voltageL1ValueMin + rand() % (voltageL1ValueMax - voltageL1ValueMin + 1); // Normal value
        voltageL1AverageCounter++; // Increment the average counter to calculate new average
        voltageL1AverageValue = calculateAverageUint32(voltageL1AverageValue * 1000, voltageL1Value, voltageL1AverageCounter) / 1000;
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
            voltageL2AverageValue = calculateAverageUint32(voltageL2AverageValue * 1000, voltageL2Value, voltageL2AverageCounter) / 1000;
        }
    }
    else
    {
        voltageL2Value = voltageL2ValueMin + rand() % (voltageL2ValueMax - voltageL2ValueMin + 1); // Normal value
        voltageL2AverageCounter++; // Increment the average counter to calculate new average
        voltageL2AverageValue = calculateAverageUint32(voltageL2AverageValue * 1000, voltageL2Value, voltageL2AverageCounter) / 1000;
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
            voltageL3AverageValue = calculateAverageUint32(voltageL3AverageValue * 1000, voltageL3Value, voltageL3AverageCounter) / 1000;
        }
    }
    else
    {
        voltageL3Value = voltageL3ValueMin + rand() % (voltageL3ValueMax - voltageL3ValueMin + 1); // Normal value
        voltageL3AverageCounter++; // Increment the average counter to calculate new average
        voltageL3AverageValue = calculateAverageUint32(voltageL3AverageValue * 1000, voltageL3Value, voltageL3AverageCounter)/ 1000;
    }
    return voltageL3Value;
}

// Function to add the currentL1 register to the DLMS server
int addCurrentL1()
{
    int ret;

#ifdef SINGLE_PHASE
    const unsigned char ln[6] = { 1, 0, 11, 7, 0, 255 };  // LN for currentL1
#elif defined(THREE_PHASE)  
    const unsigned char ln[6] = { 1, 0, 31, 7, 0, 255 };  // LN for currentL1
#endif

    if ((ret = INIT_OBJECT(currentL1, DLMS_OBJECT_TYPE_REGISTER, ln)) == 0)
    {
        currentL1Value = 10;
        GX_UINT32_BYREF(currentL1.value, currentL1Value);

#ifdef SINGLE_PHASE
        currentL1.scaler = -3;
#elif defined(THREE_PHASE)
        currentL1.scaler = -5;
#endif
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
            currentL1AverageValue = calculateAverageUint32(currentL1AverageValue * 100000, currentL1Value, currentL1AverageCounter) / 100000;
        }
    }
    else
    {
        currentL1Value = currentL1ValueMin + rand() % (currentL1ValueMax - currentL1ValueMin + 1); // Normal value
        currentL1AverageCounter++; // Increment the average counter to calculate new average
        currentL1AverageValue = calculateAverageUint32(currentL1AverageValue * 100000, currentL1Value, currentL1AverageCounter) / 100000;
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
            currentL2AverageValue = calculateAverageUint32(currentL2AverageValue * 100000, currentL2Value, currentL2AverageCounter) / 100000;
        }
    }
    else
    {
        currentL2Value = currentL2ValueMin + rand() % (currentL2ValueMax - currentL2ValueMin + 1); // Normal value
        currentL2AverageCounter++; // Increment the average counter to calculate new average
        currentL2AverageValue = calculateAverageUint32(currentL2AverageValue * 100000, currentL2Value, currentL2AverageCounter) / 100000;
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
            currentL3AverageValue = calculateAverageUint32(currentL3AverageValue * 100000, currentL3Value, currentL3AverageCounter)/ 100000;
        }
    }
    else
    {
        currentL3Value = currentL3ValueMin + rand() % (currentL3ValueMax - currentL3ValueMin + 1); // Normal value
        currentL3AverageCounter++; // Increment the average counter to calculate new average
        currentL3AverageValue = calculateAverageUint32(currentL3AverageValue * 100000, currentL3Value, currentL3AverageCounter) / 100000;
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
        GX_UINT32_BYREF(powerFactorL1.value, powerFactorL1Value);
        powerFactorL1.scaler = -3;
        powerFactorL1.unit = 0;  
    }

    return ret;
}

// Function to set the powerFactorL1 registers' value
void writePowerFactorL1Value(uint32_t value)
{
    powerFactorL1Value = value;
}

// Function to get the powerFactorL1 registers' value with garbage value injection
uint32_t readPowerFactorL1Value()
{
    if (isGarbageValuesEnabled())
    {
        if (powerFactorL1Counter == 0)
        {
            // Select a random garbage value
            powerFactorL1Value = powerFactorL1GarbageValues[rand() % (sizeof(powerFactorL1GarbageValues) / sizeof(powerFactorL1GarbageValues[0]))];
            current_timestamp = getFormattedTimestamp();
            printf("%s -> Meter sending garbage value %u for power factor L1.\n", current_timestamp, powerFactorL1Value);
            // Reset the counter
            powerFactorL1Counter = resetCounter();
        }
        else
        {
            powerFactorL1Value = powerFactorL1ValueMin + rand() % (powerFactorL1ValueMax - powerFactorL1ValueMin + 1); // Normal value
            powerFactorL1Counter--;
        }
    }
    else
    {
        powerFactorL1Value = powerFactorL1ValueMin + rand() % (powerFactorL1ValueMax - powerFactorL1ValueMin + 1); // Normal value
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
        GX_UINT32_BYREF(powerFactorL2.value, powerFactorL2Value);
        powerFactorL2.scaler = -3;
        powerFactorL2.unit = 0;  
    }

    return ret;
}

// Function to set Power Factor L2 register's value
void writePowerFactorL2Value(uint32_t value)
{
    powerFactorL2Value = value;
}

// Function to get the powerFactorL2 registers' value with garbage value injection
uint32_t readPowerFactorL2Value()
{
    if (isGarbageValuesEnabled())
    {
        if (powerFactorL2Counter == 0)
        {
            // Select a random garbage value
            powerFactorL2Value = powerFactorL2GarbageValues[rand() % (sizeof(powerFactorL2GarbageValues) / sizeof(powerFactorL2GarbageValues[0]))];
            current_timestamp = getFormattedTimestamp();
            printf("%s -> Meter sending garbage value %u for power factor L2.\n", current_timestamp, powerFactorL2Value);
            // Reset the counter
            powerFactorL2Counter = resetCounter();
        }
        else
        {
            powerFactorL2Value = powerFactorL2ValueMin + rand() % (powerFactorL2ValueMax - powerFactorL2ValueMin + 1); // Normal value
            powerFactorL2Counter--;
        }
    }
    else
    {
        powerFactorL2Value = powerFactorL2ValueMin + rand() % (powerFactorL2ValueMax - powerFactorL2ValueMin + 1); // Normal value
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
        GX_UINT32_BYREF(powerFactorL3.value, powerFactorL3Value);
        powerFactorL3.scaler = -3;
        powerFactorL3.unit = 0;  
    }

    return ret;
}

// Function to set Power Factor L3 register's value
void writePowerFactorL3Value(uint32_t value)
{
    powerFactorL3Value = value;
}

// Function to get the powerFactorL3 registers' value with garbage value injection
uint32_t readPowerFactorL3Value()
{
    if (isGarbageValuesEnabled())
    {
        if (powerFactorL3Counter == 0)
        {
            // Select a random garbage value
            powerFactorL3Value = powerFactorL3GarbageValues[rand() % (sizeof(powerFactorL3GarbageValues) / sizeof(powerFactorL3GarbageValues[0]))];
            current_timestamp = getFormattedTimestamp();
            printf("%s -> Meter sending garbage value %u for power factor L3.\n", current_timestamp, powerFactorL3Value);
            // Reset the counter
            powerFactorL3Counter = resetCounter();
        }
        else
        {
            powerFactorL3Value = powerFactorL3ValueMin + rand() % (powerFactorL3ValueMax - powerFactorL3ValueMin + 1); // Normal value
            powerFactorL3Counter--;
        }
    }
    else
    {
        powerFactorL3Value = powerFactorL3ValueMin + rand() % (powerFactorL3ValueMax - powerFactorL3ValueMin + 1); // Normal value
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

// Function to add Block Energy (kWh Export) register to the DLMS server
int addBlockEnergyKWhExport()
{
    int ret;
    const unsigned char ln[6] = { 1, 0, 2, 29, 0, 255 };  // LN for Block Energy KWh Export

    if ((ret = INIT_OBJECT(blockEnergyKWhExport, DLMS_OBJECT_TYPE_REGISTER, ln)) == 0)
    {
        blockEnergyKWhExportValue = 1000; 
        GX_UINT32_BYREF(blockEnergyKWhExport.value, blockEnergyKWhExportValue);
        blockEnergyKWhExport.scaler = 2;
        blockEnergyKWhExport.unit = 32;  
    }

    return ret;
}

// Function to set Block Energy (kWh Export) register's value
void writeBlockEnergyKWhExportValue(uint32_t value)
{
    blockEnergyKWhExportValue = value;
}

// Function to get Block Energy (kWh Export) register's value with garbage value injection
uint32_t readBlockEnergyKWhExportValue()
{
    if (isGarbageValuesEnabled())
    {
        if (blockEnergyKWhExportCounter == 0)
        {
            // Select a random garbage value
            blockEnergyKWhExportValue = blockEnergyKWhExportGarbageValues[rand() % (sizeof(blockEnergyKWhExportGarbageValues) / sizeof(blockEnergyKWhExportGarbageValues[0]))];
            current_timestamp = getFormattedTimestamp();
            printf("%s -> Meter sending garbage value %u for block energy kWh export.\n", current_timestamp, blockEnergyKWhExportValue);
            // Reset the counter
            blockEnergyKWhExportCounter = resetCounter();
        }
        else
        {
            blockEnergyKWhExportValue = blockEnergyKWhExportValueMin + rand() % (blockEnergyKWhExportValueMax - blockEnergyKWhExportValueMin + 1); // Normal value
            blockEnergyKWhExportCounter--;
        }
    }
    else
    {
        blockEnergyKWhExportValue = blockEnergyKWhExportValueMin + rand() % (blockEnergyKWhExportValueMax - blockEnergyKWhExportValueMin + 1); // Normal value
    }
    return blockEnergyKWhExportValue;
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

#ifdef SINGLE_PHASE
        cumulativeEnergyKWhImportValue = (uint32_t)(
                                        (voltageL1Value * currentL1Value * cos(signedPowerFactorValue)));
        printf("Voltage value: %u, Current value: %u, Power Factor value: %u\n", voltageL1Value, currentL1Value, signedPowerFactorValue);
        printf("Cumulative Energy (kWh Import) value: %u\n", cumulativeEnergyKWhImportValue);
#elif defined(THREE_PHASE)
        cumulativeEnergyKWhImportValue = (uint32_t)(
                                        (voltageL1Value * currentL1Value * cos(powerFactorL1Value)) +
                                        (voltageL2Value * currentL2Value * cos(powerFactorL2Value)) +
                                        (voltageL3Value * currentL3Value * cos(powerFactorL3Value)));
        printf("From three_phase Cumulative Energy (kWh Import) value: %u\n", cumulativeEnergyKWhImportValue);
#endif
        
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

// Function to add Cumulative Energy (kWh Export) register to the DLMS server
int addCumulativeEnergyKWhExport()
{
    int ret;
    const unsigned char ln[6] = { 1, 0, 2, 8, 0, 255 };  // LN for Cumulative Energy KWh Export

    if ((ret = INIT_OBJECT(cumulativeEnergyKWhExport, DLMS_OBJECT_TYPE_REGISTER, ln)) == 0)
    {
        cumulativeEnergyKWhExportValue = (uint32_t)(
                                        (voltageL1Value * currentL1Value * cos(signedPowerFactorValue)));
        printf("Voltage value: %u, Current value: %u, Power Factor value: %u\n", voltageL1Value, currentL1Value, signedPowerFactorValue);
        printf("Cumulative Energy (kWh Export) value: %u\n", cumulativeEnergyKWhExportValue);
        GX_UINT32_BYREF(cumulativeEnergyKWhExport.value, cumulativeEnergyKWhExportValue);
        cumulativeEnergyKWhExport.scaler = 2;
        cumulativeEnergyKWhExport.unit = 30;  
    }

    return ret;
}

// Function to set Cumulative Energy (kWh Export) register's value
void writeCumulativeEnergyKWhExportValue(uint32_t value)
{
    cumulativeEnergyKWhExportValue = value;
}

// Function to get Cumulative Energy (kWh Export) register's value with garbage value injection
uint32_t readCumulativeEnergyKWhExportValue()
{
    if (isGarbageValuesEnabled())
    {
        if (cumulativeEnergyKWhExportCounter == 0)
        {
            // Select a random garbage value
            lastCumulativeEnergyKWhExportValue = cumulativeEnergyKWhExportValue;
            cumulativeEnergyKWhExportValue = cumulativeEnergyKWhExportGarbageValues[rand() % (sizeof(cumulativeEnergyKWhExportGarbageValues) / sizeof(cumulativeEnergyKWhExportGarbageValues[0]))];
            current_timestamp = getFormattedTimestamp();
            printf("%s -> Meter sending garbage value %u for cumulative energy kWh export.\n", current_timestamp, cumulativeEnergyKWhExportValue);
            // Reset the counter
            cumulativeEnergyKWhExportCounter = resetCounter();
            isCumulativeEnergyKWhExportGarbageValueSent = true;
            return cumulativeEnergyKWhExportValue;
        }
        else
        {
            // Check if garbage value was sent, then restore the previous value
            if (isCumulativeEnergyKWhExportGarbageValueSent)
            {
                isCumulativeEnergyKWhExportGarbageValueSent = false;
                cumulativeEnergyKWhExportValue = lastCumulativeEnergyKWhExportValue;
            }

            if (!isCumulativeEnergyKWhExportReadOnce) 
            {
                isCumulativeEnergyKWhExportReadOnce = true;
            }
            else
            {
                // Increment the value randomly by 1, 2, or 3
                cumulativeEnergyKWhExportValue += (rand() % 3) + 1;
            }
            // cumulativeEnergyKWhExportValue = cumulativeEnergyKWhExportValueMin + rand() % (cumulativeEnergyKWhExportValueMax - cumulativeEnergyKWhExportValueMin + 1); // Normal value
            cumulativeEnergyKWhExportCounter--;
        }
    }
    else
    {
        if (!isCumulativeEnergyKWhExportReadOnce) 
        {
            isCumulativeEnergyKWhExportReadOnce = true;
        }
        else
        {
            // Increment the value randomly by 1, 2, or 3
            cumulativeEnergyKWhExportValue += (rand() % 3) + 1;
        }
        // cumulativeEnergyKWhExportValue = cumulativeEnergyKWhExportValueMin + rand() % (cumulativeEnergyKWhExportValueMax - cumulativeEnergyKWhExportValueMin + 1); // Normal value
    }
    return cumulativeEnergyKWhExportValue;
}

// Function to add Cumulative Energy (kVAh Lag) register to the DLMS server
int addCumulativeEnergyKVAhLag()
{
    int ret;
    const unsigned char ln[6] = { 1, 0, 5, 8, 0, 255 };  // LN for Cumulative Energy kVAh Lag

    if ((ret = INIT_OBJECT(cumulativeEnergyKVAhLag, DLMS_OBJECT_TYPE_REGISTER, ln)) == 0)
    {
        cumulativeEnergyKVAhLagValue = (uint32_t)(
                                        (voltageL1Value * currentL1Value * sin(powerFactorL1Value)) +
                                        (voltageL2Value * currentL2Value * sin(powerFactorL2Value)) +
                                        (voltageL3Value * currentL3Value * sin(powerFactorL3Value)));
        GX_UINT32_BYREF(cumulativeEnergyKVAhLag.value, cumulativeEnergyKVAhLagValue);
        cumulativeEnergyKVAhLag.scaler = 2;
        cumulativeEnergyKVAhLag.unit = 32;  
    }

    return ret;
}

// Function to set Cumulative Energy (kVAh Lag) register's value
void writeCumulativeEnergyKVAhLagValue(uint32_t value)
{
    cumulativeEnergyKVAhLagValue = value;
}

// Function to get Cumulative Energy (kVAh Lag) register's value with garbage value injection
uint32_t readCumulativeEnergyKVAhLagValue()
{
    if (isGarbageValuesEnabled())
    {
        if (cumulativeEnergyKVAhLagCounter == 0)
        {
            // Select a random garbage value
            lastCumulativeEnergyKVAhLagValue = cumulativeEnergyKVAhLagValue;
            cumulativeEnergyKVAhLagValue = cumulativeEnergyKVAhLagGarbageValues[rand() % (sizeof(cumulativeEnergyKVAhLagGarbageValues) / sizeof(cumulativeEnergyKVAhLagGarbageValues[0]))];
            current_timestamp = getFormattedTimestamp();
            printf("%s -> Meter sending garbage value %u for cumulative energy kVAh lag.\n", current_timestamp, cumulativeEnergyKVAhLagValue);
            // Reset the counter
            cumulativeEnergyKVAhLagCounter = resetCounter();
            isCumulativeEnergyKVAhLagGarbageValueSent = true;
            return cumulativeEnergyKVAhLagValue;
        }
        else
        {
            // Check if garbage value was sent, then restore the previous value
            if (isCumulativeEnergyKVAhLagGarbageValueSent)
            {
                isCumulativeEnergyKVAhLagGarbageValueSent = false;
                cumulativeEnergyKVAhLagValue = lastCumulativeEnergyKVAhLagValue;
            }

            if (!isCumulativeEnergyKVAhLagReadOnce) 
            {
                isCumulativeEnergyKVAhLagReadOnce = true;
            }
            else
            {
                // Increment the value randomly by 1, 2, or 3
                cumulativeEnergyKVAhLagValue += (rand() % 3) + 1;
            }
            // cumulativeEnergyKVAhLagValue = cumulativeEnergyKVAhLagValueMin + rand() % (cumulativeEnergyKVAhLagValueMax - cumulativeEnergyKVAhLagValueMin + 1); // Normal value
            cumulativeEnergyKVAhLagCounter--;
        }
    }
    else
    {
        if (!isCumulativeEnergyKVAhLagReadOnce) 
        {
            isCumulativeEnergyKVAhLagReadOnce = true;
        }
        else
        {
            // Increment the value randomly by 1, 2, or 3
            cumulativeEnergyKVAhLagValue += (rand() % 3) + 1;
        }
        // cumulativeEnergyKVAhLagValue = cumulativeEnergyKVAhLagValueMin + rand() % (cumulativeEnergyKVAhLagValueMax - cumulativeEnergyKVAhLagValueMin + 1); // Normal value
    }
    return cumulativeEnergyKVAhLagValue;
}

// Function to add Cumulative Energy (kVAh Lead) register to the DLMS server
int addCumulativeEnergyKVAhLead()
{
    int ret;
    const unsigned char ln[6] = { 1, 0, 8, 8, 0, 255 };  // LN for Cumulative Energy kVAh Lead

    if ((ret = INIT_OBJECT(cumulativeEnergyKVAhLead, DLMS_OBJECT_TYPE_REGISTER, ln)) == 0)
    {
        cumulativeEnergyKVAhLeadValue = (uint32_t)(
                                        (voltageL1Value * currentL1Value * sin(powerFactorL1Value)) +
                                        (voltageL2Value * currentL2Value * sin(powerFactorL2Value)) +
                                        (voltageL3Value * currentL3Value * sin(powerFactorL3Value)));
        GX_UINT32_BYREF(cumulativeEnergyKVAhLead.value, cumulativeEnergyKVAhLeadValue);
        cumulativeEnergyKVAhLead.scaler = 2;
        cumulativeEnergyKVAhLead.unit = 32;  
    }

    return ret;
}

// Function to set Cumulative Energy (kVAh Lead) register's value
void writeCumulativeEnergyKVAhLeadValue(uint32_t value)
{
    cumulativeEnergyKVAhLeadValue = value;
}

// Function to get Cumulative Energy (kVAh Lead) register's value with garbage value injection
uint32_t readCumulativeEnergyKVAhLeadValue()
{
    if (isGarbageValuesEnabled())
    {
        if (cumulativeEnergyKVAhLeadCounter == 0)
        {
            // Select a random garbage value
            lastCumulativeEnergyKVAhLeadValue = cumulativeEnergyKVAhLeadValue;
            cumulativeEnergyKVAhLeadValue = cumulativeEnergyKVAhLeadGarbageValues[rand() % (sizeof(cumulativeEnergyKVAhLeadGarbageValues) / sizeof(cumulativeEnergyKVAhLeadGarbageValues[0]))];
            current_timestamp = getFormattedTimestamp();
            printf("%s -> Meter sending garbage value %u for cumulative energy kVAh lead.\n", current_timestamp, cumulativeEnergyKVAhLeadValue);
            // Reset the counter
            cumulativeEnergyKVAhLeadCounter = resetCounter();
            isCumulativeEnergyKVAhLeadGarbageValueSent = true;
            return cumulativeEnergyKVAhLeadValue;
        }
        else
        {
            // Check if garbage value was sent, then restore the previous value
            if (isCumulativeEnergyKVAhLeadGarbageValueSent)
            {
                isCumulativeEnergyKVAhLeadGarbageValueSent = false;
                cumulativeEnergyKVAhLeadValue = lastCumulativeEnergyKVAhLeadValue;
            }

            if (!isCumulativeEnergyKVAhLeadReadOnce) 
            {
                isCumulativeEnergyKVAhLeadReadOnce = true;
            }
            else
            {
                // Increment the value randomly by 1, 2, or 3
                cumulativeEnergyKVAhLeadValue += (rand() % 3) + 1;
            }
            // cumulativeEnergyKVAhLeadValue = cumulativeEnergyKVAhLeadValueMin + rand() % (cumulativeEnergyKVAhLeadValueMax - cumulativeEnergyKVAhLeadValueMin + 1); // Normal value
            cumulativeEnergyKVAhLeadCounter--;
        }
    }
    else
    {
        if (!isCumulativeEnergyKVAhLeadReadOnce) 
        {
            isCumulativeEnergyKVAhLeadReadOnce = true;
        }
        else
        {
            // Increment the value randomly by 1, 2, or 3
            cumulativeEnergyKVAhLeadValue += (rand() % 3) + 1;
        }
        // cumulativeEnergyKVAhLeadValue = cumulativeEnergyKVAhLeadValueMin + rand() % (cumulativeEnergyKVAhLeadValueMax - cumulativeEnergyKVAhLeadValueMin + 1); // Normal value
    }
    return cumulativeEnergyKVAhLeadValue;
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
        voltageL1AverageValue = 115;
        GX_UINT32_BYREF(voltageL1Average.value, voltageL1AverageValue);
        // Set additional properties  
        voltageL1Average.scaler = 0;
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
        voltageL2AverageValue = 112;
        GX_UINT32_BYREF(voltageL2Average.value, voltageL2AverageValue);
        // Set additional properties  
        voltageL2Average.scaler = 0;
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
        voltageL3AverageValue = 117;
        GX_UINT32_BYREF(voltageL3Average.value, voltageL3AverageValue);
        // Set additional properties  
        voltageL3Average.scaler = 0;
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
        currentL1AverageValue = 10;
        GX_UINT32_BYREF(currentL1Average.value, currentL1AverageValue);
        // Set additional properties  
        currentL1Average.scaler = 0;
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
        currentL2AverageValue = 10;
        GX_UINT32_BYREF(currentL2Average.value, currentL2AverageValue);
        // Set additional properties  
        currentL2Average.scaler = 0;
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
        currentL3AverageValue = 10;
        GX_UINT32_BYREF(currentL3Average.value, currentL3AverageValue);
        // Set additional properties  
        currentL3Average.scaler = 0;
        currentL3Average.unit = 35;   
    }
    
    return ret;
}

// Function to get the currentL3Average register's value
uint32_t readCurrentL3AverageValue()
{
    return currentL3AverageValue;
}

int addMeterSerialNumber()
{
    int ret;
    const unsigned char ln[6] = { 0, 0, 96, 1, 0, 255 };

    if ((ret = INIT_OBJECT(meterSerialNumber, DLMS_OBJECT_TYPE_DATA, ln)) == 0)
    {
        var_addBytes(&meterSerialNumber.value, (unsigned char*)meterSerialNumberValue, strlen(meterSerialNumberValue));
    }

    return ret;
}

const char* readMeterSerialNumber()
{
    return meterSerialNumberValue;
}

int addManufacturerName()
{
    int ret;
    const unsigned char ln[6] = { 0, 0, 96, 1, 1, 255 }; // OBIS code for Manufacturer Name

    if ((ret = INIT_OBJECT(manufacturerName, DLMS_OBJECT_TYPE_DATA, ln)) == 0)
    {
        var_addBytes(&manufacturerName.value, (unsigned char*)manufacturerNameValue, strlen(manufacturerNameValue));
    }

    return ret;
}

const char* readManufacturerName()
{
    return manufacturerNameValue;
}

int addFirmwareVersion()
{
    int ret;
    const unsigned char ln[6] = { 1, 0, 0, 2, 0, 255 }; // OBIS code for Firmware Version

    if ((ret = INIT_OBJECT(firmwareVersion, DLMS_OBJECT_TYPE_DATA, ln)) == 0)
    {
        var_addBytes(&firmwareVersion.value, (unsigned char*)firmwareVersionValue, strlen(firmwareVersionValue));
    }

    return ret;
}

const char* readFirmwareVersion()
{
    return firmwareVersionValue;
}

int addMeterType()
{
    int ret;
    const unsigned char ln[6] = { 0, 0, 94, 91, 9, 255 }; // OBIS code for Meter Type

    if ((ret = INIT_OBJECT(meterType, DLMS_OBJECT_TYPE_DATA, ln)) == 0)
    {
        var_setUInt8(&meterType.value, meterTypeValue);
    }

    return ret;
}

uint8_t readMeterType()
{
    return meterTypeValue;
}

int addMeterCategory()
{
    int ret;
    const unsigned char ln[6] = { 0, 0, 94, 91, 11, 255 }; // OBIS code for Meter Category

    if ((ret = INIT_OBJECT(meterCategory, DLMS_OBJECT_TYPE_DATA, ln)) == 0)
    {
        var_addBytes(&meterCategory.value, (unsigned char*)meterCategoryValue, strlen(meterCategoryValue));
    }

    return ret;
}

const char* readMeterCategory()
{
    return meterCategoryValue;
}

int addCurrentRating()
{
    int ret;
    const unsigned char ln[6] = { 0, 0, 94, 91, 12, 255 }; // OBIS code for Current Rating

    if ((ret = INIT_OBJECT(currentRating, DLMS_OBJECT_TYPE_DATA, ln)) == 0)
    {
        var_addBytes(&currentRating.value, (unsigned char*)currentRatingValue, strlen(currentRatingValue));
    }

    return ret;
}

const char* readCurrentRating()
{
    return currentRatingValue;
}

int addCTR()
{
    int ret;
    const unsigned char ln[6] = { 1, 0, 0, 4, 2, 255 }; // OBIS code for CTR

    if ((ret = INIT_OBJECT(ctr, DLMS_OBJECT_TYPE_DATA, ln)) == 0)
    {
        var_setUInt8(&ctr.value, ctrValue);
    }

    return ret;
}

uint8_t readCTR()
{
    return ctrValue;
}

int addPTR()
{
    int ret;
    const unsigned char ln[6] = { 1, 0, 0, 4, 3, 255 }; // OBIS code for PTR

    if ((ret = INIT_OBJECT(ptr, DLMS_OBJECT_TYPE_DATA, ln)) == 0)
    {
        var_setUInt8(&ptr.value, ptrValue);
    }

    return ret;
}

uint8_t readPTR()
{
    return ptrValue;
}

int addYearOfManufacture()
{
    int ret;
    const unsigned char ln[6] = { 0, 0, 96, 1, 4, 255 }; // OBIS code for Year of Manufacture

    if ((ret = INIT_OBJECT(yearOfManufacture, DLMS_OBJECT_TYPE_DATA, ln)) == 0)
    {
        var_setUInt16(&yearOfManufacture.value, yearOfManufactureValue);
    }

    return ret;
}

uint16_t readYearOfManufacture()
{
    return yearOfManufactureValue;
}


void updateMeterSerialNumber(int value)
{
    char *numberPart = meterSerialNumberValue + 1; // Skip the first character ('X').
    int number = atoi(numberPart); // Convert the numeric part to an integer.
    number += value; // Add the value.
    snprintf(numberPart, sizeof(meterSerialNumberValue) - 1, "%07d", number); // Update the numeric part.
}


// Adding single phase meter registers
// Function to add the neutral current register to the DLMS server
int addNeutralCurrent()
{
    int ret;
    const unsigned char ln[6] = { 1, 0, 91, 7, 0, 255 };  // LN for neutral current

    if ((ret = INIT_OBJECT(neutralCurrent, DLMS_OBJECT_TYPE_REGISTER, ln)) == 0)
    {
        neutralCurrentValue = 10;  // Initialize with a default value
        GX_UINT32_BYREF(neutralCurrent.value, neutralCurrentValue);
        neutralCurrent.scaler = -3;
        neutralCurrent.unit = 33;
    }

    return ret;
}

// Function to set the neutral current register's value
void writeNeutralCurrentValue(uint32_t value)
{
    neutralCurrentValue = value;
}

// Function to get the neutral current register's value with garbage value injection
uint32_t readNeutralCurrentValue()
{
    if (isGarbageValuesEnabled())
    {
        if (neutralCurrentCounter == 0)
        {
            // Select a random garbage value
            neutralCurrentValue = neutralCurrentGarbageValues[rand() % (sizeof(neutralCurrentGarbageValues) / sizeof(neutralCurrentGarbageValues[0]))];
            current_timestamp = getFormattedTimestamp();
            printf("%s -> Meter sending garbage value %u for neutral current.\n", current_timestamp, neutralCurrentValue);
            // Reset the counter
            neutralCurrentCounter = resetCounter();
        }
        else
        {
            neutralCurrentValue = neutralCurrentValueMin + rand() % (neutralCurrentValueMax - neutralCurrentValueMin + 1); // Normal value
            neutralCurrentCounter--;
        }
    }
    else
    {
        neutralCurrentValue = neutralCurrentValueMin + rand() % (neutralCurrentValueMax - neutralCurrentValueMin + 1); // Normal value
    }
    return neutralCurrentValue;
}

// Function to add the active power register to the DLMS server
int addActivePower()
{
    int ret;
    const unsigned char ln[6] = { 1, 0, 1, 7, 0, 255 };  // OBIS code for active power

    if ((ret = INIT_OBJECT(activePower, DLMS_OBJECT_TYPE_REGISTER, ln)) == 0)
    {
        activePowerValue = 1000;  // Initialize with a default value (adjust as needed)
        GX_UINT32_BYREF(activePower.value, activePowerValue);
        activePower.scaler = -1;  // Scalar for active power
        activePower.unit = 27;    // Unit for active power
    }

    return ret;
}

// Function to set the active power register's value
void writeActivePowerValue(uint32_t value)
{
    activePowerValue = value;
}

// Function to get the active power register's value with garbage value injection
uint32_t readActivePowerValue()
{
    if (isGarbageValuesEnabled())
    {
        if (activePowerCounter == 0)
        {
            // Select a random garbage value
            activePowerValue = activePowerGarbageValues[rand() % (sizeof(activePowerGarbageValues) / sizeof(activePowerGarbageValues[0]))];
            current_timestamp = getFormattedTimestamp();
            printf("%s -> Meter sending garbage value %u for active power.\n", current_timestamp, activePowerValue);
            // Reset the counter
            activePowerCounter = resetCounter();
        }
        else
        {
            activePowerValue = activePowerValueMin + rand() % (activePowerValueMax - activePowerValueMin + 1); // Normal value
            activePowerCounter--;
        }
    }
    else
    {
        activePowerValue = activePowerValueMin + rand() % (activePowerValueMax - activePowerValueMin + 1); // Normal value
    }
    return activePowerValue;
}

// Function to add the apparent power register to the DLMS server
int addApparentPower()
{
    int ret;
    const unsigned char ln[6] = { 1, 0, 9, 7, 0, 255 };  // OBIS code for apparent power

    if ((ret = INIT_OBJECT(apparentPower, DLMS_OBJECT_TYPE_REGISTER, ln)) == 0)
    {
        apparentPowerValue = 2000;  // Initialize with a default value (adjust as needed)
        GX_UINT32_BYREF(apparentPower.value, apparentPowerValue);
        apparentPower.scaler = -1;  // Scalar for apparent power
        apparentPower.unit = 28;    // Unit for apparent power
    }

    return ret;
}

// Function to set the apparent power register's value
void writeApparentPowerValue(uint32_t value)
{
    apparentPowerValue = value;
}

// Function to get the apparent power register's value with garbage value injection
uint32_t readApparentPowerValue()
{
    if (isGarbageValuesEnabled())
    {
        if (apparentPowerCounter == 0)
        {
            // Select a random garbage value
            apparentPowerValue = apparentPowerGarbageValues[rand() % (sizeof(apparentPowerGarbageValues) / sizeof(apparentPowerGarbageValues[0]))];
            current_timestamp = getFormattedTimestamp();
            printf("%s -> Meter sending garbage value %u for apparent power.\n", current_timestamp, apparentPowerValue);
            // Reset the counter
            apparentPowerCounter = resetCounter();
        }
        else
        {
            apparentPowerValue = apparentPowerValueMin + rand() % (apparentPowerValueMax - apparentPowerValueMin + 1); // Normal value
            apparentPowerCounter--;
        }
    }
    else
    {
        apparentPowerValue = apparentPowerValueMin + rand() % (apparentPowerValueMax - apparentPowerValueMin + 1); // Normal value
    }
    return apparentPowerValue;
}

// Function to add the signed power factor register to the DLMS server
int addSignedPowerFactor()
{
    int ret;
    const unsigned char ln[6] = { 1, 0, 13, 7, 0, 255 };  // OBIS code for signed power factor

    if ((ret = INIT_OBJECT(signedPowerFactor, DLMS_OBJECT_TYPE_REGISTER, ln)) == 0)
    {
        signedPowerFactorValue = 1;  // Initialize with a default value (adjust as needed)
        GX_UINT32_BYREF(signedPowerFactor.value, signedPowerFactorValue);
        signedPowerFactor.scaler = -3;  // Scalar for signed power factor
        signedPowerFactor.unit = 0;     // Unit for signed power factor
    }

    return ret;
}

// Function to set the signed power factor register's value
void writeSignedPowerFactorValue(uint32_t value)
{
    signedPowerFactorValue = value;
}

// Function to get the signed power factor register's value with garbage value injection
uint32_t readSignedPowerFactorValue()
{
    if (isGarbageValuesEnabled())
    {
        if (signedPowerFactorCounter == 0)
        {
            // Select a random garbage value
            signedPowerFactorValue = signedPowerFactorGarbageValues[rand() % (sizeof(signedPowerFactorGarbageValues) / sizeof(signedPowerFactorGarbageValues[0]))];
            current_timestamp = getFormattedTimestamp();
            printf("%s -> Meter sending garbage value %u for signed power factor.\n", current_timestamp, signedPowerFactorValue);
            // Reset the counter
            signedPowerFactorCounter = resetCounter();
        }
        else
        {
            signedPowerFactorValue = signedPowerFactorValueMin + rand() % (signedPowerFactorValueMax - signedPowerFactorValueMin + 1); // Normal value
            signedPowerFactorCounter--;
        }
    }
    else
    {
        signedPowerFactorValue = signedPowerFactorValueMin + rand() % (signedPowerFactorValueMax - signedPowerFactorValueMin + 1); // Normal value
    }
    return signedPowerFactorValue;
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
    cumulativeEnergyKWhExportCounter = resetCounter();
    cumulativeEnergyKVAhLagCounter = resetCounter();
    cumulativeEnergyKVAhLeadCounter = resetCounter();
    cumulativeEnergyKVAhImportCounter = resetCounter();
    neutralCurrentCounter = resetCounter();
    activePowerCounter = resetCounter();
    apparentPowerCounter = resetCounter();
    signedPowerFactorCounter = resetCounter();
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
