//Include Guard
#ifndef GRBL_COMMUNICATION_INTERFACE
    #define GRBL_COMMUNICATION_INTERFACE

    #include <cstdio>
    #include <cstdlib>
    #include <string>

    #include "serial.h"
    using namespace std;

    class GCI{
        private:
            struct position{
                private:
                    struct coordinate{
                        private:
                            double value;
                            int    id;
		                public:
                            coordinate(int);
			                double get(void);
			                void   set(double);
                        };
                public:
                    coordinate X = coordinate(0);
		            coordinate Y = coordinate(1);
		            coordinate Z = coordinate(2);
                };
            string devicePath = "/dev/ttyS0";
        public:
            GCI();
            position pos;
            int connect();
        };
    GCI::GCI(string devicePath){

        }
    GCI::position::coordinate::coordinate(int input){
        GCI::position::coordinate::id = input;
        }
    double GCI::position::coordinate::get(void){
		return GCI::position::coordinate::value;
		}
	void GCI::position::coordinate::set(double input){
		GCI::position::coordinate::value = input;
        //Send GRBL 
		}
#endif

int main(){
    }
