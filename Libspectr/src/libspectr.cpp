#include <string>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#include "LinuxApplicationBaseKai.hpp"
using namespace std;

extern "C"{
    #include "libspectr.c"
    }
    
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define MAX_FLASH_READ 0x1FFE0
    
int g_DEBUG  = 0;
int g_VERBOSE= 0;

int main(int argc, char** argv){
    LAB libspectr;
    libspectr.addArg("help",           'h');
    libspectr.addArg("verbose",        'v');
    libspectr.addArg("getclbr",        'r');
    libspectr.addArg("getframe",       's');
    libspectr.addArg("clearmem",       'c');
    libspectr.addArg("resetdevice",    'e');
    libspectr.addArg("getstatus",      'q');
    libspectr.addArg("settrigger",     't');
    libspectr.addArg("eraseflash",     'd');
    libspectr.addArg("getparam",       'p');
    libspectr.addArg("getframeformat", 'f');
    libspectr.addArg("setallparam",    '~');
    libspectr.addArg("setexposure",    '~');
    
    libspectr.parse(argc, argv);
    
    //Set Verbose
    if(libspectr.isEnabled(libspectr.getIDFromName("verbose"))){
        g_VERBOSE = true;
        }
        
    //Print Help Message
    if(libspectr.isEnabled(libspectr.getIDFromName("help"))){
        printf("==== Libspectr | LR1-B Automation ====                                                    \n"
               "    Command:                                            Description:                               \n"
               "    -h or --help                                        Print this message.                        \n"
               "    -v or --verbose                                     Print an annoying amount of messages.      \n"
               "    -r or --getclbr                      !!![BROKEN]!!! Read LR1-B flash memory and output to file.\n"
               "    -s or --getframe                                    Output Singular Spectrometer Frame.        \n"
               "    -c or --clearmem                                    Clear LR1-B memory.                        \n"
               "    -e or --resetdevice                                 Reset Device Parameters and clear memory   \n"
               "    -q or --getstatus                                   Retrieve status variables.                 \n"
               "    -t or --settrigger                                  Set Software Trigger.                      \n"
               "    -d or --eraseflash                                  Erase LR1-B flash memory.                  \n"
               "    -p or --getparam                                    Retrieve parameter list from device.       \n"
               "    -f or --getframeformat                              Retrieve frame format from device.         \n"
               "    --setexposure <ExposureTime> <Force> !!![BROKEN]!!! Set LR1-B frame exposure time.             \n"
               "    --setallparam <NumberOfScans>                       Set all LR1-B frame parameters.            \n"
               "                  <NumberOfBlankScans>                                                             \n"
               "                  <ScanMode>                                                                       \n"
               "                  <ExposureTime>                                                                   \n"
               "                  <EnableMode>                                                                     \n"
               "                  <SignalFrontMode>                                                                \n"
               );
        return 0;
        }
    
    //Enable if Connecting to Device
    if(libspectr.isEnabled(libspectr.getIDFromName("getclbr"))           ||
       libspectr.isEnabled(libspectr.getIDFromName("clearmem"))          ||
       libspectr.isEnabled(libspectr.getIDFromName("setexposure"))       ||
       libspectr.isEnabled(libspectr.getIDFromName("getframe"))          ||
       libspectr.isEnabled(libspectr.getIDFromName("getstatus"))         ||
       libspectr.isEnabled(libspectr.getIDFromName("eraseflash"))        ||
       libspectr.isEnabled(libspectr.getIDFromName("getframeformat"))    ||
       libspectr.isEnabled(libspectr.getIDFromName("settrigger"))        ||
       libspectr.isEnabled(libspectr.getIDFromName("getparam"))          ||
       libspectr.isEnabled(libspectr.getIDFromName("setallparam"))       ||
       libspectr.isEnabled(libspectr.getIDFromName("setparam"))    
       ){
        if(geteuid() != 0){
            printf( ANSI_COLOR_RED "Root Privleges Required!\nExiting Application.\n" ANSI_COLOR_RESET);
            return 0;
            }
        if(connectToDevice(g_savedSerial) != false){
            return 0;
            }
        }    
        
    //Clear Memory
    if(libspectr.isEnabled(libspectr.getIDFromName("clearmem"))){
        int result = 0;
        result = clearMemory();
        printf("Clear Memory: %s | [%i]\n",result?"Error:":"Success!", result);
        }    
        
    //Reset Device
    if(libspectr.isEnabled(libspectr.getIDFromName("resetdevice"))){
        int result = 0;
        result = resetDevice();
        printf("Reset Device: %s | [%i]\n",result?"Error:":"Success!", result);
        }    
    
    //Set All Acquisition Parameters
    if(libspectr.isEnabled(libspectr.getIDFromName("setallparam"))){
        u_int16_t numOfScans      = 0;
        u_int16_t numOfBlankScans = 0;
        u_int8_t  scanMode        = 0;
        u_int32_t timeOfExposure  = 0;
        u_int8_t  enableMode      = 0;
        u_int8_t  signalFrontMode = 0;
        int       result          = 0;
        
        if(argv[libspectr.getPosition(libspectr.getIDFromName("setallparam")) + 1] <= 0 ||
           argv[libspectr.getPosition(libspectr.getIDFromName("setallparam")) + 2] <= 0 ||
           argv[libspectr.getPosition(libspectr.getIDFromName("setallparam")) + 3] <= 0 ||
           argv[libspectr.getPosition(libspectr.getIDFromName("setallparam")) + 4] <= 0 ||
           argv[libspectr.getPosition(libspectr.getIDFromName("setallparam")) + 5] <= 0 ||
           argv[libspectr.getPosition(libspectr.getIDFromName("setallparam")) + 6] <= 0){
            printf(ANSI_COLOR_RED "\"--setallparam\" Missing parameters!\n" ANSI_COLOR_RESET);
            return -1;
            }
        
        numOfScans      = stoul(argv[libspectr.getPosition(libspectr.getIDFromName("setallparam")) + 1],
                                NULL,
                                0);
        numOfBlankScans = stoul(argv[libspectr.getPosition(libspectr.getIDFromName("setallparam")) + 2],
                                NULL,
                                0);
        scanMode        = stoul(argv[libspectr.getPosition(libspectr.getIDFromName("setallparam")) + 3],
                                NULL,
                                0);
        timeOfExposure  = stoul(argv[libspectr.getPosition(libspectr.getIDFromName("setallparam")) + 4],
                                NULL,
                                0);
        enableMode      = stoul(argv[libspectr.getPosition(libspectr.getIDFromName("setallparam")) + 5],
                                NULL,
                                0);
        signalFrontMode = stoul(argv[libspectr.getPosition(libspectr.getIDFromName("setallparam")) + 6],
                                NULL,
                                0);
        
        result = setAllParameters(numOfScans,
                                  numOfBlankScans,
                                  scanMode,
                                  timeOfExposure,
                                  enableMode,
                                  signalFrontMode
                                  );
        
        //if(g_VERBOSE){
        //    printf("Number of Scans:       %3i\n"
        //           "Number of Blank Scans: %3i\n"
        //           "Scan Mode:             %3i\n"
        //           "Exposure Time:         %3i\n"
        //           "Enable Mode:           %3i\n"
        //           "Signal Front Mode:     %3i\n", numOfScans, numOfBlankScans, scanMode, timeOfExposure, enableMode, signalFrontMode);
        //    }
        
        printf("Set Acquisition Parameters: %s | [%i]\n",result?"Error:":"Success!", result);
        }
    
    //Set Exposure
    if(libspectr.isEnabled(libspectr.getIDFromName("setexposure"))){
        u_int32_t exposureTime = 1; // exposureTime = multiple of 10 us (microseconds)
        u_int8_t  force        = 1; // force = ???
        int       result       = 0;
        
        //printf("%s\n%s\n",argv[libspectr.getPosition(libspectr.getIDFromName("setexposure")) + 1],"0");
        
        if(argv[libspectr.getPosition(libspectr.getIDFromName("setexposure")) + 1] <= 0 ||
           argv[libspectr.getPosition(libspectr.getIDFromName("setexposure")) + 2] <= 0){
            printf(ANSI_COLOR_RED "\"--setexposure\" Missing parameters!\n" ANSI_COLOR_RESET);
            return -1;
            }
        
        exposureTime = stoul(argv[libspectr.getPosition(libspectr.getIDFromName("setexposure")) + 1],
                             NULL,
                             0);
        force = stoul(argv[libspectr.getPosition(libspectr.getIDFromName("setexposure")) + 2],
                      NULL,
                      0);
                      
        result = setExposure(exposureTime, force);
        printf("Set Exposure: %s | [%i]\n",result?"Error: ":"Success!", result);
        }
        
    //Set Trigger
    if(libspectr.isEnabled(libspectr.getIDFromName("settrigger"))){
        int result = 0;
        result = triggerAcquisition();
        printf("Set Trigger: %s | [%i]\n",result?"Error:":"Success!", result);
        }
    
    //Read Flash and Output as File
    if(libspectr.isEnabled(libspectr.getIDFromName("getclbr"))){
        u_int8_t buffer [MAX_FLASH_READ];
        int result = 0;
        
        result = readFlash(buffer, 0, MAX_FLASH_READ);
        
        printf("Saving Scan...\n");
        
        //Parse filename.
        char filename[1024];
        sprintf(filename, "./Spectra/%u.clbr", (unsigned)time(NULL));
        printf("Saving file as %s\n", filename);
        
        //Create folder if one doesn't exist.
        struct stat spectraStat={0};
        if (stat("Spectra", &spectraStat) == -1){
            if(mkdir("Spectra", 0775) == -1){
                printf("ERROR|  Failed to create directory \"Spectra\"!\n");
                printf("Exiting.\n");
                return -1;
                }
            }
        fflush(stdout);
        
        //Save file.
        FILE* fp = fopen(filename, "w");
        if(fp != NULL){
            unsigned char bom[3] = {0xEF,0xBB,0xBF};
            
            fwrite(bom,sizeof(char),sizeof(bom),fp);           //Write BOM Header
            fwrite(buffer,sizeof(u_int8_t),sizeof(buffer),fp); //Write Contents
            
            fclose(fp);
            printf("Saved File.\n");
            }else{
            printf("Failed to save file\n");
            return -1;
            }
            
        printf("Read Flash: %s | [%i]\n",result?"Error:":"Success!", result);
        return 0;
        }
        
    //Read Frame and Output as File
    if(libspectr.isEnabled(libspectr.getIDFromName("getframe"))){
        const int framePixelsBufferSize                    = 3694;
        u_int16_t framePixelsBuffer[framePixelsBufferSize] = {0}; //3693 | 3648
        u_int16_t frameIndex                               = 0;
        int result = 0;
        
        result = getFrame(framePixelsBuffer, frameIndex);
        
        if(g_VERBOSE){
            for(int i=0;i<framePixelsBufferSize;i++){
                printf("%i: %05hu\n", i, framePixelsBuffer[i]);
                }
            }
                         
            printf("Saving Frame...\n");
            
            //Parse filename.
            char filename[1024];
            sprintf(filename, "./Framedata/Frame_%u.csv", (unsigned)time(NULL));
            printf("Saving file as %s\n", filename);
            
            //Create folder if one doesn't exist.
            struct stat spectraStat={0};
            if (stat("Framedata", &spectraStat) == -1){
                if(mkdir("Framedata", 0775) == -1){
                    printf("ERROR|  Failed to create directory \"Framedata\"!\n");
                    printf("Exiting.\n");
                    return -1;
                    }
                }
            fflush(stdout);
            
            //Save file.
            FILE* fp = fopen(filename, "w");
            if(fp != NULL){
                //fwrite(framePixelsBuffer,sizeof(u_int16_t),sizeof(framePixelsBuffer),fp);
                for(int i=0;i<framePixelsBufferSize;i++){
                    fprintf(fp,"%hu\n",framePixelsBuffer[i]);
                    }
                fclose(fp);
                printf("Saved File.\n");
                }else{
                printf("Failed to save file\n");
                }
        
        printf("Get Frame: %s | [%i]\n",result?"Error:":"Success!", result);
        }
        
    //Erase Flash
    if(libspectr.isEnabled(libspectr.getIDFromName("eraseflash"))){
        int result = 0;
        result = eraseFlash();
        printf("Erase Flash: %s | [%i]\n",result?"Error:":"Success!", result);
        }
                
    //Print Status
    if(libspectr.isEnabled(libspectr.getIDFromName("getstatus"))){
        u_int8_t  statusFlags    = 0;
        u_int16_t framesInMemory = 0;
        int result = 0;
        
        result = getStatus(&statusFlags, &framesInMemory);
        
        printf("Status Flag:                  %5i\n"
               "Number of Frames in Memory:   %5i\n", statusFlags, framesInMemory
               );
        
        printf("Print Status: %s | [%i]\n",result?"Error:":"Success!", result);
        }
        
    //Print Frame Format
    if(libspectr.isEnabled(libspectr.getIDFromName("getframeformat"))){
        u_int16_t numOfStartElement  = 0;
        u_int16_t numOfEndElement    = 0;
        u_int8_t  reductionMode      = 0;
        u_int16_t numOfPixelsInFrame = 0;
        int result = 0;
        
        result = getFrameFormat(&numOfStartElement,
                                &numOfEndElement,
                                &reductionMode,
                                &numOfPixelsInFrame
                                );

        printf("Number of Initial Element: %5i\n"
               "Number of Final Element:   %5i\n"
               "Reduction Mode:            %5i\n"
               "Number of Pixels in Frame: %5i\n", numOfStartElement, numOfEndElement, reductionMode, numOfPixelsInFrame);
               
        printf("Get Frame Format: %s | [%i]\n",result?"Error:":"Success!", result);
        }
        
    //Print Acquisition Parameters
    if(libspectr.isEnabled(libspectr.getIDFromName("getparam"))){
        u_int16_t numOfScans      = 0;
        u_int16_t numOfBlankScans = 0;
        u_int8_t  scanMode        = 0;
        u_int32_t timeOfExposure  = 0;
        int       result          = 0;
        
        getAcquisitionParameters(&numOfScans,
                                 &numOfBlankScans,
                                 &scanMode,
                                 &timeOfExposure
                                 );
        
        printf("Number of Scans:       %3i\n"
               "Number of Blank Scans: %3i\n"
               "Scan Mode:             %3i\n"
               "Exposure Time:         %3i\n", numOfScans, numOfBlankScans, scanMode, timeOfExposure);
               
        printf("Get Parameters: %s | [%i]\n",result?"Error:":"Success!", result);
        }
        
    disconnectDevice();
    }
