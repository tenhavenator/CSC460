/*
 * Project3.cpp
 *
 * Controls the robot using our project 2 OS,
 * drives it randomly around,
 * makes more advanced driving decisions based on sensor input, 
 * and fires an IR "gun" at other robots. 
 */ 

#include "OS/os.h"

#include "roomba/roomba.h"
#include "roomba/roomba_sci.h"
#include "roomba/sensor_struct.h"

#include "irobot/irobot.hpp"

#include "Radio/radio.h"
#include "Radio/Packets/radio_packets.hpp"

#include "IR/IRTransmit.h"
#include "Sonar/sonar.h"
#include "Sensors/sensor.h"
#include "Movement/drive.h"

#include "pins.h"

#include "arduino_config.h"
#include "Arduino.h"

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define TEAM1_ID 65
#define TEAM2_ID 66 
#define TEAM3_ID 67
#define TEAM4_ID 68

#define IR_RESPAWN 5000

static uint16_t last_shot_by_team1;
static uint16_t last_shot_by_team2;
static uint16_t last_shot_by_team4;

irobot m_robot(&Serial1, ROBOT_DD_PIN);

SERVICE * drive_state_service;
SERVICE * sonar_time_slot;

volatile int drive_state = DRIVE_FORWARDS;

static uint8_t recv_addr[5] = {0x62, 0x6F, 0x74, 0x5F, 0x46};
static uint8_t send_addr[5] = {0x62, 0x6F, 0x74, 0x5F, 0x64};
	
static uint8_t hit_flags[3] = {0,0,0};	

// Handler for when the wireless radio receives a packet
extern "C" {	
void radio_rxhandler(uint8_t pipenumber)
{
	 radio_packet_t rx_packet;
	 RADIO_RX_STATUS rx_status = RADIO_RX_MORE_PACKETS;
	 while(rx_status == RADIO_RX_MORE_PACKETS)
	 {
		 rx_status = Radio_Receive(&rx_packet);
		 if ( rx_status == RADIO_RX_MORE_PACKETS ||
		 rx_status == RADIO_RX_SUCCESS )
		 {
			 // Packet received
			 switch(rx_packet.type)
			 {
				 case sonar_trigger_packet_type:
				 {
					 Service_Publish(sonar_time_slot, 0);
					 
					 digitalWrite(3, LOW);
					 int i;
					 for(i = 0; i < 3; i++) {
						 
						if(hit_flags[i] != 0) {
							digitalWrite(3, HIGH);
							radio_packet_t packet;
							packet.type = shot_packet_type;
							packet.shot.shooter_id = hit_flags[i];
							packet.shot.target_id = TEAM2_ID;
							Radio_Transmit(&packet, RADIO_RETURN_ON_TX);
							hit_flags[i] = 0;
						 }
					 }
					 
					// Serial.print("Sonar Packet");
					 break;
				 }
				 
				 case game_status_packet_type:
				 {
					 // Do something probably
					 break;
				 }
				 
				 default: 
					 break;
			}
		}
	}
}
}

// Puts the Roomba back in safe mode and aborts the OS
void kill_robot() {
	m_robot.send(irobot::op_safe_mode);
	OS_Abort();
}

// Disables the interrupt to prepare to send a new drive command
void disable_drive_interrupt() {
	TIMSK4 &= ~(1<<OCIE4A);
}

// Enables the drive interrupt so the current drive command will end after time_ms milliseconds
void enable_drive_interrupt(int time_ms) {
	
	// Set length of time to perform command	
	long int time_ticks = (long int)(time_ms * 15.625);
	
	// Set up timer to interrupt when command is done
	TCNT4 = 0;
	OCR4A = time_ticks;
	TIMSK4 |= (1<<OCIE4A);
	
	Serial.println();
}

// Interrupt to tell the current drive command to end
ISR(TIMER4_COMPA_vect)
{
	disable_drive_interrupt();
	
	digitalWrite(13, LOW);
	
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

// Sends a drive command to the Roomba based on a state (current state stored in global variable)
// Uses hardware timer 4 to time each drive command and start a new one
void drive_robot() {
	
	int new_state = drive_state;
	
	for (;;) {
		pinMode(28, OUTPUT);
		
		// Choose drive command based on new state
		switch(new_state) {
			case DRIVE_STOP:
			drive(0);
			// Put in safe mode and terminate maybe
			break;
			
			case DRIVE_FORWARDS:
			enable_drive_interrupt(4000);
			drive(200);
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
		
		// For debugging purposes
		digitalWrite(28, LOW);
		
		// Wait for service publish
		Service_Subscribe(drive_state_service, &new_state);
		
		// For debugging purposes
		digitalWrite(28, HIGH);
		
		disable_drive_interrupt();
	}
}

// Polls for sensor data from the light sensors, front bumper, IR receiver
// Shoots the IR gun if anything is detected in front of the robot
// Sends a shot packet to the base station if this robot is shot by another
// (unless already shot by that robot in the last 5 seconds)
void bump_scan() {
	for(;;) {
			
		// For logic analyzer debugging purposes
		pinMode(12, OUTPUT);
		digitalWrite(12, HIGH);
		
		SensorData data = SensorPoll();	
		
		uint16_t now = Now();
		
		// Check if we can be shot by team one
		if(now - last_shot_by_team1 > IR_RESPAWN) {
			if(data.infrared_omni == TEAM1_ID) {
				hit_flags[0] = TEAM1_ID;
				last_shot_by_team1 = now;
			}
			else {
				 last_shot_by_team1 = now - IR_RESPAWN;
			}
		}
		
		// Check if we can be shot by team two
		if(now - last_shot_by_team2 > IR_RESPAWN) {
			if(data.infrared_omni == TEAM2_ID) {
				hit_flags[1] = TEAM2_ID;
				last_shot_by_team2 = now;
			}
			else {
				last_shot_by_team2 = now - IR_RESPAWN;
			}
		}
		
		// Check if we can be shot by team four
		if(now - last_shot_by_team4 > IR_RESPAWN) {
			if(data.infrared_omni == TEAM4_ID) {
				hit_flags[2] = TEAM4_ID;
				last_shot_by_team4 = now;
			}
		
			else {
				last_shot_by_team4 = now - IR_RESPAWN;
			}
		}
		
		if ((data.wheel_drop_left == 1)||(data.wheel_drop_right == 1)) {
			kill_robot();
		}
		
		if ((data.bumper_right == 1)||(data.bumper_left == 1)||(convert(data.light_center_left) > 100)||(convert(data.light_center_right) > 100)) {
			IRFire();
			Service_Publish(drive_state_service, DRIVE_BACKWARDS);
		}

		// For debugging purposes
		digitalWrite(12, LOW);

		Task_Next();
	}	
}

// Sends a sonar ping and extends length of a "drive forwards" command to "chase" an object while it is detected
// Fires the IR if anything is detected
// If anything is detected within 10 cm, turns the robot around
void check_sonar() {
	
	for (;;) {
		
		int16_t data;
		pinMode(11, OUTPUT);
		digitalWrite(11, LOW);
		
		Service_Subscribe(sonar_time_slot, &data);
		
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

// Starts and configures the radio 
void radio_configure() {
	
	pinMode(RADIO_POWER_PIN, OUTPUT);
	
	digitalWrite(RADIO_POWER_PIN, LOW);
	delay(100);
	digitalWrite(RADIO_POWER_PIN, HIGH);
	delay(100);
	
	Radio_Init(104);
	Radio_Configure_Rx(RADIO_PIPE_0, recv_addr , ENABLE);
	Radio_Configure(RADIO_2MBPS, RADIO_HIGHEST_POWER);
	Radio_Set_Tx_Addr(send_addr);
}

int r_main(void)
{			
	// Initialize and configure hardware and libraries
	m_robot.begin();
	IRInit();
	radio_configure();
	SensorInit(&m_robot);
	drive_init(&m_robot);
	
	// Set the robot mode
	m_robot.send(irobot::op_full_mode);
	
	// Set up shot timers so we can be shot by anyone
	uint8_t now = Now();
	last_shot_by_team1 = now - IR_RESPAWN;
	last_shot_by_team2 = now - IR_RESPAWN;
	last_shot_by_team4 = now - IR_RESPAWN;
	
	// Initialize drive and sonar services
	drive_state_service = Service_Init();
	sonar_time_slot = Service_Init();
	
	// Set light pins
	pinMode(3, OUTPUT); // 
	pinMode(4, OUTPUT); // lights up a white LED when sonar is going
		
	// So that the first drive command doesn't come in too fast
	_delay_ms(25);
	
	// IR detections are kept in buffer for 60ms
	// Running a scan every 50 ms guarantees we won't miss a hit
	Task_Create_Periodic(bump_scan, 0, 12, 6, 500);
	
	// This is a system task as the timing will be unpredictable
	Task_Create_System(drive_robot, 0);	
	
	// This is a system task as it is triggered by the base station
	Task_Create_System(check_sonar, 0);
	
	Task_Terminate();
	return 0;
}



