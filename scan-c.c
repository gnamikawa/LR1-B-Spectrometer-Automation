/*
 ============================================================================
 Name        : scan-c.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
#include <sys/stat.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include <gtk/gtk.h>

#include "gpio.h"
#include "gpio.c"

#define PIN 20
#define POUT 14
#define DEBUG 0

#include "smpl1.h"
#include "smpl1.c"

static GtkLabel* lblMsg;
static char led = 0;
static GThread * workerThread = NULL;


double WavelengthArray[4096];
bool   waveRead;

static gpointer doScan(gpointer data);

void   readWavelength(double * WaveLengthArray);
void   readSpec(int ExpN, int NScans, int Blank, signed short * rawSpec);


gboolean setText(gpointer txt){
	gtk_label_set_text(lblMsg, txt);
	return FALSE;
    }

static gpointer thread_func( gpointer data ){
	while(TRUE){
		sleep(50);

		printf("Checking pin\n");
		int value = GPIOPoll(PIN);
		printf("Pin value is %d\n", value);

		char * actionTxt = "Button push";
		gdk_threads_add_idle(setText, actionTxt);
        }

	return( NULL );
    }

/*
G_MODULE_EXPORT void on_btnLed_clicked(GtkButton *button, gpointer data){
	printf("button press\n");

	gtk_label_set_text(lblMsg, "Starting");
	//GPIOWrite(POUT, led == 0 ? HIGH : LOW);
	//led = (led + 1) % 2;

	//Check to see if spec connected;
	//Move to another thread!

	if(workerThread != NULL)
		g_thread_join(workerThread);

	workerThread = g_thread_new(NULL, doScan, NULL);
    }
*/

int main(int argc, char *argv[]){
	if(DEBUG)printf("DEBUG|  Execute function: main()\n");

	if(-1 == GPIOExport(POUT) || -1 == GPIOExport(PIN)){
		printf("ERROR|  Export Option Failed\n");
        }

	if(-1 == GPIODirection(POUT, OUT) || -1 == GPIODirection(PIN, IN)){
		printf("ERROR|  Direction Option Failed\n");
        }

    /*
	//---------------------------------
	//----- CREATE THE GTK WINDOW -----
	//---------------------------------
	GtkBuilder *gtkBuilder;
	GtkWidget *window;

	gtk_init(&argc, &argv);

	gtkBuilder = gtk_builder_new();
	gtk_builder_add_from_file(gtkBuilder, "res/layout.glade", NULL);
	window = GTK_WIDGET(gtk_builder_get_object(gtkBuilder, "window1"));

	lblMsg = (GtkLabel*)GTK_WIDGET (gtk_builder_get_object (gtkBuilder, "lblMsg"));
	gtk_label_set_text(lblMsg, "This is the test label");

	gtk_builder_connect_signals(gtkBuilder, NULL );

	g_object_unref(G_OBJECT(gtkBuilder));
	gtk_widget_show(window);
	printf("Start2\n");

	//g_thread_new(NULL, thread_func, NULL );

	fflush(stdout);
	gtk_main();
    */
    
    if(workerThread != NULL){
		g_thread_join(workerThread);
        }

	workerThread = g_thread_new(NULL, doScan, NULL); //Execute doScan() in a different thread
    
	if(workerThread != NULL){
		g_thread_join(workerThread);
        }

	return 0;
    }


static gpointer doScan(gpointer data){
	if(DEBUG)printf("DEBUG|  Execute function: doScan()\n");
	fflush(stdout);
	if(!smpl_DevDetect()){ //Check if the device was detected earlier in boolean variable "DeviceDetected".
		if(!smpl_FindTheHID()){ //If the boolean variable "DeviceDetected" was not set, try to see if the device was detected.
            printf("Spectroscope not found.\n"
                   "Exiting.\n");
			return FALSE;
            }
        }
    printf("Spectroscope found.\n");
    
	GPIOWrite(POUT, HIGH);

	smpl_reset();
	printf("Wavelength\n");
	fflush(stdout);
	if(waveRead == false) {
		printf("Reading Wavelength Calibration\n");
		readWavelength(WavelengthArray);
		waveRead = true;

		//Todo check for success here.
	}
	printf("Running Scan...\n");
	fflush(stdout);
    signed short rawSpec[4096];
    
	memset(rawSpec, 0,4096);
	readSpec(21,1,0,rawSpec);
    
    
    //=== Save Results =================================================
	printf("Saving Scan...\n");
	fflush(stdout);
    
    //Parse filename.
	char filename[1024];
	sprintf(filename, "./spectra/%u.csv", (unsigned)time(NULL));
	printf("Saving file as %s\n", filename);
	fflush(stdout);
    
    //Create folder if one doesn't exist.
    struct stat spectraStat={0};
    if (stat("spectra", &spectraStat) == -1){
        if(mkdir("spectra", 0775) == -1){
            printf("ERROR|  Failed to create directory \"spectra\"!");
            printf("Exiting.");
            return FALSE;
            }
        }
        
    //Save file.
	FILE * fp = fopen(filename, "w" );
	if(fp != NULL) {

		int q = 0;

		for(q = 0; q < 3646; q++) {
			fprintf(fp, "%lf,%hd\n", WavelengthArray[q], rawSpec[q]);
		}
		fclose(fp);

		gdk_threads_add_idle(setText, "Test Complete");
	} else {
		gdk_threads_add_idle(setText, "Failed to write test results");
	}

	GPIOWrite(POUT, LOW);

    //==================================================================
    
	return FALSE;
}

void readWavelength(double* WaveLengthArray){
	unsigned char cmd[10];
	unsigned char response[70];
	unsigned char ReadArray[8192];

    //Initialize "response" and "ReadArray" to zero.
	memset(response, 0 , 70);
	memset(ReadArray, 0 , 8192);

	int BytesRd=80;
	int AddressFLASH=0;
	int readCycle;
	/*reading itself*/

	//calibration coefficients
	if((BytesRd-64*(int)(BytesRd/64))==0)
		readCycle=(int)(BytesRd/64)-1;
	else
		readCycle=(int)(BytesRd/64);

	for(int i=0;i<=readCycle;i++)
	{
		//int addr=AddressFLASH+i*64;
		//cmd[1]=0xA1;
		//cmd[2]=(char)((addr)>>16);
		//cmd[3]=(char)((addr)>>8);
		//cmd[4]=(char)(addr);
        
		int addr=AddressFLASH+i*64;
		cmd[1]=0x1A;
		cmd[2]=(char)((addr)>>16);
		cmd[3]=(char)((addr)>>8);
		cmd[4]=(char)(addr);


		smpl_ReadAndWriteToDevice(response,cmd,0);
		for(int j=0;j<64;j++)
			ReadArray[i*64+j]=response[j];
	}
    
	char StringTmp[15];
    
    //Coefficient A1
	for(int i=0;i<=15;i++)
		StringTmp[i]=ReadArray[i];
	double A1=atof(StringTmp);
    
    //Coefficient B1
	for(int i=0;i<=15;i++)
		StringTmp[i]=ReadArray[16+i];
	double B1=atof(StringTmp);
    
    //Coefficient C1
	for(int i=0;i<=15;i++)
		StringTmp[i]=ReadArray[32+i];
	double C1=atof(StringTmp);

	//end of reading calibration coefficients
    printf("Calibration Coefficients:\n");
	printf("A1= %f\n",A1);
	printf("B1= %f\n",B1);
	printf("C1= %f\n",C1);

	//end of reading calibration coefficients
	for(int i=0;i<3653;i++)
		WavelengthArray[i]=A1*i*i+B1*i+C1;
}

void readSpec(int ExpN, int NScans, int Blank, signed short * rawSpec) {
	smpl_resetAddress();

	bool Trigger=0;
	bool KeepTrigger=0;
	bool Fast=0;

	//send command to get spectra to memory
	unsigned char cmd[10];
	unsigned char recievedData[70];

	memset(cmd, 0, 10); //Set all values of cmd[] to zero.
	cmd[0]=3;           //
	cmd[1]=ExpN;	    //low
	cmd[2]=ExpN>>8;	    //high
	cmd[3]=NScans;	    //nmbScans
	cmd[4]=Blank;	    //blank scans number
	cmd[5]=1;           //
	if(Trigger==0)
		cmd[6]=0;
	else
		if(KeepTrigger==0)
			cmd[6]=1;
		else
			cmd[6]=3;

	smpl_ReadAndWriteToDevice(NULL,cmd,0);

	//check if data is already in memory
	sleep((int)(ExpN*2.375*(NScans*(Blank+1))) / 1000);

	memset(cmd,0,10); //reuse the command array

	//This will wait until the spectroscope says the data has been collected
	//cmd[1]=2;//get status
	cmd[0]=0;//get status
	cmd[1]=1;//get status
	smpl_ReadAndWriteToDevice(recievedData,cmd,0);
	while(recievedData[3]!=0)
		smpl_ReadAndWriteToDevice(recievedData,cmd,1);

	//read data
	smpl_resetAddress();

	for(int i=1;i<=NScans;i++)
	{
		smpl_GetSpectra(rawSpec, 1,0, 3652, Fast, 0, 33, 3685);
		/*  OutputReport1[1]=9;//move address
			OutputReport1[2]=0x01;
			OutputReport1[3]=0x80;
			ReadAndWriteToDevice(InputReport1,OutputReport1,1);
        */

		//todo normalize?
	}
    
	smpl_resetAddress();
}
