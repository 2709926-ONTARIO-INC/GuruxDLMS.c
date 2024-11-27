#ifndef REGISTER_H__
#define REGISTER_H__

#include <stdint.h>

// Functions to add the KIGG register
int addVoltageL1(void);
int addVoltageL2(void);
int addVoltageL3(void);
int addCurrentL1(void);
int addCurrentL2(void);
int addCurrentL3(void);
int addFrequency(void);
int addPowerFactorL1(void);
int addPowerFactorL2(void);
int addPowerFactorL3(void);
int addBlockEnergyKWhImport(void);
int addBlockEnergyKVAhLag(void);
int addBlockEnergyKVAhLead(void);
int addBlockEnergyKVAhImport(void);
int addCumulativeEnergyKWhImport(void);
int addCumulativeEnergyKVAhImport(void);

// Functions to get the KIGG registers' values
uint16_t readVoltageL1Value(void);
uint16_t readVoltageL2Value(void);
uint16_t readVoltageL3Value(void);
uint16_t readCurrentL1Value(void);
uint16_t readCurrentL2Value(void);
uint16_t readCurrentL3Value(void);
uint16_t readFrequencyValue(void);
float readPowerFactorL1Value(void);
float readPowerFactorL2Value(void);
float readPowerFactorL3Value(void);
uint32_t readBlockEnergyKWhImportValue(void);
uint32_t readBlockEnergyKVAhLagValue(void);
uint32_t readBlockEnergyKVAhLeadValue(void);
uint32_t readBlockEnergyKVAhImportValue(void);
uint32_t readCumulativeEnergyKWhImportValue(void);
uint32_t readCumulativeEnergyKVAhImportValue(void);

// Functions to set the KIGG registers' values
void writeVoltageL1Value(uint16_t value);
void writeVoltageL2Value(uint16_t value);
void writeVoltageL3Value(uint16_t value);
void writeCurrentL1Value(uint16_t value);
void writeCurrentL2Value(uint16_t value);
void writeCurrentL3Value(uint16_t value);
void writeFrequencyValue(uint16_t value);
void writePowerFactorL1Value(int16_t value);
void writePowerFactorL2Value(int16_t value);
void writePowerFactorL3Value(int16_t value);
void writeBlockEnergyKWhImportValue(uint32_t value);
void writeBlockEnergyKVAhLagValue(uint32_t value);
void writeBlockEnergyKVAhLeadValue(uint32_t value);
void writeBlockEnergyKVAhImportValue(uint32_t value);
void writeCumulativeEnergyKWhImportValue(uint32_t value);
void writeCumulativeEnergyKVAhImportValue(uint32_t value);

void initializeCounters(void);
bool initializeLimits(const char* filePath);

#endif
