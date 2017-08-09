/* Linux Application Base Template
 * Genzo Namikawa
 * 06/11/2017
 * genzo.namikawa@jacks.sdstate.edu
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "libspectr.c"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define MAX_FLASH_READ 0x1FFE0

/*
struct flags{
    int help;
    int verbose;
    int unknown;
    }g_argFlags;
*/

enum flags{UNKNOWN,HELP,VERBOSE,GETFLASH,SETPARAM,ARGNUM};
struct flag{
    bool enabled;
    char* str_name;
    enum flags id;
    };
    
struct flag g_argFlags[]={ {false, "unknown",  UNKNOWN  },
                           {false, "help",     HELP     },
                           {false, "verbose",  VERBOSE  },
                           {false, "getflash", GETFLASH },
                           {false, "setparam", SETPARAM }
                           };

//=== CONFIGURATION START ==========================================
//const string PROGNAME = "LinuxBaseTemplate"; //Name of program
//const string PROGHELP = ""; //Help message of program
int g_DEBUG   = 1;
int g_VERBOSE = 1;
const int VALIDEXEC   = 2; //Number of arguments for a valid execution of the program.
const char helpMessage[] = "Help was printed\n";
//=== CONFIGURATION END ============================================

int main(int argc, char* argv[]){ 
    
    /* argc | Argument Count    argv | Argument Values
     * 
     * argv[] holds the arguments that were defined in the command line. (including the command itself)
     * 
     * argv[0]    | holds the name of the command that was run
     * argv[argc] | holds a null pointer
     */
     
    
    int labMain(int, char**);
    void printUnknownArguments(char);
    int argSwitch(char);
    
    if(argc<VALIDEXEC){
        printf("Missing Arguments: -h for help\n");
        }else{
        for(int i=0;i<argc;i=i+1){
            if(argv[i][0]=='-'){
                if(argv[i][1]=='-'){
                    int strComp = 0;
                    if(g_DEBUG)printf("DEBUG|  == Interperting [%s] as a string literal command ==\n",argv[i]);
                    //if(g_DEBUG)printf("DEBUG|  Argument name: %s\n",string(argv[i]).substr(2,string(argv[i]).npos).c_str());
                    for(int k=0;k<ARGNUM;k++){
                        //if(g_argFlags[k].str_name==string(string(argv[i]).substr(2,string(argv[i]).npos).c_str())){
                            strComp = k;
                            //if(g_DEBUG)printf("DEBUG|  Comparison: %s | %s [%s]\n",g_argFlags[k].str_name.c_str(),
                            //                                                     string(string(argv[i]).substr(2,string(argv[i]).npos).c_str()).c_str(),
                            //                                                     g_argFlags[k].str_name==string(string(argv[i]).substr(2,string(argv[i]).npos).c_str())?"true":"false"
                            //                                                     );
                            break;
                            //}
                        //if(g_DEBUG)printf("DEBUG|  Comparison: %s | %s [%s]\n",g_argFlags[k].str_name.c_str(),
                        //                                                     string(string(argv[i]).substr(2,string(argv[i]).npos).c_str()).c_str(),
                        //                                                     g_argFlags[k].str_name==string(string(argv[i]).substr(2,string(argv[i]).npos).c_str())?"true":"false"
                        //                                                     );
                        }
                    if(argSwitch((char)strComp)){
                        //printf("Invalid option: '%s'\n", string(argv[i]).substr(2,string(argv[i]).npos).c_str());
                        }
                    }else{
                    if(g_DEBUG)printf("DEBUG|  == Interperting [%s] as a character command ==\n",argv[i]);
                    for(int j=1;argv[i][j]!='\0';j=j+1){
                        if(g_DEBUG)printf("DEBUG|  Argument: %c\n",argv[i][j]);
                        if(argSwitch(argv[i][j])){
                            printf("Invalid option: '%c'\n", argv[i][j]);
                            }
                        }
                    }
                }
            }
        }
    return labMain(argc, argv);
    }
    
//=== ARGUMENT FUNCTIONS START =========================================
int argSwitch(char input){
    switch(input){
        /* Program 'Char' Arguments Go Here */
        case 'H':
        case 'h':
        case HELP:
            g_argFlags[HELP].enabled = true;
            if(g_DEBUG)printf("DEBUG|  Help flag enabled [%s]\n\n", g_argFlags[HELP].enabled?"true":"false");
            break;
        case 'v':
        case VERBOSE:
            g_argFlags[VERBOSE].enabled = true;
            if(g_DEBUG)printf("DEBUG|  Verbose flag enabled [%s]\n\n", g_argFlags[VERBOSE].enabled?"true":"false");
            break;
        case 'g':
        case GETFLASH:
            g_argFlags[GETFLASH].enabled = true;
            if(g_DEBUG)printf("DEBUG|  GetFlash flag enabled [%s]\n\n", g_argFlags[GETFLASH].enabled?"true":"false");
            break;
        case 's':
        case SETPARAM:
            g_argFlags[SETPARAM].enabled = true;
            if(g_DEBUG)printf("DEBUG|  SetParam flag enabled [%s]\n\n", g_argFlags[SETPARAM].enabled?"true":"false");
            break;
        case UNKNOWN:
        default:
            g_argFlags[UNKNOWN].enabled = true;
            if(g_DEBUG)printf("DEBUG|  Unknown Argument flag enabled [%s]\n\n", g_argFlags[UNKNOWN].enabled?"true":"false");
            return 1;
        }
    return 0;
    }
int labMain(int argc, char** argv){
    u_int8_t buffer [MAX_FLASH_READ*2];
    u_int8_t buffer1[MAX_FLASH_READ];
    u_int8_t buffer2[MAX_FLASH_READ];
            
    if(geteuid() != 0){
        printf( ANSI_COLOR_RED "Root Privleges Required!\nExiting Application.\n" ANSI_COLOR_RESET);
        return -1;
        }
    if(g_argFlags[UNKNOWN].enabled){
        printf("-h for help\n");
        }else if(argc==1){
        return -1;
        }else if(g_argFlags[HELP].enabled){
        printf("%s",helpMessage);
        }else{
        connectToDevice(g_savedSerial);
        if(g_argFlags[GETFLASH].enabled){
            if(readFlash(buffer1, 0, MAX_FLASH_READ) == 0){
                readFlash(buffer2, MAX_FLASH_READ+1, 0x1FFFF-MAX_FLASH_READ);
                strcat(buffer, buffer1);
                strcat(buffer, buffer2);
                
                printf("Saving Scan...\n");
                
                //Parse filename.
                char filename[1024];
                sprintf(filename, "./spectra/%u.csv", (unsigned)time(NULL));
                printf("Saving file as %s\n", filename);
                
                //Create folder if one doesn't exist.
                struct stat spectraStat={0};
                if (stat("spectra", &spectraStat) == -1){
                    if(mkdir("spectra", 0775) == -1){
                        printf("ERROR|  Failed to create directory \"spectra\"!\n");
                        printf("Exiting.\n");
                        return -1;
                        }
                    }
                fflush(stdout);
                
                //Save file.
                FILE* fp = fopen(filename, "w");
                if(fp != NULL){
                    //char bom[3] = {0xEF,0xBB,0xBF};
                    //sprintf(buffer, "%s%c",bom,buffer);
                    //for(int q=0;q<MAX_FLASH_READ;q++){
                    
                    fwrite(buffer,sizeof(char),sizeof(buffer),fp);
                    
                    //    }
                    fclose(fp);
                    printf("Saved File.\n");
                    }else{
                    printf("Failed to save file\n");
                    }
                return 0;
                }
            }
        if(g_argFlags[SETPARAM].enabled){
            u_int16_t numOfScans      = argv[2];
            u_int16_t numOfBlankScans = argv[3];
            u_int8_t  scanMode        = argv[4];
            u_int32_t timeOfExposure  = argv[5];
            
            setAcquisitionParameters(numOfScans, 
                                    numOfBlankScans,
                                    scanMode,
                                    timeOfExposure);
            }
        }
    disconnectDevice();
    return 0;
    }
//=== ARGUMENT FUNCTIONS END ===========================================
