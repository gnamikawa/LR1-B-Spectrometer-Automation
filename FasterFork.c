/* FasterFork - Faster than trying to understand garbage coding.
 * Genzo Namikawa
 * genzo.namikawa@jacks.sdstate.edu
 */

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "hid.c"

#define DEBUG true
#define SUCCESS 0
#define FAILURE -1
//#define ARRAYSIZE(x)  (sizeof(x) / sizeof((x)[0]))
//#define DEBUG_HEXARRAY_PRINT(x) if(DEBUG){for(int i=0;i<=(sizeof x);i=i+1){printf("%02i: 0x%02x\n",i,x[i]);}}
//#define TRUEORFALSE(x) 

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define LOW_BYTE(x)     ((unsigned char)((x)&0xFF))
#define HIGH_BYTE(x)    ((unsigned char)(((x)>>8)&0xFF))
#define LOW_WORD(x)     ((unsigned short)((x)&0xFFFF))
#define HIGH_WORD(x)    ((unsigned short)(((x)>>16)&0xFFFF))

#define ZERO_REPORT_ID                     0x00
#define STATUS_REQUEST                     0x01
#define SET_EXPOSURE_REQUEST               0x02
#define SET_ACQUISITION_PARAMETERS_REQUEST 0x03
#define SET_FRAME_FORMAT_REQUEST           0x04
#define SET_EXTERNAL_TRIGGER_REQUEST       0x05
#define SET_SOFTWARE_TRIGGER_REQUEST       0x06
#define CLEAR_MEMORY_REQUEST               0x07
#define GET_FRAME_FORMAT_REQUEST           0x08
#define GET_ACQUISITION_PARAMETERS_REQUEST 0x09
#define SET_ALL_PARAMETERS_REQUEST         0x0C

#define STANDARD_TIMEOUT_MILLISECONDS 1000

#define GET_FRAME_REQUEST           0x0A
#define SET_OPTICAl_TRIGGER_REQUEST 0x0B

#define READ_FLASH_REQUEST  0x1A
#define WRITE_FLASH_REQUEST 0x1B
#define ERASE_FLASH_REQUEST 0x1C

#define RESET_REQUEST  0xF1
#define DETACH_REQUEST 0xF2

#define CORRECT_STATUS_REPLY                     0x81
#define CORRECT_SET_EXPOSURE_REPLY               0x82
#define CORRECT_SET_ACQUISITION_PARAMETERS_REPLY 0x83
#define CORRECT_SET_FRAME_FORMAT_REPLY           0x84
#define CORRECT_SET_EXTERNAL_TRIGGER_REPLY       0x85
#define CORRECT_SET_SOFTWARE_TRIGGER_REPLY       0x86
#define CORRECT_CLEAR_MEMORY_REPLY               0x87
#define CORRECT_GET_FRAME_FORMAT_REPLY           0x88
#define CORRECT_GET_ACQUISITION_PARAMETERS_REPLY 0x89
#define CORRECT_SET_ALL_PARAMETERS_REPLY         0x8C

#define CORRECT_GET_FRAME_REPLY           0x8A
#define CORRECT_SET_OPTICAL_TRIGGER_REPLY 0x8B
#define CORRECT_READ_FLASH_REPLY          0x9A
#define CORRECT_WRITE_FLASH_REPLY         0x9B
#define CORRECT_ERASE_FLASH_REPLY         0x9C

#define PACKET_SIZE 64
#define EXTENDED_PACKET_SIZE 1 + PACKET_SIZE //bytes
#define MAX_PACKETS_IN_FRAME 124
#define REMAINING_PACKETS_ERROR 250
#define NUM_OF_PIXELS_IN_PACKET 30
#define MAX_READ_FLASH_PACKETS 100
#define MAX_FLASH_WRITE_PAYLOAD 58

//Constant Values
const unsigned int VendorID                = 0xE220; // ASEQ Instruments vendor ID
const unsigned int ProductID               = 0x0100; // Product ID
const unsigned int CommandReportByteLength = 9;      // OutputReport
//const unsigned int ReturnReportByteLength  = 64;     // InputReport

//Variables
bool      g_deviceDetected = false;

u_int16_t g_pixelsInFrame;
char*     g_devicePath = NULL;      //DevicePath freed on exit.
char      g_commandReport[100]={0}; //OutputReport
char      g_returnReport[100]={0};  //InputReport

//Function Prototypes
int pathHID(void);
int getStatus(void);
int readReport(char*, int, int);
int calcConst(double*);
int readFlash(u_int8_t *buffer, 
              u_int32_t absoluteOffset, 
              u_int32_t bytesToRead);
int writeReport(char*);
int getFrameFormat(u_int16_t*, u_int16_t*, u_int8_t*, u_int16_t*);
int saveData(double*, signed short*);
//SetCommandReport(???);

int main(int argc, char** argv){
    //g_commandReport[1]=0;
    //g_commandReport[2]=1;
    //for(int i=0;i<100;i=i+1){printf("%i: %2i\n",i,g_commandReport[i]);}
    
    if(geteuid() != 0){
        printf( ANSI_COLOR_RED "Root Privleges Required!\nExiting Application.\n" ANSI_COLOR_RESET);
        return FAILURE;
        }
    if(pathHID() != 0){
        return FAILURE;
        }
    double WLA[4096];
    u_int8_t buffer[10000]={0};
    int e = readFlash(buffer,0,10000);
    if(e == SUCCESS){
        printf("readFlash() Success! [%i]\n",e);
        }else{
        printf("readFlash() Failure. [%i]\n",e);
        }
    //g_commandReport[0]=READ_FLASH_REQUEST;
    //writeReport(g_commandReport);
    //readReport(g_returnReport, READ_FLASH_REQUEST, PACKET_SIZE);
    //getFrameFormat(NULL,NULL,NULL,NULL);
    //readReport(g_returnReport)!=-1?"True":"False";
    //saveData(WLA,NULL);
    return SUCCESS;
    }

int getStatus(void){
    return 0;
    }
    
int pathHID(void){
    //Open HID to start interfacing.
	struct hid_device_info *devs;              //Create device information struct.
	devs = hid_enumerate(VendorID, ProductID); //Create HID device enumeration.
	if(devs != NULL) {                         //If the device can be enumerated, check if any memory was allocated for the device.
		//If device was found, take notes of its path.
        if(g_devicePath != NULL){
			free(g_devicePath);                  //Free the previously allocated memory for the device path character array.
            }
		g_devicePath = malloc((strlen(devs->path)+1)*sizeof(char)); //Allocate exact memory for the path stored in the device information enumeration.
		strcpy(g_devicePath, devs->path);  //Set "DevicePath" to the path stored in the device information enumerator.
        hid_free_enumeration(devs);      //Free the struct containing information about the HID.

		g_deviceDetected = true;           //A device was found. Let it be known globally.
        printf("Spectrometer Found.\n");
        printf("Device Path Obtained.  \"%s\"\n",g_devicePath);
		return 0;
        }

	g_deviceDetected = false;              //A device was not found. Let it be known globally.
    printf("Spectrometer not found.\n");
	return -1;
    }
int readReport(char* rtn, int returncode, int returnReportByteLength){
    int hidReadError;
    
    //Flush contents of returnReport with zeros.
    memset(rtn,0,returnReportByteLength);
    
    //Open HID to start interfacing.
    hid_device *handle;                 //Create HID container.
    handle = hid_open_path(g_devicePath); //Open HID device.
    if(handle==NULL){                   //Check if HID was successfully opened.
        printf("Failed to open HID!\n");
        g_deviceDetected = false;
        hid_close(handle);//Close the door. Its polite.
        return FAILURE;
        }
    if((hidReadError = hid_read_timeout(handle,rtn,returnReportByteLength,1000)) <= 0){ //Scan for output. Return -1 if it takes too long.
        printf("HID Read %s.\n", (hidReadError == -1)?"Error":"Timeout");
        hid_close(handle);//Close the door. Its polite.
        return FAILURE;
        }else if(LOW_BYTE(rtn[0]) != returncode){
        if(DEBUG){printf("Failed to obtain correct return code. [0x%02x != 0x%02x]\n", LOW_BYTE(rtn[0]), returncode);}
        return FAILURE;
        }
    if(DEBUG){printf("Successfully obtained return code. [0x%02x == 0x%02x]\n", LOW_BYTE(rtn[0]), returncode);}

    //Temp to make the hid api respond like windows
    memmove(&rtn[1], rtn, returnReportByteLength);
    rtn[0]=0;
    
    //Close the HID
    hid_close(handle);//Close the door. Its polite.
    if(DEBUG){for(int i=0;i<=(100);i=i+1){printf(" Input|  %02i: 0x%02x\n",i,rtn[i]);}}
    return SUCCESS;
    }
int writeReport(char* cmd_in){
    char cmd[100]={0};
    for(int i=0;i<100+1;i=i+1){
        cmd[i+1] = cmd_in[i];
        }
	cmd[0] = ZERO_REPORT_ID;
	cmd[9] = 0x0E;
    
    if(DEBUG){for(int i=0;i<=100;i=i+1){printf("Output|  %02i: 0x%02x\n",i,cmd[i]);}}
    
	hid_device *handle;
	handle = hid_open_path(g_devicePath);
	if (!handle) {
		printf("Failed to open HID!\n");
		g_deviceDetected = false;
		return -1;
        }
	if(hid_write(handle, cmd, CommandReportByteLength) < 0) {
		printf("HID write error!\n");
        return -1;
        }
	hid_close(handle);
	//memset( OutputReport, 0 , OutputReportByteLength); //Might be important (was commented out)
    return 0;
    }
int calcConst(double* waveLengthArray){
	unsigned char cmd[10];
	unsigned char response[70];
	unsigned char readArray[8192];
	int           bytesRd=80;
	int           flashOffset = 0; //Offset in flash memory.
	int           readCycle;
	char          stringTmp[15];
    double        A1 = 0;
    double        B1 = 0;
    double        C1 = 0;
    int           addr = 0;

    //Initialize "response" and "readArray" to zero.
	memset(response,  0 , 70);
	memset(readArray, 0 , 8192);
    memset(cmd,       0 , 10);

	//calibration coefficients
	if((bytesRd-64*(int)(bytesRd/64))==0){
		readCycle = (int)(bytesRd/64)-1;
        }else{
		readCycle = (int)(bytesRd/64);
        }
	for(int i=0;i<=readCycle;i++){
		addr  = flashOffset+(i*64);
		cmd[0] = READ_FLASH_REQUEST;
		cmd[1] = (char)((addr)>>16);
		cmd[2] = (char)((addr)>>8);
		cmd[3] = (char)(addr);
        cmd[5] = 0x60;
        
		//smpl_ReadAndWriteToDevice(response,cmd,0);
		writeReport(cmd);
        if(readReport(response, CORRECT_READ_FLASH_REPLY, PACKET_SIZE) == FAILURE){
            return FAILURE;
            }
        
        for(int j=0;j<64;j++){
			readArray[i*64+j]=response[j];
            }
        }
    
    //Coefficient A1
	for(int i=0;i<=15;i++){
		stringTmp[i]=readArray[i];
        }
    A1=atof(stringTmp);
    
    //Coefficient B1
	for(int i=0;i<=15;i++){
		stringTmp[i]=readArray[16+i];
        }
    B1=atof(stringTmp);
    
    //Coefficient C1
	for(int i=0;i<=15;i++){
		stringTmp[i]=readArray[32+i];
        }
    C1=atof(stringTmp);

	//end of reading calibration coefficients
    printf("Calibration Coefficients:\n");
	printf("A1= %f\n",A1);
	printf("B1= %f\n",B1);
	printf("C1= %f\n",C1);

	//end of reading calibration coefficients
	for(int i=0;i<3653;i++){
		waveLengthArray[i]=A1*i*i+B1*i+C1;
        }
    }
int readFlash(u_int8_t *buffer,         //Buffer
              u_int32_t absoluteOffset, //0
              u_int32_t bytesToRead){   //64    
    u_int8_t  rtn[EXTENDED_PACKET_SIZE];
    u_int8_t  cmd[10];
    u_int32_t numOfPacketsToGet = 0;
    u_int8_t  numOfPacketsToGetCurrent = 0;
    u_int8_t  numOfPacketsReceivedCurrent = 0;
    u_int8_t  numOfPacketsLeftCurrent = 0;
    
    bool      continueGetInReport = true;
    u_int16_t localOffset = 0;
    u_int32_t totalNumOfReceivedBytes = 0;
    
    u_int8_t  indexOfByteInPacket;
    int       indexInPacket;
    
    u_int32_t offsetIncrement = 0;
    u_int8_t  payloadSize = PACKET_SIZE - 4;
    
    int hidReadError;
    
    if(!g_deviceDetected){
        if(pathHID() != SUCCESS){
            return FAILURE;
            }
        }
    if(buffer == NULL){
        //return INPUT_PARAMETER_NOT_INITIALIZED;
        return FAILURE;
        }
    
    //Flush contents of returnReport with zeros.
    memset(rtn,0,EXTENDED_PACKET_SIZE);
    
    //Open HID to start interfacing.
    hid_device *handle;                 //Create HID container.
    handle = hid_open_path(g_devicePath); //Open HID device.
    if(handle==NULL){                   //Check if HID was successfully opened.
        printf("Failed to open HID!\n");
        g_deviceDetected = false;
        hid_close(handle);//Close the door. Its polite.
        return FAILURE;
        }
        
    numOfPacketsToGet = bytesToRead/payloadSize;        //Integer division. Decimal truncated.
    numOfPacketsToGet += (bytesToRead%payloadSize)?1:0; //Half a quanta cannot exist. If not cleanly divisible, add one.
    
    while(numOfPacketsToGet){
        numOfPacketsToGetCurrent = (numOfPacketsToGet>MAX_READ_FLASH_PACKETS)?MAX_READ_FLASH_PACKETS:numOfPacketsToGet;
        
        cmd[0] = READ_FLASH_REQUEST;
        cmd[1] = LOW_BYTE(LOW_WORD(absoluteOffset + offsetIncrement));
        cmd[2] = HIGH_BYTE(LOW_WORD(absoluteOffset + offsetIncrement));
        cmd[3] = LOW_BYTE(HIGH_WORD(absoluteOffset + offsetIncrement));
        cmd[4] = HIGH_BYTE(HIGH_WORD(absoluteOffset + offsetIncrement));
        cmd[5] = numOfPacketsToGetCurrent;
        
        writeReport(cmd);
        
        numOfPacketsReceivedCurrent = 0;
        
        while(continueGetInReport){
            if((hidReadError = hid_read_timeout(handle,rtn,EXTENDED_PACKET_SIZE,1000)) <= 0){ //Scan for output. Return -1 if it takes too long.
                printf("HID Read %s.\n", (hidReadError == -1)?"Error":"Timeout");
                return FAILURE;
                }else if(LOW_BYTE(rtn[0]) != CORRECT_READ_FLASH_REPLY){
                if(DEBUG){printf("Failed to obtain correct return code. [0x%02x != 0x%02x]\n", LOW_BYTE(rtn[0]), CORRECT_READ_FLASH_REPLY);}
                return FAILURE;
                }
            if(DEBUG){printf("Successfully obtained return code. [0x%02x == 0x%02x]\n", LOW_BYTE(rtn[0]), CORRECT_READ_FLASH_REPLY);}
        
            //Temp to make the hid api respond like windows
            memmove(&rtn[1], rtn, EXTENDED_PACKET_SIZE);
            rtn[0]=0;
            
            ++numOfPacketsReceivedCurrent;
            numOfPacketsLeftCurrent = rtn[4];
        
            if(numOfPacketsLeftCurrent >= REMAINING_PACKETS_ERROR || (numOfPacketsLeftCurrent != numOfPacketsToGetCurrent - numOfPacketsReceivedCurrent)){
                if(DEBUG){printf(ANSI_COLOR_RED"\nREAD_FLASH_REMAINING_PACKETS_ERROR: Dumping variable data.\n"ANSI_COLOR_RESET
                                 "numOfPacketsLeftCurrent >= REMAINING_PACKETS_ERROR || (numOfPacketsLeftCurrent != numOfPacketsToGetCurrent - numOfPacketsReceivedCurrent)\n\n"
                                 "%i >= %i || (%i != %i - %i)\n"
                                 "  numOfPacketsLeftCurrent:     %i\n"
                                 "  REMAINING_PACKETS_ERROR:     %i\n"
                                 "  numOfPacketsToGetCurrent:    %i\n"
                                 "  numOfPacketsReceivedCurrent: %i\n",
                                 numOfPacketsLeftCurrent,
                                 REMAINING_PACKETS_ERROR,
                                 numOfPacketsLeftCurrent,
                                 numOfPacketsToGetCurrent,
                                 numOfPacketsReceivedCurrent,
                                 numOfPacketsLeftCurrent,
                                 REMAINING_PACKETS_ERROR,
                                 numOfPacketsToGetCurrent,
                                 numOfPacketsReceivedCurrent);
                    }
                return FAILURE;
                } //Packet integrity error
            continueGetInReport = (numOfPacketsLeftCurrent > 0)? true : false;
        
            localOffset = (rtn[3] << 8) | rtn[2];
        
            indexInPacket = 4;
            indexOfByteInPacket = 0;        
        
            printf("offsetIncrement: %i\nlocalOffset: %i\nindexOfByteInPacket: %i\nindexInPacket: %i\n",offsetIncrement,localOffset,indexOfByteInPacket,indexInPacket);
        
            while ((totalNumOfReceivedBytes < bytesToRead) && (indexOfByteInPacket < payloadSize)) {
                buffer[offsetIncrement + localOffset + (indexOfByteInPacket++)] = rtn[indexInPacket++];
                ++totalNumOfReceivedBytes;
                }
            }
                
        if(numOfPacketsToGet > MAX_READ_FLASH_PACKETS){
            numOfPacketsToGet -= MAX_READ_FLASH_PACKETS;
            offsetIncrement += MAX_READ_FLASH_PACKETS * payloadSize;            
            }else{
            numOfPacketsToGet = 0;
            }
        }
    
    //Close the HID
    hid_close(handle);//Close the door. Its polite.
    if(DEBUG){for(int i=0;i<=(100);i=i+1){printf(" Input|  %02i: 0x%02x\n",i,rtn[i]);}}
    
    return SUCCESS;
    }
int getFrameFormat(u_int16_t* startElement, 
                   u_int16_t* endElement, 
                   u_int8_t*  reductionMode, 
                   u_int16_t* pixelsInFrame){
    char cmd[CommandReportByteLength];
    char report[PACKET_SIZE];
    
    memset(cmd,    0, CommandReportByteLength);
    memset(report, 0, PACKET_SIZE);\
    cmd[0] = GET_FRAME_FORMAT_REQUEST;
    
    writeReport(cmd);
    
    if(readReport(report,CORRECT_GET_FRAME_FORMAT_REPLY,PACKET_SIZE)){
        return FAILURE;
        }
    //Format data
    if(startElement != 0){
        *startElement = (report[2]<<8)|report[1];
        }
    if(endElement != 0){
        *endElement = (report[4]<<8)|report[3];
        }
    if(reductionMode){
        *reductionMode = report[5];
        }
        
    g_pixelsInFrame = (report[7]<<8)|report[6];
    
    if(pixelsInFrame){
        *pixelsInFrame = g_pixelsInFrame;
        }
    printf("Frame format successfully obtained.\n");
    return SUCCESS;
    }
/*
void deviceIO(unsigned char* deviceOutput, unsigned char* cmd){ //NOT DONE
	if(deviceDetected||detectHID()){
		WriteReport();
		ReadReport();
        }
	if(cmd != NULL){
        unsigned char t2;
		cmd[0] = 0;
		for(int i=0;i<64;i++){
			t2=InputReport[i];
			cmd[i]=t2;
            }
        }
    }
*/
int saveData(double* wavelengthArray, signed short* rawSpec){
    if((wavelengthArray != NULL) && (rawSpec != NULL)){
        //=== Save Results =================================================
        printf("Saving Scan...\n");
        
        //Parse filename.
        char filename[1024];
        sprintf(filename, "./spectra/%u.csv", (unsigned)time(NULL));
        printf("Saving file as %s\n", filename);
        
        //Create folder if one doesn't exist.
        struct stat spectraStat={0};
        if (stat("spectra", &spectraStat) == -1){
            if(mkdir("spectra", 0775) == -1){
                printf("ERROR|  Failed to create directory \"spectra\"!");
                printf("Exiting.");
                return FAILURE;
                }
            }
        fflush(stdout);
        
        //Save file.
        FILE* fp = fopen(filename, "w");
        if(fp != NULL){
            for(int q=0;q<3646;q++){
                fprintf(fp, "%lf,%hd\n", wavelengthArray[q], rawSpec[q]);
                }
            fclose(fp);
            printf("Saved File.\n");
            }else{
            printf("Failed to save file\n");
            }
        return SUCCESS;
        }else{
        printf("Null data passed as arguments! Could not save data.\nExiting.\n");
        return FAILURE;
        }
    }
