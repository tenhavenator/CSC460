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
#include "Sonar/sonar.h"
#include "Sensors/sensor.h"

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

uint64_t m_last_range;

irobot m_robot(&Serial1, 5);

#define TURN_90_DEGREES_MS 1770

void sonar_ping(void) {
		
	for(;;) {
		
		SonarPing();
		Task_Next();	
	}
}

void ir_scan(void) {
	for(;;)
	{		
		m_robot.send(irobot::op_sensor, irobot::sense_infrared_omni);
				
		uint8_t c = 0;
		
		m_robot.receive(&c, 1);
		
	    // Check for the ID and send a radio packet
		
	}
}

void drive(int16_t velocity)
{
	uint8_t send_bytes[4];
	send_bytes[0] = (velocity >> 8) & 0xFF;
	send_bytes[1] = velocity & 0xFF;
	send_bytes[2] = 0x80;
	send_bytes[3] = 0x00;
	m_robot.send(irobot::op_drive, send_bytes, 4);	
}

void spin(int16_t angle, bool cw)
{
	uint8_t send_bytes[4];
	int16_t velocity = -100;
	if (cw) {
		velocity = 100;
	}
	
	send_bytes[0] = (velocity >> 8) & 0xFF;
	send_bytes[1] = velocity & 0xFF;
	send_bytes[2] = 0;
	send_bytes[3] = 1;
	m_robot.send(irobot::op_drive, send_bytes, 4);
}

void spin_90_CW()
{
	uint8_t send_bytes[4];
	int16_t velocity = -100;
	if (cw) {
		velocity = 100;
	}
	
	send_bytes[0] = (velocity >> 8) & 0xFF;
	send_bytes[1] = velocity & 0xFF;
	send_bytes[2] = 0;
	send_bytes[3] = 1;
	m_robot.send(irobot::op_drive, send_bytes, 4);
}

void spin_90_CCW()
{
	uint8_t send_bytes[4];
	int16_t velocity = -100;
	if (cw) {
		velocity = 100;
	}
	
	send_bytes[0] = (velocity >> 8) & 0xFF;
	send_bytes[1] = velocity & 0xFF;
	send_bytes[2] = 0;
	send_bytes[3] = 1;
	m_robot.send(irobot::op_drive, send_bytes, 4);
}

uint16_t convert(uint8_t * light) {
	uint16_t high = light[0];
	uint16_t low = light[1];
	
	return (high << 8) | low;
}

void bump_scan() {
	for(;;) {
			
		SensorData data = SensorPoll();
		
		/*Serial.write("DATA: ");
		Serial.print(data.bumper_right, DEC);
		Serial.print(" ");
		Serial.print(data.bumper_left, DEC);
		Serial.print(" ");
		
		Serial.print(data.light_left, DEC);
		Serial.print(" ");
		/*Serial.print(data.light_front_left[0], DEC);
		Serial.print(data.light_front_left[1], DEC);
		Serial.print(" ");*/
		/*Serial.print(data.light_center_left[0], DEC);
		Serial.print(data.light_center_left[1], DEC);
		Serial.print(" ");
		
		Serial.print(data.light_center_right[0], DEC);
		Serial.print(data.light_center_right[1], DEC);
		Serial.print(" ");
		
		/*Serial.print(data.light_front_right[0], DEC);
		Serial.print(data.light_front_right[1], DEC);
		Serial.print(" ");
		
		Serial.print(data.light_right[0], DEC);
		Serial.print(data.light_right[1], DEC);
		Serial.print(" ");
		
		
		Serial.println("");
		//Serial.write((char)data.bumper_left, BIN);*/
		
		if(data.bumper_right == 1)
		{
			drive(0);
		}
		else if(data.bumper_left == 1)
		{
			drive(0);
		}
		else if(convert(data.light_center_left) > 100) {
			drive(0);
		}
		else if(convert(data.light_center_right) > 100) {
			drive(0);
		}
		else {
			drive(200);
		}
		Task_Next();
	}	
}

void ir_fire() {
	
	for(;;) {
		IRFire();
		Task_Next();
	}
}

void check_spin() {
	int rotated = 0;
	for(;;) {
		int16_t angle = GetAngle();
		rotated += angle;
		if (rotated > 360) {
			drive(0);
		}
		
		
		Task_Next();
	}
}

int r_main(void)
{			
	//IRInit();
	m_robot.begin();
	
	Serial.begin(9600);
	
	m_robot.send(irobot::op_full_mode);
	
	//Task_Create_Periodic(sonar_fire, 0, 100, 100, 400);
	//Task_Create_Periodic(ir_fire, 0, 200, 20, 400);
	//Task_Create_RR(ir_scan, 0);
	SensorInit(&m_robot);
	
	//Task_Create_Periodic(bump_scan, 0,20,15, 400);
	
	drive(0);
	
	//Task_Create_Periodic(check_spin, 0, 100, 45, 400);
	
	//for (;;) {
		spin(2, true);
		_delay_ms(64000);
		drive(0);
		//_delay_ms(500);
		
	
	
	
	/*for(;;) {
		drive_straight(300);
		_delay_ms(1000);
		drive_straight(0);
		_delay_ms(1000);
		
	}*/
	
	
	Task_Terminate();
	return 0;
}




