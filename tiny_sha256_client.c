#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <usb.h>

#define TIMEOUT                         5000
#define DEVICE_VENDOR_ID                0x16C0
#define DEVICE_VENDOR                   "Ilmetzik"
#define DEVICE_PRODUCT_ID               0x05DC
#define DEVICE_PRODUCT                  "SHA256 Accelerator"

#define     RECV_NEW_HASH               0u
#define     DATA_RECEIVE                1u
#define     DATA_TRANSMIT               2u

#define SIGINT 2

int usb_get_descriptor_string(struct usb_dev_handle *dev, int index, int lang_id, char *buffer, int buffer_length)
{
    char lbuffer[256];
    int return_value;
    int cindex;

    // send control message to device, requesting descriptor.
    return_value = usb_control_msg(dev, USB_TYPE_STANDARD | USB_ENDPOINT_IN | USB_RECIP_DEVICE, USB_REQ_GET_DESCRIPTOR, 
                                    ((USB_DT_STRING) << 8) + index, lang_id, lbuffer, sizeof(lbuffer), TIMEOUT);

    // error out as underlying function failed.
    if(return_value < 0)
    {
        fprintf(stderr, "ERROR: usb_control_msg errored out with return value %i.\n", return_value);
        return return_value;
    }

    // actual size of response is actually the first byte of buffer
    return_value = (uint8_t)(lbuffer[0]);
    // ensure data type is the right one or error out
    if(lbuffer[1] != USB_DT_STRING)
    {
        fprintf(stderr, "ERROR: Received invalid response type.\n");
        return 0;
    }

    // begin conversion of UTF-16 little-endian response to "ISO-Latin1" whose lower half is standard ASCII.
    return_value = return_value / 2;

    for(cindex = 1; (cindex < return_value) && (cindex < buffer_length); cindex = cindex + 1) 
    {
        if(lbuffer[(2 * cindex) + 1] == 0)
        {
            buffer[cindex - 1] = lbuffer[2 * cindex];
        }    
        else
        {
            buffer[cindex - 1] = '?'; /* outside of ISO Latin1 range */
        }    
    }
    buffer[cindex - 1] = '\0';
    return cindex - 1;               
}

static usb_dev_handle* usb_open_device(int vendor, char* match_device_vendor, int product, char* match_device_product) 
{
    struct usb_bus* bus;
    struct usb_device* dev;
    char device_vendor[256];
    char device_product[256];
    struct usb_dev_handle* handle = NULL;

    usb_init();
    usb_find_busses();
    usb_find_devices();

    // iterate over all devices attached to all busses
    for(bus = usb_get_busses(); bus; bus=bus->next) 
    {
        for(dev=bus->devices; dev; dev=dev->next) 
        {			
            if(dev->descriptor.idVendor != vendor || dev->descriptor.idProduct != product)
            {
                continue;
            }

            // open the device to pass on to usb_get_descriptor_string, error on failure
            if(!(handle = usb_open(dev))) 
            {
                fprintf(stderr, "WARNING: cannot open USB device: %s\n", usb_strerror());
                continue;
            }

            // get vendor name, 0x0409 is English Language
            if(usb_get_descriptor_string(handle, dev->descriptor.iManufacturer, 0x0409, device_vendor, 
                sizeof(device_vendor)) < 0) 
            {
                fprintf(stderr, "WARNING: cannot query manufacturer for device: %s\n", usb_strerror());
                usb_close(handle);
                continue;
            }

            // get produce name
            if(usb_get_descriptor_string(handle, dev->descriptor.iProduct, 0x0409, device_product, 
                sizeof(device_product)) < 0) 
            {
                fprintf(stderr, "WARNING: cannot query product for device: %s\n", usb_strerror());
                usb_close(handle);
                continue;
            }

            if((strcmp(device_vendor, match_device_vendor) == 0) && (strcmp(device_product, match_device_product) == 0))
            {
                return handle;
            }    
            else
            {
                usb_close(handle);
            }    
        }
    }

    // no match
    return NULL;
}

void sigint_handler()
{
    printf("\nGoodbye!\n");
    exit(0);
}

int main(int argc, char** argv)
{
    signal(SIGINT, sigint_handler);
    int return_value;
    int read_bytes;
    int chunk_count = 0;
    unsigned char buffer[64];

    struct usb_dev_handle* handle = usb_open_device(DEVICE_VENDOR_ID, DEVICE_VENDOR, DEVICE_PRODUCT_ID, DEVICE_PRODUCT);
    if(handle == NULL)
    {
        fprintf(stderr, "ERROR: Could not find requested device.\n");
        return 1;
    }

    return_value = return_value = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, 
            RECV_NEW_HASH, 0, 0, buffer, sizeof(buffer), TIMEOUT); 

    FILE* file_handle = fopen(argv[argc - 1u], "rb");
    if(file_handle == NULL)
    {
        fprintf(stderr, "ERROR: Could not open file.");
        exit(1);
    }    

    printf("Dispatching chunks:           ");
    while(!feof(file_handle))
    {
        read_bytes = fread(buffer, 1u, 64u, file_handle);
        return_value = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT, 
            DATA_TRANSMIT, read_bytes, read_bytes, buffer, read_bytes, TIMEOUT);   
        chunk_count = chunk_count + 1;  
        if((chunk_count % 100) == 0)
        {   
            printf("\b\b\b\b\b\b\b\b\b%09d", chunk_count);
            fflush(stdout);
        }            
    }

    printf("\b\b\b\b\b\b\b\b\b%09d", chunk_count);
    fflush(stdout);
    printf("\n");
    sleep(1);
    return_value = return_value = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, 
            DATA_RECEIVE, 0, 0, buffer, sizeof(buffer), TIMEOUT); 

    printf("%s  -   ", argv[argc - 1u]);
    for(read_bytes = 0; read_bytes < 8u; read_bytes++)
    {
        printf("%08x", *((uint32_t*)(&buffer[4 * read_bytes])));
    }
    printf("\n");

    fclose(file_handle);
    return 0;
}    