// Include Guard
#ifndef LINUX_APPLICATION_BASE
    #include <string>
    #include <cstring>
    #include <stdio.h>
    #include <stdlib.h>
    
    #define LINUX_APPLICATION_BASE
    
    struct argument{
        public:
            std::string name;
            char        symbol;
        };
    
    class LAB{
        private:
            std::string title = "Default LAB Application";
            int         argNum;
        public:
            LAB(argument*,int);
            ~LAB();
            std::string getTitle(void);
            void        setTitle(std::string);
            void        setTitle(char*);
            int         getArgNum(void);
            argument*   args;
        };
    
    // Constructor
    LAB::LAB(argument* args_in, int argNum_in){
        LAB::argNum = argNum_in;
        LAB::args = (argument*)malloc(LAB::argNum * sizeof(argument));
        std::memcpy(LAB::args, args_in, sizeof(argument) * LAB::argNum);
        }
    
    // Destructor
    LAB::~LAB(){
        std::free(LAB::args);
        return;
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
        
    // Sets Application Title With Cstr
    void LAB::setTitle(char* title_in){
        LAB::title = std::string(title_in);
        return;
        }
    
    // Returns Argument Count
    int LAB::getArgNum(void){
        return LAB::argNum;
        }
#endif

int main(){
    argument arglist[3] = {{"help",   'h'},
                           {"verbose",'v'},
                           {"getdata",'d'}
                          };
    LAB libspectr(arglist,3);
    printf("argNum: %i\nargument#0: %s\nargument#1: %s\nargument#2: %s\n", libspectr.getArgNum(), 
                                                                           libspectr.args[0].name.c_str(), 
                                                                           libspectr.args[1].name.c_str(), 
                                                                           libspectr.args[2].name.c_str()
                                                                           );
    
    }
