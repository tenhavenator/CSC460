/*
 * Project3.cpp
 *
 * Roomba control
 */ 

#include "OS/os.h"
#include "roomba/roomba.h"
#include "roomba/roomba_sci.h"

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define BUFFER_SIZE 10

volatile uint8_t m_id_buffer[] = {1,0,1,0,0,0,0,0,1};
volatile int m_bits_to_send = 0;

uint64_t m_last_range;

void sonar_fire(void) {
	
	// If this is interrupted, the range will be incorrect
	
	for(;;) {
		
		// The PING))) is triggered by a HIGH pulse of 2 or more microseconds.
		// Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
	
		DDRB = DDRB | _BV(6);
		PORTB = PORTB & ~_BV(6);
		_delay_us(2);
		PORTB = PORTB |_BV(6);
		_delay_us(5);
		PORTB = PORTB & ~_BV(6);

		// The same pin is used to read the signal from the PING))): a HIGH
		// pulse whose duration is the time (in microseconds) from the sending
		// of the ping to the reception of its echo off of an object.
	
		//pinMode(pingPin, INPUT);
		DDRB = DDRB & ~_BV(6);
		//duration = pulseIn(pingPin, HIGH);
	
		// Code taken from arduino
		uint8_t bit = _BV(6);
		unsigned long width = 0; 
		
		// Figure out what an actual good value would be
		// The 16 cycles is provbably lower without the extra instructions here
		unsigned long timeout = 100000;
	
		// the initial loop; it takes 16 clock cycles per iteration.
		unsigned long numloops = 0;
		// Convert this to real loop value
		unsigned long maxloops = timeout;
	
		// wait for any previous pulse to end
		while ((PINB & bit) == bit)
		{
			if (numloops++ >= maxloops)
			{
				break;
			}
		}
	
		// wait for the pulse to start
		while ((PINB & bit) != bit)
		{
			if (numloops++ >= maxloops)
			{
				break;
			}
		}
	
		// wait for the pulse to stop
		while (((PINB & bit)  & bit) == bit) {
			if (numloops++ >= maxloops)
			{
				break;;
			}
			width++;
		}
		
		// convert the reading to microseconds. The loop has been determined
		// to be 20 clock cycles long and have about 16 clocks between the edge
		// and the start of the loop. There will be some error introduced by
		// the interrupt handlers.
		
		// The speed of sound is 340 m/s or 29 microseconds per centimeter.
		// The ping travels out and back, so to find the distance of the
		// object we take half of the distance travelled.
		
		// This equaltion seems to produce almost perfect results. 
		
		if(numloops < maxloops)
		{
			m_last_range = (width * 21 + 16) / ((F_CPU / 1000000L) * 29 * 2);
				
			// We might want to consider putting this in a system task so that firing is never pre-empted.
			DDRB = DDRB | _BV(7);
			if(m_last_range <=100)
			{
				// Target detected within 100cm. Fire IR
				if(m_bits_to_send == 0)
				{
					m_bits_to_send = BUFFER_SIZE;
				}
					
				PORTB = PORTB | _BV(7);
			}
			else
			{
				PORTB = PORTB & ~_BV(7);
			}
		}
		
		Task_Next();	
	}
}


// See p. 101 for interrupts. Triggered when 500 us timer goes off
ISR(TIMER3_COMPA_vect)
{
	if(m_bits_to_send > 0)
	{
		OCR5C = m_id_buffer[BUFFER_SIZE - m_bits_to_send] == 1 ? 105 : 0; // 421 * 25%
		m_bits_to_send--;
	}
	else
	{
		OCR5C = 0;
	}
}

int r_main(void)
{
	// Some rr task can read all the sensor data and give out drive commands
	
	// ======== Set up timer 3 to be the 500 us counter
	// Clear timer config.
	TCCR3A = 0;
	TCCR3B = 0;
	
	//Set to CTC (mode 4)
	TCCR3B |= (1<<WGM32);
	
	// No Prescalar
	TCCR3B |= (1<<CS30);
	
	// Set TOP value (0.0005 seconds)
	OCR3A = 8000;
	
	// Enable interupt for when OCR3A matches TCNT3 (Timer register)
	TIMSK3 |= (1<<OCIE3A);
	
	//Set timer 3 to 0 (optional here).
	TCNT3 = 0;
	
	// ======== Set up timer 5 to be the 38 KHz timer =======================
	
	// Clear timer config
	TCCR5A = 0;
	TCCR5B = 0;
	
	// Set to Fast PWM (mode 15)
	TCCR5A |= (1<<WGM50) | (1<<WGM51);
	TCCR5B |= (1<<WGM52) | (1<<WGM53);
	
	DDRL |= _BV(5);
	
	// Generate wave form in OC5C (digital 44, see pin mapping) using OCR5C as compare register. Set at bottom, clear at match.
	TCCR5A |= (1<<COM5C1);
	
	// No prescaler
	TCCR5B |= (1<<CS50);
	
	OCR5A = 421;  // 38 KHz (Top value)
	OCR5C = 0;  // Target
	
	
	Task_Create_Periodic(sonar_fire, 0, 100, 100, 400);
	Task_Terminate();
	return 0;
}




