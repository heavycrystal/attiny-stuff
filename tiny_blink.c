#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define LED PB1
#define DATA PB5

uint32_t ticks = 0;

void delay(uint32_t milliseconds)
{
	for(uint32_t i = 0; i < (milliseconds / 10); i++)
	{
		_delay_ms(16);
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

void bit_toggle(int* reg, int index)
{
	if((*reg >> (16 - index)) % 2 == 0)
    {
		*reg = *reg | (1 << index);
		PORTB = PORTB | (1 << DATA);
		_delay_us(10);
		PORTB = PORTB & ~(1 << DATA);
		_delay_us(10);
	}
	else if((*reg >> (16 - index)) % 2 == 1)
 	{
		*reg = *reg & ~(1 << index);
		PORTB = PORTB & ~(1 << DATA);
		_delay_us(10);
		PORTB = PORTB & ~(1 << DATA);
		_delay_us(10);
   	}
}

ISR(TIMER0_COMPA_vect)
{
	ticks = ticks + 1;
	// led_on();
	// _delay_ms(100);
	// led_off();
	// _delay_ms(100);	
}

int main()
{

	cli();
	DDRB = DDRB | (1 << LED) | (1 << DATA);
	TCCR0A = 0 | (1 << WGM01);
	TCCR0B = 0 | (1 << CS02);
	OCR0A = 255;
	TIMSK = 0 | (1 << OCIE0A);
	TCNT0 = 0;
	sei();
	led_off();

	/*for(int i = 0; i < 1; i++)
   	{

		uint32_t seed = 17;
		uint32_t increment = 12345;
		uint32_t modulus = 2147483648;
		uint32_t multiplier = 1103515245;
     	uint32_t iterator = ((multiplier * seed) + increment) % modulus;
     	uint32_t counter = 1;
  
     	while(iterator != seed)
     	{
        	iterator = ((multiplier * iterator) + increment) % modulus;
        	counter = counter + 1;
     	}
   	}*/

	int array[225];
	for(uint32_t i = 0; i < 1000; i++)
	{
		for(int j = 0; j < 225; j++)
		{
			for(int k = 0; k < 16; k++)
			{
				bit_toggle(&array[j], k);
			}
		}
	}

	TIMSK = 0 & ~(1 << OCIE0A);
	cli();
	ticks = ticks / 122;

	while(1)
	{
		led_on();
		delay(2000);
		led_off();
		for(int i = 0; i < 32; i++)
      	{
        	if((ticks >> i) % 2 == 0)
        	{
          		led_on();
          		delay(250);
          		led_off();
          		delay(1000);
        	}
        	else if((ticks >> i) % 2 == 1)
        	{
          		led_on();
          		delay(750);
          		led_off();
          		delay(1000);
        	}
      	}
	}
}

