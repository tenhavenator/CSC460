#include <radio.h>
#include <scheduler.h>

#define SWITCH_PIN 4
#define RADIO_POWER_PIN 47

uint8_t send_addr[] = {0xAA,0xAA,0xAA,0xAA,0xAA};
uint8_t recv_addr[] = { 0xAA, 0xAB, 0xAC, 0xAD, 0xAE };

volatile uint8_t rxflag = 0;
radiopacket_t packet;

void radio_sendPacket()
{
 Radio_Transmit(&packet, RADIO_WAIT_FOR_TX);
  
 if (rxflag)
 {
    if (Radio_Receive(&packet) != RADIO_RX_MORE_PACKETS)
    {
      rxflag = 0;
    }
  }
  else
  {
    // Add packet to the front of the queue.
  }
}

void stick_poll(uint32_t idle_period)
{
  uint32_t start = millis();
  
  while(millis() - start < idle_period)
  {
    if(digitalRead(SWITCH_PIN) == LOW)
    {
       // Queue up and IR packet
    }
    
    
    
    
  }
}

void setup()
{
  pinMode(SWITCH_PIN, INPUT);
  digitalWrite(SWITCH_PIN, HIGH); 
  
  pinMode(13, OUTPUT);
  pinMode(RADIO_POWER_PIN, OUTPUT);
 
  digitalWrite(RADIO_POWER_PIN, LOW); 
  delay(100);
  digitalWrite(RADIO_POWER_PIN, HIGH);
  delay(100);
  
  // Configure scheduler
  Scheduler_Init();
  Scheduler_StartTask(0, 10, radio_sendPacket);
  
  // Configure Radio
  Radio_Init();
  Radio_Configure_Rx(RADIO_PIPE_0, recv_addr , ENABLE);
  Radio_Configure(RADIO_2MBPS, RADIO_HIGHEST_POWER);
  Radio_Set_Tx_Addr(send_addr); // Think this can be be here
  
  // Load return addresses into packet
  memcpy(packet.payload.message.address, recv_addr, RADIO_ADDRESS_LENGTH);
  memcpy(packet.payload.command.sender_address, recv_addr, RADIO_ADDRESS_LENGTH);
  
}

void radio_rxhandler(uint8_t pipenumber)
{
  rxflag = 1;
}

void loop()
{
  
  
  uint32_t idle_period = Scheduler_Dispatch();
  stick_poll(idle_period);
  
  packet.type = COMMAND;
  packet.payload.command.command = 137;
  packet.payload.command.num_arg_bytes = 4;
  packet.payload.command.arguments[0] = 0;
  packet.payload.command.arguments[1] = 200;
  packet.payload.command.arguments[2] = 128;
  packet.payload.command.arguments[3] = 0;
}
