#include <radio.h>

uint8_t send_addr[] = {0xAA,0xAA,0xAA,0xAA,0xAA};
uint8_t recv_addr[] = { 0xAA, 0xAB, 0xAC, 0xAD, 0xAE };
volatile uint8_t rxflag = 0;
radiopacket_t packet;


void setup()
{
  Serial.begin(9600);
  pinMode(13, OUTPUT);
  pinMode(47, OUTPUT);
 
  digitalWrite(47, LOW); 
  delay(100);
  digitalWrite(47, HIGH);
  delay(100);
  
  Radio_Init();
  Radio_Configure_Rx(RADIO_PIPE_0, recv_addr , ENABLE);
  Radio_Configure(RADIO_2MBPS, RADIO_HIGHEST_POWER);
}

void radio_rxhandler(uint8_t pipenumber)
{
  rxflag = 1;
}

void loop()
{
  packet.type = COMMAND;
  memcpy(packet.payload.message.address, recv_addr, RADIO_ADDRESS_LENGTH);
  memcpy(packet.payload.command.sender_address, recv_addr, RADIO_ADDRESS_LENGTH);
  packet.payload.command.command = 137;
  packet.payload.command.num_arg_bytes = 4;
  packet.payload.command.arguments[0] = 0;
  packet.payload.command.arguments[1] = 200;
  packet.payload.command.arguments[2] = 128;
  packet.payload.command.arguments[3] = 0;
 
  Radio_Set_Tx_Addr(send_addr);
  
  Radio_Transmit(&packet, RADIO_WAIT_FOR_TX);
  
  if (rxflag)
  {
    if (Radio_Receive(&packet) != RADIO_RX_MORE_PACKETS)
    {
      rxflag = 0;
    }
  }
  delay(1000);
}
