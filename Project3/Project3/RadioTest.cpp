/*
 * RadioTest.cpp
 *
 * Created: 2015-04-06 4:04:14 PM
 *  Author: Jeff
 */ 
#if 0
#include "Arduino.h"
#include "pins.h"
#include "Radio/radio.h"
#include "Radio/Packets/radio_packets.hpp"
#include "OS/os.h"

static uint8_t recv_addr[5] = {0x42, 0x4F, 0x54, 0x5F, 0x43};
static uint8_t send_addr[5] = {0x44, 0x42, 0x41, 0x53, 0x45};

extern "C" {
void radio_rxhandler(uint8_t pipenumber)
{
	/* radio_packet_t rx_packet;
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
					 Serial.print("Sonar Packet");
					 break;
				 }
				 
				 case game_status_packet_type:
				 {
					 Serial.print("Game Status Packet");
					 break;
				 }
				 
				
				 default: 
				 {
					Serial.print("Wrong packet");
					break;
				 }
			
			 }
		 }
	 }*/

}
}



void radio_init_test() {
	
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

void send(void) {
	
	for(;;) {
				
		radio_packet_t packet;
		packet.type = shot_packet_type;
		packet.shot.shooter_id = 67;
		packet.shot.target_id = 66;
			
		Radio_Transmit(&packet, RADIO_RETURN_ON_TX);
		
		Task_Next();
	}
	
}

void receive(void) {
	
	
	for(;;) {
		

		
		Task_Next();
	}
	
}

int r_main(void) {
	
	
	//Serial.begin(9600);
	radio_init_test();
	
	Task_Create_Periodic(send, 0, 200, 100, 500);
	//Task_Create_Periodic(receive, 0, 200, 100, 500);
	
	return 0;
}



#endif