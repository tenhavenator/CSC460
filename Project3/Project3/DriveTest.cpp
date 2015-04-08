/*
 * DriveTest.cpp
 *
 * Checks the driving functionality (with all sensors but the IR)
 */ 
#if 0

#include "OS/os.h"
#include "roomba/roomba.h"
#include "roomba/roomba_sci.h"
#include "roomba/sensor_struct.h"
#include "irobot/irobot.hpp"
#include "arduino_config.h"
#include "Arduino.h"

#include "IR/IRTransmit.h"
#include "Sonar/sonar.h"
#include "Sensors/sensor.h"
#include "Movement/drive.h"
#include "pins.h"

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

irobot m_robot(&Serial1, ROBOT_DD_PIN);
SERVICE * drive_state_service;
volatile int drive_state = DRIVE_FORWARDS;

void kill_robot() {
	m_robot.send(irobot::op_safe_mode);
	OS_Abort();
}

//  Needed for the program to compile
extern "C" {
	void radio_rxhandler(uint8_t pipenumber) {}
};
	
void disable_drive_interrupt() {
	// Disable the interrupt
	TIMSK4 &= ~(1<<OCIE4A);
}

void enable_drive_interrupt(int time_ms) {
	
	// Set length of time to perform command
	long int time_ticks = (long int)(time_ms * 15.625);
	
	// Set up timer to interrupt when command is done
	TCNT4 = 0;
	OCR4A = time_ticks;
	TIMSK4 |= (1<<OCIE4A);
}

void sensor_scan() {
	for(;;) {
			
		SensorData data = SensorPoll();
		
		if ((data.wheel_drop_left == 1)||(data.wheel_drop_right == 1)) {
			kill_robot();
		}
		
		if ((data.bumper_right == 1)||(data.bumper_left == 1)||(convert(data.light_center_left) > 100)||(convert(data.light_center_right) > 100)) {
			Service_Publish(drive_state_service, DRIVE_BACKWARDS);
		}
			 
		Task_Next();
	}	
}

ISR(TIMER4_COMPA_vect)
{
	disable_drive_interrupt();
	
	int new_state = drive_state;
	
	// Choose new drive state based on current state
	switch(drive_state) {
		case DRIVE_STOP:
			break;
		
		case DRIVE_FORWARDS:
			{
				// pick direction, CW or CCW
				int cw = rand() % 3;	
				if (cw == 1) {
					new_state = DRIVE_TURN_CW_90;
				} else if (cw == 2) {
					new_state = DRIVE_TURN_CCW_90;
				} 
				// Else continue forwards
			}
			break;
		
		case DRIVE_BACKWARDS:
			new_state = DRIVE_TURN_AROUND;
			break;
		
		case DRIVE_TURN_CW_90:
			new_state = DRIVE_FORWARDS;
			break;
		
		case DRIVE_TURN_CCW_90:
			new_state = DRIVE_FORWARDS;
			break;
		
		case DRIVE_TURN_AROUND:
			new_state = DRIVE_FORWARDS;
			break;
			
		default:
			break;
	}
	
	Service_Publish(drive_state_service, new_state);
}

void drive_robot() {
	
	int new_state = drive_state;
	
	for (;;) {
		
		// Choose drive command based on new state	
		switch(new_state) {
			case DRIVE_STOP:
				drive(0);
				// Put in safe mode and terminate maybe
				break;
				
			case DRIVE_FORWARDS:
				drive(200);
				enable_drive_interrupt(4000);
				break;
				
			case DRIVE_BACKWARDS:
				drive(-100);
				enable_drive_interrupt(1000);
				break;
				
			case DRIVE_TURN_CW_90:
				spin(true);
				enable_drive_interrupt(TURN_90_DEGREES_MS);
				break;
				
			case DRIVE_TURN_CCW_90:
				spin(false);
				enable_drive_interrupt(TURN_90_DEGREES_MS);
				break;
				
			case DRIVE_TURN_AROUND:
				
				{
					// Calculate a random number of ms between 0 and time it takes to go 90 degrees
					// Then when turning around we go between 90 and 180 degrees total
					int random_angle = rand() % 1770;
				
					spin(true);
					enable_drive_interrupt(TURN_90_DEGREES_MS + random_angle);
				}
				break;
				
			default:
				break;
		}

		drive_state = new_state;

		// Wait for service publish
		Service_Subscribe(drive_state_service, &new_state);
		disable_drive_interrupt();
	}
}

void check_sonar() {
	
	for (;;) {
		
		int16_t data;
		pinMode(11, OUTPUT);
		digitalWrite(11, LOW);
		
		// For debugging purposes
		
		digitalWrite(11, HIGH);
		
		digitalWrite(4, HIGH);
		
		uint16_t range = SonarFire(100, SONAR_SIGNAL_PIN);
		
		
		// check if going forwards and detecting something, if so, make it go forwards forever (extend timer)
		if ((range > 10)&&(range < 100)) {
			IRFire();
			if(drive_state == DRIVE_FORWARDS) {
				TCNT4 = 0;
			}
		}
		
		// check if something very close, if so, back up and turn around
		// This prevents us from running into higher objects that the bumper can't detect
		else if ((range > 0)&&(range < 10)) {
			disable_drive_interrupt();
			Service_Publish(drive_state_service, DRIVE_BACKWARDS);
		}
		
		digitalWrite(4, LOW);
	}
}

int r_main(void)
{				
	m_robot.begin();
	m_robot.send(irobot::op_full_mode);
	
	SensorInit(&m_robot);
	
	drive_state_service = Service_Init();
	drive_init(&m_robot);
	
	// So that the first drive command doesn't come in too fast
	_delay_ms(25);
	
	Task_Create_System(drive_robot, 0);
	Task_Create_Periodic(sensor_scan, 0, 100, 40, 550);
	Task_Create_Periodic(check_sonar, 0, 100, 40, 500);

	Task_Terminate();
	return 0;
}

#endif

