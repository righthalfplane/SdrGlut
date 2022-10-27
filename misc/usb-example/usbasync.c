#include <stdio.h>
#include <stdlib.h>
#include <libusb-1.0/libusb.h>

// cc -o usbasync usbasync.c -lusb-1.0

int do_exit = 0;

void my_callback(struct libusb_transfer *transfer)
{


    switch(transfer->status)
    {
        case LIBUSB_TRANSFER_ERROR: break;
        case LIBUSB_TRANSFER_NO_DEVICE: break;
        case LIBUSB_TRANSFER_CANCELLED:  break;
        case LIBUSB_TRANSFER_COMPLETED:
                                printf("LIBUSB_TRANSFER_COMPLETED\n");
								//save_file(transfer->buffer,transfer->actual_length);
                                 break;
        case LIBUSB_TRANSFER_TIMED_OUT:
									
                                printf("LIBUSB_TRANSFER_TIMED_OUT\n");
									break;
        case LIBUSB_TRANSFER_STALL: 
									break;
        case LIBUSB_TRANSFER_OVERFLOW:
									break;
        default: 
        							break;
    }

        do_exit = 1;
}



int main()
{
 // libusb initialization done

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




 		struct libusb_transfer *transfer_in  = libusb_alloc_transfer(0);
                              
      	{
			unsigned char cmd[] = { 0xFE, 0xFE, 0x96, 0xE0,  0x1A, 0x13, 0x00, 0x01,  0xFD, 0xFF };
                        
                              
 		libusb_fill_bulk_transfer( transfer_in, handle, 0X02,cmd, 10,my_callback, NULL, 200);
 		
 		}
 
		int ret = libusb_submit_transfer(transfer_in);
 
 while(!do_exit)
 {
    libusb_handle_events(NULL);
 }
 	libusb_release_interface(handle, 0);
	libusb_close(handle);
	libusb_exit(ctx);
	return 0;

}

/*
ii) Synchronous

main()
{

 ret = libusb_bulk_transfer(dh, 0x82,rcvBuff,200000,&transferred, 0);
 
}
*/