#include <stdio.h>
#include <stdlib.h>
#include <libusb-1.0/libusb.h>

// cc -o usbTry3 usbTry3.c -lusb-1.0

int main()
{
	libusb_context *ctx;
	libusb_device_handle *handle;

	libusb_init(&ctx);
	//libusb_set_debug(ctx, 3);
	libusb_set_option(ctx, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_DEBUG);
	handle = libusb_open_device_with_vid_pid(ctx, 0x0c26, 0x0022);
	if(handle == NULL){printf("libusb_open_device_with_vid_pid failed\n");exit(1);}
	//libusb_set_configuration(handle, 1);
	
	int e=libusb_claim_interface(handle,0);
	if(e != 0){printf("libusb_claim_interface error %d\n",e);return 1;}

	e=libusb_set_interface_alt_setting(handle,0,1);
	if(e != 0){printf("libusb_set_interface_alt_setting error %d\n",e);return 1;}


	{
		unsigned char cmd[] = { 0xFE, 0xFE, 0x96, 0xE0,  0x1A, 0x13, 0x00, 0x01,  0xFD, 0xFF };
		
		int transferred;
		transferred=0;
		e=libusb_bulk_transfer(handle, 0X02,cmd,(int)sizeof(cmd), &transferred,2000);
	    if(e != 0)printf("libusb_bulk_transfer1 %d\n",e);
	    printf(" transferred %d\n",transferred);
		transferred=0;
		e=libusb_bulk_transfer(handle, 0X88,cmd,(int)sizeof(cmd), &transferred,2000);
	    if(e != 0)printf("libusb_bulk_transfer2 %d\n",e);
	    printf(" transferred %d\n",transferred);

	}



	libusb_release_interface(handle, 0);
	libusb_close(handle);
	libusb_exit(ctx);
	return 0;
}
