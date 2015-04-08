/*
 * IRTransmit.cpp
 *
 * Created: 2015-04-04 11:42:20 AM
 *  Author: Jeff
 */ 


#define BUFFER_SIZE 37

#include <avr/interrupt.h>
#include "../irobot/irobot.hpp"

volatile uint8_t m_id_buffer[] = {0, 1,0,0,0, 1,1,1,0, 1,0,0,0, 1,0,0,0, 1,0,0,0, 1,0,0,0, 1,0,0,0, 1,1,1,0, 0,0,0,0};
	
volatile int m_bits_to_send = 0;
volatile int m_burst = 0;


void IRInit() {
	
	// ======== Set up timer 3 to be the 1 ms counter
	// Clear timer config.
	TCCR3A = 0;
	TCCR3B = 0;
	
	//Set to CTC (mode 4)
	TCCR3B |= (1<<WGM32);
	
	// No Prescalar
	TCCR3B |= (1<<CS30);
	
	// Set TOP value (0.001 seconds)
	OCR3A = 16000;
	
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
	
	m_bits_to_send = 0;
	m_burst = 0;
}

void IRFire() {
	
	if(m_bits_to_send == 0 && m_burst == 0)
	{
		m_burst = 1;
		m_bits_to_send = BUFFER_SIZE;
		TIMSK3 |= (1<<OCIE3A);
	}
}

void ir_scan(irobot * m_robot) {
	for(;;)
	{		
		m_robot->send(irobot::op_sensor, irobot::sense_infrared_omni);
				
		uint8_t c = 0;
		
		m_robot->receive(&c, 1);
		
	    // Check for the ID and send a radio packet
		
	}
}

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
		if(m_burst > 0) {
			m_bits_to_send = BUFFER_SIZE;
			m_burst--;
		}
		else
		{
			TIMSK3 &= ~(1<<OCIE3A);	
		}
		
			
	}
}



