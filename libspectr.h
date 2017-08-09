#include "stdbool.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

extern int g_DEBUG;
extern int g_VERBOSE;

#define USBD_VID 0xE220
#define USBD_PID 0x0100

//#ifdef WIN32
//#include <windows.h>
//#endif

#include "hidapi.h"
#include "hid.c"
//#include "libspectr.h"

#define LOW_BYTE(x)     ((unsigned char)((x)&0xFF))
#define HIGH_BYTE(x)    ((unsigned char)(((x)>>8)&0xFF))
#define LOW_WORD(x)     ((unsigned short)((x)&0xFFFF))
#define HIGH_WORD(x)    ((unsigned short)(((x)>>16)&0xFFFF))

#define HIDAPI_OPERATION_ERROR -1
#define HID_OPERATION_READ_SUCCESS PACKET_SIZE
#define HID_OPERATION_WRITE_SUCCESS PACKET_SIZE + 1
#define STANDARD_TIMEOUT_MILLISECONDS 100
#define ERASE_FLASH_TIMEOUT_MILLISECONDS 5000

#define PACKET_SIZE 64
#define EXTENDED_PACKET_SIZE 1 + PACKET_SIZE //bytes
#define MAX_PACKETS_IN_FRAME 124
#define REMAINING_PACKETS_ERROR 250
#define NUM_OF_PIXELS_IN_PACKET 30
#define MAX_READ_FLASH_PACKETS 100
#define MAX_FLASH_WRITE_PAYLOAD 58

#define ZERO_REPORT_ID 0

#define STATUS_REQUEST 1
#define SET_EXPOSURE_REQUEST 2
#define SET_ACQUISITION_PARAMETERS_REQUEST 3
#define SET_FRAME_FORMAT_REQUEST 4
#define SET_EXTERNAL_TRIGGER_REQUEST 5
#define SET_SOFTWARE_TRIGGER_REQUEST 6
#define CLEAR_MEMORY_REQUEST 7
#define GET_FRAME_FORMAT_REQUEST 8
#define GET_ACQUISITION_PARAMETERS_REQUEST 9
#define SET_ALL_PARAMETERS_REQUEST 0x0C


/*...*/
#define GET_FRAME_REQUEST 0x0A
#define SET_OPTICAl_TRIGGER_REQUEST 0x0B

#define READ_FLASH_REQUEST 0x1A
#define WRITE_FLASH_REQUEST 0x1B
#define ERASE_FLASH_REQUEST 0x1C

#define RESET_REQUEST 0xF1
#define DETACH_REQUEST 0xF2

#define CORRECT_STATUS_REPLY 0x81
#define CORRECT_SET_EXPOSURE_REPLY 0x82
#define CORRECT_SET_ACQUISITION_PARAMETERS_REPLY 0x83
#define CORRECT_SET_FRAME_FORMAT_REPLY 0x84
#define CORRECT_SET_EXTERNAL_TRIGGER_REPLY  0x85
#define CORRECT_SET_SOFTWARE_TRIGGER_REPLY  0x86
#define CORRECT_CLEAR_MEMORY_REPLY 0x87
#define CORRECT_GET_FRAME_FORMAT_REPLY 0x88
#define CORRECT_GET_ACQUISITION_PARAMETERS_REPLY 0x89
#define CORRECT_SET_ALL_PARAMETERS_REPLY 0x8C
/*...*/
#define CORRECT_GET_FRAME_REPLY 0x8A
#define CORRECT_SET_OPTICAL_TRIGGER_REPLY 0x8B
#define CORRECT_READ_FLASH_REPLY 0x9A
#define CORRECT_WRITE_FLASH_REPLY 0x9B
#define CORRECT_ERASE_FLASH_REPLY 0x9C


#define OK 0
#define CONNECT_ERROR_WRONG_ID 500
#define CONNECT_ERROR_NOT_FOUND 501
#define CONNECT_ERROR_FAILED 502
#define DEVICE_NOT_INITIALIZED 503
#define WRITING_PROCESS_FAILED 504
#define READING_PROCESS_FAILED 505
#define WRONG_ANSWER 506
#define GET_FRAME_REMAINING_PACKETS_ERROR 507
#define NUM_OF_PACKETS_IN_FRAME_ERROR 508
#define INPUT_PARAMETER_NOT_INITIALIZED 509
#define READ_FLASH_REMAINING_PACKETS_ERROR 510
    
hid_device* g_Device;
u_int16_t g_numOfPixelsInFrame;

char* g_savedSerial = NULL;

void disconnectDevice();

void _freeSavedSerial()
{
    free(g_savedSerial);
    g_savedSerial = NULL;
}

#ifdef WIN32
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    switch(fdwReason) {
        case DLL_PROCESS_ATTACH:
            g_Device = NULL;
            g_numOfPixelsInFrame = 0;
            break;
        case DLL_PROCESS_DETACH:
            disconnectDevice();

            if (g_savedSerial) {
                _freeSavedSerial();
            }
            break;
    }
    return TRUE;
}
#endif

/** @brief Finds the requested device and connects to it.
    vendorId and productId should be positive numbers

    This function should always precede all other operations.

    If the requested device is already connected, disconnects and initializes again

    @ingroup API

    @returns
        This function returns 0 on success and error code in case of error.
*/
int connectToDevice(const char *serialNumber)
{   
    hid_init();
    wchar_t *serialToUse = NULL;
    int cSize = (serialNumber)? strlen(serialNumber) : 0;
    
    if (cSize) {
        ++cSize;       //for \0
        serialToUse = (wchar_t *)malloc(sizeof(wchar_t) * cSize);
        mbstowcs((wchar_t *)serialToUse, serialNumber, cSize);
    }

    if (g_Device) {
        if(g_VERBOSE){printf("HID Device already connected. Disconnecting.\n");}
        disconnectDevice();
    }
    if(g_VERBOSE){printf("Requesting HID handle.\n");}
    g_Device = hid_open(USBD_VID, USBD_PID, (const wchar_t *)serialToUse); //Once to initialize
    g_Device = hid_open(USBD_VID, USBD_PID, (const wchar_t *)serialToUse); //Once more to poll for device
    
    if (!g_Device) {
        if(g_VERBOSE){printf("Failed to obtain HID handle.\n");}
        if (serialToUse) {
            if(g_VERBOSE){printf("Freeing serial number.\n");}
            free(serialToUse);
        }
        return CONNECT_ERROR_FAILED;
    }
    
    if (serialToUse) {
        if(g_VERBOSE){printf("Obtained HID serial number.\n");}
        if (g_savedSerial) {
            if(g_VERBOSE){printf("Freeing saved serial number.\n");}
            _freeSavedSerial();
        }
        if(g_VERBOSE){printf("Setting serial number");}
        g_savedSerial = (char*)malloc(sizeof(char) * cSize);
        strcpy(g_savedSerial, serialNumber);
        free(serialToUse);
    }
    if(g_VERBOSE){printf("HID device connected.\n");}
    return OK;
}

int _reconnect()
{
    int result = g_savedSerial? connectToDevice(g_savedSerial) : connectToDevice(NULL);
    return result;
}

/** @brief Disconnects from the device.

    @ingroup API

    @returns 
        No value to return
*/
void disconnectDevice()
{
    if (g_Device) {
        hid_close(g_Device);
        g_Device = NULL;
        hid_exit();
    }
}


int _tryWrite(unsigned char* report) 
{
    int writeResult;
	int reconnectResult;
    bool reconnectAttempted = false;

    do {
        writeResult = hid_write(g_Device, (const unsigned char*)report, EXTENDED_PACKET_SIZE); 
        if (writeResult != HID_OPERATION_WRITE_SUCCESS) {
            if (reconnectAttempted) {
                return WRITING_PROCESS_FAILED;
            }

            reconnectResult = _reconnect();
            if (reconnectResult != OK) {
                return reconnectResult;
            }
            reconnectAttempted = true;
        }
    } while (writeResult != HID_OPERATION_WRITE_SUCCESS);

    return OK;
}

int _tryRead(unsigned char* report, unsigned char correctAnswer, u_int16_t timeout)
{
    if(g_VERBOSE){printf("Requesting read operation...\n");}
    int result = hid_read_timeout(g_Device, report, EXTENDED_PACKET_SIZE, timeout);
    if (result != HID_OPERATION_READ_SUCCESS){
        if(g_VERBOSE){printf("Read Failed.\n");}
        return READING_PROCESS_FAILED;
    }

    if (report[0] != correctAnswer) {
        if(g_VERBOSE){printf("Data aquisition error!\n");}
        return WRONG_ANSWER;
    }

    return OK;
}

int _writeOnlyFunction(unsigned char* report)
{
    int result;    

    if (!g_Device) {
        result = _reconnect();
        if (result != OK) {
            return result;
        }
    }

    result = _tryWrite(report);
    return result;
}

int _writeReadFunction(unsigned char* report, u_int8_t correctReply, u_int16_t timeout)
{
    int result;

    if (!g_Device) {
        result = _reconnect();
        if (result != OK) {
            return result;
        }
    }

    result = _tryWrite(report);
    if (result != OK) {
        return result;
    }

    result = _tryRead(report, correctReply, timeout);
    return result;
}

/** @brief Sets exposure parameter

    exposureTime = multiple of 10 us (microseconds)
    force = 

    sends:
    outReport[1] = 2;
    outReport[2] = exposure[0];
    outReport[3] = exposure[1];
    outReport[4] = exposure[2];
    outReport[5] = exposure[3];
    outReport[6] = force;

    gets:
    inReport[0] = 0x82;
    inReport[1] = errorCode;

    @ingroup API

    @returns
        This function returns 0 on success and error code in case of error.
*/
int setExposure(const u_int32_t timeOfExposure, const u_int8_t force)
{
    unsigned char report[EXTENDED_PACKET_SIZE];
    int result;
    int errorCode;

    report[0] = ZERO_REPORT_ID;
    report[1] = SET_EXPOSURE_REQUEST;
    report[2] = LOW_BYTE(LOW_WORD(timeOfExposure));           //(exposure >> 24) & 0xFF;
    report[3] = HIGH_BYTE(LOW_WORD(timeOfExposure));          //(exposure >> 16) & 0xFF;       
    report[4] = LOW_BYTE(HIGH_WORD(timeOfExposure));          //(exposure >> 8)  & 0xFF;
    report[5] = HIGH_BYTE(HIGH_WORD(timeOfExposure));         //exposure & 0xFF;
    report[6] = force;

    result = _writeReadFunction(report, CORRECT_SET_EXPOSURE_REPLY, STANDARD_TIMEOUT_MILLISECONDS);
    if (result != OK) {
        return result;
    }

    errorCode = report[1];
    return errorCode;
}

/** @brief Sets acquisition parameters

    0 - continuous scanMode
    1 - idle scan mode
    2 - every frame idle scan mode


    sends:
    outReport[1] = 3;
    outReport[2] = LO(scans);
    outReport[3] = HI(scans);
    outReport[4] = LO(blankScans);
    outReport[5] = HI(blankScans);
    outReport[6] = continuousScanMode;

    gets:
    inReport[0]=0x83;
    inReport[1]=errorCode;

    @ingroup API

    @returns
        This function returns 0 on success and error code in case of error.
*/
int setAcquisitionParameters(u_int16_t numOfScans, u_int16_t numOfBlankScans, u_int8_t scanMode, u_int32_t timeOfExposure)
{
    unsigned char report[EXTENDED_PACKET_SIZE];
    int result;
    int errorCode;

    report[0] = ZERO_REPORT_ID;
    report[1] = SET_ACQUISITION_PARAMETERS_REQUEST;
    report[2] = LOW_BYTE(numOfScans);      
    report[3] = HIGH_BYTE(numOfScans);     
    report[4] = LOW_BYTE(numOfBlankScans); 
    report[5] = HIGH_BYTE(numOfBlankScans);
    report[6] = scanMode;
    report[7] = LOW_BYTE(LOW_WORD(timeOfExposure));
    report[8] = HIGH_BYTE(LOW_WORD(timeOfExposure));
    report[9] = LOW_BYTE(HIGH_WORD(timeOfExposure));
    report[10] = HIGH_BYTE(HIGH_WORD(timeOfExposure));

    result = _writeReadFunction(report, CORRECT_SET_ACQUISITION_PARAMETERS_REPLY, STANDARD_TIMEOUT_MILLISECONDS);
    if (result != OK) {
        return result;
    }

    errorCode = report[1];
    return errorCode;    
}

/** @brief Sets exposure parameters

    sends:
    outReport[1]=4;
    outReport[2]=LO(startElement);
    outReport[3]=HI(startElement);
    outReport[4]=LO(endElement);
    outReport[5]=HI(endElement);
    outReport[6]=reduce;

    gets:
    inReport[0]=0x84;
    inReport[1]=errorCode;
    inReport[2]=LO(frameElements);
    inReport[3]=HI(frameElements);

    @ingroup API

    @returns
        This function returns 0 on success and error code in case of error.
*/
int setFrameFormat(const u_int16_t numOfStartElement, const u_int16_t numOfEndElement, const u_int8_t reductionMode, u_int16_t *numOfPixelsInFrame)
{
    unsigned char report[EXTENDED_PACKET_SIZE];
    int result;
    int errorCode;

    report[0] = ZERO_REPORT_ID;
    report[1] = SET_FRAME_FORMAT_REQUEST;
    report[2] = LOW_BYTE(numOfStartElement);
    report[3] = HIGH_BYTE(numOfStartElement);
    report[4] = LOW_BYTE(numOfEndElement);
    report[5] = HIGH_BYTE(numOfEndElement);
    report[6] = reductionMode;

    result = _writeReadFunction(report, CORRECT_SET_FRAME_FORMAT_REPLY, STANDARD_TIMEOUT_MILLISECONDS);
    if (result != OK) {
        return result;
    }

    errorCode = report[1];
    if (!errorCode) {
        g_numOfPixelsInFrame = (report[3] << 8) | report[2];

        if (numOfPixelsInFrame) {
            *numOfPixelsInFrame = g_numOfPixelsInFrame;
        }
    } //else set g_numOfPixelsInFrame = 0?

    return errorCode;
}

/*
getFrameFormat
{
    outReport[1] = 8;

    inReport[0] = 0x88;
    inReport[1] = LO(startElement);
    inReport[2] = HI(startElement);
    inReport[3] = LO(endElement);
    inReport[4] = HI(endElement);
    inReport[5] = redux;
    inReport[6] = LO(numOfFrameElements);
    inReport[7] = HI(numOfFrameElements);
}

reductionMode - 0 - ������ �������
- 1 - ���������� �� ����

*/

int getFrameFormat(u_int16_t *numOfStartElement, u_int16_t *numOfEndElement, u_int8_t *reductionMode, u_int16_t *numOfPixelsInFrame)
{
    unsigned char report[EXTENDED_PACKET_SIZE];
    int result;    

    g_numOfPixelsInFrame = 0;

    report[0] = ZERO_REPORT_ID;
    report[1] = GET_FRAME_FORMAT_REQUEST;

    result = _writeReadFunction(report, CORRECT_GET_FRAME_FORMAT_REPLY, STANDARD_TIMEOUT_MILLISECONDS);
    if (result != OK) {
        return result;
    }

    if (numOfStartElement) {
        *numOfStartElement = (report[2] << 8) | report[1];
    }

    if (numOfEndElement) {
        *numOfEndElement = (report[4] << 8) | report[3];
    }

    if (reductionMode) {
        *reductionMode = report[5];
    }

    g_numOfPixelsInFrame = (report[7] << 8) | report[6];

    if (numOfPixelsInFrame) {
        *numOfPixelsInFrame = g_numOfPixelsInFrame;
    }

    return OK;
}

/*

enableMode:
0 - trigger disabled
1 - trigger enabled
2 - one time trigger

triggerFront:
0 - trigger disabled
1 - front rising
2 - front falling
3 - both rising and fall

outReport[1]=5;
outReport[2]=triggerEnabled;
outReport[3]=triggerFront;

inReport[0]=0x85;
inReport[1]=errorCode;
*/
int setExternalTrigger(const u_int8_t enableMode, const u_int8_t signalFrontMode)
{
    unsigned char report[EXTENDED_PACKET_SIZE];
    int result;
    int errorCode;

    report[0] = ZERO_REPORT_ID;
    report[1] = SET_EXTERNAL_TRIGGER_REQUEST;
    report[2] = enableMode;
    report[3] = signalFrontMode;

    result = _writeReadFunction(report, CORRECT_SET_EXTERNAL_TRIGGER_REPLY, STANDARD_TIMEOUT_MILLISECONDS);
    if (result != OK) {
        return result;
    }

    errorCode = report[1];
    return errorCode;
}

/*
outReport[1] = 0x0B;
outReport[2] = triggerMode;
outReport[3] = LO(pixel);
outReport[4] = HI(pixel);
outReport[5] = LO(threshold);
outReport[6] = HI(threshold);

inReport[0] = 0x8B;
inReport[1] = errorCode;
*/

int setOpticalTrigger(const u_int8_t enableMode, const u_int16_t pixel, const u_int16_t threshold)
{
    int result;
    u_int8_t report[EXTENDED_PACKET_SIZE];
    int errorCode;

    report[0] = ZERO_REPORT_ID;
    report[1] = SET_OPTICAl_TRIGGER_REQUEST;
    report[2] = enableMode;
    report[3] = LOW_BYTE(pixel);
    report[4] = HIGH_BYTE(pixel);
    report[5] = LOW_BYTE(threshold);
    report[6] = HIGH_BYTE(threshold);

    result = _writeReadFunction(report, CORRECT_SET_OPTICAL_TRIGGER_REPLY, STANDARD_TIMEOUT_MILLISECONDS);
    if (result != OK) {
        return result;
    }

    errorCode = report[1];
    return errorCode;
}


/*
outReport[1]=6;

Start acquisition by software
*/
int triggerAcquisition()
{
    unsigned char report[EXTENDED_PACKET_SIZE];
    int result;    

    report[0] = ZERO_REPORT_ID;
    report[1] = SET_SOFTWARE_TRIGGER_REQUEST;

    result = _writeOnlyFunction(report);
    return result;
}

/** @brief 
    Gets device status.

    sends:
    outReport[0] = 0
    outReport[1] = 1;
    outReport[2] = 0;

    gets:
    inReport[0] = 0x81;
    inReport[1] = flags;
    inReport[2] = LO(framesInMemory);
    inReport[3] = HI(framesInMemory);

    @ingroup API

    @returns
        This function returns 0 on success and -1 on error.
*/
int getStatus(u_int8_t *statusFlags, u_int16_t *framesInMemory)
{
    unsigned char report[EXTENDED_PACKET_SIZE];
    int result;

    report[0] = ZERO_REPORT_ID;
    report[1] = STATUS_REQUEST;

    result = _writeReadFunction(report, CORRECT_STATUS_REPLY, STANDARD_TIMEOUT_MILLISECONDS);
    if (result != OK) {
        return result;
    }

    if (statusFlags) {
        *statusFlags = report[1];
    }

    if (framesInMemory) {
        *framesInMemory = (report[3] << 8) | (report[2]);
    }
    
    return OK;
}

/*
outReport[0]=7;

inReport[0]=0x87;
inReport[1]=errorCode;
*/
int clearMemory()
{
    unsigned char report[EXTENDED_PACKET_SIZE];
    int result;
    int errorCode;

    report[0] = ZERO_REPORT_ID;
    report[1] = CLEAR_MEMORY_REQUEST;

    result = _writeReadFunction(report, CORRECT_CLEAR_MEMORY_REPLY, STANDARD_TIMEOUT_MILLISECONDS);
    if (result != OK) {
        return result;
    }

    errorCode = report[1];
    return errorCode;
}

/*
outReport[1]=0x0A;
outReport[2]=LO(offset);
outReport[3]=HI(offset);
outReport[4]=LO(frameNum);
outReport[5]=HI(frameNum);
outReport[6]=packets;

inReport[0]=0x8A;
inReport[1]=LO(offset);
inReport[2]=HI(offset);
inReport[3]=packetsRemaining_or_IsError;
inReport[4]=LO(frame[offset]);
inReport[5]=HI(frame[offset]);
inReport[6]=LO(frame[offset+1]);
inReport[7]=HI(frame[offset+1]);
...
inReport[62]=LO(frame[offset+29]); 
inReport[63]=HI(frame[offset+29]);
*/

int getFrame(u_int16_t *framePixelsBuffer, const u_int16_t numOfFrame)
{
    int result;
    u_int8_t report[EXTENDED_PACKET_SIZE];

    /* Total frame request parameters: */
    u_int16_t pixelOffset = 0;
    u_int8_t numOfPacketsToGet = 0, numOfPacketsLeft = 0, numOfPacketsReceived = 0;

    bool continueGetInReport = true;
    u_int16_t totalNumOfReceivedPixels = 0;
    
    u_int8_t indexOfPixelInPacket;
    int indexInPacket;

    if (!g_Device) {
        result = _reconnect();
        if (result != OK) {
            return result;
        }
    }

    if (!framePixelsBuffer) {
        return INPUT_PARAMETER_NOT_INITIALIZED;
    }
         
    if (!g_numOfPixelsInFrame) {
        result = getFrameFormat(NULL, NULL, NULL, NULL);
		if (result != OK) 
            return result;
    }

    numOfPacketsToGet = g_numOfPixelsInFrame / NUM_OF_PIXELS_IN_PACKET;
    numOfPacketsToGet += (g_numOfPixelsInFrame % NUM_OF_PIXELS_IN_PACKET)? 1 : 0;

    if (numOfPacketsToGet > MAX_PACKETS_IN_FRAME) {
        return NUM_OF_PACKETS_IN_FRAME_ERROR;
    }

    report[0] = ZERO_REPORT_ID;
    report[1] = GET_FRAME_REQUEST;
    report[2] = LOW_BYTE(pixelOffset);
    report[3] = HIGH_BYTE(pixelOffset);
    report[4] = LOW_BYTE(numOfFrame);
    report[5] = HIGH_BYTE(numOfFrame);
    report[6] = numOfPacketsToGet;

    result = _tryWrite(report);
    if (result != OK) {
        return result;
    }   

    while (continueGetInReport) {
        result = hid_read_timeout(g_Device, report, EXTENDED_PACKET_SIZE, STANDARD_TIMEOUT_MILLISECONDS);
        if (result != HID_OPERATION_READ_SUCCESS){
            return READING_PROCESS_FAILED;
        }

        if (report[0] != CORRECT_GET_FRAME_REPLY) {
            return WRONG_ANSWER;
        }

        ++numOfPacketsReceived;

        numOfPacketsLeft = report[3];
        if (numOfPacketsLeft >= REMAINING_PACKETS_ERROR ||
            (numOfPacketsLeft != numOfPacketsToGet - numOfPacketsReceived)) {
            return GET_FRAME_REMAINING_PACKETS_ERROR;
        }

        if (numOfPacketsLeft != numOfPacketsToGet - numOfPacketsReceived) {
            return GET_FRAME_REMAINING_PACKETS_ERROR;
        }

        continueGetInReport = (numOfPacketsLeft > 0)? true : false;

        pixelOffset = (report[2] << 8) | report[1];

        indexInPacket = 4;
        indexOfPixelInPacket = 0;        

        while ((totalNumOfReceivedPixels < g_numOfPixelsInFrame) && (indexOfPixelInPacket < NUM_OF_PIXELS_IN_PACKET)) {
            u_int16_t pixel = (report[indexInPacket + 1] << 8) | report[indexInPacket];
            framePixelsBuffer[pixelOffset + indexOfPixelInPacket] = pixel;

            indexInPacket += 2;
            ++indexOfPixelInPacket;
            ++totalNumOfReceivedPixels;
        }
    }

    return OK;
}

/*
outReport[0] = 0x1C;

inReport[0] = 0x9C;
inReport[1] = errorCode;
*/
int eraseFlash()
{
    int result;
    u_int8_t report[EXTENDED_PACKET_SIZE];
    int errorCode;

    report[0] = ZERO_REPORT_ID;
    report[1] = ERASE_FLASH_REQUEST;

    result = _writeReadFunction(report, CORRECT_ERASE_FLASH_REPLY, ERASE_FLASH_TIMEOUT_MILLISECONDS);
    if (result != OK) {
        return result;
    }

    errorCode = report[1];
    return errorCode;
}

/*
outReport[1] = 0x1A;
outReport[2] = absoluteOffset[0];     //low byte
outReport[3] = absoluteOffset[1];     //little endian!
outReport[4] = absoluteOffset[2];
outReport[5] = absoluteOffset[3];     //high byte
outReport[6] = packets;

inReport[0] = 0x9A;
inReport[1] = LO(localOffset); 
inReport[2] = HI(localOffset);
inReport[3] = packetsRemaining_or_IsError;
inReport[4] = flash[absoluteOffset + localOffset];
inReport[5] = flash[absoluteOffset + localOffset + 1];
..
inReport[63] = flash[absoluteOffset + localOffset + 59];
*/

int readFlash(u_int8_t *buffer, u_int32_t absoluteOffset, u_int32_t bytesToRead)
{
    int result;
    u_int8_t report[EXTENDED_PACKET_SIZE];
    u_int32_t numOfPacketsToGet = 0;
    u_int8_t numOfPacketsToGetCurrent = 0, numOfPacketsReceivedCurrent = 0, numOfPacketsLeftCurrent = 0;

    bool continueGetInReport = true;
    u_int16_t localOffset = 0;
    u_int32_t totalNumOfReceivedBytes = 0;

    u_int8_t indexOfByteInPacket;
    int indexInPacket;

    u_int32_t offsetIncrement = 0;
    u_int8_t payloadSize = PACKET_SIZE - 4;

    if (!g_Device) {
        result = _reconnect();
        if (result != OK) {
            return result;
        }
    }

    if (!buffer) {
        return INPUT_PARAMETER_NOT_INITIALIZED;
    }

    numOfPacketsToGet = bytesToRead / payloadSize;
    numOfPacketsToGet += (bytesToRead % payloadSize)? 1 : 0;
  
    while(numOfPacketsToGet) {
        numOfPacketsToGetCurrent = (numOfPacketsToGet > MAX_READ_FLASH_PACKETS)? MAX_READ_FLASH_PACKETS : numOfPacketsToGet;

        report[0] = ZERO_REPORT_ID;
        report[1] = READ_FLASH_REQUEST;
        report[2] = LOW_BYTE(LOW_WORD(absoluteOffset + offsetIncrement));
        report[3] = HIGH_BYTE(LOW_WORD(absoluteOffset + offsetIncrement));
        report[4] = LOW_BYTE(HIGH_WORD(absoluteOffset + offsetIncrement));
        report[5] = HIGH_BYTE(HIGH_WORD(absoluteOffset + offsetIncrement));
        report[6] = numOfPacketsToGetCurrent;

        result = hid_write(g_Device, (const unsigned char*)report, EXTENDED_PACKET_SIZE);
        if (result != HID_OPERATION_WRITE_SUCCESS) {
            return WRITING_PROCESS_FAILED;
        }

        numOfPacketsReceivedCurrent = 0;

        while (continueGetInReport) {
            result = hid_read_timeout(g_Device, report, EXTENDED_PACKET_SIZE, STANDARD_TIMEOUT_MILLISECONDS);
            if (result != HID_OPERATION_READ_SUCCESS){
                return READING_PROCESS_FAILED;
            }

            ++numOfPacketsReceivedCurrent;

            if (report[0] != CORRECT_READ_FLASH_REPLY) {
                return WRONG_ANSWER;
            }

            numOfPacketsLeftCurrent = report[3];

            if (numOfPacketsLeftCurrent >= REMAINING_PACKETS_ERROR || (numOfPacketsLeftCurrent != numOfPacketsToGetCurrent - numOfPacketsReceivedCurrent)) {
                return READ_FLASH_REMAINING_PACKETS_ERROR;
            }
            continueGetInReport = (numOfPacketsLeftCurrent > 0)? true : false;

            localOffset = (report[2] << 8) | report[1];

            indexInPacket = 4;
            indexOfByteInPacket = 0;        

            while ((totalNumOfReceivedBytes < bytesToRead) && (indexOfByteInPacket < payloadSize)) {
                buffer[offsetIncrement + localOffset + indexOfByteInPacket++] = report[indexInPacket++];
                ++totalNumOfReceivedBytes;
            }
        }
        
        if (numOfPacketsToGet > MAX_READ_FLASH_PACKETS) {
            numOfPacketsToGet -= MAX_READ_FLASH_PACKETS;
            offsetIncrement += MAX_READ_FLASH_PACKETS * payloadSize;            
        } else {
            numOfPacketsToGet = 0;
        }
    }

    return OK;
}

/*
outReport[1] = 0x1B;
outReport[2] = offset[0];     //low byte
outReport[3] = offset[1];     //little endian!
outReport[4] = offset[2];
outReport[5] = offset[3];     //high byte
outReport[6] = numberOfbytes;     // <= 58 (max payload length)

outReport[7] = bytesToWrite[0]; //payload begin
outReport[8] = bytesToWrite[1];
...
outReport[n] = bytesToWrite[m]; //payload end
//n = numberOfbytes + 6, m = numberOfbytes - 1;

inReport[0] = 0x9B;
inReport[1] = errorCode;
*/

int writeFlash(u_int8_t *buffer, u_int32_t offset, u_int32_t bytesToWrite)
{
    int result;
    u_int8_t report[EXTENDED_PACKET_SIZE];    
    u_int8_t errorCode = OK;
    
    u_int32_t index, byteIndex = 0;
    u_int32_t bytesLeftToWrite = bytesToWrite;
    
    if (!g_Device) {
        result = _reconnect();
        if (result != OK) {
            return result;
        }
    }

    if (!buffer) {
        return INPUT_PARAMETER_NOT_INITIALIZED;
    }

    while (bytesLeftToWrite) {
        report[0] = ZERO_REPORT_ID;
        report[1] = WRITE_FLASH_REQUEST;
        report[2] = LOW_BYTE(LOW_WORD(offset));
        report[3] = HIGH_BYTE(LOW_WORD(offset));
        report[4] = LOW_BYTE(HIGH_WORD(offset));
        report[5] = HIGH_BYTE(HIGH_WORD(offset));
        report[6] = (bytesLeftToWrite > MAX_FLASH_WRITE_PAYLOAD) ? MAX_FLASH_WRITE_PAYLOAD : bytesLeftToWrite;

        index = 7;
        while ((byteIndex < bytesToWrite) && (index < EXTENDED_PACKET_SIZE)) {
            report[index++] = buffer[byteIndex++];
        }

        result = hid_write(g_Device, (const unsigned char*)report, EXTENDED_PACKET_SIZE);
        if (result != HID_OPERATION_WRITE_SUCCESS) {
            return WRITING_PROCESS_FAILED;
        }   

        result = hid_read_timeout(g_Device, report, EXTENDED_PACKET_SIZE, STANDARD_TIMEOUT_MILLISECONDS);
        if (result != HID_OPERATION_READ_SUCCESS){
            return READING_PROCESS_FAILED;
        }

        if (report[0] != CORRECT_WRITE_FLASH_REPLY) {
            return WRONG_ANSWER;
        }

        errorCode = report[1];
        if (errorCode != OK) {
            return errorCode;
        }

        if (bytesLeftToWrite > MAX_FLASH_WRITE_PAYLOAD) {
            bytesLeftToWrite -= MAX_FLASH_WRITE_PAYLOAD;
            offset += MAX_FLASH_WRITE_PAYLOAD;
        } else 
            bytesLeftToWrite = 0;
    }

    return errorCode;
}

int resetDevice()
{
    unsigned char report[EXTENDED_PACKET_SIZE];
    int result;    

    report[0] = ZERO_REPORT_ID;
    report[1] = RESET_REQUEST;

    result = _writeOnlyFunction(report);
    return result;
}

int detachDevice()
{
    unsigned char report[EXTENDED_PACKET_SIZE];
    int result;    

    report[0] = ZERO_REPORT_ID;
    report[1] = DETACH_REQUEST;

    result = _writeOnlyFunction(report);
    return result;
}

int getAcquisitionParameters(u_int16_t* numOfScans, u_int16_t* numOfBlankScans, u_int8_t* scanMode, u_int32_t* timeOfExposure)
{
    unsigned char report[EXTENDED_PACKET_SIZE];
    int result; 

    report[0] = ZERO_REPORT_ID;
    report[1] = GET_ACQUISITION_PARAMETERS_REQUEST;

    result = _writeReadFunction(report, CORRECT_GET_ACQUISITION_PARAMETERS_REPLY, STANDARD_TIMEOUT_MILLISECONDS);
    if (result != OK) {
        return result;
    }

    if (numOfScans) {
        *numOfScans = (report[2] << 8) | report[1];
    }

    if (numOfBlankScans) {
        *numOfBlankScans = (report[4] << 8) | report[3];
    }

    if (scanMode) {
        *scanMode = report[5];
    }

    if (timeOfExposure) {
        *timeOfExposure = (report[9] << 24) | (report[8] << 16) | (report[7] << 8) | report[6];
    }

    return OK;
}


/* combination of setAcquisitionParameters and setExternalTrigger */
int setAllParameters(u_int16_t numOfScans, u_int16_t numOfBlankScans, u_int8_t scanMode, u_int32_t timeOfExposure, u_int8_t enableMode, u_int8_t signalFrontMode)
{
    unsigned char report[EXTENDED_PACKET_SIZE];
    int result;
    int errorCode;
    
    report[0] = ZERO_REPORT_ID;
    report[1] = SET_ALL_PARAMETERS_REQUEST;
    report[2] = LOW_BYTE(numOfScans);      
    report[3] = HIGH_BYTE(numOfScans);     
    report[4] = LOW_BYTE(numOfBlankScans); 
    report[5] = HIGH_BYTE(numOfBlankScans);
    report[6] = scanMode;
    report[7] = LOW_BYTE(LOW_WORD(timeOfExposure));
    report[8] = HIGH_BYTE(LOW_WORD(timeOfExposure));
    report[9] = LOW_BYTE(HIGH_WORD(timeOfExposure));
    report[10] = HIGH_BYTE(HIGH_WORD(timeOfExposure));
    report[11] = enableMode;
    report[12] = signalFrontMode;

    result = _writeReadFunction(report, CORRECT_GET_ACQUISITION_PARAMETERS_REPLY, STANDARD_TIMEOUT_MILLISECONDS);
    if (result != OK) {
        return result;
    }

    errorCode = report[1];
    return errorCode;
}

