#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>

#define MAX_LOADSTRING 256

//extern "C" {
#include "smpl1.h"
#include "hidapi.h"
#include "hid.c"

//Application global variables 
unsigned long ActualBytesRead;
unsigned long BytesRead;
//HIDP_CAPS     Capabilities;
unsigned long cbBytesRead;
//PSP_DEVICE_INTERFACE_DETAIL_DATA	detailData;
bool          DeviceDetected;
//HANDLE        DeviceHandle;

char * DevicePath = NULL; //freed in shutdown

unsigned long	         dwError;
unsigned char	         InputReport[100];
unsigned long	         Length;
unsigned long	         NumberOfBytesRead;
unsigned long	         Required;
const unsigned short int MAXREPORTSIZE = 256;
unsigned char*           OutputReport;//[MAXREPORTSIZE];
int                      temp1;
//unsigned int             ProductID=0x0001;
unsigned int             ProductID=0x0100;
unsigned char            DevGeneralNmb;
unsigned char            DevInitNmb=1;




bool smpl_FindTheHID(){ //Working.
    //e220:0100
	//const unsigned int VendorID = 0x20E2;	// ASEQ Instruments vendor ID
	const unsigned int VendorID = 0xE220;	// ASEQ Instruments vendor ID

	struct hid_device_info *devs;

	devs = hid_enumerate(VendorID, ProductID);
	if(devs != NULL) { //Just find the first spectrocope. Multiple scopes not currently supported
		if(DevicePath != NULL){
			free(DevicePath);
        }
		DevicePath = malloc((strlen(devs->path) + 1) * sizeof(char));
		strcpy(DevicePath, devs->path);

		hid_free_enumeration(devs);

		//GetDeviceCapabilities();
		DeviceDetected = true;
		return true;
	}

	DeviceDetected = false;
	return false;

	//////////////////////////////////////////////////////////////////////
}

#define OutputReportByteLength 9
#define InputReportByteLength 64
void WriteReport()
{
	OutputReport[0]=0;
	OutputReport[9]=15;
	//	OutputReport[65]=15;

	hid_device *handle;
	handle = hid_open_path(DevicePath);
	if (!handle) {
		printf("unable to open device\n");
		DeviceDetected = false;
		return;
	}
	int res = hid_write(handle, OutputReport, OutputReportByteLength);
	if(res < 0) {
		// error
		printf("Unable to write report\n");
	}

	hid_close(handle);

	//memset( OutputReport, 0 , OutputReportByteLength);
}

void ReadReport()
{
	memset(InputReport,0,InputReportByteLength);

	hid_device *handle;
	handle = hid_open_path(DevicePath);
	if (!handle) {
		printf("unable to open device\n");
		DeviceDetected = false;
		return;
	}
	int res = hid_read_timeout(handle, InputReport, InputReportByteLength, 1*1000);
	if(res <= 0) {
		// error
	}
	//Temp to make the hid api respond like windows
	memmove(&InputReport[1], InputReport, InputReportByteLength );
	InputReport[0]=0;

	hid_close(handle);
}


void smpl_ReadAndWriteToDevice_new(unsigned char	*InputReport1, unsigned char	*OutputReport1, int	DevDet)
{
	int k1;
	unsigned char t2;

	OutputReport=OutputReport1;

	if(DevDet!=0)
	{
		DeviceDetected=false;
	}

	if (DeviceDetected==false)
		DeviceDetected=smpl_FindTheHID();

	if (DeviceDetected==true)
	{
		WriteReport();
		ReadReport();
	}

	if(InputReport1 != NULL) {
		for(k1=1;k1<=64;k1++)
		{
			t2=InputReport[k1];
			InputReport1[k1-1]=t2;
		}
	}
}

void smpl_ReadAndWriteToDevice(unsigned char	*InputReport1, unsigned char * OutputReport1, int DevDet)
{
	unsigned char t2;

	OutputReport=OutputReport1;

	if(DevDet!=0)
	{
		DeviceDetected=false;
	}

	if (DeviceDetected==false)
		DeviceDetected=smpl_FindTheHID();

	if (DeviceDetected==true)
	{
		WriteReport();
		ReadReport();
	}

	if(InputReport1 != NULL) {
		InputReport1[0] = 0;
		for(int k1=0;k1<=63;k1++)
		{
			t2=InputReport[k1];
			InputReport1[k1]=t2;
		}

	}
}

int smpl_GetSpectra(signed short *InputSpec1, unsigned char SpecNmb, unsigned short startPix, unsigned short endPix, unsigned char Fast, unsigned char test1, unsigned short tot_startPix, unsigned short tot_endPix)
{
	unsigned short tmp1;
	unsigned short tmp2;
	signed   short InputSpec_loc[4096];
	unsigned char  startTrans;
	unsigned char  endTrans;
	unsigned short startPTshift;
	unsigned short endPTshift;
	int            RLn;
	int            devd;
	int            cnt1;

	RLn=32;


	startPix+=tot_startPix;
	endPix+=tot_startPix;
	if(Fast==false)//get all pixels in the range
	{
		if((startPix<64)&(endPix>=3616))//get full spectra
		{
            OutputReport=(char*)malloc(100);
			OutputReport[1]=4;//read
			OutputReport[3]=SpecNmb;
			OutputReport[4]=0;//read every pixel
			for(int k1=0;k1<=115;k1++)
			{
				if(k1==0)
				{
					OutputReport[2]=1;//start ping-pong
				}
				if(k1==115)
				{
					OutputReport[2]=2;//end ping-pong
				}
				if((k1>=1)&(k1<=114))
				{
					OutputReport[2]=0;//normal reading
				}
				if (DeviceDetected==true)
				{
					WriteReport();
					ReadReport();
				}
				for (int ByteNumber=0; ByteNumber <= 31; ByteNumber++)
				{
					cnt1=k1*RLn+ByteNumber;
					tmp1=((unsigned short)(InputReport[ByteNumber*2+1]))&0x00ff;
					if(InputReport[ByteNumber*2+2]<=0x3F)
					{
						tmp2=(((unsigned short)(InputReport[ByteNumber*2+2]))<<8)&0x3f00;
					}
					tmp1=tmp1|tmp2;
					InputSpec_loc[cnt1]=0x3FFF-tmp1;//16383 temporal line
				}
			}
		}
		//transmission to *InputSpec1

		/* JDS
			for(k1=0;k1<=115;k1++)
			{
				for (ByteNumber=0; ByteNumber <= 31; ByteNumber++)
				{
					cnt1=k1*RLn+ByteNumber;
					InputSpec_loc[cnt1]=InputSpec_loc[cnt1]*1.05;
				}
			}*/

		devd=0;
		//			devd=InputSpec_loc[3687]+InputSpec_loc[3688]+InputSpec_loc[3689]+InputSpec_loc[3690]+InputSpec_loc[3691]+InputSpec_loc[3692]+InputSpec_loc[3693]+InputSpec_loc[3694];
		//			devd/=8;
		for(int k1=15;k1<=31;k1++)
			devd+=InputSpec_loc[k1];
		devd/=17;

		//if(devd>16000)
		//	devd=0;
		for(int k1=0;k1<=115;k1++)
		{
			for (int ByteNumber=0; ByteNumber <= 31; ByteNumber++)
			{
				cnt1=k1*RLn+ByteNumber;
				if(InputSpec_loc[cnt1]>16300 && false)
				{
					if((cnt1>=startPix)&(cnt1<=endPix)) InputSpec1[cnt1-startPix]=16383-devd;
				}
				else
				{
					if((cnt1>=startPix)&(cnt1<=endPix)) InputSpec1[cnt1-startPix]=(InputSpec_loc[cnt1]/*-devd*/);//*(16383-450)/(16383-devd)+450;
				}
			}
		}
		//end of transmission to *InputSpec1
	}
    free(OutputReport);
	return devd;
}

bool smpl_DevDetect()
{
	return DeviceDetected;
}

void smpl_reset() {
	/*reset, initialization*/
	unsigned char cmd[10];
	memset(cmd, 0, 10);
	cmd[1]=0xF1;
	smpl_ReadAndWriteToDevice(NULL,cmd,1);
}

void smpl_resetAddress() {
	unsigned char cmd[10];
	memset(cmd, 0, 10);

	cmd[1]=0x03;
	smpl_ReadAndWriteToDevice(NULL,cmd,1);
}

void smpl_shutdown() {
	free(DevicePath);
	DevicePath = NULL;
}
