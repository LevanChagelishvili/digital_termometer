#define F_CPU 800000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdlib.h>
#include <avr/wdt.h>

#define SEVEN_SEGMENT_CONTROL	PORTD
#define TRANSISTOR_CONTROL		PORTB



uint8_t seven_segment_buffer[10] = {0x3F, 0x06, 0x5B, 0x57, 0x66, 0x75, 0x7D, 0x07, 0x7F, 0x77};// common anode
uint16_t temp1 = 0, temp2 = 0, temp3 = 0, temp4 = 0;
uint8_t transistor_out = 0;
uint8_t timer1_counter;
uint16_t currentTemp;


// Timer0 initialization function
void timer0_init(void) 
{
	// Timer/Counter Control Register B, 
	TCCR0B &= ~(1 << CS02); 
	TCCR0B &= ~(1 << CS00); 
	// Clock Select CS02 = 0, CS01 = 1 CS00 = 0 - 1/8 prescaler
	TCCR0B |= (1 << CS01);
	// Timer/Counter Interrupt Mask Register, TOIE0: Timer/Counter0 Overflow Interrupt Enable
	TIMSK0 |= (1 << TOIE0);
	// Timer/Counter Register 
	TCNT0 = 0x00;
	// Global interrupt enable
	sei ();
}


void divide_number(uint16_t number)
{
	temp1 = number/1000;
	temp2 = number%1000/100;
	temp3 = number%100/10;
	temp4 = number%10;
}

// Interrupt function for timer0
ISR (TIMER0_OVF_vect)
{
	if(transistor_out == 1){TRANSISTOR_CONTROL = 0x10; SEVEN_SEGMENT_CONTROL = seven_segment_buffer[temp1];}
	if(transistor_out == 2){TRANSISTOR_CONTROL = 0x08; SEVEN_SEGMENT_CONTROL = seven_segment_buffer[temp2];}
	if(transistor_out == 3){TRANSISTOR_CONTROL = 0x04; SEVEN_SEGMENT_CONTROL = seven_segment_buffer[temp3];}
	if(transistor_out == 4){TRANSISTOR_CONTROL = 0x02; SEVEN_SEGMENT_CONTROL = seven_segment_buffer[temp4];}
	transistor_out++;
	if(transistor_out > 4) transistor_out = 1;
}


ISR(ADC_vect)
{
	// Read ADC value
	uint32_t adcValue = ADC;
	
	// Scale ADC value to desired range (0 - 255)
	currentTemp = adcValue * 500 / 1023;
	
	// Start the next ADC conversion
	ADCSRA |= (1 << ADSC);
}

int main(void)
{
	DDRB = 0x1F;
	DDRD = 0xFF;
	TRANSISTOR_CONTROL |= (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4);
	DDRC = 0x00;
	
	timer0_init();
	
	// Enable ADC and set ADC prescaler to 128
	ADCSRA |= (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
	
	// Set ADC reference voltage to AVCC
	ADMUX |= (1 << REFS0);
	
	// Enable ADC interrupt
	ADCSRA |= (1 << ADIE);
	
	// Start the first ADC conversion
	ADCSRA |= (1 << ADSC);
	
	sei ();
	
	

	while (1)
	{
		divide_number(currentTemp);
		_delay_ms(500);
	}
}

