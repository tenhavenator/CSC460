/*
 * drive.cpp
 *
 * Created: 2015-04-05 10:39:59 AM
 *  Author: Colin
 */ 

#include "../irobot/irobot.hpp"
#include <Arduino.h>
#include "drive.h"

static irobot * m_robot = NULL;

void drive_init(irobot * robot)
{
	m_robot = robot;
	
	// ======== Set up timer 4 to be the 1 ms counter
	// Clear timer config.
	TCCR4A = 0;
	TCCR4B = 0;
	
	//Set to CTC (mode 4)
	TCCR4B |= (1<<WGM42);
	
	// 1024 prescaler
	TCCR4B |= (1<<CS40) | (1<<CS42);
	
	// Set TOP value 
	OCR4A = 0;
	
	//Set timer 3 to 0 (optional here).
	TCNT4 = 0;
	
	// Clear interrupt flag
	TIFR4 = _BV(OCF4A);
	

}


void drive(int16_t velocity)
{
	uint8_t send_bytes[4];
	send_bytes[0] = (velocity >> 8) & 0xFF;
	send_bytes[1] = velocity & 0xFF;
	send_bytes[2] = 0x80;
	send_bytes[3] = 0x00;
	m_robot->send(irobot::op_drive, send_bytes, 4);	
}

void spin(bool cw)
{
	uint8_t send_bytes[4];
	int16_t velocity = -200;
	if (cw) {
		velocity = 200;
	}
	
	send_bytes[0] = (velocity >> 8) & 0xFF;
	send_bytes[1] = velocity & 0xFF;
	send_bytes[2] = 0;
	send_bytes[3] = 1;
	m_robot->send(irobot::op_drive, send_bytes, 4);
}