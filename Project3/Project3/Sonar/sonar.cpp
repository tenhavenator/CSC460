/*
 * sonar.cpp
 *
 * Created: 2015-04-04 3:29:58 PM
 *  Author: Jeff
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include "../OS/os.h"

SERVICE * m_sonar_service;

void SonarInit() {
	
	m_sonar_service = Service_Init();	
}

void SonarFire(int timeout_ticks) {
	
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
		
		// ADD the real timeout here
		//##################################################################################
			// If this is interrupted, the range will be incorrect
		//##################################################################################
		
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
			int range = (width * 21 + 16) / ((F_CPU / 1000000L) * 29 * 2);
			
			// Do something with the service
		}
}