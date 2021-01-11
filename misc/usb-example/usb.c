#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <libusb-1.0/libusb.h>

// cc -o usb usb.c -lusb-1.0

int sub();

int main()
{
	libusb_device_handle *handle;
	
	//sub();
	
	int r = libusb_init(NULL);
	if (r < 0)return 0;
	
	handle=libusb_open_device_with_vid_pid(NULL,(uint16_t )0x0c26,(uint16_t )0x0022);
	if(handle == NULL){printf("libusb_open_device_with_vid_pid failed\n");return 1;}
	
	int e=libusb_claim_interface(handle,0);
	if(e != 0){printf("libusb_claim_interface error %d\n",e);return 1;}
	
	//e=libusb_set_interface_alt_setting(handle,0,1);
	//if(e != 0){printf("libusb_set_interface_alt_setting error %d\n",e);return 1;}

	{
		unsigned char cmd[] = { 0xFE, 0xFE, 0x96, 0xE0,  0x1A, 0x13, 0x00, 0x01,  0xFD, 0xFF };
		
		int transferred;
		transferred=0;
		e=libusb_bulk_transfer(handle, 0X02,cmd,(int)sizeof(cmd), &transferred,1000);
	    if(e != 0)printf("libusb_bulk_transfer1 %d\n",e);
	    printf(" transferred %d\n",transferred);
		transferred=0;
		e=libusb_bulk_transfer(handle, 0X88,cmd,(int)sizeof(cmd), &transferred,1000);
	    if(e != 0)printf("libusb_bulk_transfer2 %d\n",e);
	    printf(" transferred %d\n",transferred);

	}

	if(handle)libusb_close(handle);
	
	printf("Done\n");
	return 0;
}

void Sleep(int milliseconds)
{
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

int sub()
{
	int r = libusb_init(NULL);
	if (r < 0)
	    return 0;

	int res = 0;

    libusb_device **devs;
	ssize_t cnt = libusb_get_device_list(NULL, &devs);
    for (int i = 0; i < cnt; i++) {
        libusb_device *device = devs[i];

        struct libusb_device_descriptor desc;
		int r = libusb_get_device_descriptor(device, &desc);
        if (r < 0) continue;

        printf("Device %04x:%04x, Manufacturer: %d\n", desc.idVendor, desc.idProduct, desc.iManufacturer);
        if(desc.idVendor == 0x0c26 && desc.idProduct == 0x0022){
        	printf("Found Device\n");
        }

        libusb_device_handle *handle;
        int err = libusb_open(device, &handle);
        if (err) {
            printf("  Error: cannot open device\n");
            continue;
        }
        // Get description
        const int STRING_LENGTH = 255;
        unsigned char stringDescription[STRING_LENGTH];
        if (desc.iManufacturer > 0) {
            r = libusb_get_string_descriptor_ascii(handle, desc.iManufacturer, stringDescription, STRING_LENGTH);
            if (r >= 0)
                printf("Manufacturer = %s\n",  stringDescription);
        }
        if (desc.iProduct > 0) {
            r = libusb_get_string_descriptor_ascii(handle, desc.iProduct, stringDescription, STRING_LENGTH);
            if ( r >= 0 )
                printf("Product = %s\n", stringDescription);
        }
        if (desc.iSerialNumber > 0) {
            r = libusb_get_string_descriptor_ascii(handle, desc.iSerialNumber, stringDescription, STRING_LENGTH);
            if (r >= 0)
                printf("SerialNumber = %s\n", stringDescription);
        }
        printf("\n");
        libusb_close(handle);
    }

	libusb_free_device_list(devs, 1);
	libusb_exit(NULL);
    return res;
}

 
