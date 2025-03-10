#ifndef REGISTER_H__
#define REGISTER_H__

#include <stdint.h>
#include <stdbool.h>

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
int addBlockEnergyKWhExport(void);
int addBlockEnergyKVAhLag(void);
int addBlockEnergyKVAhLead(void);
int addBlockEnergyKVAhImport(void);
int addCumulativeEnergyKWhImport(void);
int addCumulativeEnergyKWhExport(void);
int addCumulativeEnergyKVAhLag(void);
int addCumulativeEnergyKVAhLead(void);
int addCumulativeEnergyKVAhImport(void);

// Functions to add the KIGG single phase meter registers
int addNeutralCurrent(void);
int addActivePower(void);
int addApparentPower(void);
int addSignedPowerFactor(void);

// Functions to get the KIGG registers' values
uint32_t readVoltageL1Value(void);
uint32_t readVoltageL2Value(void);
uint32_t readVoltageL3Value(void);
uint32_t readCurrentL1Value(void);
uint32_t readCurrentL2Value(void);
uint32_t readCurrentL3Value(void);
uint32_t readFrequencyValue(void);
uint32_t readPowerFactorL1Value(void);
uint32_t readPowerFactorL2Value(void);
uint32_t readPowerFactorL3Value(void);
uint32_t readBlockEnergyKWhImportValue(void);
uint32_t readBlockEnergyKWhExportValue(void);
uint32_t readBlockEnergyKVAhLagValue(void);
uint32_t readBlockEnergyKVAhLeadValue(void);
uint32_t readBlockEnergyKVAhImportValue(void);
uint32_t readCumulativeEnergyKWhImportValue(void);
uint32_t readCumulativeEnergyKWhExportValue(void);
uint32_t readCumulativeEnergyKVAhLagValue(void);
uint32_t readCumulativeEnergyKVAhLeadValue(void);
uint32_t readCumulativeEnergyKVAhImportValue(void);

// Functions to get the KIGG single phase meter registers' values
uint32_t readNeutralCurrentValue(void);
uint32_t readActivePowerValue(void);
uint32_t readApparentPowerValue(void);
uint32_t readSignedPowerFactorValue(void);

// Functions to set the KIGG registers' values
void writeVoltageL1Value(uint32_t value);
void writeVoltageL2Value(uint32_t value);
void writeVoltageL3Value(uint32_t value);
void writeCurrentL1Value(uint32_t value);
void writeCurrentL2Value(uint32_t value);
void writeCurrentL3Value(uint32_t value);
void writeFrequencyValue(uint32_t value);
void writePowerFactorL1Value(uint32_t value);
void writePowerFactorL2Value(uint32_t value);
void writePowerFactorL3Value(uint32_t value);
void writeBlockEnergyKWhImportValue(uint32_t value);
void writeBlockEnergyKWhExportValue(uint32_t value);
void writeBlockEnergyKVAhLagValue(uint32_t value);
void writeBlockEnergyKVAhLeadValue(uint32_t value);
void writeBlockEnergyKVAhImportValue(uint32_t value);
void writeCumulativeEnergyKWhImportValue(uint32_t value);
void writeCumulativeEnergyKWhExportValue(uint32_t value);
void writeCumulativeEnergyKVAhLagValue(uint32_t value);
void writeCumulativeEnergyKVAhLeadValue(uint32_t value);
void writeCumulativeEnergyKVAhImportValue(uint32_t value);

// Functions to set the KIGG single phase meter registers' values
void writeNeutralCurrentValue(uint32_t value);
void writeActivePowerValue(uint32_t value);
void writeApparentPowerValue(uint32_t value);
void writeSignedPowerFactorValue(uint32_t value);

// Functions to add the KIGG average registers to the DLMS server
int addVoltageL1Average(void);
int addVoltageL2Average(void);
int addVoltageL3Average(void);
int addCurrentL1Average(void);
int addCurrentL2Average(void);
int addCurrentL3Average(void);

// Functions to get the KIGG average registers' values
uint32_t readVoltageL1AverageValue(void);
uint32_t readVoltageL2AverageValue(void);
uint32_t readVoltageL3AverageValue(void);
uint32_t readCurrentL1AverageValue(void);
uint32_t readCurrentL2AverageValue(void);
uint32_t readCurrentL3AverageValue(void);

// Functions to add nameplate profile data to the DLMS server
int addMeterSerialNumber(void);
int addManufacturerName(void);
int addFirmwareVersion(void);
int addMeterType(void);
int addMeterCategory(void);
int addCurrentRating(void);
int addCTR(void);
int addPTR(void);
int addYearOfManufacture(void);

// Functions to get the nameplate profile data values
const char* readMeterSerialNumber(void);
const char* readManufacturerName(void);
const char* readFirmwareVersion(void);
uint8_t readMeterType(void);
const char* readMeterCategory(void);
const char* readCurrentRating(void);
uint8_t readCTR(void);
uint8_t readPTR(void);
uint16_t readYearOfManufacture(void);

void updateMeterSerialNumber(int value);
void initializeCounters(void);
bool setRegisterLimits(const char* filePath);

#endif
