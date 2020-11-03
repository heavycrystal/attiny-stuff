#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <math.h>
#include <stdbool.h>

#define LED PB1
#define INT PB2
#define PCINT PB0
#define PWM PB4
#define SIZE 100
#define ITERATIONS 1000000

volatile uint32_t ticks = 0;
volatile uint32_t debounce = 0;
volatile bool flag = false;
volatile double array[SIZE];

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
	if(flag == false)
	{
		PORTB = PORTB | (1 << PWM);
		flag = true;
	}	
	else if(flag == true)
	{
		PORTB = PORTB & ~(1 << PWM);
		flag = false;
	}
}

ISR(TIMER1_COMPA_vect)
{
	ticks = ticks + 1;
	if(debounce > 0)
	{
		debounce = debounce - 1;
	}
	if((ticks % 122) == 0)
	{
		led_on();
		delay(10);
		led_off();
	}
}

ISR(INT0_vect)
{
	if(((debounce > 0) || ((PINB >> INT) % 2 == 0)))
	{
		return;
	}

	led_off();

	array[7] = array[7] + 21.73456;
	debounce = 30;
}

ISR(PCINT0_vect)
{
	if(((debounce > 0) || ((PINB >> PCINT) % 2 == 0)))
	{
		return;
	}

	led_on();

	array[17] = array[17] + 10091.89;
	debounce = 30;
}

int main(void)
{
    cli();
	GTCCR = GTCCR | (1 << TSM) | (1 << PSR0) | (1 << PSR1);
	MCUCR = MCUCR | (1 << ISC01) | (1 << ISC00);
	GIMSK = GIMSK | (1 << INT0) | (1 << PCIE);
	PCMSK = PCMSK | (1 << PCINT3);
	DDRB = DDRB | (1 << LED) | (1 << PWM) | (0 << INT) | (0 << PCINT);
	PORTB = PORTB | (1 << INT) | (1 << PCINT);
	TCCR0A = 0 | (1 << WGM01);
	TCCR0B = 0 | (1 << CS02);
	OCR0A = 63;
	TIMSK = 0 | (1 << OCIE0A) | (1 << OCIE1A);
	TCCR1 = 0 | (1 << CTC1) | (1 << CS13) | (1 << CS10);
	OCR1C = 255;
	OCR1A = 255;
	TCNT0 = 0;
	GTCCR = GTCCR & ~(1 << TSM);
	sei();
	led_off();

   	for(int i = 0; i < SIZE; i++)
    {
        array[i] = 1000000UL / 7.0;
    }
    for(uint32_t i = 0; i < ITERATIONS; i++)
    {
        for(int j = 0; j < SIZE; j++)
        {
            array[j] = array[j] + 23.002;
            array[j] = array[j] - 23.002;

            array[j] = array[j] / 17.97;
            array[j] = array[j] * 17.97;

            array[j] = pow(3.14159, array[j]);
            array[j] = log(array[j]) / log(3.14159);

            array[j] = sin(array[j]);
            array[j] = asin(array[j]);

            array[j] = cos(array[j]);
            array[j] = acos(array[j]);

            array[j] = tan(array[j]);
            array[j] = atan(array[j]);
        }
    }
     
    cli();
    ticks = ticks / 122;

	while(1)
	{
        flash_num(ticks);
    }
}
