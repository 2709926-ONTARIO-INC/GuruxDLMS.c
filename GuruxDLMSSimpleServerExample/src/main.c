//
// --------------------------------------------------------------------------
//  Gurux Ltd
//
//
//
// Filename:        $HeadURL:  $
//
// Version:         $Revision:  $,
//                  $Date:  $
//                  $Author: $
//
// Copyright (c) Gurux Ltd
//
//---------------------------------------------------------------------------
#include <stdio.h>
#include <stdbool.h>
#include <sys/stat.h>

#if defined(_WIN32) || defined(_WIN64)//Windows includes
#if _MSC_VER > 1400
#include <crtdbg.h>
#endif
#include <tchar.h>
#include <conio.h>
#include <Winsock2.h> //Add support for sockets
#include <time.h>
#include <process.h>//Add support for threads
#include "../include/getopt.h"
#else //Linux includes.
#define closesocket close
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/socket.h> //Add support for sockets
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>
#include <fcntl.h>
#endif

#include "../include/registers.h"
#include "../../development/include/gxmem.h"
#include "../../development/include/dlmssettings.h"
#include "../../development/include/variant.h"
#include "../../development/include/cosem.h"
#include "../../development/include/server.h"
//Add this if you want to send notify messages.
#include "../../development/include/notify.h"
//Add support for serialization.
#include "../../development/include/gxserializer.h"

//DLMS settings.
dlmsServerSettings settings;

GX_TRACE_LEVEL trace = GX_TRACE_LEVEL_OFF;

const static char* FLAG_ID = "GRX";

//Space for client password.
static unsigned char PASSWORD[20] = {0};
//Space for client challenge.
static unsigned char C2S_CHALLENGE[64];
//Space for server challenge.
static unsigned char S2C_CHALLENGE[64];

unsigned char testMode = 1;
int socket1 = -1;
uint32_t SERIAL_NUMBER = 123456;

#define HDLC_HEADER_SIZE 17
#define HDLC_BUFFER_SIZE 128
#define PDU_BUFFER_SIZE 4096
#define WRAPPER_BUFFER_SIZE 8 + PDU_BUFFER_SIZE

#define STACK_SIZE 4 * 1024 * 1024  // 4 MB stack size
#define LOG_FILE_SIZE   2048U

//TODO: Allocate space where profile generic row values are serialized.
#define PDU_MAX_PROFILE_GENERIC_COLUMN_SIZE LOG_FILE_SIZE * 4U

#define LOAD_PROFILE_COUNTER    4U

//Buffer where frames are saved.
static unsigned char frameBuff[HDLC_BUFFER_SIZE + HDLC_HEADER_SIZE];
//Buffer where PDUs are saved.
static unsigned char pduBuff[PDU_BUFFER_SIZE];
static unsigned char replyFrame[HDLC_BUFFER_SIZE + HDLC_HEADER_SIZE];
//Define server system title.
static unsigned char SERVER_SYSTEM_TITLE[8] = { 0 };
static gxByteBuffer reply;

uint32_t time_current(void)
{
    //Get current time somewhere.
    return (uint32_t)time(NULL);
}

uint32_t time_elapsed(void)
{
    return (uint32_t)clock() / (CLOCKS_PER_SEC / 1000);
}

//Don't use clock as a name. Some compilers are using clock as reserved word.
static gxClock clock1;
static gxIecHdlcSetup hdlc;
static gxData ldn;
static gxData eventCode;
static gxData unixTime;
static gxData invocationCounter;
static gxAssociationLogicalName associationNone;
static gxAssociationLogicalName associationLow;
static gxAssociationLogicalName associationHigh;
static gxAssociationLogicalName associationHighGMac;
static gxRegister activePowerL1;
static gxScriptTable scriptTableGlobalMeterReset;
static gxScriptTable scriptTableDisconnectControl;
static gxScriptTable scriptTableActivateTestMode;
static gxScriptTable scriptTableActivateNormalMode;
static gxProfileGeneric eventLog;
static gxActionSchedule actionScheduleDisconnectOpen;
static gxActionSchedule actionScheduleDisconnectClose;
static gxPushSetup pushSetup;
static gxDisconnectControl disconnectControl;
static gxProfileGeneric loadProfile, dailyLoadProfile, nameplateProfile, billingProfile, instantaneousProfile;
static gxSapAssignment sapAssignment;
//Security Setup High is for High authentication.
static gxSecuritySetup securitySetupHigh;
//Security Setup HighGMac is for GMac authentication.
static gxSecuritySetup securitySetupHighGMac;

// Define external KIGG registers
#ifdef SINGLE_PHASE
// Define external KIGG registers for single phase meter
extern gxRegister neutralCurrent;
extern gxRegister activePower, apparentPower;
extern gxRegister signedPowerFactor;
extern gxRegister blockEnergyKWhExport;
extern gxRegister cumulativeEnergyKWhExport;
#elif defined(THREE_PHASE)
extern gxRegister voltageL2, voltageL3;
extern gxRegister currentL2, currentL3;
extern gxRegister powerFactorL1, powerFactorL2, powerFactorL3;
extern gxRegister blockEnergyKVAhLag, blockEnergyKVAhLead, blockEnergyKVAhImport;
extern gxRegister cumulativeEnergyKVAhLag, cumulativeEnergyKVAhLead, cumulativeEnergyKVAhImport;

// Define external KIGG Average registers
extern gxRegister voltageL2Average, voltageL3Average;
extern gxRegister currentL2Average, currentL3Average;

// Define external KIGG nameplate profile data
extern gxData ctr, ptr;
#endif

extern gxRegister voltageL1;
extern gxRegister currentL1;
extern gxRegister frequency;
extern gxRegister blockEnergyKWhImport;
extern gxRegister cumulativeEnergyKWhImport;

// Define external KIGG Average registers
extern gxRegister voltageL1Average;
extern gxRegister currentL1Average;

// Define external KIGG nameplate profile data
extern gxData meterSerialNumber, manufacturerName, firmwareVersion, meterType, meterCategory;
extern gxData currentRating, yearOfManufacture;

static gxObject *NONE_OBJECTS[] = {BASE(associationNone), BASE(ldn), BASE(sapAssignment), BASE(clock1)};

static gxObject *ALL_OBJECTS[] = {
    BASE(associationNone),
    BASE(associationLow),
    BASE(associationHigh),
    BASE(associationHighGMac),
    BASE(securitySetupHigh),
    BASE(securitySetupHighGMac),
    BASE(ldn),
    BASE(sapAssignment),
    BASE(eventCode),
    BASE(clock1),
    
    // Add KIGG registers using conditional compilation
#ifdef SINGLE_PHASE
    BASE(neutralCurrent),
    BASE(activePower),
    BASE(apparentPower),
    BASE(signedPowerFactor),
    BASE(cumulativeEnergyKWhExport),
#elif defined(THREE_PHASE)
    BASE(voltageL2), 
    BASE(voltageL3),
    BASE(currentL2),
    BASE(currentL3),
    BASE(voltageL1Average),
    BASE(voltageL2Average),
    BASE(voltageL3Average),
    BASE(currentL1Average),
    BASE(currentL2Average),
    BASE(currentL3Average),
    BASE(powerFactorL1),
    BASE(powerFactorL2),
    BASE(powerFactorL3),
    BASE(blockEnergyKWhImport),
    BASE(blockEnergyKVAhLag),
    BASE(blockEnergyKVAhLead),
    BASE(blockEnergyKVAhImport),
    BASE(cumulativeEnergyKVAhLag),
    BASE(cumulativeEnergyKVAhLead),
    BASE(cumulativeEnergyKVAhImport),
    BASE(activePowerL1), 
#endif
    BASE(voltageL1),
    BASE(currentL1), 
    BASE(frequency),
    BASE(cumulativeEnergyKWhImport),

    BASE(pushSetup),
    BASE(scriptTableGlobalMeterReset),
    BASE(scriptTableDisconnectControl),
    BASE(scriptTableActivateTestMode),
    BASE(scriptTableActivateNormalMode),
    BASE(loadProfile),
    BASE(dailyLoadProfile),
    BASE(nameplateProfile),
    BASE(billingProfile),
    BASE(eventLog),
    BASE(hdlc),
    BASE(disconnectControl),
    BASE(actionScheduleDisconnectOpen),
    BASE(actionScheduleDisconnectClose),
    BASE(unixTime),
    BASE(invocationCounter),
};

////////////////////////////////////////////////////
//Define what is serialized to decrease EEPROM usage.
gxSerializerIgnore NON_SERIALIZED_OBJECTS[] = {
    //Nothing is saved when authentication is not used.
    IGNORE_ATTRIBUTE(BASE(associationNone), GET_ATTRIBUTE_ALL()),
    //Only password is saved for low and high authentication.
    IGNORE_ATTRIBUTE(BASE(associationLow), GET_ATTRIBUTE_EXCEPT(7)),
    IGNORE_ATTRIBUTE(BASE(associationHigh), GET_ATTRIBUTE_EXCEPT(7)),
    //Nothing is saved when authentication is not used.
    IGNORE_ATTRIBUTE(BASE(associationHighGMac), GET_ATTRIBUTE_ALL()),    
    //Only scaler and unit are saved for all register objects.
    IGNORE_ATTRIBUTE_BY_TYPE(DLMS_OBJECT_TYPE_REGISTER, GET_ATTRIBUTE(2)) };

static uint32_t executeTime = 0;

static uint16_t activePowerL1Value = 0;

static bool enableGarbageValues = false;

typedef enum
{
    //Meter is powered.
    GURUX_EVENT_CODES_POWER_UP = 0x1,
    //User has change the time.
    GURUX_EVENT_CODES_TIME_CHANGE = 0x2,
    //DST status is changed.
    GURUX_EVENT_CODES_DST = 0x4,
    //Push message is sent.
    GURUX_EVENT_CODES_PUSH = 0x8,
    //Meter makes auto connect.
    GURUX_EVENT_CODES_AUTO_CONNECT = 0x10,
    //User has change the password.
    GURUX_EVENT_CODES_PASSWORD_CHANGED = 0x20,
    //Wrong password tried 3 times.
    GURUX_EVENT_CODES_WRONG_PASSWORD = 0x40,
    //Disconnect control state is changed.
    GURUX_EVENT_CODES_OUTPUT_RELAY_STATE = 0x80,
    //User has reset the meter.
    GURUX_EVENT_CODES_GLOBAL_METER_RESET = 0x100
} GURUX_EVENT_CODES;

///////////////////////////////////////////////////////////////////////
// Write trace to the serial port.
//
// This can be used for debugging.
///////////////////////////////////////////////////////////////////////
void GXTRACE(const char* str, const char* data)
{
    //Send trace to the serial port in test mode.
    if (testMode)
    {
        if (data == NULL)
        {
            printf("%s\r\n", str);
        }
        else
        {
            printf("%s %s\r\n", str, data);
        }
    }
}

///////////////////////////////////////////////////////////////////////
// Write trace to the serial port.
//
// This can be used for debugging.
///////////////////////////////////////////////////////////////////////
void GXTRACE_INT(const char* str, int32_t value)
{
    char data[10];
    sprintf(data, " %d", value);
    GXTRACE(str, data);
}

///////////////////////////////////////////////////////////////////////
// Write trace to the serial port.
//
// This can be used for debugging.
///////////////////////////////////////////////////////////////////////
void GXTRACE_LN(const char* str, uint16_t type, unsigned char* ln)
{
    char buff[30];
    sprintf(buff, "%d %d.%d.%d.%d.%d.%d", type, ln[0], ln[1], ln[2], ln[3], ln[4], ln[5]);
    GXTRACE(str, buff);
}

/////////////////////////////////////////////////////////////////////////////
// Save security settings to the EEPROM.
//
// Only updated value is saved. This is done because write to EEPROM is slow
// and there is a limit how many times value can be written to the EEPROM.
/////////////////////////////////////////////////////////////////////////////
int saveSecurity()
{
    int ret = 0;
    char fileName[64U] = {'\0'};
    snprintf(fileName, sizeof(fileName), "%s/security.raw", readMeterSerialNumber());
    //Save keys to own block in EEPROM.
#if _MSC_VER > 1400
    FILE* f = NULL;
    fopen_s(&f, fileName, "wb");
#else
    FILE* f = fopen(fileName, "wb");
#endif
    gxByteBuffer bb;
    bb_init(&bb);
    bb_capacity(&bb, 56);
    if (f != NULL)
    {
        if ((ret = bb_set(&bb, settings.base.cipher.blockCipherKey.data, 16)) == 0 &&
            (ret = bb_set(&bb, settings.base.cipher.authenticationKey.data, 16)) == 0 &&
            (ret = bb_set(&bb, settings.base.kek.data, 16)) == 0 &&
            //Save server IV.
            (ret = bb_setUInt32(&bb, settings.base.cipher.invocationCounter)) == 0 &&
            //Save last client IV.
            (ret = bb_setUInt32(&bb, securitySetupHighGMac.minimumInvocationCounter)) == 0)
        {
            fwrite(bb.data, 1, bb.size, f);
        }
        bb_clear(&bb);
        fclose(f);
    }
    else
    {
        printf("%s\r\n", "Failed to open keys file.");
    }
    return ret;
}


/////////////////////////////////////////////////////////////////////////////
// Save data to the EEPROM.
//
// Only updated value is saved. This is done because write to EEPROM is slow
// and there is a limit how many times value can be written to the EEPROM.
/////////////////////////////////////////////////////////////////////////////
int saveSettings()
{
    int ret = 0;
    char fileName[64U] = {'\0'};
    snprintf(fileName, sizeof(fileName), "%s/settings.raw", readMeterSerialNumber());
    //Save keys to own block in EEPROM.
#if _MSC_VER > 1400
    FILE* f = NULL;
    fopen_s(&f, fileName, "wb");
#else
    FILE* f = fopen(fileName, "wb");
#endif
    if (f != NULL)
    {
        gxSerializerSettings serializerSettings;
        ser_init(&serializerSettings);
        serializerSettings.stream = f;
        serializerSettings.ignoredAttributes = NON_SERIALIZED_OBJECTS;
        serializerSettings.count = sizeof(NON_SERIALIZED_OBJECTS) / sizeof(NON_SERIALIZED_OBJECTS[0]);
        ret = ser_saveObjects(&serializerSettings, ALL_OBJECTS, sizeof(ALL_OBJECTS) / sizeof(ALL_OBJECTS[0]));
        fclose(f);
    }
    else
    {
        printf("%s\r\n", "Failed to open settings file.");
    }
    return ret;
}

//Allocate profile generic buffer.
void allocateProfileGenericBuffer(const char* fileName, uint32_t size)
{
    char directoryPath[32U] = {'\0'};

    // Create directory path using meter serial number if it does not exist.
    snprintf(directoryPath, sizeof(directoryPath), "%s", readMeterSerialNumber());
#if defined(_MSC_VER)
    if (_mkdir(directoryPath) != 0 && errno != EEXIST)
    {
        printf("Failed to create directory: %s\n", directoryPath);
        return;
    }
#else
    if (mkdir(directoryPath, 0777) != 0 && errno != EEXIST)
    {
        printf("Failed to create directory: %s\n", directoryPath);
        return;
    }
#endif

    uint32_t pos;
    FILE* f = NULL;
#if _MSC_VER > 1400
    fopen_s(&f, fileName, "ab");
#else
    f = fopen(fileName, "ab");
#endif
    if (f != NULL)
    {
        fseek(f, 0, SEEK_END);
        if (ftell(f) == 0)
        {
            for (pos = 0; pos != size; ++pos)
            {
                if (fputc(0x00, f) != 0)
                {
                    printf("Error Writing to %s\n", fileName);
                    break;
                }
            }
    }
        fclose(f);
}
}

int getProfileGenericFileName(gxProfileGeneric* pg, char* fileName)
{
    char tempFileName[64U] = {'\0'}; // Temporary buffer to avoid overlap

    int ret = hlp_getLogicalNameToString(pg->base.logicalName, tempFileName);
    // Prepend the directory path to the filename.
    snprintf(fileName, 64U, "%s/%s", readMeterSerialNumber(), tempFileName);

#if defined(_WIN64)
    strcat(fileName, "64.raw");
#else // defined(_WIN32) || defined(__linux__)
    strcat(fileName, ".raw");
#endif //defined(_WIN32) || defined(__linux__)
    return ret;
}
//Returns profile generic buffer column sizes.
int getProfileGenericBufferColumnSizes(
    gxProfileGeneric* pg,
    DLMS_DATA_TYPE* dataTypes,
    uint8_t* columnSizes,
    uint16_t* rowSize)
{
    int ret = 0;
    uint8_t pos;
    gxKey* it;
    gxValueEventArg e;
    ve_init(&e);
    *rowSize = 0;
    uint16_t size;
    unsigned char type;
    //Loop capture columns and get values.
    for (pos = 0; pos != pg->captureObjects.size; ++pos)
    {
        if ((ret = arr_getByIndex(&pg->captureObjects, (uint16_t)pos, (void**)&it)) != 0)
        {
            break;
        }
        //Date time is saved in EPOCH to save space.
        if (((gxObject*)it->key)->objectType == DLMS_OBJECT_TYPE_CLOCK && ((gxTarget*)it->value)->attributeIndex == 2)
        {
            type = DLMS_DATA_TYPE_UINT32;
            size = 4;
        }
        else
        {
            e.target = (gxObject*)it->key;
            e.index = ((gxTarget*)it->value)->attributeIndex;
            if ((ret = cosem_getValue(&settings.base, &e)) != 0)
            {
                break;
            }
            if (bb_size(e.value.byteArr) != 0)
            {
                if ((ret = bb_getUInt8(e.value.byteArr, &type)) != 0)
                {
                    break;
                }
                size = bb_available(e.value.byteArr);
            }
            else
            {
                type = DLMS_DATA_TYPE_NONE;
                size = 0;
            }
        }
        if (dataTypes != NULL)
        {
            dataTypes[pos] = type;
        }
        if (columnSizes != NULL)
        {
            columnSizes[pos] = (uint8_t)size;
        }
        *rowSize += (uint16_t)size;
        ve_clear(&e);
    }
    ve_clear(&e);
    return ret;
}

//Get max row count for allocated buffer.
uint16_t getProfileGenericBufferMaxRowCount(
    gxProfileGeneric* pg)
{
    uint16_t count = 0;
    char fileName[64U];
    //Allocate space for load profile buffer.
    getProfileGenericFileName(pg, fileName);
    uint16_t rowSize = 0;
    FILE* f = NULL;
#if _MSC_VER > 1400
    fopen_s(&f, fileName, "r+b");
#else
    f = fopen(fileName, "r+b");
#endif
    if (f == NULL)
    {
        //Allocate space for the profile generic buffer.
        allocateProfileGenericBuffer(fileName, LOG_FILE_SIZE);
#if _MSC_VER > 1400
        fopen_s(&f, fileName, "r+b");
#else
        f = fopen(fileName, "r+b");
#endif
    }
    if (f != NULL)
    {
        getProfileGenericBufferColumnSizes(pg, NULL, NULL, &rowSize);
        if (rowSize != 0)
        {
            fseek(f, 0L, SEEK_END);
            count = (uint16_t)ftell(f);
            //Decrease current index and total amount of the entries.
            count -= 4;
            count /= rowSize;
        }
        fclose(f);
    }
    GXTRACE_INT("Number of rows in profile generic", count);
    return count;
}

//Get current row count for allocated buffer.
uint16_t getProfileGenericBufferEntriesInUse(gxProfileGeneric* pg)
{
    uint16_t index = 0;
    char fileName[64];
    getProfileGenericFileName(pg, fileName);
    FILE* f = NULL;
#if _MSC_VER > 1400
    fopen_s(&f, fileName, "r+b");
#else
    f = fopen(fileName, "r+b");
#endif
    if (f != NULL)
    {
        //Load current entry index from the begin of the data.
        unsigned char pduBuff[2];
        gxByteBuffer pdu;
        bb_attach(&pdu, pduBuff, 0, sizeof(pduBuff));
        if (fread(pdu.data, 1, 2, f) == 2)
        {
            pdu.size = 2;
            bb_getUInt16(&pdu, &index);
            fseek(f, 0, SEEK_SET);
            bb_empty(&pdu);
        }
        fclose(f);
    }
    return index;
}

static int openOrCreateFile(const char* filePath)
{
    FILE* file;
    // Try to open the file in read/write mode
    file = fopen(filePath, "r+b");
    if (file == NULL)
    {
        // If the file doesn't exist, create it
        if (errno == ENOENT) // File not found
        {
            printf("File not found. Creating file: %s\n", filePath);
            file = fopen(filePath, "wb+"); // Create the file
            if (file == NULL)
            {
                // Log the error if creation fails
                printf("Failed to create file: %s. Error code: %d. Description: %s\n",
                       filePath, errno, strerror(errno));
                return -1;
            }
        }
        else
        {
            // Log other errors (e.g., permission issues)
            printf("Failed to open file: %s. Error code: %d. Description: %s\n",
                   filePath, errno, strerror(errno));
            return -1;
        }
    }
    return 0; // Success
}

int captureProfileGeneric(gxProfileGeneric* pg)
{
    unsigned char pos;
    gxKey* it;
    int ret = 0;
    char fileName[64] = {'\0'};
    FILE* f = NULL;
    getProfileGenericFileName(pg, &fileName[0U]);
    printf("Filename: %s\n", fileName);
    //openOrCreateFile(fileName);
    static unsigned char pduBuff[PDU_MAX_PROFILE_GENERIC_COLUMN_SIZE];
    gxByteBuffer pdu;
    bb_attach(&pdu, pduBuff, 0, sizeof(pduBuff));
    gxValueEventArg e;
    ve_init(&e);
    GXTRACE(("Running captureProfileGeneric"), NULL);
#if _MSC_VER > 1400
    fopen_s(&f, fileName, "r+b");
#else
    f = fopen(fileName, "r+b");
#endif
    if (f != NULL)
    {
        uint16_t dataSize = 0;
        uint8_t columnSizes[11];
        DLMS_DATA_TYPE dataTypes[11];
        //Load current entry index from the begin of the data.
        uint16_t index = 0;
        #if 1
        if((NULL != strstr(fileName, "0.0.94.91.10.255.raw")))
        {
            uint8_t _data[16U];
            if (fread(&_data[0U], 1, sizeof(_data), f) == 16)
            {
                for(uint8_t i = 0U; i < 16U; i++)
                {
                    if(0U != _data[i])
                    {
                        fclose(f);
                        GXTRACE(("Name plate already present"), NULL);
                        return 0;
                    }
                }
                fseek(f, 0, SEEK_SET);
            }
        }
        #endif
        if (fread(pdu.data, 1, 2, f) == 2)
        {
            pdu.size = 2;
            bb_getUInt16(&pdu, &index);
            fseek(f, 0, SEEK_SET);
            bb_empty(&pdu);
        }
        //Current index in ring buffer.
        if (pg->profileEntries != 0)
        {
            bb_setUInt16(&pdu, (1 + index) % (pg->profileEntries));
        }
        //Update how many entries is used until buffer is full.
        if (ret == 0 && pg->entriesInUse != pg->profileEntries)
        {
            //Total amount of the entries.
            ++pg->entriesInUse;
        }
        bb_setUInt16(&pdu, (uint16_t)pg->entriesInUse);
        //Update values to the EEPROM.
        fwrite(pdu.data, 1, 4, f);
        getProfileGenericBufferColumnSizes(pg, dataTypes, columnSizes, &dataSize);
        if (index != 0 && pg->profileEntries != 0)
        {
            fseek(f, 4 + ((index % pg->profileEntries) * dataSize), SEEK_SET);
        }
        //Loop capture columns and get values.
        for (pos = 0; pos != pg->captureObjects.size; ++pos)
        {
            if ((ret = arr_getByIndex(&pg->captureObjects, pos, (void**)&it)) != 0)
            {
                break;
            }
            bb_clear(&pdu);
            //Date time is saved in EPOCH to save space.
            if ((((gxObject*)it->key)->objectType == DLMS_OBJECT_TYPE_CLOCK || ((gxObject*)it->key) == BASE(unixTime)) &&
                ((gxTarget*)it->value)->attributeIndex == 2)
            {
                e.value.ulVal = time_current();
                e.value.vt = DLMS_DATA_TYPE_UINT32;
                fwrite(&e.value.bVal, 4, 1, f);
            }
            else
            {
                e.target = (gxObject*)it->key;
                e.index = ((gxTarget*)it->value)->attributeIndex;
                e.value.byteArr = &pdu;
                e.value.vt = DLMS_DATA_TYPE_OCTET_STRING;
                if ((ret = cosem_getValue(&settings.base, &e)) != 0)
                {
                    break;
                }
                //Data type is not serialized. For that reason first byte is ignored.
                fwrite(&e.value.byteArr->data[1], e.value.byteArr->size - 1, 1, f);
            }
        }
        fclose(f);
        if (ret != 0)
        {
            //Total amount of the entries.
            --pg->entriesInUse;
        }
    }
    else
    {
        printf("Failed to open %s. Error code: %d. Description: %s.\n", fileName, errno, strerror(errno));
    }
    return ret;
}

void updateState(uint16_t value)
{
    GX_UINT16(eventCode.value) = value;
    captureProfileGeneric(&eventLog);
}

///////////////////////////////////////////////////////////////////////
//Print detailed information to the serial port.
void svr_trace(const char* str, const char* data)
{
    GXTRACE(str, data);
}
///////////////////////////////////////////////////////////////////////

void time_now(
    gxtime* value,
    unsigned char meterTime)
{
    uint32_t offset = time_current();
    time_initUnix(value, offset);
    //If date time is wanted in meter time, not in utc time.
    if (meterTime)
    {
        clock_utcToMeterTime(&clock1, value);
    }
}


///////////////////////////////////////////////////////////////////////
//This method adds example Logical Name Association object.
///////////////////////////////////////////////////////////////////////
int addAssociationNone()
{
    int ret;
    const unsigned char ln[6] = { 0, 0, 40, 0, 1, 255 };
    if ((ret = INIT_OBJECT(associationNone, DLMS_OBJECT_TYPE_ASSOCIATION_LOGICAL_NAME, ln)) == 0)
    {
        //All objects are shown also without authentication.
        // OA_ATTACH(associationNone.objectList, ALL_OBJECTS);
        //Uncomment this if you want to show only part of the objects without authentication.
        OA_ATTACH(associationNone.objectList, NONE_OBJECTS);
        associationNone.authenticationMechanismName.mechanismId = DLMS_AUTHENTICATION_NONE;
        associationNone.clientSAP = 0x10;
        //Max PDU is half of PDU size. This is for demonstration purposes only.
        associationNone.xDLMSContextInfo.maxSendPduSize = associationNone.xDLMSContextInfo.maxReceivePduSize = PDU_BUFFER_SIZE / 2;
        associationNone.xDLMSContextInfo.conformance = DLMS_CONFORMANCE_GET;
    }
    return ret;
}

///////////////////////////////////////////////////////////////////////
//This method adds example Logical Name Association object.
///////////////////////////////////////////////////////////////////////
int addAssociationLow()
{
    int ret;
    static char SECRET[20];
    strcpy(SECRET, "Gurux");
    const unsigned char ln[6] = { 0, 0, 40, 0, 2, 255 };
    if ((ret = INIT_OBJECT(associationLow, DLMS_OBJECT_TYPE_ASSOCIATION_LOGICAL_NAME, ln)) == 0)
    {
        //Only Logical Device Name is add to this Association View.
        OA_ATTACH(associationLow.objectList, ALL_OBJECTS);
        associationLow.authenticationMechanismName.mechanismId = DLMS_AUTHENTICATION_LOW;
        associationLow.clientSAP = 0x20;
        associationLow.xDLMSContextInfo.maxSendPduSize = associationLow.xDLMSContextInfo.maxReceivePduSize = PDU_BUFFER_SIZE;
        associationLow.xDLMSContextInfo.conformance = (DLMS_CONFORMANCE)(DLMS_CONFORMANCE_BLOCK_TRANSFER_WITH_ACTION |
            DLMS_CONFORMANCE_BLOCK_TRANSFER_WITH_SET_OR_WRITE |
            DLMS_CONFORMANCE_BLOCK_TRANSFER_WITH_GET_OR_READ |
            DLMS_CONFORMANCE_SET |
            DLMS_CONFORMANCE_SELECTIVE_ACCESS |
            DLMS_CONFORMANCE_ACTION |
            DLMS_CONFORMANCE_MULTIPLE_REFERENCES |
            DLMS_CONFORMANCE_GET);
        BB_ATTACH_STR(associationLow.secret, SECRET, (uint16_t)strlen(SECRET));
        associationLow.securitySetup = NULL;
    }
    return ret;
}

///////////////////////////////////////////////////////////////////////
//This method adds example Logical Name Association object for High authentication.
// UA in Indian standard.
///////////////////////////////////////////////////////////////////////
int addAssociationHigh()
{
    int ret;
    //Dedicated key.
    static unsigned char CYPHERING_INFO[20] = { 0 };
    const unsigned char ln[6] = { 0, 0, 40, 0, 3, 255 };
    if ((ret = INIT_OBJECT(associationHigh, DLMS_OBJECT_TYPE_ASSOCIATION_LOGICAL_NAME, ln)) == 0)
    {
        associationHigh.authenticationMechanismName.mechanismId = DLMS_AUTHENTICATION_HIGH;
        OA_ATTACH(associationHigh.objectList, ALL_OBJECTS);
        BB_ATTACH(associationHigh.xDLMSContextInfo.cypheringInfo, CYPHERING_INFO, 0);
        associationHigh.clientSAP = 0x12;
        associationHigh.xDLMSContextInfo.maxSendPduSize = associationHigh.xDLMSContextInfo.maxReceivePduSize = PDU_BUFFER_SIZE;
        associationHigh.xDLMSContextInfo.conformance = (DLMS_CONFORMANCE)(DLMS_CONFORMANCE_BLOCK_TRANSFER_WITH_ACTION |
            DLMS_CONFORMANCE_BLOCK_TRANSFER_WITH_SET_OR_WRITE |
            DLMS_CONFORMANCE_BLOCK_TRANSFER_WITH_GET_OR_READ |
            DLMS_CONFORMANCE_SET |
            DLMS_CONFORMANCE_SELECTIVE_ACCESS |
            DLMS_CONFORMANCE_ACTION |
            DLMS_CONFORMANCE_MULTIPLE_REFERENCES |
            DLMS_CONFORMANCE_GET);
        bb_addString(&associationHigh.secret, "Gurux");
#ifndef DLMS_IGNORE_OBJECT_POINTERS
        associationHigh.securitySetup = &securitySetupHigh;
#else
        memcpy(associationHigh.securitySetupReference, securitySetupHigh.base.logicalName, 6);
#endif //DLMS_IGNORE_OBJECT_POINTERS
    }
    return ret;
}


///////////////////////////////////////////////////////////////////////
//This method adds example Logical Name Association object for GMAC High authentication.
// UA in Indian standard.
///////////////////////////////////////////////////////////////////////
int addAssociationHighGMac()
{
    int ret;
    const unsigned char ln[6] = { 0, 0, 40, 0, 4, 255 };
    if ((ret = INIT_OBJECT(associationHighGMac, DLMS_OBJECT_TYPE_ASSOCIATION_LOGICAL_NAME, ln)) == 0)
    {
        associationHighGMac.authenticationMechanismName.mechanismId = DLMS_AUTHENTICATION_HIGH_GMAC;
        OA_ATTACH(associationHighGMac.objectList, ALL_OBJECTS);
        associationHighGMac.clientSAP = 0x1;
        associationHighGMac.xDLMSContextInfo.maxSendPduSize = associationHighGMac.xDLMSContextInfo.maxReceivePduSize = PDU_BUFFER_SIZE;
        associationHighGMac.xDLMSContextInfo.conformance = (DLMS_CONFORMANCE)(DLMS_CONFORMANCE_BLOCK_TRANSFER_WITH_ACTION |
            DLMS_CONFORMANCE_BLOCK_TRANSFER_WITH_SET_OR_WRITE |
            DLMS_CONFORMANCE_BLOCK_TRANSFER_WITH_GET_OR_READ |
            DLMS_CONFORMANCE_SET |
            DLMS_CONFORMANCE_SELECTIVE_ACCESS |
            DLMS_CONFORMANCE_ACTION |
            DLMS_CONFORMANCE_MULTIPLE_REFERENCES |
            DLMS_CONFORMANCE_GET);
        //GMAC authentication don't need password.
#ifndef DLMS_IGNORE_OBJECT_POINTERS
        associationHighGMac.securitySetup = &securitySetupHighGMac;
#else
        memcpy(associationHighGMac.securitySetupReference, securitySetupHigh.base.logicalName, 6);
#endif //DLMS_IGNORE_OBJECT_POINTERS
    }
    return ret;
}

///////////////////////////////////////////////////////////////////////
//This method adds security setup object for High authentication.
///////////////////////////////////////////////////////////////////////
int addSecuritySetupHigh()
{
    int ret;
    //Define client system title.
    static unsigned char CLIENT_SYSTEM_TITLE[8] = { 0 };
    const unsigned char ln[6] = { 0, 0, 43, 0, 1, 255 };
    if ((ret = INIT_OBJECT(securitySetupHigh, DLMS_OBJECT_TYPE_SECURITY_SETUP, ln)) == 0)
    {
        BB_ATTACH(securitySetupHigh.serverSystemTitle, SERVER_SYSTEM_TITLE, 8);
        BB_ATTACH(securitySetupHigh.clientSystemTitle, CLIENT_SYSTEM_TITLE, 8);
        securitySetupHigh.securityPolicy = DLMS_SECURITY_POLICY_NOTHING;
        securitySetupHigh.securitySuite = DLMS_SECURITY_SUITE_V0;
    }
    return ret;
}

///////////////////////////////////////////////////////////////////////
//This method adds security setup object for High GMAC authentication.
///////////////////////////////////////////////////////////////////////
int addSecuritySetupHighGMac()
{
    int ret;
    //Define client system title.
    static unsigned char CLIENT_SYSTEM_TITLE[8] = { 0 };
    const unsigned char ln[6] = { 0, 0, 43, 0, 2, 255 };
    if ((ret = INIT_OBJECT(securitySetupHighGMac, DLMS_OBJECT_TYPE_SECURITY_SETUP, ln)) == 0)
    {
        BB_ATTACH(securitySetupHighGMac.serverSystemTitle, SERVER_SYSTEM_TITLE, 8);
        BB_ATTACH(securitySetupHighGMac.clientSystemTitle, CLIENT_SYSTEM_TITLE, 8);
        //Only Authenticated encrypted connections are allowed.
        securitySetupHighGMac.securityPolicy = DLMS_SECURITY_POLICY_AUTHENTICATED_ENCRYPTED;
        securitySetupHighGMac.securitySuite = DLMS_SECURITY_SUITE_V0;
        securitySetupHighGMac.minimumInvocationCounter = 1;
    }
    return ret;
}

///////////////////////////////////////////////////////////////////////
//Add SAP Assignment object.
///////////////////////////////////////////////////////////////////////
int addSapAssignment()
{
    int ret;
    const unsigned char ln[6] = { 0, 0, 41, 0, 0, 255 };
    if ((ret = INIT_OBJECT(sapAssignment, DLMS_OBJECT_TYPE_SAP_ASSIGNMENT, ln)) == 0)
    {
        char tmp[17];
        gxSapItem* it = (gxSapItem*)gxmalloc(sizeof(gxSapItem));
        bb_init(&it->name);
        ret = sprintf(tmp, "%s%.13u", FLAG_ID, SERIAL_NUMBER);
        bb_addString(&it->name, tmp);
        it->id = 1;
        ret = arr_push(&sapAssignment.sapAssignmentList, it);
    }
    return ret;
}

///////////////////////////////////////////////////////////////////////
//Add Logical Device Name. 123456 is meter serial number.
///////////////////////////////////////////////////////////////////////
// COSEM Logical Device Name is defined as an octet-string of 16 octets.
// The first three octets uniquely identify the manufacturer of the device and it corresponds
// to the manufacturer's identification in IEC 62056-21.
// The following 13 octets are assigned by the manufacturer.
//The manufacturer is responsible for guaranteeing the uniqueness of these octets.
int addLogicalDeviceName()
{
    int ret;
    const unsigned char ln[6] = { 0, 0, 42, 0, 0, 255 };
    if ((ret = INIT_OBJECT(ldn, DLMS_OBJECT_TYPE_DATA, ln)) == 0)
    {
        char tmp[17];
        sprintf(tmp, "%s%.13u", FLAG_ID, SERIAL_NUMBER);
        var_addBytes(&ldn.value, (unsigned char*)tmp, 16);
    }
    return ret;
}

//Add event code object.
int addEventCode()
{
    int ret;
    const unsigned char ln[6] = { 0, 0, 96, 11, 0, 255 };
    if ((ret = INIT_OBJECT(eventCode, DLMS_OBJECT_TYPE_DATA, ln)) == 0)
    {
        GX_UINT16(eventCode.value) = 0;
    }
    return ret;
}

//Add unix time object.
int addUnixTime()
{
    int ret;
    const unsigned char ln[6] = { 0, 0, 1, 1, 0, 255 };
    if ((ret = INIT_OBJECT(unixTime, DLMS_OBJECT_TYPE_DATA, ln)) == 0)
    {
        //Set initial value.
        GX_UINT32(unixTime.value) = 0;
    }
    return ret;
}

//Add invocation counter object.
int addInvocationCounter()
{
    int ret;
    const unsigned char ln[6] = { 0,0,43,1,0,255 };
    if ((ret = INIT_OBJECT(invocationCounter, DLMS_OBJECT_TYPE_DATA, ln)) == 0)
    {
        //Initial invocation counter value.
        GX_UINT32_BYREF(invocationCounter.value, securitySetupHighGMac.minimumInvocationCounter);
    }
    return ret;
}

///////////////////////////////////////////////////////////////////////
//Add push setup object. (On Connectivity)
///////////////////////////////////////////////////////////////////////
int addPushSetup()
{
    int ret;
    const unsigned char ln[6] = { 0, 0, 25, 9, 0, 255 };
    if ((ret = INIT_OBJECT(pushSetup, DLMS_OBJECT_TYPE_PUSH_SETUP, ln)) == 0)
    {
        pushSetup.service = DLMS_SERVICE_TYPE_HDLC;
        gxtime* start, * end;
        start = (gxtime*)gxmalloc(sizeof(gxtime));
        end = (gxtime*)gxmalloc(sizeof(gxtime));
        //This push is sent every minute, but max 10 seconds over.
        time_init(start, -1, -1, -1, -1, -1, 0, 0, 0);
        time_init(end, -1, -1, -1, -1, -1, 10, 0, 0);
        arr_push(&pushSetup.communicationWindow, key_init(start, end));
        //This push is sent every half minute, but max 40 seconds over.
        start = (gxtime*)gxmalloc(sizeof(gxtime));
        end = (gxtime*)gxmalloc(sizeof(gxtime));
        time_init(start, -1, -1, -1, -1, -1, 30, 0, 0);
        time_init(end, -1, -1, -1, -1, -1, 40, 0, 0);
        arr_push(&pushSetup.communicationWindow, key_init(start, end));
        // Add logical device name.
        gxTarget* capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&pushSetup.pushObjectList, key_init(BASE(ldn), capture));

        // Add push object logical name. This is needed to tell structure of data to the Push listener.
        // Also capture object list can be used here.
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 1;
        capture->dataIndex = 0;
        arr_push(&pushSetup.pushObjectList, key_init(BASE(pushSetup), capture));
    }
    return ret;
}

///////////////////////////////////////////////////////////////////////
//This method adds example clock object.
///////////////////////////////////////////////////////////////////////
int addClockObject()
{
    int ret = 0;
    //Add default clock. Clock's Logical Name is 0.0.1.0.0.255.
    const unsigned char ln[6] = { 0, 0, 1, 0, 0, 255 };
    if ((ret = INIT_OBJECT(clock1, DLMS_OBJECT_TYPE_CLOCK, ln)) == 0)
    {
        //Set default values.
        time_init(&clock1.begin, -1, 3, -1, 2, 0, 0, 0, 0);
        clock1.begin.extraInfo = DLMS_DATE_TIME_EXTRA_INFO_LAST_DAY;
        time_init(&clock1.end, -1, 10, -1, 3, 0, 0, 0, 0);
        clock1.end.extraInfo = DLMS_DATE_TIME_EXTRA_INFO_LAST_DAY;
        //Meter is using UTC time zone.
        clock1.timeZone = 0;
        //Deviation is 60 minutes.
        clock1.deviation = 60;
        clock1.clockBase = DLMS_CLOCK_BASE_FREQUENCY_50;
    }
    return ret;
}

///////////////////////////////////////////////////////////////////////
//This method adds example register object.
///////////////////////////////////////////////////////////////////////
int addRegisterObject()
{
    int ret;
    const unsigned char ln[6] = { 1, 1, 21, 25, 0, 255 };
    if ((ret = INIT_OBJECT(activePowerL1, DLMS_OBJECT_TYPE_REGISTER, ln)) == 0)
    {
        activePowerL1Value = 10;
        GX_UINT16_BYREF(activePowerL1.value, activePowerL1Value);
        //10 ^ 3 =  1000
        activePowerL1.scaler = 3;
        activePowerL1.unit = 30;
    }
    return ret;
}

uint16_t readActivePowerL1Value()
{
    return ++activePowerL1Value;
}

#ifdef SINGLE_PHASE
int addRegisterNeutralCurrent()
{
    int ret = addNeutralCurrent();
    return ret;
}

int addRegisterActivePower()
{
    int ret = addActivePower();
    return ret;
}

int addRegisterApparentPower()
{
    int ret = addApparentPower();
    return ret;
}

int addRegisterSignedPowerFactor()
{
    int ret = addSignedPowerFactor();
    return ret;
}

int addRegisterBlockEnergyKWhExport()
{
    int ret = addBlockEnergyKWhExport();
    return ret;
}

int addRegisterCumulativeEnergyKWhExport()
{
    int ret = addCumulativeEnergyKWhExport();
    return ret;
}

#elif defined(THREE_PHASE)
int addRegisterVoltageL2() 
{
    int ret = addVoltageL2();
    return ret;
}

int addRegisterVoltageL3() 
{
    int ret = addVoltageL3();
    return ret;
}

int addRegisterCurrentL2() 
{
    int ret = addCurrentL2();
    return ret;
}

int addRegisterCurrentL3() 
{
    int ret = addCurrentL3();
    return ret;
}

int addRegisterPowerFactorL1() 
{
    int ret = addPowerFactorL1();
    return ret;
}

int addRegisterPowerFactorL2() 
{
    int ret = addPowerFactorL2();
    return ret;
}

int addRegisterPowerFactorL3() 
{
    int ret = addPowerFactorL3();
    return ret;
}

int addRegisterBlockEnergyKVAhLag()
{
    int ret = addBlockEnergyKVAhLag();
    return ret;
}

int addRegisterBlockEnergyKVAhLead()
{
    int ret = addBlockEnergyKVAhLead();
    return ret;
}

int addRegisterBlockEnergyKVAhImport()
{
    int ret = addBlockEnergyKVAhImport();
    return ret;
}

int addRegisterCumulativeEnergyKVAhLag()
{
    int ret = addCumulativeEnergyKVAhLag();
    return ret;
}

int addRegisterCumulativeEnergyKVAhLead()
{
    int ret = addCumulativeEnergyKVAhLead();
    return ret;
}

int addRegisterCumulativeEnergyKVAhImport()
{
    int ret = addCumulativeEnergyKVAhImport();
    return ret;
}

int addRegisterVoltageL2Average()
{
    int ret = addVoltageL2Average();
    return ret;
}

int addRegisterVoltageL3Average()
{
    int ret = addVoltageL3Average();
    return ret;
}

int addRegisterCurrentL2Average()
{
    int ret = addCurrentL2Average();
    return ret;
}

int addRegisterCurrentL3Average()
{
    int ret = addCurrentL3Average();
    return ret;
}

int addDataCTR()
{
    int ret = addCTR();
    return ret;
}

int addDataPTR()
{
    int ret = addPTR();
    return ret;
}
#endif

int addRegisterVoltageL1() 
{
    int ret = addVoltageL1();
    return ret;
}

int addRegisterCurrentL1() 
{
    int ret = addCurrentL1();
    return ret;
}

int addRegisterFrequency() 
{
    int ret = addFrequency();
    return ret;
}

int addRegisterBlockEnergyKWhImport()
{
    int ret = addBlockEnergyKWhImport();
    return ret;
}

int addRegisterCumulativeEnergyKWhImport()
{
    int ret = addCumulativeEnergyKWhImport();
    return ret;
}

int addRegisterVoltageL1Average()
{
    int ret = addVoltageL1Average();
    return ret;
}

int addRegisterCurrentL1Average()
{
    int ret = addCurrentL1Average();
    return ret;
}

int addDataMeterSerialNumber()
{
    int ret = addMeterSerialNumber();
    return ret;
}

int addDataManufacturerName()
{
    int ret = addManufacturerName();
    return ret;
}

int addDataFirmwareVersion()
{
    int ret = addFirmwareVersion();
    return ret;
}

int addDataMeterType()
{
    int ret = addMeterType();
    return ret;
}

int addDataMeterCategory()
{
    int ret = addMeterCategory();
    return ret;
}

int addDataCurrentRating()
{
    int ret = addCurrentRating();
    return ret;
}

int addDataYearOfManufacture()
{
    int ret = addYearOfManufacture();
    return ret;
}

uint16_t readEventCode()
{
    return eventCode.value.uiVal;
}
///////////////////////////////////////////////////////////////////////
//Add script table object for meter reset. This will erase the EEPROM.
///////////////////////////////////////////////////////////////////////
int addscriptTableGlobalMeterReset()
{
    int ret;
    const unsigned char ln[6] = { 0, 0, 10, 0, 0, 255 };
    if ((ret = INIT_OBJECT(scriptTableGlobalMeterReset, DLMS_OBJECT_TYPE_SCRIPT_TABLE, ln)) == 0)
    {
        gxScript* s = (gxScript*)gxmalloc(sizeof(gxScript));
        s->id = 1;
        arr_init(&s->actions);
        //Add executed script to script list.
        arr_push(&scriptTableGlobalMeterReset.scripts, s);
    }
    return ret;
}

/////////////////////////////////////////////////////////////////////
//Add script table object for disconnect control.
//Action 1 calls remote_disconnect #1 (close).
//Action 2 calls remote_connect #2(open).
///////////////////////////////////////////////////////////////////////
int addscriptTableDisconnectControl()
{
    int ret;
    const unsigned char ln[6] = { 0, 0, 10, 0, 106, 255 };
    if ((ret = INIT_OBJECT(scriptTableDisconnectControl, DLMS_OBJECT_TYPE_SCRIPT_TABLE, ln)) == 0)
    {
        gxScript* s = (gxScript*)gxmalloc(sizeof(gxScript));
        s->id = 1;
        arr_init(&s->actions);
        gxScriptAction* a = (gxScriptAction*)gxmalloc(sizeof(gxScriptAction));

        a->type = DLMS_SCRIPT_ACTION_TYPE_EXECUTE;
        a->target = BASE(disconnectControl);
        a->index = 1;
        var_init(&a->parameter);
        //Action data is Int8 zero.
        GX_INT8(a->parameter) = 0;
        arr_push(&s->actions, a);
        //Add executed script to script list.
        arr_push(&scriptTableDisconnectControl.scripts, s);

        s = (gxScript*)gxmalloc(sizeof(gxScript));
        s->id = 2;
        arr_init(&s->actions);
        a = (gxScriptAction*)gxmalloc(sizeof(gxScriptAction));
        a->type = DLMS_SCRIPT_ACTION_TYPE_EXECUTE;
        a->target = BASE(disconnectControl);
        a->index = 2;
        var_init(&a->parameter);
        //Action data is Int8 zero.
        GX_INT8(a->parameter) = 0;
        arr_push(&s->actions, a);
        //Add executed script to script list.
        arr_push(&scriptTableDisconnectControl.scripts, s);
    }
    return ret;
}

///////////////////////////////////////////////////////////////////////
//Add script table object for test mode. In test mode meter is sending trace to the serial port.
///////////////////////////////////////////////////////////////////////
int addscriptTableActivateTestMode()
{
    int ret;
    const unsigned char ln[6] = { 0, 0, 10, 0, 101, 255 };
    if ((ret = INIT_OBJECT(scriptTableActivateTestMode, DLMS_OBJECT_TYPE_SCRIPT_TABLE, ln)) == 0)
    {
        gxScript* s = (gxScript*)gxmalloc(sizeof(gxScript));
        s->id = 1;
        arr_init(&s->actions);
        //Add executed script to script list.
        arr_push(&scriptTableActivateTestMode.scripts, s);
    }
    return ret;
}

///////////////////////////////////////////////////////////////////////
//Add script table object for Normal mode. In normal mode meter is NOT sending trace to the serial port.
///////////////////////////////////////////////////////////////////////
int addscriptTableActivateNormalMode()
{
    int ret;
    const unsigned char ln[6] = { 0, 0, 10, 0, 102, 255 };
    if ((ret = INIT_OBJECT(scriptTableActivateNormalMode, DLMS_OBJECT_TYPE_SCRIPT_TABLE, ln)) == 0)
    {
        gxScript* s = (gxScript*)gxmalloc(sizeof(gxScript));
        s->id = 1;
        arr_init(&s->actions);
        //Add executed script to script list.
        arr_push(&scriptTableActivateNormalMode.scripts, s);
    }
    return ret;
}

///////////////////////////////////////////////////////////////////////
//Add profile generic (historical data) object.
///////////////////////////////////////////////////////////////////////
int addInstantaneousProfileProfileGeneric()
{
    int ret;
    const unsigned char ln[6] = {1, 0, 94, 91, 0, 255};
    if ((ret = INIT_OBJECT(instantaneousProfile, DLMS_OBJECT_TYPE_PROFILE_GENERIC, ln)) == 0)
    {
        gxTarget* capture;
        //Set default values if load the first time.
        instantaneousProfile.sortMethod = DLMS_SORT_METHOD_FIFO;
        ///////////////////////////////////////////////////////////////////
        //Add columns.
        //Add clock obect.
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&instantaneousProfile.captureObjects, key_init(&clock1, capture));
        //Add L1 voltage.
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&instantaneousProfile.captureObjects, key_init(&voltageL1, capture));

#ifdef THREE_PHASE
        //Add L2 voltage.
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&instantaneousProfile.captureObjects, key_init(&voltageL2, capture));
        //Add L3 voltage.
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&instantaneousProfile.captureObjects, key_init(&voltageL3, capture));
#endif

        //Add L1 current.
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&instantaneousProfile.captureObjects, key_init(&currentL1, capture));

#ifdef THREE_PHASE
        //Add L2 current.
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&instantaneousProfile.captureObjects, key_init(&currentL2, capture));
        //Add L3 current.
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&instantaneousProfile.captureObjects, key_init(&currentL3, capture));
#endif

#ifdef SINGLE_PHASE
        //Add neutral current.
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&instantaneousProfile.captureObjects, key_init(&neutralCurrent, capture));
        //Add signed power factor.
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&instantaneousProfile.captureObjects, key_init(&signedPowerFactor, capture));
#elif defined(THREE_PHASE)
        //Add L1 power factor.
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&instantaneousProfile.captureObjects, key_init(&powerFactorL1, capture));
        //Add L2 power factor.
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&instantaneousProfile.captureObjects, key_init(&powerFactorL2, capture));
        //Add L3 power factor.
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&instantaneousProfile.captureObjects, key_init(&powerFactorL3, capture));
#endif

        //Add frequency.
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&instantaneousProfile.captureObjects, key_init(&frequency, capture));

#ifdef SINGLE_PHASE
        //Add apparent Power
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&instantaneousProfile.captureObjects, key_init(&apparentPower, capture));
        //Add active Power
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&instantaneousProfile.captureObjects, key_init(&activePower, capture));
#endif

        //Add Cumulative Energy Wh - Import.
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&instantaneousProfile.captureObjects, key_init(&cumulativeEnergyKWhImport, capture));

#ifdef SINGLE_PHASE
        // Add Cumulative Energy Wh - Export
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&instantaneousProfile.captureObjects, key_init(&cumulativeEnergyKWhExport, capture));
#elif defined(THREE_PHASE)
        //Add Cumulative Energy VAh - Lag.
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&instantaneousProfile.captureObjects, key_init(&cumulativeEnergyKVAhLag, capture));
        //Add Cumulative Energy VAh - Lead.
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&instantaneousProfile.captureObjects, key_init(&cumulativeEnergyKVAhLead, capture));
        //Add Cumulative Energy VAh - Import.
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&instantaneousProfile.captureObjects, key_init(&cumulativeEnergyKVAhImport, capture));
#endif

        ///////////////////////////////////////////////////////////////////
        //Update amount of capture objects.
        //Set clock to sort object.
        instantaneousProfile.sortObject = BASE(clock1);
        instantaneousProfile.sortObjectAttributeIndex = 2;
        instantaneousProfile.profileEntries = getProfileGenericBufferMaxRowCount(&instantaneousProfile);
        instantaneousProfile.entriesInUse = getProfileGenericBufferEntriesInUse(&instantaneousProfile);
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////
//Add profile generic (historical data) object.
///////////////////////////////////////////////////////////////////////
int addLoadProfileProfileGeneric()
{
    int ret;
    const unsigned char ln[6] = { 1, 0, 99, 1, 0, 255 };
    if ((ret = INIT_OBJECT(loadProfile, DLMS_OBJECT_TYPE_PROFILE_GENERIC, ln)) == 0)
    {
        gxTarget* capture;
        //Set default values if load the first time.
        loadProfile.sortMethod = DLMS_SORT_METHOD_LIFO;
        ///////////////////////////////////////////////////////////////////
        //Add columns.
        //Add clock obect.
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&loadProfile.captureObjects, key_init(&clock1, capture));
        //Add L1 voltage Average.
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&loadProfile.captureObjects, key_init(&voltageL1Average, capture));

#ifdef THREE_PHASE
        //Add L2 voltage Average.
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&loadProfile.captureObjects, key_init(&voltageL2Average, capture));
        //Add L3 voltage Average.
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&loadProfile.captureObjects, key_init(&voltageL3Average, capture));
#endif

        //Add L1 current Average.
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&loadProfile.captureObjects, key_init(&currentL1Average, capture));

#ifdef THREE_PHASE
        //Add L2 current Average.
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&loadProfile.captureObjects, key_init(&currentL2Average, capture));
        //Add L3 current Average.
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&loadProfile.captureObjects, key_init(&currentL3Average, capture));
#endif

        //Add Block Energy Wh - Import.
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&loadProfile.captureObjects, key_init(&blockEnergyKWhImport, capture));

#ifdef SINGLE_PHASE
        //Add Block Energy Wh - Export.
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&loadProfile.captureObjects, key_init(&blockEnergyKWhExport, capture));
#elif defined(THREE_PHASE)
        //Add Block Energy VAh - Lag.
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&loadProfile.captureObjects, key_init(&blockEnergyKVAhLag, capture));
        //Add Block Energy VAh - Lead.
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&loadProfile.captureObjects, key_init(&blockEnergyKVAhLead, capture));
        //Add Block Energy VAh - Import.
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&loadProfile.captureObjects, key_init(&blockEnergyKVAhImport, capture));
#endif

        ///////////////////////////////////////////////////////////////////
        //Update amount of capture objects.
        //Set clock to sort object.
        loadProfile.sortObject = BASE(clock1);
        loadProfile.sortObjectAttributeIndex = 2;
        loadProfile.profileEntries = getProfileGenericBufferMaxRowCount(&loadProfile);
        loadProfile.entriesInUse = getProfileGenericBufferEntriesInUse(&loadProfile);
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////
//Add profile generic (historical data) object.
///////////////////////////////////////////////////////////////////////
int addDailyLoadProfileProfileGeneric()
{
    int ret;
    const unsigned char ln[6] = { 1, 0, 99, 2, 0, 255 };
    if ((ret = INIT_OBJECT(dailyLoadProfile, DLMS_OBJECT_TYPE_PROFILE_GENERIC, ln)) == 0)
    {
        gxTarget* capture;
        //Set default values if load the first time.
        dailyLoadProfile.sortMethod = DLMS_SORT_METHOD_LIFO;
        ///////////////////////////////////////////////////////////////////
        //Add columns.
        //Add clock obect.
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&dailyLoadProfile.captureObjects, key_init(&clock1, capture));
        //Add Cumulative Energy Wh - Import.
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&dailyLoadProfile.captureObjects, key_init(&cumulativeEnergyKWhImport, capture));

#ifdef SINGLE_PHASE
        // Add Cumulative Energy Wh - Export
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&dailyLoadProfile.captureObjects, key_init(&cumulativeEnergyKWhExport, capture));
#elif defined(THREE_PHASE)
        //Add Cumulative Energy VAh - Lag.
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&dailyLoadProfile.captureObjects, key_init(&cumulativeEnergyKVAhLag, capture));
        //Add Cumulative Energy VAh - Import.
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&dailyLoadProfile.captureObjects, key_init(&cumulativeEnergyKVAhImport, capture));
#endif

        ///////////////////////////////////////////////////////////////////
        //Update amount of capture objects.
        //Set clock to sort object.
        dailyLoadProfile.sortObject = BASE(clock1);
        dailyLoadProfile.sortObjectAttributeIndex = 2;
        dailyLoadProfile.profileEntries = getProfileGenericBufferMaxRowCount(&dailyLoadProfile);
        dailyLoadProfile.entriesInUse = getProfileGenericBufferEntriesInUse(&dailyLoadProfile);
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////
//Add profile generic (historical data) object.
///////////////////////////////////////////////////////////////////////
int addNameplateProfileProfileGeneric()
{
    int ret;
    const unsigned char ln[6] = { 0, 0, 94, 91, 10, 255 };
    if ((ret = INIT_OBJECT(nameplateProfile, DLMS_OBJECT_TYPE_PROFILE_GENERIC, ln)) == 0)
    {
        gxTarget* capture;
        //Set default values if load the first time.
        nameplateProfile.sortMethod = DLMS_SORT_METHOD_FIFO;
        ///////////////////////////////////////////////////////////////////
        //Add columns.
        //Add meter serial number object.
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&nameplateProfile.captureObjects, key_init(&meterSerialNumber, capture));
        //Add manufacturer name object.
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&nameplateProfile.captureObjects, key_init(&manufacturerName, capture));
        //Add firmware version object.
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&nameplateProfile.captureObjects, key_init(&firmwareVersion, capture));
        //Add meter type object.
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&nameplateProfile.captureObjects, key_init(&meterType, capture));
        //Add meter category object.
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&nameplateProfile.captureObjects, key_init(&meterCategory, capture));
        //Add current rating object.
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&nameplateProfile.captureObjects, key_init(&currentRating, capture));
#ifdef THREE_PHASE
        //Add ctr object
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&nameplateProfile.captureObjects, key_init(&ctr, capture));
        //Add ptr object
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&nameplateProfile.captureObjects, key_init(&ptr, capture));
#endif
        //Add year of manufacture object.
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&nameplateProfile.captureObjects, key_init(&yearOfManufacture, capture));
        ///////////////////////////////////////////////////////////////////
        //Update amount of capture objects.
        nameplateProfile.profileEntries = getProfileGenericBufferMaxRowCount(&nameplateProfile);
        nameplateProfile.entriesInUse = getProfileGenericBufferEntriesInUse(&nameplateProfile);
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////
//Add profile generic (historical data) object.
///////////////////////////////////////////////////////////////////////
int addBillingProfileProfileGeneric()
{
    int ret;
    const unsigned char ln[6] = { 1, 0, 98, 1, 0, 255 };
    if ((ret = INIT_OBJECT(billingProfile, DLMS_OBJECT_TYPE_PROFILE_GENERIC, ln)) == 0)
    {
        gxTarget* capture;
        //Set default values if load the first time.
        billingProfile.sortMethod = DLMS_SORT_METHOD_LIFO;
        ///////////////////////////////////////////////////////////////////
        //Add columns.
        //Add Cumulative Energy Wh - Import.
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&billingProfile.captureObjects, key_init(&cumulativeEnergyKWhImport, capture));

#ifdef SINGLE_PHASE
        //Add Cumulative Energy Wh - Export.
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&billingProfile.captureObjects, key_init(&cumulativeEnergyKWhExport, capture));
#elif defined(THREE_PHASE)
        //Add Cumulative Energy VAh - Lag.
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&billingProfile.captureObjects, key_init(&cumulativeEnergyKVAhLag, capture));
        //Add Cumulative Energy VAh - Lead.
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&billingProfile.captureObjects, key_init(&cumulativeEnergyKVAhLead, capture));
        //Add Cumulative Energy VAh - Import.
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&billingProfile.captureObjects, key_init(&cumulativeEnergyKVAhImport, capture));
#endif

        ///////////////////////////////////////////////////////////////////
        //Update amount of capture objects.
        billingProfile.profileEntries = getProfileGenericBufferMaxRowCount(&billingProfile);
        billingProfile.entriesInUse = getProfileGenericBufferEntriesInUse(&billingProfile);
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////
//Add profile generic (historical data) object.
///////////////////////////////////////////////////////////////////////
int addEventLogProfileGeneric()
{
    int ret;
    const unsigned char ln[6] = { 1, 0, 99, 98, 0, 255 };
    if ((ret = INIT_OBJECT(eventLog, DLMS_OBJECT_TYPE_PROFILE_GENERIC, ln)) == 0)
    {
        eventLog.sortMethod = DLMS_SORT_METHOD_FIFO;
        ///////////////////////////////////////////////////////////////////
        //Add 2 columns as default.
        gxTarget* capture;
        //Add clock obect.
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&eventLog.captureObjects, key_init(&clock1, capture));

        //Add event code.
        capture = (gxTarget*)gxmalloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&eventLog.captureObjects, key_init(&eventCode, capture));
        //Set clock to sort object.
        eventLog.sortObject = BASE(clock1);
        eventLog.sortObjectAttributeIndex = 2;
        eventLog.profileEntries = getProfileGenericBufferMaxRowCount(&eventLog);
        eventLog.entriesInUse = getProfileGenericBufferEntriesInUse(&eventLog);
    }
    return 0;
}
///////////////////////////////////////////////////////////////////////
//Add action schedule object for disconnect control to close the led.
///////////////////////////////////////////////////////////////////////
int addActionScheduleDisconnectClose()
{
    int ret;
    const unsigned char ln[6] = { 0, 0, 15, 0, 1, 255 };
    if ((ret = INIT_OBJECT(actionScheduleDisconnectClose, DLMS_OBJECT_TYPE_ACTION_SCHEDULE, ln)) == 0)
    {
        actionScheduleDisconnectClose.executedScript = &scriptTableDisconnectControl;
        actionScheduleDisconnectClose.executedScriptSelector = 1;
        actionScheduleDisconnectClose.type = DLMS_SINGLE_ACTION_SCHEDULE_TYPE1;
    }
    return ret;
}

///////////////////////////////////////////////////////////////////////
//Add action schedule object for disconnect control to open the led.
///////////////////////////////////////////////////////////////////////
int addActionScheduleDisconnectOpen()
{
    int ret;
    const unsigned char ln[6] = { 0, 0, 15, 0, 3, 255 };
    //Action schedule execution times.
    if ((ret = INIT_OBJECT(actionScheduleDisconnectOpen, DLMS_OBJECT_TYPE_ACTION_SCHEDULE, ln)) == 0)
    {
        actionScheduleDisconnectOpen.executedScript = &scriptTableDisconnectControl;
        actionScheduleDisconnectOpen.executedScriptSelector = 2;
        actionScheduleDisconnectOpen.type = DLMS_SINGLE_ACTION_SCHEDULE_TYPE1;
    }
    return ret;
}

///////////////////////////////////////////////////////////////////////
//Add Disconnect control object.
///////////////////////////////////////////////////////////////////////
int addDisconnectControl()
{
    int ret;
    const unsigned char ln[6] = { 0, 0, 96, 3, 10, 255 };
    if ((ret = INIT_OBJECT(disconnectControl, DLMS_OBJECT_TYPE_DISCONNECT_CONTROL, ln)) == 0)
    {
    }
    return ret;
}


///////////////////////////////////////////////////////////////////////
//Add IEC HDLC Setup object.
///////////////////////////////////////////////////////////////////////
int addIecHdlcSetup()
{
    int ret = 0;
    unsigned char ln[6] = { 0, 0, 22, 0, 0, 255 };
    if ((ret = INIT_OBJECT(hdlc, DLMS_OBJECT_TYPE_IEC_HDLC_SETUP, ln)) == 0)
    {
        hdlc.communicationSpeed = DLMS_BAUD_RATE_9600;
        hdlc.windowSizeReceive = hdlc.windowSizeTransmit = 1;
        hdlc.maximumInfoLengthTransmit = hdlc.maximumInfoLengthReceive = 128;
        hdlc.inactivityTimeout = 120;
        hdlc.deviceAddress = 0x10;
    }
    settings.hdlc = &hdlc;
    return ret;
}

/////////////////////////////////////////////////////////////////////////////
// Load security settings from the EEPROM.
/////////////////////////////////////////////////////////////////////////////
int loadSecurity()
{
    char fileName[64U] = {'\0'};
    snprintf(fileName, sizeof(fileName), "%s/security.raw", readMeterSerialNumber());
    int ret = 0;
    //Update keys.
#if _MSC_VER > 1400
    FILE* f = NULL;
    fopen_s(&f, fileName, "rb");
#else
    FILE* f = fopen(fileName, "rb");
#endif
    if (f != NULL)
    {
        //Check that file is not empty.
        fseek(f, 0L, SEEK_END);
        long size = ftell(f);
        if (size != 0)
        {
            fseek(f, 0L, SEEK_SET);
            gxByteBuffer bb;
            bb_init(&bb);
            bb_capacity(&bb, size);
            bb.size += fread(bb.data, 1, size, f);
            fclose(f);
            if ((ret = bb_clear(&settings.base.cipher.blockCipherKey)) != 0 ||
                (ret = bb_clear(&settings.base.cipher.authenticationKey)) != 0 ||
                (ret = bb_clear(&settings.base.kek)) != 0 ||
                (ret = bb_set2(&settings.base.cipher.blockCipherKey, &bb, 0, 16)) != 0 ||
                (ret = bb_set2(&settings.base.cipher.authenticationKey, &bb, bb.position, 16)) != 0 ||
                (ret = bb_set2(&settings.base.kek, &bb, bb.position, 16)) != 0 ||
                //load last server IC.
                (ret = bb_getUInt32(&bb, &settings.base.cipher.invocationCounter)) != 0 ||
                //load last client IC.
                (ret = bb_getUInt32(&bb, &securitySetupHighGMac.minimumInvocationCounter)) != 0)
            {
            }
            bb_clear(&bb);
            return ret;
        }
    }
    return saveSecurity();
}


/////////////////////////////////////////////////////////////////////////////
// Load data from the EEPROM.
// Returns serialization version or zero if data is not saved.
/////////////////////////////////////////////////////////////////////////////
int loadSettings()
{
    char fileName[64U] = {'\0'};
    snprintf(fileName, sizeof(fileName), "%s/settings.raw", readMeterSerialNumber());
    int ret = 0;
    //Update keys.
#if _MSC_VER > 1400
    FILE* f = NULL;
    fopen_s(&f, fileName, "rb");
#else
    FILE* f = fopen(fileName, "rb");
#endif
    if (f != NULL)
    {
        //Check that file is not empty.
        fseek(f, 0L, SEEK_END);
        long size = ftell(f);
        if (size != 0)
        {
            fseek(f, 0L, SEEK_SET);
            gxSerializerSettings serializerSettings;
            ser_init(&serializerSettings);
            serializerSettings.stream = f;
            serializerSettings.ignoredAttributes = NON_SERIALIZED_OBJECTS;
            serializerSettings.count = sizeof(NON_SERIALIZED_OBJECTS) / sizeof(NON_SERIALIZED_OBJECTS[0]);
            ret = ser_loadObjects(&settings.base, &serializerSettings, ALL_OBJECTS, sizeof(ALL_OBJECTS) / sizeof(ALL_OBJECTS[0]));
            fclose(f);
            return ret;
        }
    }
    return saveSettings();
}

//Create objects and load values from EEPROM.
int createObjects()
{
    int ret;
    OA_ATTACH(settings.base.objects, ALL_OBJECTS);
    if ((ret = addLogicalDeviceName()) != 0 ||
        (ret = addSapAssignment()) != 0 ||
        (ret = addEventCode()) != 0 ||
        (ret = addUnixTime()) != 0 ||
        (ret = addInvocationCounter()) != 0 ||
        (ret = addClockObject()) != 0 ||
        (ret = addRegisterObject()) != 0 ||
        (ret = addRegisterVoltageL1()) != 0 ||
        (ret = addRegisterCurrentL1()) != 0 ||
#ifdef SINGLE_PHASE
        (ret = addRegisterNeutralCurrent()) != 0 ||
        (ret = addRegisterActivePower()) != 0 ||
        (ret = addRegisterApparentPower()) != 0 ||
        (ret = addRegisterSignedPowerFactor()) != 0 ||
        (ret = addRegisterBlockEnergyKWhExport()) != 0 ||
        (ret = addRegisterCumulativeEnergyKWhExport()) != 0 ||
#elif defined(THREE_PHASE)
        (ret = addRegisterVoltageL2()) != 0 ||
        (ret = addRegisterVoltageL3()) != 0 ||
        (ret = addRegisterCurrentL2()) != 0 ||
        (ret = addRegisterCurrentL3()) != 0 ||
        (ret = addRegisterPowerFactorL1()) != 0 ||
        (ret = addRegisterPowerFactorL2()) != 0 ||
        (ret = addRegisterPowerFactorL3()) != 0 ||
        (ret = addRegisterBlockEnergyKVAhLag()) != 0 ||
        (ret = addRegisterBlockEnergyKVAhLead()) != 0 ||
        (ret = addRegisterBlockEnergyKVAhImport()) != 0 ||
        (ret = addRegisterCumulativeEnergyKVAhLag()) != 0 ||
        (ret = addRegisterCumulativeEnergyKVAhLead()) != 0 ||
        (ret = addRegisterCumulativeEnergyKVAhImport()) != 0 ||
        (ret = addRegisterVoltageL2Average()) != 0 ||
        (ret = addRegisterVoltageL3Average()) != 0 ||
        (ret = addRegisterCurrentL2Average()) != 0 ||
        (ret = addRegisterCurrentL3Average()) != 0 ||
        (ret = addDataCTR()) != 0 ||
        (ret = addDataPTR()) != 0 ||
#endif
        (ret = addRegisterFrequency()) != 0 ||
        (ret = addRegisterBlockEnergyKWhImport()) != 0 ||
        (ret = addRegisterCumulativeEnergyKWhImport()) != 0 ||
        (ret = addRegisterVoltageL1Average()) != 0 ||
        (ret = addRegisterCurrentL1Average()) != 0 ||
        (ret = addDataMeterSerialNumber()) != 0 ||
        (ret = addDataManufacturerName()) != 0 ||
        (ret = addDataFirmwareVersion()) != 0 ||
        (ret = addDataMeterType()) != 0 ||
        (ret = addDataMeterCategory()) != 0 ||
        (ret = addDataCurrentRating()) != 0 ||
        (ret = addDataYearOfManufacture()) != 0 ||
        (ret = addAssociationNone()) != 0 ||
        (ret = addAssociationLow()) != 0 ||
        (ret = addAssociationHigh()) != 0 ||
        (ret = addAssociationHighGMac()) != 0 ||
        (ret = addSecuritySetupHigh()) != 0 ||
        (ret = addSecuritySetupHighGMac()) != 0 ||
        (ret = addPushSetup()) != 0 ||
        (ret = addscriptTableGlobalMeterReset()) != 0 ||
        (ret = addscriptTableDisconnectControl()) != 0 ||
        (ret = addscriptTableActivateTestMode()) != 0 ||
        (ret = addscriptTableActivateNormalMode()) != 0 ||
        (ret = addInstantaneousProfileProfileGeneric()) != 0 ||
        (ret = addLoadProfileProfileGeneric()) != 0 ||
        (ret = addDailyLoadProfileProfileGeneric()) != 0 ||
        (ret = addNameplateProfileProfileGeneric()) != 0 ||
        (ret = addBillingProfileProfileGeneric()) != 0 ||
        (ret = addEventLogProfileGeneric()) != 0 ||
        (ret = addActionScheduleDisconnectOpen()) != 0 ||
        (ret = addActionScheduleDisconnectClose()) != 0 ||
        (ret = addDisconnectControl()) != 0 ||
        (ret = addIecHdlcSetup()) != 0 ||
        (ret = oa_verify(&settings.base.objects)) != 0 ||
        (ret = svr_initialize(&settings)) != 0)
    {
        GXTRACE_INT(("Failed to start the meter!"), ret);
        executeTime = 0;
        return ret;
    }
    if ((ret = loadSettings()) != 0)
    {
        GXTRACE_INT(("Failed to load settings!"), ret);
        executeTime = 0;
        return ret;
    }
    if ((ret = loadSecurity()) != 0)
    {
        GXTRACE_INT(("Failed to load security settings!"), ret);
        executeTime = 0;
        return ret;
    }
    updateState(GURUX_EVENT_CODES_POWER_UP);
    GXTRACE(("Meter started."), NULL);
    return 0;
}

//Get HDLC communication speed.
int32_t getCommunicationSpeed()
{
    int32_t ret = 9600;
    switch (hdlc.communicationSpeed)
    {
    case DLMS_BAUD_RATE_300:
        ret = 300;
        break;
    case DLMS_BAUD_RATE_600:
        ret = 600;
        break;
    case DLMS_BAUD_RATE_1200:
        ret = 1200;
        break;
    case DLMS_BAUD_RATE_2400:
        ret = 2400;
        break;
    case DLMS_BAUD_RATE_4800:
        ret = 4800;
        break;
    case DLMS_BAUD_RATE_9600:
        ret = 9600;
        break;
    case DLMS_BAUD_RATE_19200:
        ret = 19200;
        break;
    case DLMS_BAUD_RATE_38400:
        ret = 38400;
        break;
    case DLMS_BAUD_RATE_57600:
        ret = 57600;
        break;
    case DLMS_BAUD_RATE_115200:
        ret = 115200;
        break;
    }
    return ret;
}

int svr_findObject(
    dlmsSettings* settings,
    DLMS_OBJECT_TYPE objectType,
    int sn,
    unsigned char* ln,
    gxValueEventArg* e)
{
    GXTRACE_LN(("findObject"), objectType, ln);
    if (objectType == DLMS_OBJECT_TYPE_ASSOCIATION_LOGICAL_NAME)
    {
        uint16_t pos;
        objectArray objects;
        gxObject* tmp[6];
        oa_attach(&objects, tmp, sizeof(tmp) / sizeof(tmp[0]));
        objects.size = 0;
        if (oa_getObjects(&settings->objects, DLMS_OBJECT_TYPE_ASSOCIATION_LOGICAL_NAME, &objects) == 0)
        {
            gxAssociationLogicalName* a;
            for (pos = 0; pos != objects.size; ++pos)
            {
                if (oa_getByIndex(&objects, pos, (gxObject**)&a) == 0)
                {
                    if (a->clientSAP == settings->clientAddress &&
                        a->authenticationMechanismName.mechanismId == settings->authentication)
                    {
                        e->target = (gxObject*)a;
                        break;
                    }
                }
            }
        }
    }
    if (e->target == NULL)
    {
        GXTRACE_LN(("Unknown object"), objectType, ln);
    }
    return 0;
}


/**
  Find restricting object.
*/
int getRestrictingObject(dlmsSettings* settings,
    gxValueEventArg* e,
    gxObject** obj,
    short* index)
{
    int ret;
    dlmsVARIANT* it, * it2;
    if ((ret = va_getByIndex(e->parameters.Arr, 0, &it)) != 0)
    {
        return ret;
    }
    DLMS_OBJECT_TYPE ot;
    unsigned char* ln;
    if ((ret = va_getByIndex(it->Arr, 0, &it2)) != 0)
    {
        return ret;
    }
    ot = it2->iVal;
    if ((ret = va_getByIndex(it->Arr, 1, &it2)) != 0)
    {
        return ret;
    }
    ln = it2->byteArr->data;
    if ((ret = va_getByIndex(it->Arr, 3, &it2)) != 0)
    {
        return ret;
    }
    *index = it2->iVal;
    if ((ret = oa_findByLN(&settings->objects, ot, ln, obj)) != 0)
    {
        return ret;
    }
    return ret;
}

/**
  Find start index and row count using start and end date time.

  settings: DLMS settings.
             Start time.
  type: Profile Generic type.
  e: Parameters
*/
int getProfileGenericDataByRangeFromRingBuffer(
    const char* fileName,
    gxValueEventArg* e)
{
    //Get all data if there are no sort object.
    uint32_t s = 0;
    uint32_t l = 0xFFFFFFFF;
    dlmsVARIANT tmp;
    int ret = 0;
    dlmsVARIANT* it;
    uint32_t pos;
    uint32_t last = 0;
    gxObject* obj = NULL;
    short index;
    if ((ret = getRestrictingObject(&settings.base, e, &obj, &index)) != 0)
    {
        return ret;
    }
    var_init(&tmp);
    //Check sort object
    if ((ret = va_getByIndex(e->parameters.Arr, 1, &it)) != 0)
    {
        return ret;
    }
    if (it->vt == DLMS_DATA_TYPE_UINT32)
    {
        s = it->ulVal;
    }
    else
    {
        if ((ret = dlms_changeType(it->byteArr, DLMS_DATA_TYPE_DATETIME, &tmp)) != 0)
        {
            var_clear(&tmp);
            return ret;
        }
        //Start time.
        s = time_toUnixTime2(tmp.dateTime);
        var_clear(&tmp);
    }
    if ((ret = va_getByIndex(e->parameters.Arr, 2, &it)) != 0)
    {
        return ret;
    }
    if (it->vt == DLMS_DATA_TYPE_UINT32)
    {
        l = it->ulVal;
    }
    else
    {
        if ((ret = dlms_changeType(it->byteArr, DLMS_DATA_TYPE_DATETIME, &tmp)) != 0)
        {
            var_clear(&tmp);
            return ret;
        }
        l = time_toUnixTime2(tmp.dateTime);
        var_clear(&tmp);
    }

    uint32_t t;
    gxProfileGeneric* pg = (gxProfileGeneric*)e->target;
    if (pg->entriesInUse != 0)
    {
#if _MSC_VER > 1400
        FILE* f = NULL;
        fopen_s(&f, fileName, "rb");
#else
        FILE* f = fopen(fileName, "rb");
#endif
        uint16_t rowSize = 0;
        uint8_t columnSizes[10];
        DLMS_DATA_TYPE dataTypes[10];
        if (f != NULL)
        {
            getProfileGenericBufferColumnSizes(pg, dataTypes, columnSizes, &rowSize);
            //Skip current index and total amount of the entries.
            fseek(f, 4, SEEK_SET);
            for (pos = 0; pos != pg->entriesInUse; ++pos)
            {
                //Load time from EEPROM.
                fread(&t, sizeof(uint32_t), 1, f);
                //seek to begin of next row.
                fseek(f, rowSize - sizeof(uint32_t), SEEK_CUR);
                //If value is inside of start and end time.
                if (t >= s && t <= l)
                {
                    if (last == 0)
                    {
                        //Save end position if we have only one row.
                        e->transactionEndIndex = e->transactionStartIndex = 1 + pos;
                    }
                    else
                    {
                        if (last <= t)
                        {
                            e->transactionEndIndex = pos + 1;
                        }
                        else
                        {
                            //Index is one based, not zero.
                            if (e->transactionEndIndex == 0)
                            {
                                ++e->transactionEndIndex;
                            }
                            e->transactionEndIndex += pg->entriesInUse - 1;
                            e->transactionStartIndex = pos;
                            break;
                        }
                    }
                    last = t;
                }
            }
            fclose(f);
    }
}
    return ret;
}

int readProfileGeneric(
    dlmsSettings* settings,
    gxProfileGeneric* pg,
    gxValueEventArg* e)
{
    printf("Starting readProfileGeneric function.\n");
    unsigned char first = e->transactionEndIndex == 0;
    int ret = 0;
    gxArray captureObjects;
    arr_init(&captureObjects);
    char fileName[30];
    getProfileGenericFileName(pg, fileName);
    printf("ProfileGeneric file name: %s\n", fileName);

    e->byteArray = 1;
    e->handled = 1;

    if (first)
    {
        printf("First read operation detected. Selector: %d\n", e->selector);
        if (e->selector == 0)
        {
            printf("Selector is 0 - Reading all entries.\n");
            e->transactionStartIndex = 1;
            e->transactionEndIndex = pg->entriesInUse;
            printf("Transaction Start Index: %d, End Index: %d\n", e->transactionStartIndex, e->transactionEndIndex);
        }
        else if (e->selector == 1)
        {
            printf("Selector is 1 - Reading by entry range.\n");
            if ((ret = getProfileGenericDataByRangeFromRingBuffer(fileName, e)) != 0)
            {
                printf("Error getting profile generic data by range from ring buffer. Error code: %d\n", ret);
            }
            else if ((ret = cosem_getColumns(&pg->captureObjects, e->selector, &e->parameters, &captureObjects)) != 0)
            {
                printf("Error getting columns from captureObjects. Error code: %d\n", ret);
            }
        }
        else if (e->selector == 2)
        {
            printf("Selector is 2 - Reading by specific entry.\n");
            dlmsVARIANT* it;
            if ((ret = va_getByIndex(e->parameters.Arr, 0, &it)) == 0)
            {
                e->transactionStartIndex = var_toInteger(it);
                if ((ret = va_getByIndex(e->parameters.Arr, 1, &it)) == 0)
                {
                    e->transactionEndIndex = var_toInteger(it);
                }
            }
            if (ret != 0)
            {
                printf("Error reading by specific entry. Error code: %d\n", ret);
                e->transactionStartIndex = e->transactionEndIndex = 0;
            }
            else
            {
                if (e->transactionStartIndex > pg->entriesInUse)
                {
                    printf("Start index too high, adjusting to 0.\n");
                    e->transactionStartIndex = e->transactionEndIndex = 0;
                }
                if (e->transactionEndIndex > pg->entriesInUse)
                {
                    printf("End index too high, adjusting to maximum entries.\n");
                    e->transactionEndIndex = pg->entriesInUse;
                }
            }
            printf("Transaction Start Index: %d, End Index: %d\n", e->transactionStartIndex, e->transactionEndIndex);
        }
    }

    bb_clear(e->value.byteArr);
    arr_clear(&captureObjects);

    if (ret == 0 && first)
    {
        if (e->transactionEndIndex == 0)
        {
            ret = cosem_setArray(e->value.byteArr, 0);
            printf("No entries to process, setting empty array.\n");
        }
        else
        {
            ret = cosem_setArray(e->value.byteArr, (uint16_t)(e->transactionEndIndex - e->transactionStartIndex + 1));
            printf("Setting array with number of rows: %d\n", (uint16_t)(e->transactionEndIndex - e->transactionStartIndex + 1));
        }
    }

    if (ret == 0 && e->transactionEndIndex != 0)
    {
        printf("Starting to read data from file.\n");
        uint32_t pos;
        gxtime tm;
        uint16_t pduSize;
        FILE* f = NULL;

#if _MSC_VER > 1400
        if (fopen_s(&f, fileName, "rb") != 0)
        {
            printf("Failed to open %s.\r\n", fileName);
            return -1;
        }
#else
        if ((f = fopen(fileName, "rb")) == NULL)
        {
            printf("Failed to open %s. Error code: %d. Description: %s.\n", fileName, errno, strerror(errno));
            return -1;
        }
#endif

        uint16_t dataSize = 0;
        uint8_t columnSizes[11];
        DLMS_DATA_TYPE dataTypes[11];
        if (f != NULL)
        {
            getProfileGenericBufferColumnSizes(pg, dataTypes, columnSizes, &dataSize);
            printf("Data size per row: %d bytes\n", dataSize);
        }

        if (ret == 0 && dataSize != 0)
        {
            printf("Skipping current index and starting to seek file to entry %d.\n", e->transactionStartIndex);
            if (fseek(f, 4 + ((e->transactionStartIndex - 1) * dataSize), SEEK_SET) != 0)
            {
                printf("Failed to seek %s.\r\n", fileName);
                fclose(f);
                return -1;
            }

            for (pos = e->transactionStartIndex - 1; pos != e->transactionEndIndex; ++pos)
            {
                pduSize = (uint16_t)e->value.byteArr->size;
                printf("Reading entry %d\n", pos + 1);
                if ((ret = cosem_setStructure(e->value.byteArr, pg->captureObjects.size)) != 0)
                {
                    printf("Error setting structure for capture objects. Error code: %d\n", ret);
                    break;
                }

                uint8_t colIndex;
                gxKey* it;
                for (colIndex = 0; colIndex != pg->captureObjects.size; ++colIndex)
                {
                    if ((ret = arr_getByIndex(&pg->captureObjects, colIndex, (void**)&it)) == 0)
                    {
                        printf("Processing column index: %d\n", colIndex);
                        if ((((gxObject*)it->key)->objectType == DLMS_OBJECT_TYPE_CLOCK || (gxObject*)it->key == BASE(unixTime)) &&
                            ((gxTarget*)it->value)->attributeIndex == 2)
                        {
                            uint32_t time;
                            fread(&time, 4, 1, f);
                            printf("Read clock value: %u\n", time);
                            time_initUnix(&tm, time);
                            if (((gxObject*)it->key) != BASE(unixTime))
                            {
                                clock_utcToMeterTime(&clock1, &tm);
                            }
                            if ((ret = cosem_setDateTimeAsOctetString(e->value.byteArr, &tm)) != 0)
                            {
                                printf("Error converting time to octet string. Error code: %d\n", ret);
                            }
                        }
                        else
                        {
                            e->value.byteArr->data[e->value.byteArr->size] = dataTypes[colIndex];
                            ++e->value.byteArr->size;
                            fread(&e->value.byteArr->data[e->value.byteArr->size], columnSizes[colIndex], 1, f);
                            printf("Read column data (size %d): %.*s\n", columnSizes[colIndex], columnSizes[colIndex], &e->value.byteArr->data[e->value.byteArr->size]);
                            e->value.byteArr->size += columnSizes[colIndex];
                        }
                    }
                    if (ret != 0)
                    {
                        if (ret == DLMS_ERROR_CODE_OUTOFMEMORY)
                        {
                            printf("Out of memory error encountered. Resetting PDU.\n");
                            --e->transactionStartIndex;
                            e->value.byteArr->size = pduSize;
                            ret = 0;
                        }
                        else
                        {
                            printf("Error processing column index %d. Error code: %d\n", colIndex, ret);
                            break;
                        }
                    }
                }
                ++e->transactionStartIndex;
            }
            fclose(f);
            printf("Finished reading from file.\n");
        }
        else
        {
            printf("Data size is zero, no data to read.\n");
            fclose(f);
            return -1;
        }
    }
    printf("Finished readProfileGeneric function.\n");
    return ret;
}

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
void svr_preRead(
    dlmsSettings* settings,
    gxValueEventCollection* args)
{
    gxValueEventArg* e;
    int ret, pos;
    DLMS_OBJECT_TYPE type;
    for (pos = 0; pos != args->size; ++pos)
    {
        if ((ret = vec_getByIndex(args, pos, &e)) != 0)
        {
            return;
        }
        GXTRACE_LN(("svr_preRead: "), e->target->objectType, e->target->logicalName);
        //Let framework handle Logical Name read.
        if (e->index == 1)
        {
            continue;
        }

        //Get target type.
        type = (DLMS_OBJECT_TYPE)e->target->objectType;
        //Let Framework will handle Association objects and profile generic automatically.
        if (type == DLMS_OBJECT_TYPE_ASSOCIATION_LOGICAL_NAME ||
            type == DLMS_OBJECT_TYPE_ASSOCIATION_SHORT_NAME)
        {
            continue;
        }
        //Update value by one every time when user reads register.
        if (e->target == BASE(activePowerL1) && e->index == 2)
        {
            readActivePowerL1Value();
        }
#ifdef SINGLE_PHASE
        //Update value every time when user reads register.
        if (e->target == BASE(neutralCurrent) && e->index == 2)
        {
            readNeutralCurrentValue();
        }
        //Update value every time when user reads register.
        if (e->target == BASE(activePower) && e->index == 2)
        {
            readActivePowerValue();
        }
        //Update value every time when user reads register.
        if (e->target == BASE(apparentPower) && e->index == 2)
        {
            readApparentPowerValue();
        }
        //Update value every time when user reads register.
        if (e->target == BASE(signedPowerFactor) && e->index == 2)
        {
            readSignedPowerFactorValue();
        }
        //Update value every time when user reads register.
        if (e->target == BASE(blockEnergyKWhImport) && e->index == 2)
        {
            readBlockEnergyKWhExportValue();
        }
        //Update value every time when user reads register.
        if (e->target == BASE(cumulativeEnergyKWhExport) && e->index == 2)
        {
            readCumulativeEnergyKWhExportValue();
        }
#elif defined(THREE_PHASE)
        //Update value every time when user reads register.
        if (e->target == BASE(voltageL2) && e->index == 2)
        {
            readVoltageL2Value();
        }
        //Update value every time when user reads register.
        if (e->target == BASE(voltageL3) && e->index == 2)
        {
            readVoltageL3Value();
        }
        //Update value every time when user reads register.
        if (e->target == BASE(currentL2) && e->index == 2)
        {
            readCurrentL2Value();
        }
        //Update value every time when user reads register.
        if (e->target == BASE(currentL3) && e->index == 2)
        {
            readCurrentL3Value();
        }
        //Update value every time when user reads register.
        if (e->target == BASE(powerFactorL1) && e->index == 2)
        {
            readPowerFactorL1Value();
        }
        //Update value every time when user reads register.
        if (e->target == BASE(powerFactorL2) && e->index == 2)
        {
            readPowerFactorL2Value();
        }
        //Update value every time when user reads register.
        if (e->target == BASE(powerFactorL3) && e->index == 2)
        {
            readPowerFactorL3Value();
        }
        //Update value every time when user reads register.
        if (e->target == BASE(blockEnergyKVAhLag) && e->index == 2)
        {
            readBlockEnergyKVAhLagValue();
        }
        //Update value every time when user reads register.
        if (e->target == BASE(blockEnergyKVAhLead) && e->index == 2)
        {
            readBlockEnergyKVAhLeadValue();
        }
        //Update value every time when user reads register.
        if (e->target == BASE(blockEnergyKVAhImport) && e->index == 2)
        {
            readBlockEnergyKVAhImportValue();
        }
        // Update value every time when user reads register.
        if (e->target == BASE(cumulativeEnergyKVAhLag) && e->index == 2)
        {
            readCumulativeEnergyKVAhLagValue();
        }
        // Update value every time when user reads register.
        if (e->target == BASE(cumulativeEnergyKVAhLead) && e->index == 2)
        {
            readCumulativeEnergyKVAhLeadValue();
        }
        // Update value every time when user reads register.
        if (e->target == BASE(cumulativeEnergyKVAhImport) && e->index == 2)
        {
            readCumulativeEnergyKVAhImportValue();
        }
        // Read value every time when user reads register.
        if (e->target == BASE(voltageL1Average) && e->index == 2)
        {
            readVoltageL1AverageValue();
        }
        // Read value every time when user reads register.
        if (e->target == BASE(voltageL2Average) && e->index == 2)
        {
            readVoltageL2AverageValue();
        }
        // Read value every time when user reads register.
        if (e->target == BASE(voltageL3Average) && e->index == 2)
        {
            readVoltageL3AverageValue();
        }
        // Read value every time when user reads register.
        if (e->target == BASE(currentL1Average) && e->index == 2)
        {
            readCurrentL1AverageValue();
        }
        // Read value every time when user reads register.
        if (e->target == BASE(currentL2Average) && e->index == 2)
        {
            readCurrentL2AverageValue();
        }
        // Read value every time when user reads register.
        if (e->target == BASE(currentL3Average) && e->index == 2)
        {
            readCurrentL3AverageValue();
        }
        // Read value every time when user reads register.
        if (e->target == BASE(ctr) && e->index == 2)
        {
            readCTR();
        }
        // Read value every time when user reads register.
        if (e->target == BASE(ptr) && e->index == 2)
        {
            readPTR();
        }
#endif
        //Update value every time when user reads register.
        if (e->target == BASE(voltageL1) && e->index == 2)
        {
            readVoltageL1Value();
        }
        //Update value every time when user reads register.
        if (e->target == BASE(currentL1) && e->index == 2)
        {
            readCurrentL1Value();
        }
        //Update value every time when user reads register.
        if (e->target == BASE(frequency) && e->index == 2)
        {
            readFrequencyValue();
        }
        //Update value every time when user reads register.
        if (e->target == BASE(blockEnergyKWhImport) && e->index == 2)
        {
            readBlockEnergyKWhImportValue();
        }
        // Update value every time when user reads register.
        if (e->target == BASE(cumulativeEnergyKWhImport) && e->index == 2)
        {
            readCumulativeEnergyKWhImportValue();
        }
        // Read value every time when user reads register.
        if (e->target == BASE(meterSerialNumber) && e->index == 2)
        {
            readMeterSerialNumber();
        }
        // Read value every time when user reads register.
        if (e->target == BASE(manufacturerName) && e->index == 2)
        {
            readManufacturerName();
        }
        // Read value every time when user reads register.
        if (e->target == BASE(firmwareVersion) && e->index == 2)
        {
            readFirmwareVersion();
        }
        // Read value every time when user reads register.
        if (e->target == BASE(meterType) && e->index == 2)
        {
            readMeterType();
        }
        // Read value every time when user reads register.
        if (e->target == BASE(meterCategory) && e->index == 2)
        {
            readMeterCategory();
        }
        // Read value every time when user reads register.
        if (e->target == BASE(currentRating) && e->index == 2)
        {
            readCurrentRating();
        }
        // Read value every time when user reads register.
        if (e->target == BASE(yearOfManufacture) && e->index == 2)
        {
            readYearOfManufacture();
        }
        //Get time if user want to read date and time.
        if (e->target == BASE(clock1) && e->index == 2)
        {
            gxtime dt;
            time_now(&dt, 1);
            if (e->value.byteArr == NULL)
            {
                e->value.byteArr = (gxByteBuffer*)gxmalloc(sizeof(gxByteBuffer));
                bb_init(e->value.byteArr);
            }
            e->error = cosem_setDateTimeAsOctetString(e->value.byteArr, &dt);
            e->value.vt = DLMS_DATA_TYPE_OCTET_STRING;
            e->handled = 1;
        }
        else if (e->target->objectType == DLMS_OBJECT_TYPE_PROFILE_GENERIC && e->index == 2)
        {
            e->error = (DLMS_ERROR_CODE)readProfileGeneric(settings, (gxProfileGeneric*)e->target, e);
        }
        //Update Unix time.
        if (e->target == BASE(unixTime) && e->index == 2)
        {
            gxtime dt;
            time_now(&dt, 0);
            e->value.ulVal = time_toUnixTime2(&dt);
            e->value.vt = DLMS_DATA_TYPE_UINT32;
            e->handled = 1;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
void svr_preWrite(
    dlmsSettings* settings,
    gxValueEventCollection* args)
{
#if defined(_WIN32) || defined(_WIN64) || defined(__linux__)//If Windows or Linux
    char str[25];
    gxValueEventArg* e;
    int ret, pos;
    for (pos = 0; pos != args->size; ++pos)
    {
        if ((ret = vec_getByIndex(args, pos, &e)) != 0)
        {
            return;
        }
        if (e->target == BASE(clock1) && e->index == 2)
        {
            updateState(GURUX_EVENT_CODES_TIME_CHANGE);
        }
        //If client try to update low level password when high level authentication is established.
        //This is possible in Indian standard.
        else if (e->target == BASE(associationHigh) && e->index == 7)
        {
            ret = cosem_getOctetString(e->value.byteArr, &associationLow.secret);
            saveSettings();
            e->handled = 1;
        }
        hlp_getLogicalNameToString(e->target->logicalName, str);
        printf("Writing %s\r\n", str);

    }
#endif //defined(_WIN32) || defined(_WIN64) || defined(__linux__)//If Windows or Linux
}

int sendPush(
    dlmsSettings* settings,
    gxPushSetup* push)
{
    GXTRACE_LN(("sendPush"), push->base.objectType, push->base.logicalName);
    int ret = 0, pos;
    if (socket1 != -1)
    {
        message messages;
        if (push->pushObjectList.size == 0)
        {
            GXTRACE(("sendPush Failed. No objects selected."), NULL);
            return DLMS_ERROR_CODE_INVALID_PARAMETER;
        }
        mes_init(&messages);
        if ((ret = notify_generatePushSetupMessages(settings, 0, push, &messages)) == 0)
        {
            for (pos = 0; pos != messages.size; ++pos)
            {
                if (send(socket1, (const char*)messages.data[pos]->data, bb_available(messages.data[pos]), 0) == -1)
                {
                    break;
                }
                bb_clear(&reply);
            }
        }
        if (ret != 0)
        {
            GXTRACE(("generatePushSetupMessages Failed."), NULL);
        }
        mes_clear(&messages);
    }
    return ret;
}

int sendEventNotification(dlmsSettings* settings)
{
    GXTRACE_LN(("sendEventNotification"), eventLog.base.objectType, eventLog.base.logicalName);
    int ret = 0, pos;
    if (socket1 != -1)
    {
        gxListItem item;
        item.key = BASE(eventLog);
        item.value = 2;
        message messages;
        mes_init(&messages);
        variantArray values;
        va_init(&values);
        gxtime dt;
        time_now(&dt, 1);
        dlmsVARIANT* tmp = (dlmsVARIANT*)gxmalloc(sizeof(dlmsVARIANT));
        GX_DATETIME(*tmp) = &dt;
        va_push(&values, tmp);
        tmp = (dlmsVARIANT*)gxmalloc(sizeof(dlmsVARIANT));
        GX_UINT16(*tmp) = activePowerL1Value;
        va_push(&values, tmp);
        gxByteBuffer pdu;
        bb_init(&pdu);
        if ((ret = notify_generateEventNotificationMessages(settings, 0, &item, &values, &pdu, &messages)) == 0)
        {
            for (pos = 0; pos != messages.size; ++pos)
            {
                if (send(socket1, (const char*)messages.data[pos]->data, bb_available(messages.data[pos]), 0) == -1)
                {
                    break;
                }
                bb_clear(&reply);
            }
        }
        bb_clear(&pdu);
        if (ret != 0)
        {
            GXTRACE(("generatePushSetupMessages Failed."), NULL);
        }
        mes_clear(&messages);
    }
    return ret;
}

//This example sends data in the structure.
int sendEventNotification2(dlmsSettings* settings)
{
    GXTRACE_LN(("sendEventNotification2"), eventLog.base.objectType, eventLog.base.logicalName);
    int ret = 0, pos;
    if (socket1 != -1)
    {
        gxListItem item;
        item.key = BASE(eventLog);
        item.value = 2;
        message messages;
        mes_init(&messages);
        gxByteBuffer data;
        bb_init(&data);
        gxByteBuffer pdu;
        bb_init(&pdu);
        gxtime dt;
        time_now(&dt, 1);
        //Data is send in structure. Amount of the items in structure is 2.
        cosem_setStructure(&data, 2);
        cosem_setDateAsOctetString(&data, &dt);
        cosem_setUInt16(&data, activePowerL1Value);
        if ((ret = notify_generateEventNotificationMessages2(settings, 0, &item, &data, &pdu, &messages)) == 0)
        {
            for (pos = 0; pos != messages.size; ++pos)
            {
                if (send(socket1, (const char*)messages.data[pos]->data, bb_available(messages.data[pos]), 0) == -1)
                {
                    break;
                }
                bb_clear(&reply);
            }
        }
        bb_clear(&pdu);
        bb_clear(&data);
        if (ret != 0)
        {
            GXTRACE(("generatePushSetupMessages Failed."), NULL);
        }
        mes_clear(&messages);
    }
    return ret;
}

void handleProfileGenericActions(
    gxValueEventArg* it)
{
    if (it->index == 1)
    {
        char fileName[30];
        getProfileGenericFileName((gxProfileGeneric*)it->target, fileName);
        // Profile generic clear is called. Clear data.
        ((gxProfileGeneric*)it->target)->entriesInUse = 0;
        FILE* f = NULL;
#if _MSC_VER > 1400
        fopen_s(&f, fileName, "r+b");
#else
        f = fopen(fileName, "r+b");
#endif
        if (f != NULL)
        {
            gxByteBuffer pdu;
            bb_init(&pdu);
            //Current index in ring buffer.
            bb_setUInt16(&pdu, 0);
            bb_setUInt16(&pdu, 0);
            //Update values to the EEPROM.
            fwrite(pdu.data, 1, 4, f);
            fclose(f);
        }
    }
    else if (it->index == 2)
    {
        //Increase power value before each load profile read to increase the value.
        //This is needed for demo purpose only.
        #if 0
        if (it->target == BASE(loadProfile))
        {
            readActivePowerL1Value();
        }
        captureProfileGeneric(((gxProfileGeneric*)it->target));
        #endif
    }
    saveSettings();
}

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
void svr_preAction(
    dlmsSettings* settings,
    gxValueEventCollection* args)
{
    char fileName[64U] = {'\0'};
    snprintf(fileName, sizeof(fileName), "%s/settings.raw", readMeterSerialNumber());
    gxValueEventArg* e;
    int ret, pos;
    for (pos = 0; pos != args->size; ++pos)
    {
        if ((ret = vec_getByIndex(args, pos, &e)) != 0)
        {
            return;
        }
        GXTRACE_LN(("svr_preAction: "), e->target->objectType, e->target->logicalName);
        if (e->target->objectType == DLMS_OBJECT_TYPE_PROFILE_GENERIC)
        {
            handleProfileGenericActions(e);
            e->handled = 1;
        }
        else if (e->target == BASE(activePowerL1))
        {
            //Set default value for active power.
            activePowerL1Value = 0;
            e->handled = 1;
        }
        else if (e->target == BASE(pushSetup) && e->index == 1)
        {
            updateState(GURUX_EVENT_CODES_PUSH);
            sendPush(settings, (gxPushSetup*)e->target);
            e->handled = 1;
        }
        //If client wants to clear EEPROM data using Global meter reset script.
        else if (e->target == BASE(scriptTableGlobalMeterReset) && e->index == 1)
        {
            //Initialize data size so default values are used on next connection.
#if _MSC_VER > 1400
            FILE* f = NULL;
            fopen_s(&f, fileName, "wb");
#else
            FILE* f = fopen(fileName, "wb");
#endif
            if (f != NULL)
            {
                fclose(f);
            }
            //Load objects again.
            if ((ret = loadSettings()) != 0)
            {
                GXTRACE_INT(("Failed to load settings!"), ret);
                executeTime = 0;
                e->error = ret;
                break;
            }
            if ((ret = loadSecurity()) != 0)
            {
                GXTRACE_INT(("Failed to load security settings!"), ret);
                executeTime = 0;
                e->error = ret;
                break;
            }
            updateState(GURUX_EVENT_CODES_GLOBAL_METER_RESET);
            e->handled = 1;
        }
        else if (e->target == BASE(disconnectControl))
        {
            updateState(GURUX_EVENT_CODES_OUTPUT_RELAY_STATE);
            //Disconnect. Turn led OFF.
            if (e->index == 1)
            {
                printf("%s\r\n", "Led is OFF.");
            }
            else //Reconnnect. Turn LED ON.
            {
                printf("%s\r\n", "Led is ON.");
            }
        }
        else if (e->target == BASE(scriptTableActivateTestMode))
        {
            //Activate test mode.
            testMode = 1;
            saveSettings();
        }
        else if (e->target == BASE(scriptTableActivateNormalMode))
        {
            //Activate normal mode.
            testMode = 0;
            saveSettings();
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
void svr_postRead(
    dlmsSettings* settings,
    gxValueEventCollection* args)
{
    gxValueEventArg* e;
    int ret, pos;
    for (pos = 0; pos != args->size; ++pos)
    {
        if ((ret = vec_getByIndex(args, pos, &e)) != 0)
        {
            return;
        }
        GXTRACE_LN(("svr_postRead: "), e->target->objectType, e->target->logicalName);
    }
}
/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
void svr_postWrite(
    dlmsSettings* settings,
    gxValueEventCollection* args)
{
    gxValueEventArg* e;
    int ret, pos;
    for (pos = 0; pos != args->size; ++pos)
    {
        if ((ret = vec_getByIndex(args, pos, &e)) != 0)
        {
            return;
        }
        GXTRACE_LN(("svr_postWrite: "), e->target->objectType, e->target->logicalName);
        if (e->target->objectType == DLMS_OBJECT_TYPE_PROFILE_GENERIC)
        {
            //Use want to change capture objects.
            if (e->index == 3)
            {
                saveSettings();
                //Clear buffer if user changes captured objects.
                gxValueEventArg it;
                ve_init(&it);
                it.index = 1;
                handleProfileGenericActions(&it);
                //Count how many rows fit to the buffer.
                ((gxProfileGeneric*)e->target)->profileEntries = getProfileGenericBufferMaxRowCount(((gxProfileGeneric*)e->target));
                if (((gxProfileGeneric*)e->target)->captureObjects.size != 0)
                {
                    gxKey* k = NULL;
                    arr_getByIndex(&((gxProfileGeneric*)e->target)->captureObjects, 0, (void**)&k);
                    //Set 1st object to sort object.
                    ((gxProfileGeneric*)e->target)->sortObject = (gxObject*)k->key;
                }
                else
                {
                    ((gxProfileGeneric*)e->target)->sortObject = NULL;
                }
            }
            //Use want to change max amount of profile entries.
            else if (e->index == 8)
            {
                //Count how many rows fit to the buffer.
                uint16_t maxCount = getProfileGenericBufferMaxRowCount(((gxProfileGeneric*)e->target));
                //If use try to set max profileEntries bigger than can fit to allocated memory.
                if (maxCount < ((gxProfileGeneric*)e->target)->profileEntries)
                {
                    ((gxProfileGeneric*)e->target)->profileEntries = maxCount;
                }
            }
        }
        if (e->error == 0)
        {
            //Save settings to EEPROM.
            saveSettings();
        }
        else
        {
            //Reject changes loading previous settings if there is an error.
            loadSettings();
        }
    }
    //Reset execute time to update execute time if user add new execute times or changes the time.
    executeTime = 0;
}

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
void svr_postAction(
    dlmsSettings* settings,
    gxValueEventCollection* args)
{
    gxValueEventArg* e;
    int ret, pos;
    for (pos = 0; pos != args->size; ++pos)
    {
        if ((ret = vec_getByIndex(args, pos, &e)) != 0)
        {
            return;
        }
        GXTRACE_LN(("svr_postAction: "), e->target->objectType, e->target->logicalName);
        if (e->target == BASE(securitySetupHigh) ||
            e->target == BASE(securitySetupHighGMac))
        {
            //Update block cipher key authentication key or broadcast key.
            //Save settings to EEPROM.
            if (e->error == 0)
            {
                saveSecurity();
            }
            else
            {
                //Load default settings if there is an error.
                loadSecurity();
            }
        }
        //Check is client changing the settings with action.
        else if (svr_isChangedWithAction(e->target->objectType, e->index))
        {
            //Save settings to EEPROM.
            if (e->error == 0)
            {
                saveSettings();
            }
            else
            {
                //Load default settings if there is an error.
                loadSettings();
            }
        }
    }
}

unsigned char svr_isTarget(
    dlmsSettings* settings,
    unsigned long serverAddress,
    unsigned long clientAddress)
{
    GXTRACE(("svr_isTarget."), NULL);
    objectArray objects;
    oa_init(&objects);
    unsigned char ret = 0;
    uint16_t pos;
    gxObject* tmp[6];
    oa_attach(&objects, tmp, sizeof(tmp) / sizeof(tmp[0]));
    objects.size = 0;
    if (oa_getObjects(&settings->objects, DLMS_OBJECT_TYPE_ASSOCIATION_LOGICAL_NAME, &objects) == 0)
    {
        gxAssociationLogicalName* a;
        for (pos = 0; pos != objects.size; ++pos)
        {
            if (oa_getByIndex(&objects, pos, (gxObject**)&a) == 0)
            {
                if (a->clientSAP == clientAddress)
                {
                    ret = 1;
                    switch (a->authenticationMechanismName.mechanismId)
                    {
                    case DLMS_AUTHENTICATION_NONE:
                        //Client connects without authentication.
                        GXTRACE(("Connecting without authentication."), NULL);
                        break;
                    case DLMS_AUTHENTICATION_LOW:
                        //Client connects using low authentication.
                        GXTRACE(("Connecting using Low authentication."), NULL);
                        break;
                    default:
                        //Client connects using High authentication.
                        GXTRACE(("Connecting using High authentication."), NULL);
                        break;
                    }
                    settings->proposedConformance = a->xDLMSContextInfo.conformance;
                    settings->expectedClientSystemTitle = NULL;
                    //Set Invocation counter value.
                    settings->expectedInvocationCounter = NULL;
                    //Client can establish a ciphered connection only with Security Suite 1.
                    settings->expectedSecuritySuite = 0;
                    //Security policy is not defined by default. Client can connect using any security policy.
                    settings->expectedSecurityPolicy = 0xFF;
                    if (a->securitySetup != NULL)
                    {
                        //Set expected client system title. If this is set only client that is using expected client system title can connect to the meter.
                        if (a->securitySetup->clientSystemTitle.size == 8)
                        {
                            settings->expectedClientSystemTitle = a->securitySetup->clientSystemTitle.data;
                        }
                        //GMac authentication uses innocation counter.
                        if (a->securitySetup == &securitySetupHighGMac)
                        {
                            //Set invocation counter value. If this is set client's invocation counter must match with server IC.
                            settings->expectedInvocationCounter = &securitySetupHighGMac.minimumInvocationCounter;
                        }

                        //Set security suite that client must use.
                        settings->expectedSecuritySuite = a->securitySetup->securitySuite;
                        //Set security policy that client must use if it is set.
                        if (a->securitySetup->securityPolicy != 0)
                        {
                            settings->expectedSecurityPolicy = a->securitySetup->securityPolicy;
                        }
                    }
                    break;
                }
            }
        }
    }
    if (ret == 0)
    {
        GXTRACE_INT(("Invalid authentication level."), clientAddress);
        //Authentication is now allowed. Meter is quiet and doesn't return an error.
    }
    else
    {
        // If address is not broadcast or serial number.
        //Remove logical address from the server address.
        unsigned char broadcast = (serverAddress & 0x3FFF) == 0x3FFF || (serverAddress & 0x7F) == 0x7F;
        if (!(broadcast ||
            (serverAddress & 0x3FFF) == SERIAL_NUMBER % 10000 + 1000))
        {
            ret = 0;
            // Find address from the SAP table.
            gxSapAssignment* sap;
            objects.size = 0;
            if (oa_getObjects(&settings->objects, DLMS_OBJECT_TYPE_SAP_ASSIGNMENT, &objects) == 0)
            {
                gxSapItem* it;
                uint16_t sapIndex, pos;
                for (sapIndex = 0; sapIndex != objects.size; ++sapIndex)
                {
                    if (oa_getByIndex(&objects, sapIndex, (gxObject**)&sap) == 0)
                    {
                        for (pos = 0; pos != sap->sapAssignmentList.size; ++pos)
                        {
                            if (arr_getByIndex(&sap->sapAssignmentList, pos, (void**)&it) == 0)
                            {
                                //Check server address with one byte.
                                if (((serverAddress & 0xFFFFFF00) == 0 && (serverAddress & 0x7F) == it->id) ||
                                    //Check server address with two bytes.
                                    ((serverAddress & 0xFFFF0000) == 0 && (serverAddress & 0x7FFF) == it->id))
                                {
                                    ret = 1;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            oa_empty(&objects);
        }
        //Set serial number as meter address if broadcast is used.
        if (broadcast)
        {
            settings->serverAddress = SERIAL_NUMBER % 10000 + 1000;
        }
        if (ret == 0)
        {
            GXTRACE_INT(("Invalid server address"), serverAddress);
        }
    }
    return ret;
}

DLMS_SOURCE_DIAGNOSTIC svr_validateAuthentication(
    dlmsServerSettings* settings,
    DLMS_AUTHENTICATION authentication,
    gxByteBuffer* password)
{
    GXTRACE(("svr_validateAuthentication"), NULL);
    if (authentication == DLMS_AUTHENTICATION_NONE)
    {
        //Uncomment this if authentication is always required.
        //return DLMS_SOURCE_DIAGNOSTIC_AUTHENTICATION_MECHANISM_NAME_REQUIRED;
        return DLMS_SOURCE_DIAGNOSTIC_NONE;
    }
    //Check Low Level security..
    if (authentication == DLMS_AUTHENTICATION_LOW)
    {
        if (bb_compare(password, associationLow.secret.data, associationLow.secret.size) == 0)
        {
            char* str = bb_toString(password);
            GXTRACE(("Low level password passed."), str);
            gxfree(str);
            GXTRACE_INT("Password length.", associationLow.secret.size);
            GXTRACE(("Invalid low level password."), (const char*)associationLow.secret.data);
            return DLMS_SOURCE_DIAGNOSTIC_AUTHENTICATION_FAILURE;
        }
    }
    // Hith authentication levels are check on phase two.
    return DLMS_SOURCE_DIAGNOSTIC_NONE;
}


//Get attribute access level for profile generic.
DLMS_ACCESS_MODE getProfileGenericAttributeAccess(
    dlmsSettings* settings,
    gxObject* obj,
    unsigned char index)
{
    //Only read is allowed for event log.
    if (obj == BASE(eventLog))
    {
        return DLMS_ACCESS_MODE_READ;
    }
    //Write is allowed only for High authentication.
    if (settings->authentication > DLMS_AUTHENTICATION_LOW)
    {
        switch (index)
        {
        case 3://captureObjects.
            return DLMS_ACCESS_MODE_READ_WRITE;
        case 4://capturePeriod
            return DLMS_ACCESS_MODE_READ_WRITE;
        case 8://Profile entries.
            return DLMS_ACCESS_MODE_READ_WRITE;
        default:
            break;
        }
    }
    return DLMS_ACCESS_MODE_READ;
}


//Get attribute access level for Push Setup.
DLMS_ACCESS_MODE getPushSetupAttributeAccess(
    dlmsSettings* settings,
    unsigned char index)
{
    //Write is allowed only for High authentication.
    if (settings->authentication > DLMS_AUTHENTICATION_LOW)
    {
        switch (index)
        {
        case 2://pushObjectList
        case 4://communicationWindow
            return DLMS_ACCESS_MODE_READ_WRITE;
        default:
            break;
        }
    }
    return DLMS_ACCESS_MODE_READ;
}

//Get attribute access level for Disconnect Control.
DLMS_ACCESS_MODE getDisconnectControlAttributeAccess(
    dlmsSettings* settings,
    unsigned char index)
{
    return DLMS_ACCESS_MODE_READ;
}

//Get attribute access level for register schedule.
DLMS_ACCESS_MODE getActionSchduleAttributeAccess(
    dlmsSettings* settings,
    unsigned char index)
{
    //Write is allowed only for High authentication.
    if (settings->authentication > DLMS_AUTHENTICATION_LOW)
    {
        switch (index)
        {
        case 4://Execution time.
            return DLMS_ACCESS_MODE_READ_WRITE;
        default:
            break;
        }
    }
    return DLMS_ACCESS_MODE_READ;
}

//Get attribute access level for register.
DLMS_ACCESS_MODE getRegisterAttributeAccess(
    dlmsSettings* settings,
    unsigned char index)
{
    return DLMS_ACCESS_MODE_READ;
}

//Get attribute access level for data objects.
DLMS_ACCESS_MODE getDataAttributeAccess(
    dlmsSettings* settings,
    unsigned char index)
{
    return DLMS_ACCESS_MODE_READ;
}

//Get attribute access level for script table.
DLMS_ACCESS_MODE getScriptTableAttributeAccess(
    dlmsSettings* settings,
    unsigned char index)
{
    return DLMS_ACCESS_MODE_READ;
}

//Get attribute access level for IEC HDLS setup.
DLMS_ACCESS_MODE getHdlcSetupAttributeAccess(
    dlmsSettings* settings,
    unsigned char index)
{
    //Write is allowed only for High authentication.
    if (settings->authentication > DLMS_AUTHENTICATION_LOW)
    {
        switch (index)
        {
        case 2: //Communication speed.
        case 7:
        case 8:
            return DLMS_ACCESS_MODE_READ_WRITE;
        default:
            break;
        }
    }
    return DLMS_ACCESS_MODE_READ;
}


//Get attribute access level for association LN.
DLMS_ACCESS_MODE getAssociationAttributeAccess(
    dlmsSettings* settings,
    unsigned char index)
{
    //If secret
    if (settings->authentication == DLMS_AUTHENTICATION_LOW && index == 7)
    {
        return DLMS_ACCESS_MODE_READ_WRITE;
    }
    return DLMS_ACCESS_MODE_READ;
}

//Get attribute access level for security setup.
DLMS_ACCESS_MODE getSecuritySetupAttributeAccess(
    dlmsSettings* settings,
    unsigned char index)
{
    //Only client system title is writable.
    if (settings->authentication > DLMS_AUTHENTICATION_LOW && index == 4)
    {
        return DLMS_ACCESS_MODE_READ_WRITE;
    }
    return DLMS_ACCESS_MODE_READ;
}

/**
* Get attribute access level.
*/
DLMS_ACCESS_MODE svr_getAttributeAccess(
    dlmsSettings* settings,
    gxObject* obj,
    unsigned char index)
{
    GXTRACE("svr_getAttributeAccess", NULL);
    // Only read is allowed if authentication is not used.
    if (index == 1 || settings->authentication == DLMS_AUTHENTICATION_NONE)
    {
        return DLMS_ACCESS_MODE_READ;
    }
    if (obj->objectType == DLMS_OBJECT_TYPE_ASSOCIATION_LOGICAL_NAME)
    {
        return getAssociationAttributeAccess(settings, index);
    }
    if (obj->objectType == DLMS_OBJECT_TYPE_PROFILE_GENERIC)
    {
        return getProfileGenericAttributeAccess(settings, obj, index);
    }
    if (obj->objectType == DLMS_OBJECT_TYPE_PUSH_SETUP)
    {
        return getPushSetupAttributeAccess(settings, index);
    }
    if (obj->objectType == DLMS_OBJECT_TYPE_DISCONNECT_CONTROL)
    {
        return getDisconnectControlAttributeAccess(settings, index);
    }
    if (obj->objectType == DLMS_OBJECT_TYPE_DISCONNECT_CONTROL)
    {
        return getDisconnectControlAttributeAccess(settings, index);
    }
    if (obj->objectType == DLMS_OBJECT_TYPE_ACTION_SCHEDULE)
    {
        return getActionSchduleAttributeAccess(settings, index);
    }
    if (obj->objectType == DLMS_OBJECT_TYPE_SCRIPT_TABLE)
    {
        return getScriptTableAttributeAccess(settings, index);
    }
    if (obj->objectType == DLMS_OBJECT_TYPE_REGISTER)
    {
        return getRegisterAttributeAccess(settings, index);
    }
    if (obj->objectType == DLMS_OBJECT_TYPE_DATA)
    {
        return getDataAttributeAccess(settings, index);
    }
    if (obj->objectType == DLMS_OBJECT_TYPE_IEC_HDLC_SETUP)
    {
        return getHdlcSetupAttributeAccess(settings, index);
    }
    if (obj->objectType == DLMS_OBJECT_TYPE_SECURITY_SETUP)
    {
        return getSecuritySetupAttributeAccess(settings, index);
    }
    // Only clock write is allowed.
    if (settings->authentication == DLMS_AUTHENTICATION_LOW)
    {
        if (obj->objectType == DLMS_OBJECT_TYPE_CLOCK)
        {
            return DLMS_ACCESS_MODE_READ_WRITE;
        }
        return DLMS_ACCESS_MODE_READ;
    }
    // All writes are allowed.
    return DLMS_ACCESS_MODE_READ_WRITE;
}

/**
* Get method access level.
*/
DLMS_METHOD_ACCESS_MODE svr_getMethodAccess(
    dlmsSettings* settings,
    gxObject* obj,
    unsigned char index)
{
    // Methods are not allowed.
    if (settings->authentication == DLMS_AUTHENTICATION_NONE)
    {
        return DLMS_METHOD_ACCESS_MODE_NONE;
    }
    // Only clock methods are allowed.
    if (settings->authentication == DLMS_AUTHENTICATION_LOW)
    {
        if (obj->objectType == DLMS_OBJECT_TYPE_CLOCK)
        {
            return DLMS_METHOD_ACCESS_MODE_ACCESS;
        }
        return DLMS_METHOD_ACCESS_MODE_NONE;
    }
    return DLMS_METHOD_ACCESS_MODE_ACCESS;
}

/////////////////////////////////////////////////////////////////////////////
//Client has made connection to the server.
/////////////////////////////////////////////////////////////////////////////
int svr_connected(
    dlmsServerSettings* settings)
{
    if (settings->base.connected == DLMS_CONNECTION_STATE_HDLC)
    {
        GXTRACE(("svr_connected to HDLC level."), NULL);
    }
    else if ((settings->base.connected & DLMS_CONNECTION_STATE_DLMS) != 0)
    {
        GXTRACE(("svr_connected DLMS level."), NULL);
    }
    return 0;
}

/**
    * Client has try to made invalid connection. Password is incorrect.
    *
    * @param connectionInfo
    *            Connection information.
    */
int svr_invalidConnection(dlmsServerSettings* settings)
{
    GXTRACE(("svr_invalidConnection"), NULL);
    updateState(GURUX_EVENT_CODES_WRONG_PASSWORD);
    return 0;
}

/////////////////////////////////////////////////////////////////////////////
// Client has close the connection.
/////////////////////////////////////////////////////////////////////////////
int svr_disconnected(
    dlmsServerSettings* settings)
{
    GXTRACE(("svr_disconnected"), NULL);
    if (settings->base.cipher.security != 0 && (settings->base.connected & DLMS_CONNECTION_STATE_DLMS) != 0)
    {
        //Save Invocation counter value when connection is closed.
        saveSecurity();
    }
    return 0;
}

void svr_preGet(
    dlmsSettings* settings,
    gxValueEventCollection* args)
{
    gxValueEventArg* e;
    int ret, pos;
    for (pos = 0; pos != args->size; ++pos)
    {
        if ((ret = vec_getByIndex(args, pos, &e)) != 0)
        {
            return;
        }
    }
}

void svr_postGet(
    dlmsSettings* settings,
    gxValueEventCollection* args)
{

}

#if defined(_WIN32) || defined(_WIN64)//If Windows
void serialPortThread(void* pVoid)
{
    int ret;
    unsigned char data;
    DWORD bytesHandled = 0;
    HANDLE comPort = *((HANDLE*)pVoid);
    OVERLAPPED		osWrite;
    OVERLAPPED		osReader;
    memset(&osReader, 0, sizeof(OVERLAPPED));
    memset(&osWrite, 0, sizeof(OVERLAPPED));
    osReader.hEvent = CreateEvent(NULL, 1, FALSE, NULL);
    osWrite.hEvent = CreateEvent(NULL, 1, FALSE, NULL);
    unsigned char first = 1;
    while (1)
    {
        if (!ReadFile(comPort, &data, 1, &bytesHandled, &osReader))
        {
            ret = GetLastError();
            if (ret != ERROR_IO_PENDING)
            {
                break;
            }
            //Wait until data is actually read
            if (WaitForSingleObject(osReader.hEvent, -1) != WAIT_OBJECT_0)
            {
                break;
            }
            if (!GetOverlappedResult(comPort, &osReader, &bytesHandled, TRUE))
            {
                break;
            }
        }
        if (trace > GX_TRACE_LEVEL_WARNING)
        {
            if (first)
            {
                printf("\nRX:\t");
                first = 0;
            }
            printf("%.2X ", data);
        }
        if (svr_handleRequest3(&settings, data, &reply) != 0)
        {
            first = 1;
            break;
        }
        if (reply.size != 0)
        {
            first = 1;
            if (trace > GX_TRACE_LEVEL_WARNING)
            {
                int pos;
                printf("\nTX\t");
                for (pos = 0; pos != reply.size; ++pos)
                {
                    printf("%.2X ", reply.data[pos]);
                }
                printf("\n");
            }
            if ((ret = WriteFile(comPort, reply.data, reply.size, &bytesHandled, &osWrite)) == 0)
            {
                int err = GetLastError();
                //If error occurs...
                if (err != ERROR_IO_PENDING)
                {
                    printf("WriteFile %d\r\n", err);
                    break;
                }
                //Wait until data is actually sent
                WaitForSingleObject(osWrite.hEvent, 5000);
            }
        }
    }
    CloseHandle(osReader.hEvent);
    CloseHandle(osWrite.hEvent);
}
#endif

void ListenerThread(void* pVoid)
{
#if defined(_WIN32) || defined(_WIN64)//If Windows
    int len;
#else //If Linux
    socklen_t len;
#endif
    unsigned char data;
    int ret;
    int ls = *((int*)pVoid);
    struct sockaddr_in client;
    while (1)
    {
        if ((ret = listen(ls, 1)) == -1)
        {
            //socket listen failed.
            return;
        }
        len = sizeof(client);
        uint16_t pos;
        unsigned char first = 1;
        socket1 = accept(ls, (struct sockaddr*)&client, &len);
        while (1)
        {
            //Read one char at the time.
            if ((ret = recv(socket1, (char*)&data, 1, 0)) == -1)
            {
#if defined(_WIN32) || defined(_WIN64)//If Windows
                closesocket(socket1);
                socket1 = INVALID_SOCKET;
#else //If Linux
                close(socket1);
                socket1 = -1;
#endif
                break;
            }
            //If client closes the connection.
            if (ret == 0)
            {
#if defined(_WIN32) || defined(_WIN64)//If Windows
                closesocket(socket1);
                socket1 = INVALID_SOCKET;
#else //If Linux
                close(socket1);
                socket1 = -1;
#endif
                break;
        }
#if defined(_WIN32) || defined(_WIN64) || defined(__linux__)
            if (trace > GX_TRACE_LEVEL_WARNING)
            {
                if (first)
                {
                    printf("\nRX:\t");
                    first = 0;
                }
                printf("%.2X ", data);
}
#endif //OS
            if (svr_handleRequest3(&settings, data, &reply) != 0)
            {
#if defined(_WIN32) || defined(_WIN64)//If Windows
                closesocket(socket1);
                socket1 = INVALID_SOCKET;
#else //If Linux
                close(socket1);
                socket1 = -1;
#endif
                break;
            }
            if (reply.size != 0)
            {
#if defined(_WIN32) || defined(_WIN64) || defined(__linux__)
                if (trace > GX_TRACE_LEVEL_WARNING)
                {
                    first = 1;
                    printf("\nTX %u:\t", (uint16_t)reply.size);
                    for (pos = 0; pos != reply.size; ++pos)
                    {
                        printf("%.2X ", reply.data[pos]);
                    }
                    printf("\n");
                }
#endif //defined(_WIN32) || defined(_WIN64) || defined(__linux__)
                if (send(socket1, (const char*)reply.data, reply.size - reply.position, 0) == -1)
                {
#if defined(_WIN32) || defined(_WIN64)//If Windows
                    closesocket(socket1);
                    socket1 = INVALID_SOCKET;
#else //If Linux
                    close(socket1);
                    socket1 = -1;
#endif
                    break;
                }
                bb_clear(&reply);
            }
        }
    }
}

//If Linux
#if defined(__linux__)
void* UnixSerialPortThread(void* pVoid)
{
    int ret;
    int comPort = *((int*)pVoid);
    unsigned char data;
    unsigned char first = 1;
    uint16_t pos;
    int bytesRead;
    while (1)
    {
        bytesRead = read(comPort, &data, 1);
        if (bytesRead < 1)
        {
            //If there is no data on the read buffer.
            if (errno != EAGAIN)
            {
                break;
            }
        }
        else
        {
            if (trace > GX_TRACE_LEVEL_WARNING)
            {
                if (first)
                {
                    printf("\nRX:\t");
                    first = 0;
                }
                printf("%.2X ", data);
            }
            if (svr_handleRequest3(&settings, data, &reply) != 0)
            {
                break;
            }
            if (reply.size != 0)
            {
                first = 1;
                if (trace > GX_TRACE_LEVEL_WARNING)
                {
                    printf("\nTX\t");
                    for (pos = 0; pos != reply.size; ++pos)
                    {
                        printf("%.2X ", reply.data[pos]);
                    }
                    printf("\n");
                }
                ret = write(comPort, reply.data, reply.size);
                if (ret != reply.size)
                {
                    printf("Write failed\n");
                }
            }
        }
    }
    return NULL;
}

void* UnixListenerThread(void* pVoid)
{
    ListenerThread(pVoid);
    return NULL;
}

void* captureThreadFunction(void* pVoid)
{
    (void) pVoid;
    unsigned int loadProfileCounter = 0;

    while(true)
    {
        captureProfileGeneric(&nameplateProfile);
        sleep(30);
        captureProfileGeneric(&instantaneousProfile);
        captureProfileGeneric(&loadProfile);
        loadProfileCounter++;

        if (loadProfileCounter == LOAD_PROFILE_COUNTER)
        {
            captureProfileGeneric(&dailyLoadProfile);
            loadProfileCounter = 0;
        }
        captureProfileGeneric(&billingProfile);
    }
    return NULL;
}

char _getch()
{
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
    if (FD_ISSET(0, &fds))
    {
        return getchar();
    }
    return 0;
}
#endif

// Getter function for the enableGarbageValues flag.
bool isGarbageValuesEnabled()
{
    return enableGarbageValues;
}

void showHelp()
{
    printf("Gurux DLMS example Server implements four DLMS/COSEM devices.\n");
    printf(" -t <trace>\t\t [Error, Warning, Info, Verbose] Trace messages.\n");
    printf(" -p <port>\t\t Start port number. Default is 4061.\n");
    printf(" -S <serialPort>\t serial port.\n");
    printf(" -c <json file>\t\t Provide a configuration file with register limits.\n");
    printf(" -g\t\t\t Enable meter to send garbage values at random counts.\n");
    printf(" -I <number>\t\t Use the specified instance number (e.g., 0, 1, 2, etc.) to modify the meter serial number.\n");
    printf(" -h, -help\t\t Show this help.\n");
}

void println(char* desc, unsigned char* data, uint32_t size)
{
    if (data != NULL)
    {
        char str[100];
        hlp_bytesToHex2(data, (unsigned short)size, str, sizeof(str));
        printf("%s: %s\r\n", desc, str);
    }
}

#if defined(_WIN32) || defined(_WIN64)
int com_setCommState(HANDLE hWnd, LPDCB DCB)
{
    if (!SetCommState(hWnd, DCB))
    {
        DWORD err = GetLastError(); //Save occured error.
        if (err == 995)
        {
            COMSTAT comstat;
            unsigned long RecieveErrors;
            if (!ClearCommError(hWnd, &RecieveErrors, &comstat))
            {
                return DLMS_ERROR_TYPE_COMMUNICATION_ERROR | GetLastError();
            }
            if (!SetCommState(hWnd, DCB))
            {
                return DLMS_ERROR_TYPE_COMMUNICATION_ERROR | GetLastError();
            }
        }
        else
        {
            //If USB to serial port converters do not implement this.
            if (err != ERROR_INVALID_FUNCTION)
            {
                return DLMS_ERROR_TYPE_COMMUNICATION_ERROR | err;
            }
        }
    }
    return DLMS_ERROR_CODE_OK;
}

int com_initializeSerialPort(
    HANDLE* comPort,
    char* serialPort,
    unsigned char iec)
{
    int ret = 0;
    char buff[10];
#if _MSC_VER > 1000
    sprintf_s(buff, 10, "\\\\.\\%s", serialPort);
#else
    sprintf(buff, "\\\\.\\%s", serialPort);
#endif
    //Open serial port for read / write. Port can't share.
    * comPort = CreateFileA(buff,
        GENERIC_READ | GENERIC_WRITE, 0, NULL,
        OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
    if (*comPort == INVALID_HANDLE_VALUE)
    {
        ret = GetLastError();
        printf("Failed to open serial port: \"%s\"\n", buff);
        return DLMS_ERROR_TYPE_COMMUNICATION_ERROR | ret;
}
    DCB dcb = { 0 };
    unsigned long sendSize = 0;
    if (*comPort == INVALID_HANDLE_VALUE)
    {
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    dcb.DCBlength = sizeof(DCB);
    if (!GetCommState(*comPort, &dcb))
    {
        ret = GetLastError();
        return DLMS_ERROR_TYPE_COMMUNICATION_ERROR | ret;
    }
    dcb.fBinary = 1;
    dcb.fOutX = dcb.fInX = 0;
    //Abort all reads and writes on Error.
    dcb.fAbortOnError = 1;
    if (iec)
    {
        dcb.BaudRate = 300;
        dcb.ByteSize = 7;
        dcb.StopBits = ONESTOPBIT;
        dcb.Parity = EVENPARITY;
    }
    else
    {
        dcb.BaudRate = 9600;
        dcb.ByteSize = 8;
        dcb.StopBits = ONESTOPBIT;
        dcb.Parity = NOPARITY;
    }
    if ((ret = com_setCommState(*comPort, &dcb)) != 0)
    {
        return ret;
    }
    return 0;
}
#else //#if defined(__LINUX__)
int com_initializeSerialPort(
    int* comPort,
    char* serialPort,
    unsigned char iec)
{
    int ret;
    // read/write | not controlling term | don't wait for DCD line signal.
    *comPort = open(serialPort, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (*comPort == -1) // if open is unsuccessful.
    {
        ret = errno;
        printf("Failed to open serial port: %s\n", serialPort);
        return DLMS_ERROR_TYPE_COMMUNICATION_ERROR | ret;
    }
    if (!isatty(*comPort))
    {
        ret = errno;
        printf("Failed to Open port %s. This is not a serial port.\n", serialPort);
        return DLMS_ERROR_TYPE_COMMUNICATION_ERROR | ret;
    }
    struct termios options;
    memset(&options, 0, sizeof(options));
    options.c_iflag = 0;
    options.c_oflag = 0;
    if (iec)
    {
        options.c_cflag |= PARENB;
        options.c_cflag &= ~PARODD;
        options.c_cflag &= ~CSTOPB;
        options.c_cflag &= ~CSIZE;
        options.c_cflag |= CS7;
        //Set Baud Rates
        cfsetospeed(&options, B300);
        cfsetispeed(&options, B300);
    }
    else
    {
        // 8n1, see termios.h for more information
        options.c_cflag = CS8 | CREAD | CLOCAL;
        /*
        options.c_cflag &= ~PARENB
        options.c_cflag &= ~CSTOPB
        options.c_cflag &= ~CSIZE;
        options.c_cflag |= CS8;
        */
        //Set Baud Rates
        cfsetospeed(&options, B9600);
        cfsetispeed(&options, B9600);
    }
    options.c_lflag = 0;
    options.c_cc[VMIN] = 1;
    //How long we are waiting reply charachter from serial port.
    options.c_cc[VTIME] = 5;

    //hardware flow control is used as default.
    //options.c_cflag |= CRTSCTS;
    if (tcsetattr(*comPort, TCSAFLUSH, &options) != 0)
    {
        ret = errno;
        printf("Failed to Open port. tcsetattr failed.\r");
        return DLMS_ERROR_TYPE_COMMUNICATION_ERROR | ret;
    }
    return 0;
}
#endif

#if defined(_WIN32) || defined(_WIN64)//Windows includes
int _tmain(int argc, _TCHAR* argv[])
#else
int main(int argc, char* argv[])
#endif
{
    int opt, port = 4061;
#if defined(_WIN32) || defined(_WIN64)//If Windows
    //Receiver thread handle.
    HANDLE receiverThread;
    HANDLE captureThread;
#else //If Linux.
    //Receiver thread handle.
    pthread_t receiverThread;
    pthread_t captureThread;
    pthread_attr_t attr; // Thread attributes to set stack size
#endif

    //Serial port handlers.
#if defined(_WIN32) || defined(_WIN64)// If Windows
    HANDLE comPort = INVALID_HANDLE_VALUE;
#else //If Linux
    int comPort = -1;
#endif

    int ret, ls = 0;
    struct sockaddr_in add = { 0 };
    char* serialPort = NULL;
    while ((opt = getopt(argc, argv, "t:p:S:c:hgP:I:m:")) != -1)
    {
        switch (opt)
        {
        case 't':
            //Trace.
            if (strcmp("Error", optarg) == 0)
                trace = GX_TRACE_LEVEL_ERROR;
            else  if (strcmp("Warning", optarg) == 0)
                trace = GX_TRACE_LEVEL_WARNING;
            else  if (strcmp("Info", optarg) == 0)
                trace = GX_TRACE_LEVEL_INFO;
            else  if (strcmp("Verbose", optarg) == 0)
                trace = GX_TRACE_LEVEL_VERBOSE;
            else  if (strcmp("Off", optarg) == 0)
                trace = GX_TRACE_LEVEL_OFF;
            else
            {
                printf("Invalid trace option '%s'. (Error, Warning, Info, Verbose, Off)", optarg);
                return 1;
            }
            break;
        case 'p':
            //Port.
            port = atoi(optarg);
            break;
        case 'S':
            serialPort = optarg;
            break;
        case 'h':
            showHelp();
            return 0;
        case 'g':
            enableGarbageValues = true; // Set flag to true when -g is used
            initializeCounters();
            printf("The meter is set to send garbage values at random counts.\n");
            break;
        case 'P':
            strncpy((char *)&PASSWORD[0U], optarg, sizeof(PASSWORD) - 1U);
            GXTRACE_INT("Password len.", strlen((const char *)PASSWORD));
            break;
        case 'c':
            if (setRegisterLimits(optarg))
            {
                printf("Register limits successfully set from the configuration file.\n");
            }
            else
            {
                printf("Failed to set registers limits from the configuration file.\n");
                return 1;
            }
            break;
        case 'I':
            {
                uint32_t instanceNumber = 0U;
                instanceNumber = atoi(optarg);
                updateMeterSerialNumber(instanceNumber);
            }
            break;
        case '?':
        {
            if (optarg[0] == 'p')
            {
                printf("Missing mandatory port option.\n");
            }
            else if (optarg[0] == 't')
            {
                printf("Missing mandatory trace option.\n");
            }
            else if (optarg[0] == 'S')
            {
                printf("Missing mandatory Serial port option.\n");
            }
            else
            {
                showHelp();
                return 1;
            }
        }
        break;
        default:
            showHelp();
            return 1;
        }
}
#if defined(_WIN32) || defined(_WIN64)//Windows includes
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        // Tell the user that we could not find a usable WinSock DLL.
        return 1;
    }
#endif
    //Add FLAG ID.
    memcpy(SERVER_SYSTEM_TITLE, FLAG_ID, 3);
    //ADD serial number.
    memcpy(SERVER_SYSTEM_TITLE + 4, &SERIAL_NUMBER, 4);

    bb_attach(&reply, replyFrame, 0, sizeof(replyFrame));
    //Start server using logical name referencing and HDLC framing.
    svr_init(&settings, 1, DLMS_INTERFACE_TYPE_HDLC, HDLC_BUFFER_SIZE, PDU_BUFFER_SIZE, frameBuff, HDLC_HEADER_SIZE + HDLC_BUFFER_SIZE, pduBuff, PDU_BUFFER_SIZE);
    //Allocate space for client password.
    BB_ATTACH(settings.base.password, PASSWORD, 0);
    //Allocate space for client challenge.
    BB_ATTACH(settings.base.ctoSChallenge, C2S_CHALLENGE, 0);
    //Allocate space for server challenge.
    BB_ATTACH(settings.base.stoCChallenge, S2C_CHALLENGE, 0);
    //Set master key (KEK) to 1111111111111111.
    unsigned char KEK[16] = { 0x31,0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31 };
    BB_ATTACH(settings.base.kek, KEK, sizeof(KEK));
    //Add COSEM objects.
    if ((ret = createObjects()) != 0)
    {
        return ret;
    }

    //Set default clock.
    settings.defaultClock = &clock1;
    if (serialPort != NULL)
    {
        printf("Serial port server started in port: %s\n", serialPort);
        if ((ret = com_initializeSerialPort(
            &comPort,
            serialPort,
            0)) != 0)
        {
            return ret;
        }
#if defined(_WIN32) || defined(_WIN64)//Windows includes
        // receiverThread = (HANDLE)_beginthread(serialPortThread, 0, &comPort);
        receiverThread = (HANDLE)_beginthreadex(NULL, STACK_SIZE, serialPortThread, &comPort, 0, NULL);
#else
        // ret = pthread_create(&receiverThread, NULL, UnixSerialPortThread, &comPort);
        pthread_attr_init(&attr);
        pthread_attr_setstacksize(&attr, STACK_SIZE);
        ret = pthread_create(&receiverThread, &attr, UnixSerialPortThread, &comPort);
        pthread_attr_destroy(&attr);
#endif

    }
    else
    {
        printf("TCP/IP server started in port: %d\n", port);
        ls = socket(AF_INET, SOCK_STREAM, 0);
        add.sin_port = htons(port);
        add.sin_addr.s_addr = htonl(INADDR_ANY);
        add.sin_family = AF_INET;
        if ((ret = bind(ls, (struct sockaddr*)&add, sizeof(add))) == -1)
        {
#if defined(_WIN32) || defined(_WIN64)//Windows
            ret = GetLastError();
#else
            ret = errno;
#endif
            return DLMS_ERROR_TYPE_COMMUNICATION_ERROR | ret;
        }
#if defined(_WIN32) || defined(_WIN64)//Windows includes
        // receiverThread = (HANDLE)_beginthread(ListenerThread, 0, &ls);
        receiverThread = (HANDLE)_beginthreadex(NULL, STACK_SIZE, ListenerThread, &ls, 0, NULL);
#else
        // ret = pthread_create(&receiverThread, NULL, UnixListenerThread, &ls);
        pthread_attr_init(&attr);
        pthread_attr_setstacksize(&attr, STACK_SIZE);
        ret = pthread_create(&receiverThread, &attr, UnixListenerThread, &ls);
        pthread_attr_destroy(&attr);
#endif
    }
    printf("----------------------------------------------------------\n");
    printf("Authentication levels:\n");
    printf("None: Client address 16 (0x10)\n");
    printf("Low: Client address 32 (0x20)\n");
    printf("High: Client address 18 (0x12)\n");
    printf("HighGMac: Client address 1 (1)\n");
    printf("----------------------------------------------------------\n");
    println("System Title", settings.base.cipher.systemTitle.data, settings.base.cipher.systemTitle.size);
    println("Authentication key", settings.base.cipher.authenticationKey.data, settings.base.cipher.authenticationKey.size);
    println("Block cipher key", settings.base.cipher.blockCipherKey.data, settings.base.cipher.blockCipherKey.size);
    if (settings.base.preEstablishedSystemTitle != NULL)
    {
        println("Client System title", settings.base.preEstablishedSystemTitle->data, settings.base.preEstablishedSystemTitle->size);
    }
    println("Master key (KEK)", settings.base.kek.data, settings.base.kek.size);
    printf("----------------------------------------------------------\n");
    printf("Press Enter to close application.\r\n");

    //Now start a thread for running capture at regular interval
    #if defined(_WIN32) || defined(_WIN64)//Windows includes
        // captureThread = (HANDLE)_beginthread(serialPortThread, 0, NULL);
        captureThread = (HANDLE)_beginthreadex(NULL, STACK_SIZE, serialPortThread, NULL, 0, NULL);
    #else
        // ret = pthread_create(&captureThread, NULL, captureThreadFunction, NULL);
        pthread_attr_init(&attr);
        pthread_attr_setstacksize(&attr, STACK_SIZE);
        ret = pthread_create(&captureThread, &attr, captureThreadFunction, NULL);
        pthread_attr_destroy(&attr);
    #endif
    while (1)
    {
        uint32_t start = time_current();
        if (executeTime <= start)
        {
            svr_run(&settings, start, &executeTime);
            if (executeTime != -1)
            {
                time_t tmp = start;
                printf("%s", ctime(&tmp));
                tmp = executeTime;
                printf("%u seconds before next invoke %s", executeTime - start, ctime(&tmp));
            }
        }
#if defined(_WIN32) || defined(_WIN64)//Windows includes
        if (_kbhit()) {
            char c = _getch();
            if (c == '\r')
            {
                printf("Closing the server.\n");
                if (comPort != INVALID_HANDLE_VALUE)
                {
                    CloseHandle(comPort);
                }
                else
                {
                    closesocket(ls);
                    WaitForSingleObject(receiverThread, 5000);
                }
                break;
            }
        }
        Sleep(1000);
#else
        char ch = _getch();
        if (ch == '\n')
        {
            printf("Closing the server.\n");
            if (comPort != -1)
            {
                close(comPort);
                void* res;
                pthread_join(receiverThread, (void**)&res);
                free(res);
            }
            else
            {
                shutdown(ls, SHUT_RDWR);
                closesocket(ls);
                void* res;
                pthread_join(receiverThread, (void**)&res);
                free(res);
            }
            break;
        }
        usleep(1000000);
#endif
    }
    oa_clear(&settings.base.objects, 0);
    svr_clear(&settings);
#if defined(_WIN32) || defined(_WIN64)//Windows
    WSACleanup();
#if _MSC_VER > 1400
    _CrtDumpMemoryLeaks();
#endif
#endif
    return 0;
            }
