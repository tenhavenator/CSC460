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
		
		/*
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
		}*/
		
		if ((data.wheel_drop_left == 1)||(data.wheel_drop_right == 1)) {
			kill_robot();
		}
		
		if ((data.bumper_right == 1)||(data.bumper_left == 1)||(convert(data.light_center_left) > 100)||(convert(data.light_center_right) > 100)) {
			Service_Publish(drive_state_service, DRIVE_BACKWARDS);
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

ISR(TIMER4_COMPA_vect)
{
	disable_drive_interrupt();
	
	// Change the drive state to the desired state
	if (drive_state == DRIVE_FORWARDS) {
		
	} 
	else if ((drive_state == DRIVE_TURN_CCW_90)||(drive_state == DRIVE_TURN_CW_90)) {
		
	}
	else if (drive_state == DRIVE_BACKWARDS) {
		
	}
	
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
			new_state = DRIVE_TURN_CCW_180;
			break;
		
		case DRIVE_TURN_CW_90:
			new_state = DRIVE_FORWARDS;
			break;
		
		case DRIVE_TURN_CCW_90:
			new_state = DRIVE_FORWARDS;
			break;
		
		case DRIVE_TURN_CW_180:
			new_state = DRIVE_FORWARDS;
			break;
		
		case DRIVE_TURN_CCW_180:
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
				
			case DRIVE_TURN_CW_180:
				spin(true);
				enable_drive_interrupt(TURN_90_DEGREES_MS * 2);
				break;
				
			case DRIVE_TURN_CCW_180:
				spin(false);
				enable_drive_interrupt(TURN_90_DEGREES_MS * 2);
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

void normalize() {
	for (;;) {
		
		drive(0);
		
		// All three sonars detect something
		if ((sonar_sensor_values[0])&&(sonar_sensor_values[1])&&(sonar_sensor_values[2])) {
			// Don't do anything
			//Serial.write("Stay");
			//Serial.println();
			Task_Next();
		}
		// "Left" sonar only
		if ((sonar_sensor_values[0])&&(!sonar_sensor_values[1])&&(!sonar_sensor_values[2])) {
			spin(false);
			enable_drive_interrupt(TURN_8_DEGREES_MS);
			//Serial.write("Spin CW FAST");
			//Serial.println();
			Task_Next();
		}
		// "Left" and "middle" sonars detect
		if ((sonar_sensor_values[0])&&(sonar_sensor_values[1])&&(!sonar_sensor_values[2])) {
			spin(false);
			enable_drive_interrupt(TURN_4_DEGREES_MS);
			//Serial.write("Spin CW SLOW");
			//Serial.println();
			Task_Next();
		}
		// "Middle" sonar only
		if ((!sonar_sensor_values[0])&&(sonar_sensor_values[1])&&(!sonar_sensor_values[2])) {
			// Don't do anything
			//Serial.write("Stay");
			//Serial.println();
			Task_Next();
		}
		// "Middle" and "right" sonars detect
		if ((!sonar_sensor_values[0])&&(sonar_sensor_values[1])&&(sonar_sensor_values[2])) {
			spin(true);
			enable_drive_interrupt(TURN_4_DEGREES_MS);
			//Serial.write("Spin CCW SLOW");
			//Serial.println();
			Task_Next();
		}
		// "Right" sonar only
		if ((!sonar_sensor_values[0])&&(!sonar_sensor_values[1])&&(sonar_sensor_values[2])) {
			spin(true);
			enable_drive_interrupt(TURN_8_DEGREES_MS);
			//Serial.write("Spin CCW FAST");
			//Serial.println();
			Task_Next();
		}
		//Serial.write("Stay");
		//Serial.println();
		Task_Next();
	}
}

void check_sonar() {
	
	for (;;) {
		// Left (from roomba's perspective) is pin 48, middle pin 49, right pin 50
		uint16_t range_left = SonarFire(100, SONAR1_SIGNAL_PIN);
		uint16_t range_mid = SonarFire(100, SONAR2_SIGNAL_PIN);
		uint16_t range_right = SonarFire(100, SONAR3_SIGNAL_PIN);
		
		//if ((range < 300) && (range != 0)) {
			
			
		//} else {
			
		//}
		
		pinMode(2, OUTPUT);
		pinMode(3, OUTPUT);
		pinMode(4, OUTPUT);
		
		if ((range_left > 10)&&(range_left < 220)) {
			digitalWrite(2, HIGH);
			sonar_sensor_values[0] = 1;
		} else {
			digitalWrite(2, LOW);
			sonar_sensor_values[0] = 0;
		}
		
		if ((range_mid > 10)&&(range_mid < 220)) {
			digitalWrite(3, HIGH);
			sonar_sensor_values[1] = 1;
			} else {
			digitalWrite(3, LOW);
			sonar_sensor_values[1] = 0;
		}
		
		if ((range_right > 10)&&(range_right < 220)) {
			digitalWrite(4, HIGH);
			sonar_sensor_values[2] = 1;
			} else {
			digitalWrite(4, LOW);
			sonar_sensor_values[2] = 0;
		}
		
		/*
		Serial.print(range_left, DEC);
		Serial.write(" ");
		Serial.print(range_mid, DEC);
		Serial.write(" ");
		Serial.print(range_right, DEC);
		Serial.println();
		*/
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
	m_robot.send(irobot::op_safe_mode);
	//m_robot.send(irobot::op_full_mode);
	
	//SensorInit(&m_robot);
	
	//drive_state_service = Service_Init();
	
	//drive_init(&m_robot);
	
	//Task_Create_Periodic(bump_scan, 0,20,15, 400);
	
	//Task_Create_System(drive_robot, 0);
	
	//Task_Create_Periodic(check_sonar, 0, 100, 40, 500);
	//Task_Create_Periodic(normalize, 0, 100, 40, 550);
	//Task_Create_Periodic(radio_stuff, 0, 200, 199, 150);
	
	
	
	Task_Terminate();
	return 0;
}




