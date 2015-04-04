/*
 * Project3.cpp
 *
 * Roomba control
 */ 

#include "OS/os.h"
#include "roomba/roomba.h"
#include "roomba/roomba_sci.h"
#include "roomba/sensor_struct.h"
#include "irobot/irobot.hpp"
#include "arduino_config.h"
#include "IR/IRTransmit.h"

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

uint64_t m_last_range;

irobot m_robot(&Serial1, 5);


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

void ir_scan(void) {
	for(;;)
	{
				
		//DDRB |= _BV(6);
			
		/*IRFire();
		
		m_robot.send(irobot::op_sensor, irobot::sense_infrared_omni);
				
		uint8_t c = 0;*/
		
		//PORTB |= _BV(6);
		//PORTB &= ~_BV(6);
		/*m_robot.receive(&c, 1);
		/
		PORTB |= _BV(6);
		PORTB &= ~_BV(6);
		
	
		if(c == 255) {
			Serial.write("Got it");
		}*/
		
		Task_Next();
	}
}




int r_main(void)
{	
	
	//IRInit();
	//m_robot.begin();
	
	//Serial.begin(9600);
	
	//Task_Create_Periodic(sonar_fire, 0, 100, 100, 400);
	//Task_Create_Periodic(ir_scan, 0, 20, 20, 400);
	
	Task_Terminate();
	return 0;
}




