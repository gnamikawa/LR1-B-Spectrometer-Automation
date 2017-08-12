//Include Guard
#ifndef GRBL_COMMUNICATION_INTERFACE
    #define GRBL_COMMUNICATION_INTERFACE

    #include <cstdio>
    #include <cstdlib>
    #include <string>
    
    extern "C"{
        #include "libusb.h"
        }
    using namespace std;
    
    #define SUCCESS 0
    
    #define BULK_TRANSFER_FAILED 500
    #define DEVICE_NOT_FOUND     501
    #define LIBUSB_VERBOSITY_LEVEL 3

    class GCI{
        private:
            const libusb_interface *inter;
            libusb_endpoint_descriptor  endPointDescriptor;
            libusb_interface_descriptor interfaceDescriptor;
            libusb_device_descriptor    deviceDescriptor;
            libusb_config_descriptor*   configDescriptor;
            
            libusb_device_handle*       deviceHandle = NULL;
            libusb_device*              device;
            //libusb_context**            sessionContext;
            const int VENDOR_ID;
            const int PRODUCT_ID;
            int endpointAddressIn  = 0x00;
            int endpointAddressOut = 0x00;
            struct position{
                private:
                    struct coordinate{
                        private:
                            double     value     = 0;
                            int        id        = 0;
                            static int initCount;
		                public:
                            coordinate(void);
                            int    getId(void);
			                double get(void);
			                void   set(double);
                        };
                public:
                    coordinate X = coordinate();
		            coordinate Y = coordinate();
		            coordinate Z = coordinate();
                };
            int readFromDevice(void);
        public:
            position pos;
            GCI(int,int);
            ~GCI(void);
            int connect(void);
            int writeToDevice(string);
            //void printdev(libusb_device *dev);
           
        };
    GCI::GCI(int vid, int pid) : VENDOR_ID(vid), PRODUCT_ID(pid) {
        libusb_init(NULL);
        libusb_set_debug(NULL, LIBUSB_VERBOSITY_LEVEL);
        //libusb_get_endpoint_descriptor(this->device, 0, &config);
        //libusb_get_config_descriptor(this->device, 0, &config);
        
        //printf("Number of endpoints: %i\n", (int)this->interfaceDescriptor.bNumEndpoints);
        //for(int i=0; i<(int)this->interfaceDescriptor.bNumEndpoints; i++) {
        //    this->endPointDescriptor = this->interfaceDescriptor.endpoint[i];
        //    printf("Descriptor Type: %i\n", (int)this->endPointDescriptor.bDescriptorType);
        //    printf("EP Address:      %i\n", (int)this->endPointDescriptor.bEndpointAddress);
        //    }
        }
    GCI::~GCI(void){
        if(this->deviceHandle != NULL){
            libusb_close(this->deviceHandle);
            }
        libusb_exit(NULL);
        }
    
    int GCI::position::coordinate::initCount = 0;

    GCI::position::coordinate::coordinate(){
        this->id = this->initCount;
        this->initCount++;
        }
    int GCI::position::coordinate::getId(void){
        return this->id;
        }
    double GCI::position::coordinate::get(void){
		return this->value;
		}
    int GCI::connect(void){
        this->deviceHandle = libusb_open_device_with_vid_pid(NULL, this->VENDOR_ID, this->PRODUCT_ID);
        
        if(!this->deviceHandle){
            printf("Error finding USB device\n");
            libusb_close(this->deviceHandle);
            libusb_exit(NULL);
            return DEVICE_NOT_FOUND;
            }else{
            printf("USB device connected!\n");
            return SUCCESS;
            }
        }
	void GCI::position::coordinate::set(double input){
		this->value = input;
        //this->getId()
        //GRBL: Send Coordinate Through Serial Port
		}
    int GCI::writeToDevice(string message){
        int actual_length;
        unsigned char buffer;
        
        /* To send a char to the device simply initiate a bulk_transfer to the
         * Endpoint with address ep_out_addr.
         */
        for(uint i=0;i<message.size();i++){
            buffer = message.at(i);
            if (libusb_bulk_transfer(this->deviceHandle,       //Device Handle
                                     this->endpointAddressOut, //Endpoint Output Address
                                     &buffer,                  //Output Character
                                     1,                        //Length of Data Buffer
                                     &actual_length,           //Callback
                                     1000) < 0) {              //Timeout in (ms)
                printf("Error while sending char\n");
                return BULK_TRANSFER_FAILED;
                }
            printf("%c",buffer);
            }
        return SUCCESS;
        }
    //void GCI::printdev(libusb_device *dev) {
    //    int r = libusb_get_device_descriptor(dev, &(this->deviceDescriptor));
    //    if (r < 0) {
    //        printf("failed to get device descriptor\n");
    //        return;
    //    }
    //    printf("Number of possible configurations: %i\n", (int)this->deviceDescriptor.bNumConfigurations);
    //    printf("Device Class: %i\n",(int)this->deviceDescriptor.bDeviceClass);
    //    printf("VendorID: %i\n", this->deviceDescriptor.idVendor);
    //    printf("ProductID: %i\n", this->deviceDescriptor.idProduct);
    //    libusb_get_config_descriptor(dev, 0, &(this->configDescriptor));
    //    printf("Interfaces: %i\n", (int)this->configDescriptor->bNumInterfaces);
    //    for(int i=0; i<(int)this->configDescriptor->bNumInterfaces; i++) {
    //        inter = &(this->configDescriptor)->interface[i];
    //        printf("Number of alternate settings: %i\n", inter->num_altsetting);
    //        for(int j=0; j<inter->num_altsetting; j++) {
    //            interdesc = &inter->altsetting[j];
    //            printf("Interface Number: %i\n", (int)interdesc->bInterfaceNumber);
    //            printf("Number of endpoints: %i\n", (int)interdesc->bNumEndpoints);
    //            for(int k=0; k<(int)interdesc->bNumEndpoints; k++) {
    //                epdesc = &interdesc->endpoint[k];
    //                printf("Descriptor Type: %i\n", (int)epdesc->bDescriptorType);
    //                printf("EP Address: %i\n", (int)epdesc->bEndpointAddress);
    //            }
    //        }
    //    }
    //    libusb_free_config_descriptor(this->configDescriptor);
    //}
#endif

void printdev(libusb_device *dev); //prototype of the function

int main(){
    GCI LR1B(0x2341,0x0043); //Vendor ID, Product ID
    LR1B.connect();
    //LR1B.printdev(this->device);
    //LR1B.writeToDevice("G0 X10");
    //printf("Printing in order:\nX: %i\nY: %i\nZ: %i\n", LR1B.pos.X.getId(), LR1B.pos.Y.getId(), LR1B.pos.Z.getId());
    }
//int main() {
//	libusb_device **devs; //pointer to pointer of device, used to retrieve a list of devices
//	libusb_context *ctx = NULL; //a libusb session
//	int r; //for return values
//	ssize_t cnt; //holding number of devices in list
//	r = libusb_init(&ctx); //initialize a library session
//	if(r < 0) {
//		cout<<"Init Error "<<r<<endl; //there was an error
//				return 1;
//	}
//	libusb_set_debug(ctx, 3); //set verbosity level to 3, as suggested in the documentation
//	cnt = libusb_get_device_list(ctx, &devs); //get the list of devices
//	if(cnt < 0) {
//		cout<<"Get Device Error"<<endl; //there was an error
//	}
//	cout<<cnt<<" Devices in list."<<endl; //print total number of usb devices
//		ssize_t i; //for iterating through the list
//	for(i = 0; i < cnt; i++) {
//				printdev(devs[i]); //print specs of this device
//		}
//		libusb_free_device_list(devs, 1); //free the list, unref the devices in it
//		libusb_exit(ctx); //close the session
//		return 0;
//}


