#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libusb-1.0/libusb.h>


// cc -o usbTry2 usbTry2.c -lusb-1.0

#define MFGR_ID 0x0c26 // given manufacturer ID 
#define DEV_ID 0x0022  // given device ID

/* If device IDs are not known, use libusb_get_device_list() to see a 
list of all USB devices connected to the machine. Follow this call with    
libusb_free_device_list() to free the allocated device list memory.
*/


int main() {
    int init = libusb_init(NULL); // NULL is the default libusb_context

    if (init < 0) {
        printf("Error 1\n");
    }

    struct libusb_device_handle *dh = NULL; // The device handle
    dh = libusb_open_device_with_vid_pid(NULL,MFGR_ID,DEV_ID);

    if (!dh) {
        printf("Error 2\n");
    }

    // set fields for the setup packet as needed              
    uint8_t       bmReqType = 0;   // the request type (direction of transfer)
    uint8_t            bReq = 0;   // the request field for this packet
    uint16_t           wVal = 0;   // the value field for this packet
    uint16_t         wIndex = 0;   // the index field for this packet
    unsigned char   data[10];   // the data buffer for the in/output data
    uint16_t         wLen = 0;   // length of this setup packet 
    unsigned int     to = 0;       // timeout duration (if transfer fails)

    // transfer the setup packet to the USB device
    int config =     
    libusb_control_transfer(dh,bmReqType,bReq,wVal,wIndex,data,wLen,to);

    if (config < 0) {
        printf("config %d\n",config);
    }

    // now you can use libusb_bulk_transfer to send raw data to the device

    libusb_exit(NULL);
}
