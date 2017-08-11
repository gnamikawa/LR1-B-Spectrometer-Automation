#ifndef _cplusplus
extern "C"{
    extern int g_DEBUG;
    extern int g_VERBOSE;
    }
#endif

#ifdef _cplusplus
    extern int g_DEBUG;
    extern int g_VERBOSE;
#endif

// Include Guard
#ifndef LINUX_APPLICATION_BASE
    #include <string>
    #include <cstring>
    #include <stdio.h>
    #include <stdlib.h>
    #include <new>
    
    #define LINUX_APPLICATION_BASE
    

    
    struct argument{
        public:
            std::string name;
            char        symbol;
            int         id         = -1;
            int         position   = -1;
            bool        enabled    = false;
        };
    
    class LAB{
        private:
            std::string title = "Default LAB Application";
            int         argNum = 0;
            argument    args[100];
        public:
            LAB(void);
            ~LAB(void);
            std::string getTitle(void);
            void        setTitle(std::string);
            void        setTitle(char*);
            void        addArg(std::string,char);
            int         getArgNum(void);
            bool        checkArgExist(std::string);
            bool        checkArgExist(char);
            bool        isEnabled(int);
            int         getPosition(int);
            std::string getNameFromID(int);
            std::string getNameFromSymbol(char);
            int         getIDFromSymbol(char);
            int         getIDFromName(std::string);
            char        getSymbolFromName(std::string);
            char        getSymbolFromID(int);
            void        parse(int, char**);
        };
    
    //Constructor
    LAB::LAB(void){
        } 
    
    // Destructor
    LAB::~LAB(void){
        }
    
    // Returns Application Title
    std::string LAB::getTitle(void){
        return LAB::title;
        }
        
    // Sets Application Title
    void LAB::setTitle(std::string title_in){
        LAB::title = title_in;
        return;
        }
    void LAB::setTitle(char* title_in){
        LAB::title = std::string(title_in);
        return;
        }
    
    // Add Argument and Increment argNum
    void LAB::addArg(std::string name, char symbol){
        argument newArg = {name, symbol, LAB::argNum};
        LAB::args[LAB::argNum] = newArg;
        LAB::argNum++;
        }
    
    // Returns Argument Count
    int LAB::getArgNum(void){
        return LAB::argNum;
        }
        
    // Returns True if Argument Exists
    bool LAB::checkArgExist(std::string arg_in){
        for(int i=0; i<LAB::argNum; i++){
            if(arg_in == LAB::args[i].name){
                LAB::args[i].enabled = true;
                return true;
                }
            }
        return false;
        }
    bool LAB::checkArgExist(char arg_in){
        for(int i=0; i<LAB::argNum; i++){
            if(arg_in == LAB::args[i].symbol){
                LAB::args[i].enabled = true;
                return true;
                }
            }
        return false;
        }
    
    //Return Enabled State
    bool LAB::isEnabled(int id_in){
        return LAB::args[id_in].enabled;
        }
    
    //Return Argument Position 
    int LAB::getPosition(int id){
        return LAB::args[id].position;
        }
    
    // Args Manipulation
    std::string LAB::getNameFromID(int id_in){
        for(int i=0; i<LAB::argNum; i++){
            if(id_in == LAB::args[i].id){
                return LAB::args[i].name;
                }
            }
        return "-1";
        }
    std::string LAB::getNameFromSymbol(char symbol_in){
        for(int i=0; i<LAB::argNum; i++){
            if(symbol_in == LAB::args[i].symbol){
                return LAB::args[i].name;
                }
            }
        return "(null)";
        }
    int LAB::getIDFromSymbol(char symbol_in){
        for(int i=0; i<LAB::argNum; i++){
            if(symbol_in == LAB::args[i].symbol){
                return LAB::args[i].id;
                }
            }
        return -1;
        }
    int LAB::getIDFromName(std::string name_in){
        for(int i=0; i<LAB::argNum; i++){
            if(name_in == LAB::args[i].name){
                return LAB::args[i].id;
                }
            }
        return -1;
        }
    char LAB::getSymbolFromName(std::string name_in){
        for(int i=0; i<LAB::argNum; i++){
            if(name_in == LAB::args[i].name){
                return LAB::args[i].symbol;
                }
            }
        return '\0';
        }
    char LAB::getSymbolFromID(int id_in){
        for(int i=0; i<LAB::argNum; i++){
            if(id_in == LAB::args[i].id){
                return LAB::args[i].symbol;
                }
            }
        return '\0';
        }
    
    //Parse Input
    void LAB::parse(int argc, char** argv){
        if(argc > 1){
            for(int i=0;i<argc;i=i+1){
                if(argv[i][0]=='-'){
                    if(argv[i][1]=='-'){
                        if(g_DEBUG)printf("DEBUG|  == Interperting [%s] as a string literal command ==\n",argv[i]);
                        if(g_DEBUG)printf("DEBUG|  Argument name: %s\n",std::string(argv[i]).substr(2,std::string(argv[i]).npos).c_str());
                        if(checkArgExist(std::string(argv[i]).substr(2,std::string(argv[i]).npos)) == true){
                            LAB::args[LAB::getIDFromName(std::string(argv[i]).substr(2,std::string(argv[i]).npos))].position = i;
                            }else{
                            printf("Invalid option: '%s'\n", std::string(argv[i]).substr(2,std::string(argv[i]).npos).c_str());
                            }
                        }else{
                        if(g_DEBUG)printf("DEBUG|  == Interperting [%s] as a character command ==\n",argv[i]);
                        for(int j=1;argv[i][j]!='\0';j=j+1){
                            if(g_DEBUG)printf("DEBUG|  Argument: %c\n",argv[i][j]);
                            if(checkArgExist(argv[i][j]) == true){
                                LAB::args[LAB::getIDFromSymbol(argv[i][j])].position = i;
                                }else{
                                printf("Invalid option: '%c'\n", argv[i][j]);
                                }
                            }
                        }
                    }
                }
            }else{
            printf("Missing Arguments: -h for help\n");
            }
        }
    
#endif
