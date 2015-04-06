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
#include "Arduino.h"
#include "Radio/radio.h"
#include "Radio/Packets/radio_packets.hpp"

#include "IR/IRTransmit.h"
#include "Sonar/sonar.h"
#include "Sensors/sensor.h"
#include "Movement/drive.h"
#include "pins.h"

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define TEAM1_ID 100
#define TEAM3_ID 65
#define TEAM4_ID 66

#define IR_RESPAWN 5000

static uint16_t last_shot_by_team1;
static uint16_t last_shot_by_team3;
static uint16_t last_shot_by_team4;

uint64_t m_last_range;

irobot m_robot(&Serial1, ROBOT_DD_PIN);
SERVICE * drive_state_service;
volatile int drive_state = DRIVE_FORWARDS;
uint8_t sonar_sensor_values[3] = {0, 0, 0};

static uint8_t recv_addr[5] = {0x62, 0x6F, 0x74, 0x5F, 0x63};
static uint8_t send_addr[5] = {0x62, 0x6F, 0x74, 0x5F, 0x63};

void kill_robot() {
	m_robot.send(irobot::op_safe_mode);
	OS_Abort();
}

void disable_drive_interrupt() {
	// Disable the interrupt
	TIMSK4 &= ~(1<<OCIE4A);
}

void enable_drive_interrupt(int time_ms) {
	
	// Set length of time to perform command
	//float time_float = float(time_ms);
	//Serial.print(time_float, DEC);
	
	long int time_ticks = (long int)(time_ms * 15.625);
	
	// Set up timer to interrupt when command is done
	TCNT4 = 0;
	OCR4A = time_ticks;
	TIMSK4 |= (1<<OCIE4A);
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
			
		// Check if we can be shot by team one
		uint16_t now = Now();
		
		if(now - last_shot_by_team1 > IR_RESPAWN) {
			if(data.infrared_omni == TEAM1_ID) {
				// send radio packet
				last_shot_by_team1 = now;
			}
			else {
				 last_shot_by_team1 = now - IR_RESPAWN;
			}
		}
		
		// Check if we can be shot by team three
		if(now - last_shot_by_team3 > IR_RESPAWN) {
			if(data.infrared_omni == TEAM3_ID) {
				// send radio packet
				last_shot_by_team3 = now;
			}
			else {
				last_shot_by_team3 = now - IR_RESPAWN;
			}
		}
		
		
		// Check if we can be shot by team four
		if(now - last_shot_by_team4 > IR_RESPAWN) {
			if(data.infrared_omni == TEAM4_ID) {
				// send radio packet
				last_shot_by_team4 = now;
			}
		
			else {
				last_shot_by_team4 = now - IR_RESPAWN;
			}
		}
	
		// Turn the light on if we shoot ourselves
		pinMode(3, OUTPUT);
		if(data.infrared_omni == 67) {
				
			digitalWrite(3, HIGH);
		}
		else
		{
			digitalWrite(3, LOW);
		}
		
		if ((data.wheel_drop_left == 1)||(data.wheel_drop_right == 1)) {
			kill_robot();
		}
		
		if ((data.bumper_right == 1)||(data.bumper_left == 1)||(convert(data.light_center_left) > 100)||(convert(data.light_center_right) > 100)) {
			IRFire();
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
		
//		Serial.print(new_state, DEC);
//		Serial.println();

		// Wait for service publish
		Service_Subscribe(drive_state_service, &new_state);
		disable_drive_interrupt();
	}
}

void check_sonar() {
	
	for (;;) {
		
		uint16_t range = SonarFire(100, SONAR_SIGNAL_PIN);
			
		pinMode(2, OUTPUT);
		
		// check if going forwards, if so, make it go forwards forever (extend timer)		
		if ((range > 10)&&(range < 100)) {
			digitalWrite(2, HIGH);
			IRFire();
			if(drive_state == DRIVE_FORWARDS) {
				TCNT4 = 0;
			}
		} else {
			digitalWrite(2, LOW);
		}
		
		
		Task_Next();
	}
}

extern "C" {
	void radio_rxhandler(uint8_t pipenumber)
	{
	}
}

void radio_init() {
	
	pinMode(RADIO_POWER_PIN, OUTPUT);
	
	digitalWrite(RADIO_POWER_PIN, LOW);
	delay(100);
	digitalWrite(RADIO_POWER_PIN, HIGH);
	delay(100);
	
	// Configure Radio
	Radio_Init(104);
	Radio_Configure_Rx(RADIO_PIPE_0, recv_addr , ENABLE);
	Radio_Configure(RADIO_2MBPS, RADIO_HIGHEST_POWER);
	Radio_Set_Tx_Addr(send_addr);
}

void radio_stuff() {
	for(;;) {

		radio_packet_t packet;
		
		packet.game_status.it_id = 0x45;
		packet.shot.shooter_id = 0x44;
		packet.shot.target_id = 0x33;
		packet.sonar_trigger.id = 0x12;
		
		
		Radio_Transmit(&packet, RADIO_RETURN_ON_TX);
		
		radio_packet_t received;
		RADIO_RX_STATUS status = Radio_Receive(&received);
		
		Serial.print(status,DEC);
		
		Task_Next();
	}
}

int r_main(void)
{			
	//IRInit();
	//Task_Create_Periodic(sonar_fire, 0, 100, 100, 400);
	//Task_Create_Periodic(ir_fire, 0, 200, 20, 400);
	//Task_Create_RR(ir_scan, 0);
	
	m_robot.begin();
	
	//Serial.begin(9600);
	
	//radio_init();
	//m_robot.send(irobot::op_safe_mode);
	m_robot.send(irobot::op_full_mode);
	
	SensorInit(&m_robot);
	
	drive_state_service = Service_Init();
	
	drive_init(&m_robot);
	
	Task_Create_Periodic(bump_scan, 0, 100, 40, 550);
	
	Task_Create_System(drive_robot, 0);
	
	Task_Create_Periodic(check_sonar, 0, 100, 40, 500);
	//Task_Create_Periodic(normalize, 0, 100, 40, 550);
	//Task_Create_Periodic(radio_stuff, 0, 200, 199, 150);
	
	
	
	Task_Terminate();
	return 0;
}




