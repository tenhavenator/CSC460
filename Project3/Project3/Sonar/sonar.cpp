/*
 * sonar.cpp
 *
 * Created: 2015-04-04 3:29:58 PM
 *  Author: Jeff
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include "../OS/os.h"
#include <Arduino.h>

uint16_t SonarFire(int timeout_ticks, int pin) {
	
		// The PING))) is triggered by a HIGH pulse of 2 or more microseconds.
		// Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
		
		pinMode(pin, OUTPUT);
		digitalWrite(pin, LOW);
		_delay_us(2);
		digitalWrite(pin, HIGH);
		_delay_us(5);
		digitalWrite(pin, LOW);
				
		// The same pin is used to read the signal from the PING))): a HIGH
		// pulse whose duration is the time (in microseconds) from the sending
		// of the ping to the reception of its echo off of an object.
		
		pinMode(pin, INPUT);
		
		// Code taken from arduino		
		//uint8_t bit = _BV(6);
		//uint8_t bit = digitalPinToBitMask(pin);
		
		uint8_t bit = digitalPinToBitMask(pin);
		uint8_t port = digitalPinToPort(pin);
		uint8_t stateMask = bit;
		unsigned long width = 0;
		
		// ADD the real timeout here
		//##################################################################################
			// If this is interrupted, the range will be incorrect
		//##################################################################################
		
		// The 16 cycles is provbably lower without the extra instructions here
		unsigned long timeout = 200000;
		
		// the initial loop; it takes 16 clock cycles per iteration.
		unsigned long numloops = 0;
		// Convert this to real loop value
		unsigned long maxloops = microsecondsToClockCycles(timeout) / 16;
		
		// wait for any previous pulse to end
		while ((*portInputRegister(port) & bit) == stateMask)
			if (numloops++ == maxloops)
				break;
	
		// wait for the pulse to start
		while ((*portInputRegister(port) & bit) != stateMask)
			if (numloops++ == maxloops)
				break;
	
		// wait for the pulse to stop
		while ((*portInputRegister(port) & bit) == stateMask) {
			if (numloops++ == maxloops)
				break;
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
			int range = (width * 34 + 16) / ((F_CPU / 1000000L) * 29 * 2);
			//int range = clockCyclesToMicroseconds(width * 21 + 16) / (29 * 2); 
			return range;
		}
		
		
		return 0;
}