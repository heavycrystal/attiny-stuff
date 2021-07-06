#include <stdint.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "vusb/usbdrv/usbdrv.h"

#define     AS_U32(input)               ((u32) input)                                          
#define     RIGHT_ROTATE(input, dist)   ((input >> dist) | (input << (32u - dist)))

#define     LED                         PB1 

#define     RECV_NEW_HASH               0u
#define     DATA_RECEIVE                1u
#define     DATA_TRANSMIT               2u

#define     ABS(x)                      ((x) > 0 ? (x) : (-x))

typedef     uint8_t                     u8;
typedef     uint32_t                    u32;

static u8 working_buffer[256];
static const u32 start_hash_values[8] PROGMEM = 
{   0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19 };
static u32 hash_values[8];
static u32 round_hash_values[8];
static const u32 round_constants[64] PROGMEM = 
{   0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5, 
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2 };  

static uint64_t total_file_size;
static u8 data_length;
static u8 data_received;

u32 construct_u32(u32 index)
{
    return ((((u32)working_buffer[(4u * index)]) << 24u) + (((u32)working_buffer[(4u * index) + 1u]) << 16u) + (((u32)working_buffer[(4u * index) + 2u]) << 8u) + ((u32)working_buffer[(4u * index) + 3u]));
}

void hadUsbReset() 
{
    int frameLength, targetLength = (unsigned)(1499 * (double)F_CPU / 10.5e6 + 0.5);
    int bestDeviation = 9999;
    uchar trialCal;
    uchar bestCal;
    uchar step;
    uchar region;

    for(region = 0; region <= 1; region++) 
    {
        frameLength = 0;
        trialCal = (region == 0) ? 0 : 128;
        
        for(step = 64; step > 0; step >>= 1) 
        { 
            if(frameLength < targetLength) 
                trialCal += step; 
            else
                trialCal -= step; 
                
            OSCCAL = trialCal;
            frameLength = usbMeasureFrameLength();
            
            if(ABS(frameLength-targetLength) < bestDeviation) 

            {
                bestCal = trialCal;
                bestDeviation = ABS(frameLength -targetLength);
            }
        }
    }

    OSCCAL = bestCal;
}

void pulse_led(int milliseconds)
{
    PORTB |= (1 << LED);
    milliseconds = milliseconds / 10;    
    for(int i = 0; i < milliseconds; i++)
    {
        _delay_ms(10);
    }    
    PORTB &= ~(1 << LED);
}

USB_PUBLIC uchar usbFunctionSetup(uchar data[8])
{
    usbRequest_t* setup_data = (void *)data; 
	
    switch(setup_data->bRequest) 
    { 
        case RECV_NEW_HASH:
            pulse_led(100);
            for(int i = 0; i < 8; i++)
            {
                hash_values[i] = ((u32)pgm_read_dword_near(start_hash_values + i));
            }
            total_file_size = 0;
            return 0;

        case DATA_RECEIVE:
            usbMsgPtr = hash_values;
            return sizeof(hash_values);

        case DATA_TRANSMIT: 
            data_length = (uchar)(setup_data->wLength.word);
            data_received = 0; 
            total_file_size = total_file_size + (8 * data_length);            

            return USB_NO_MSG;
    }
    return 0;
}

void u32_to_buffer(u32 number, u8 index)
{
    u8 loop_var = 0u;

    for(; loop_var < 4u; loop_var++)
    {
        working_buffer[(4u * index) + loop_var] = (number) >> (8u * (3u - loop_var)); 
    }
}

void processor()
{
    u8 loop_var = 16u;
    u32 temp_vars[6u];     

    for(; loop_var < 64u; loop_var++)
    {   
        temp_vars[0u] = RIGHT_ROTATE(construct_u32(loop_var - 15u), 7u) ^ RIGHT_ROTATE(construct_u32(loop_var - 15u), 18u) ^ (construct_u32(loop_var - 15u) >> 3u);
        temp_vars[1u] = RIGHT_ROTATE(construct_u32(loop_var - 2u), 17u) ^ RIGHT_ROTATE(construct_u32(loop_var - 2u), 19u) ^ (construct_u32(loop_var - 2u) >> 10u); 
        u32_to_buffer(construct_u32(loop_var - 16u) + temp_vars[0u] + construct_u32(loop_var - 7u) + temp_vars[1u], loop_var);       
    }
     
    for(loop_var = 0u; loop_var < 8u; loop_var++)
    {
        round_hash_values[loop_var] = hash_values[loop_var];
    }

    for(loop_var = 0u; loop_var < 64u; loop_var++)
    {
        temp_vars[0u] = RIGHT_ROTATE(round_hash_values[4u], 6u) ^ RIGHT_ROTATE(round_hash_values[4u], 11u) ^ RIGHT_ROTATE(round_hash_values[4u], 25u);
        temp_vars[1u] = (round_hash_values[4u] & round_hash_values[5u]) ^ (~(round_hash_values[4u]) & round_hash_values[6u]);
        temp_vars[2u] = round_hash_values[7u] + temp_vars[0u] + temp_vars[1u] + ((u32)pgm_read_dword_near(round_constants + loop_var)) + construct_u32(loop_var);
        temp_vars[3u] = RIGHT_ROTATE(round_hash_values[0u], 2u) ^ RIGHT_ROTATE(round_hash_values[0u], 13u) ^ RIGHT_ROTATE(round_hash_values[0u], 22u);
        temp_vars[4u] = (round_hash_values[0u] & round_hash_values[1u]) ^ (round_hash_values[0u] & round_hash_values[2u]) ^ (round_hash_values[1u] & round_hash_values[2u]);
        temp_vars[5u] = temp_vars[3u] + temp_vars[4u];

        round_hash_values[7u] = round_hash_values[6u];
        round_hash_values[6u] = round_hash_values[5u];
        round_hash_values[5u] = round_hash_values[4u];
        round_hash_values[4u] = round_hash_values[3u] + temp_vars[2u];
        round_hash_values[3u] = round_hash_values[2u];
        round_hash_values[2u] = round_hash_values[1u];
        round_hash_values[1u] = round_hash_values[0u];
        round_hash_values[0u] = temp_vars[2u] + temp_vars[5u];
    }

    for(loop_var = 0u; loop_var < 8u; loop_var++)
    {
        hash_values[loop_var] = hash_values[loop_var] + round_hash_values[loop_var];
    }
}

USB_PUBLIC uchar usbFunctionWrite(uchar* data, uchar len)
{
    for(uchar i = 0; (data_received < data_length) && (i < len); i++, data_received++)
    {
        working_buffer[data_received] = data[i];
    }    

    if((data_received == data_length) && (data_received < 64u))
    {
        if(data_received < 56u)
        {
            // pulse_led(100);
            working_buffer[data_received++] = 0x80u;
            for(; data_received < 56u; data_received++)
            {
                working_buffer[data_received] = 0u; 
            }
            for(; data_received < 64u; data_received++)
            {
                working_buffer[data_received] = (total_file_size) >> (8u * (63u - data_received));     
            }                
            processor();
        }
        else
        {
            // pulse_led(200);            
            working_buffer[data_received++] = 0x80u;
            for(; data_received < 64u; data_received++)
            {
                working_buffer[data_received] = 0u; 
            }
            processor();

            for(data_received = 0u; data_received < 56u; data_received++)
            {
                working_buffer[data_received] = 0u;
            }
            for(; data_received < 64u; data_received++)
            {
                working_buffer[data_received] = (total_file_size) >> (8u * (63u - data_received));     
            }
            processor();             
        }
    }
    else if(data_received == data_length)
    {
        processor();
    }

    return (data_received == data_length);
} 

int main(void)
{
    wdt_enable(WDTO_1S);

    usbInit();

    usbDeviceDisconnect();
    _delay_ms(500);
    wdt_reset();
    usbDeviceConnect();

    sei();

    while(1)
    {
        wdt_reset();
        usbPoll();
    }
}