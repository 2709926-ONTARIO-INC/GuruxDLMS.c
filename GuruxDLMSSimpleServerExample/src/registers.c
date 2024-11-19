#include <stdlib.h>

#include "../include/registers.h"
#include "../../development/include/gxmem.h"
#include "../../development/include/dlmssettings.h"
#include "../../development/include/variant.h"
#include "../../development/include/cosem.h"

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
static int16_t powerFactorL1Value = 0, powerFactorL2Value = 0, powerFactorL3Value = 0;
static uint32_t blockEnergyKWhImportValue = 0, blockEnergyKVAhLagValue = 0, blockEnergyKVAhLeadValue = 0, blockEnergyKVAhImportValue = 0;
static uint32_t cumulativeEnergyKWhImportValue = 0, cumulativeEnergyKVAhImportValue = 0;

// Function to add the voltageL1 register to the DLMS server
int addVoltageL1()
{
    int ret;
    const unsigned char ln[6] = { 1, 0, 32, 7, 0, 255 };  // Example LN for voltageL1

    // Initialize the voltageL1 register object
    if ((ret = INIT_OBJECT(voltageL1, DLMS_OBJECT_TYPE_REGISTER, ln)) == 0)
    {
        voltageL1Value = 230;
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

// Function to get the voltageL1 registers' value
uint16_t readVoltageL1Value()
{
    voltageL1Value = 230 + (rand() % 11 - 5);
    return voltageL1Value;
}

// Function to add the voltageL2 register to the DLMS server
int addVoltageL2()
{
    int ret;
    const unsigned char ln[6] = { 1, 0, 52, 7, 0, 255 };  // LN for voltageL2

    if ((ret = INIT_OBJECT(voltageL2, DLMS_OBJECT_TYPE_REGISTER, ln)) == 0)
    {
        voltageL2Value = 230;
        GX_UINT16_BYREF(voltageL2.value, voltageL2Value);
        voltageL2.scaler = 3;
        voltageL2.unit = 35;
    }

    return ret;
}

// Function to set the voltageL2 registers' value
void writeVoltageL2Value(uint16_t value)
{
    voltageL2Value = value;
}

// Function to get the voltageL2 registers' value
uint16_t readVoltageL2Value()
{
    voltageL2Value = 230 + (rand() % 11 - 5);
    return voltageL2Value;
}

// Function to add the voltageL3 register to the DLMS server
int addVoltageL3()
{
    int ret;
    const unsigned char ln[6] = { 1, 0, 72, 7, 0, 255 };  // LN for voltageL3

    if ((ret = INIT_OBJECT(voltageL3, DLMS_OBJECT_TYPE_REGISTER, ln)) == 0)
    {
        voltageL3Value = 230;
        GX_UINT16_BYREF(voltageL3.value, voltageL3Value);
        voltageL3.scaler = 3;
        voltageL3.unit = 35;
    }

    return ret;
}

// Function to set the voltageL3 registers' value
void writeVoltageL3Value(uint16_t value)
{
    voltageL3Value = value;
}

// Function to get the voltageL3 registers' value
uint16_t readVoltageL3Value()
{
    voltageL3Value = 230 + (rand() % 11 - 5);
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

// Function to get the currentL1 registers' value
uint16_t readCurrentL1Value()
{
    currentL1Value = 10 + (rand() % 3 - 1);
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

// Function to get the currentL2 registers' value
uint16_t readCurrentL2Value()
{
    currentL2Value = 10 + (rand() % 3 - 1);
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

// Function to get the currentL3 registers' value
uint16_t readCurrentL3Value()
{
    currentL3Value = 10 + (rand() % 3 - 1);
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

// Function to get the frequency registers' value
uint16_t readFrequencyValue()
{
    frequencyValue = 50 + (rand() % 3 - 1);
    return frequencyValue;
}

// Function to add the powerFactorL1 register to the DLMS server
int addPowerFactorL1()
{
    int ret;
    const unsigned char ln[6] = { 1, 0, 33, 7, 0, 255 };  // LN for powerFactorL1

    if ((ret = INIT_OBJECT(powerFactorL1, DLMS_OBJECT_TYPE_REGISTER, ln)) == 0)
    {
        powerFactorL1Value = 100;
        GX_INT16_BYREF(powerFactorL1.value, powerFactorL1Value);
        powerFactorL1.scaler = 3;
        powerFactorL1.unit = 27;  
    }

    return ret;
}

// Function to set the powerFactorL1 registers' value
void writePowerFactorL1Value(int16_t value)
{
    powerFactorL1Value = value;
}

// Function to get the powerFactorL1 registers' value
int16_t readPowerFactorL1Value()
{
    powerFactorL1Value = 100 + (rand() % 3 - 1);
    return powerFactorL1Value;
}

// Function to add Power Factor L2 register to the DLMS server
int addPowerFactorL2()
{
    int ret;
    const unsigned char ln[6] = { 1, 0, 53, 7, 0, 255 };  // LN for powerFactorL2

    if ((ret = INIT_OBJECT(powerFactorL2, DLMS_OBJECT_TYPE_REGISTER, ln)) == 0)
    {
        powerFactorL2Value = 100; 
        GX_INT16_BYREF(powerFactorL2.value, powerFactorL2Value);
        powerFactorL2.scaler = 3;
        powerFactorL2.unit = 40;  
    }

    return ret;
}

// Function to set Power Factor L2 register's value
void writePowerFactorL2Value(int16_t value)
{
    powerFactorL2Value = value;
}

// Function to get Power Factor L2 register's value
int16_t readPowerFactorL2Value()
{
    powerFactorL2Value = 100 + (rand() % 3 - 1);
    return powerFactorL2Value;
}

// Function to add Power Factor L3 register to the DLMS server
int addPowerFactorL3()
{
    int ret;
    const unsigned char ln[6] = { 1, 0, 73, 7, 0, 255 };  // LN for powerFactorL3

    if ((ret = INIT_OBJECT(powerFactorL3, DLMS_OBJECT_TYPE_REGISTER, ln)) == 0)
    {
        powerFactorL3Value = 100; 
        GX_INT16_BYREF(powerFactorL3.value, powerFactorL3Value);
        powerFactorL3.scaler = 3;
        powerFactorL3.unit = 40;  
    }

    return ret;
}

// Function to set Power Factor L3 register's value
void writePowerFactorL3Value(int16_t value)
{
    powerFactorL3Value = value;
}

// Function to get Power Factor L3 register's value
int16_t readPowerFactorL3Value()
{
    powerFactorL3Value = 100 + (rand() % 3 - 1);
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

// Function to get Block Energy (kWh Import) register's value
uint32_t readBlockEnergyKWhImportValue()
{
    blockEnergyKWhImportValue = 1000 + (rand() % 3 - 1);
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

// Function to get Block Energy (kVAh Lag) register's value
uint32_t readBlockEnergyKVAhLagValue()
{
    blockEnergyKVAhLagValue = 1000 + (rand() % 3 - 1);
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

// Function to get Block Energy (kVAh Lead) register's value
uint32_t readBlockEnergyKVAhLeadValue()
{
    blockEnergyKVAhLeadValue = 1000 + (rand() % 3 - 1);
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

// Function to get Block Energy (kVAh Import) register's value  
uint32_t readBlockEnergyKVAhImportValue()
{
    blockEnergyKVAhImportValue = 1000 + (rand() % 3 - 1);
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

// Function to get Cumulative Energy (kWh Import) register's value
uint32_t readCumulativeEnergyKWhImportValue()
{
    cumulativeEnergyKWhImportValue = 1000 + (rand() % 3 - 1);
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

// Function to get Cumulative Energy (kVAh Import) register's value
uint32_t readCumulativeEnergyKVAhImportValue()
{
    cumulativeEnergyKVAhImportValue = 1000 + (rand() % 3 - 1);
    return cumulativeEnergyKVAhImportValue;
}
