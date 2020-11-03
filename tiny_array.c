#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define LED PB1
#define ITERATIONS 1000

uint32_t ticks = 0;
uint32_t sticks = 0;
uint32_t iticks = 0;
uint32_t lticks = 0;
uint32_t llticks = 0;
uint32_t fticks = 0;
uint32_t dticks = 0;
uint32_t ddticks = 0;

void delay(uint32_t milliseconds)
{
	for(uint32_t i = 0; i < (milliseconds / 10); i++)
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

/*void short_array()
{
    short array[50];
    for(int i = 0; i < 50; i++)
    {
        array[i] = 1000UL / 7;
    }
    for(uint32_t i = 0; i < ITERATIONS; i++)
    {
        for(int j = 0; j < 50; j++)
        {
            array[j] = array[j] / 18;
            array[j] = array[j] * 18;
        }
    }
}*/

void int_array()
{
    int array[50];
    for(int i = 0; i < 50; i++)
    {
        array[i] = 1000UL / 7;
    }
    for(uint32_t i = 0; i < ITERATIONS; i++)
    {
        for(int j = 0; j < 50; j++)
        {
            array[j] = array[j] / 18;
            array[j] = array[j] * 18;
        }
    }
}

/*void long_array()
{
    long array[50];
    for(int i = 0; i < 50; i++)
    {
        array[i] = 1000UL / 7;
    }
    for(uint32_t i = 0; i < ITERATIONS; i++)
    {
        for(int j = 0; j < 50; j++)
        {
            array[j] = array[j] / 18;
            array[j] = array[j] * 18;
        }
    }
}

void longlong_array()
{
    long long array[50];
    for(int i = 0; i < 50; i++)
    {
        array[i] = 1000UL / 7;
    }
    for(uint32_t i = 0; i < ITERATIONS; i++)
    {
        for(int j = 0; j < 50; j++)
        {
            array[j] = array[j] / 18;
            array[j] = array[j] * 18;
        }
    }
}

void float_array()
{
    float array[100];
    for(int i = 0; i < 50; i++)
    {
        array[i] = 1000UL / 7.0;
    }
    for(uint32_t i = 0; i < ITERATIONS; i++)
    {
        for(int j = 0; j < 50; j++)
        {
           array[j] = array[j] / 17.97;
           array[j] = array[j] * 17.97;
        }
    }
}

void double_array()
{
    double array[100];
    for(int i = 0; i < 50; i++)
    {
        array[i] = 1000UL / 7.0;
    }
    for(uint32_t i = 0; i < ITERATIONS; i++)
    {
        for(int j = 0; j < 50; j++)
        {
           array[j] = array[j] / 17.97;
           array[j] = array[j] * 17.97;
        }
    }
}

void longdouble_array()
{
    long double array[50];
    for(int i = 0; i < 50; i++)
    {
        array[i] = 1000UL / 7.0;
    }
    for(uint32_t i = 0; i < ITERATIONS; i++)
    {
        for(int j = 0; j < 50; j++)
        {
           array[j] = array[j] / 17.97;
           array[j] = array[j] * 17.97;
        }
    }
}*/

void flash_num(uint32_t input)
{
    led_on();
	delay(2000);
	led_off();
	for(int i = 0; i < 32; i++)
    {
     	if((input >> i) % 2 == 0)
     	{
       		led_on();
       		delay(250);
      		led_off();
      		delay(1000);
    	}
    	else if((input >> i) % 2 == 1)
    	{
      		led_on();
      		delay(750);
      		led_off();
      		delay(1000);
    	}
    }
}

ISR(TIMER0_COMPA_vect)
{
	ticks = ticks + 1;	
}

int main(void)
{
    cli();
	DDRB = DDRB | (1 << LED);
	TCCR0A = 0 | (1 << WGM01);
	TCCR0B = 0 | (1 << CS02);
	OCR0A = 255;
	TIMSK = 0 | (1 << OCIE0A);
	TCNT0 = 0;
	sei();
	led_off();


    /*short_array();
    sticks = (ticks / 122);*/

    int_array();
    iticks = (ticks / 122) - sticks;

    /* long_array();
    lticks = (ticks / 122) - iticks;

    longlong_array();
    llticks = (ticks / 122) - lticks;

    float_array();
    fticks = (ticks / 122) - llticks;

    double_array();
    dticks = (ticks / 122) - fticks;

    longdouble_array();
    ddticks = (ticks / 122) - dticks; */         
    
    cli();
    ticks = ticks / 122;

	while(1)
	{
		led_on();
		delay(10000);
		led_off();

        /*flash_num(sticks);*/
        flash_num(iticks);
       /* flash_num(lticks);
        flash_num(llticks);
        flash_num(fticks);
        flash_num(dticks); 
        flash_num(ddticks);*/
    }
}