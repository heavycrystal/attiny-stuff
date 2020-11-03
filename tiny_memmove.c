#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define LED PB1
#define SIZE 230
#define ITERATIONS 1000000

volatile uint32_t ticks = 0;
bool flag = false;
uint32_t loop_const;
uint16_t delay_const;
uint8_t source[SIZE];
uint8_t destination[SIZE];

void delay(uint16_t milliseconds)
{
	for(delay_const = 0; delay_const < (milliseconds / 10); delay_const++)
	{
		_delay_ms(10);
	}
}

void led_on()
{
	PORTB = PORTB | (1 << LED);
}

void led_off()
{
	PORTB = PORTB & ~(1 << LED);
}

void flash_num(uint32_t input)
{
    led_on();
	delay(2000);
	led_off();
	for(loop_const = 0; loop_const < 32; loop_const++)
    {
     	if((input >> loop_const) % 2 == 0)
     	{
       		led_on();
       		delay(250);
      		led_off();
      		delay(1000);
    	}
    	else if((input >> loop_const) % 2 == 1)
    	{
      		led_on();
      		delay(750);
      		led_off();
      		delay(1000);
    	}
    }
}

ISR(TIMER1_COMPA_vect)
{
	ticks = ticks + 1;
    if((ticks % (122L * 5L)) == 0)
    {
        led_on();
        delay(10);
        led_off();
    }
}

int main(void)
{
    cli();
    GTCCR = GTCCR | (1 << TSM) | (1 << PSR0) | (1 << PSR1);
	DDRB = DDRB | (1 << LED);
	//TCCR0A = 0 | (1 << WGM01);
	//TCCR0B = 0 | (1 << CS02);
	//OCR0A = 255;
	TIMSK = 0 | (1 << OCIE1A);
	TCCR1 = 0 | (1 << CTC1) | (1 << CS13) | (1 << CS10);
	OCR1C = 255;
	OCR1A = 255;
	//TCNT0 = 0;
    TCNT1 = 0;
	GTCCR = GTCCR & ~(1 << TSM);
	sei();
	led_off();

    for(loop_const = 0; loop_const < SIZE; loop_const++)
    {
        source[loop_const] = loop_const;
    }
    ticks = 0;

    for(loop_const = 0; loop_const < ITERATIONS / 2; loop_const++)
    {
        memcpy(destination, source, sizeof(uint8_t) * SIZE);
        memcpy(source, destination, sizeof(uint8_t) * SIZE);
    }

    cli();
    ticks = ticks / 122;

    for(loop_const = 0; loop_const < SIZE; loop_const++)
    {
        if(source[loop_const] != loop_const)
        {
            flag = true;
        }
    }

	if(flag == false)
    {
        while(1)
	    {  
            flash_num(ticks);
        }
    }
    else
    {
        led_on();
        while(1)
        {

        }
    }        
}