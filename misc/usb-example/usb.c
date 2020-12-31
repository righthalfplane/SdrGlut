

#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <libusb-1.0/libusb.h>

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

// cc -o usb usb.c -lusb-1.0

void Sleep(int milliseconds)
{
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
}


int main()
{
	int r = libusb_init(NULL);
	if (r < 0)
	    return FALSE;

	int res = FALSE;

    libusb_device **devs;
	ssize_t cnt = libusb_get_device_list(NULL, &devs);
    for (int i = 0; i < cnt; i++) {
        libusb_device *device = devs[i];

        struct libusb_device_descriptor desc;
		int r = libusb_get_device_descriptor(device, &desc);
        if (r < 0) continue;

        printf("Device %04x:%04x, Manufacturer: %d\n", desc.idVendor, desc.idProduct, desc.iManufacturer);

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

 