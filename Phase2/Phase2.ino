#include <radio.h>
#include <scheduler.h>
#include <stick.h>

#define STICK_XPIN 0
#define STICK_YPIN 1
#define SWITCH_PIN 7

uint8_t send_addr[] = {0xAA,0xAA,0xAA,0xAA,0xAA};
uint8_t recv_addr[] = { 0xAA, 0xAB, 0xAC, 0xAD, 0xAE };
volatile uint8_t rxflag = 0;
radiopacket_t packet;

void radio_sendPacket()
{
  // Create packet based on stick state and sendS
}

void idle(uint32_t idle_period)
{
  uint32_t start = millis();
  
  while(millis() - start < idle_period)
  {
    STICK_STATE state = Stick_State_Current();
    // Do things with this information
  }
  
}

void setup()
{
  Serial.begin(9600);
  pinMode(13, OUTPUT);
  pinMode(47, OUTPUT);
 
  digitalWrite(47, LOW); 
  delay(100);
  digitalWrite(47, HIGH);
  delay(100);
  
  // Configure Stick
  Stick_State_Init(STICK_XPIN, STICK_YPIN, SWITCH_PIN);
  
  // Configure scheduler
  Scheduler_Init();
  Scheduler_StartTask(0, 100, radio_sendPacket);
  
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
  idle(idle_period);
  
  /*packet.type = COMMAND;
  memcpy(packet.payload.message.address, recv_addr, RADIO_ADDRESS_LENGTH);
  memcpy(packet.payload.command.sender_address, recv_addr, RADIO_ADDRESS_LENGTH);
  packet.payload.command.command = 137;
  packet.payload.command.num_arg_bytes = 4;
  packet.payload.command.arguments[0] = 0;
  packet.payload.command.arguments[1] = 200;
  packet.payload.command.arguments[2] = 128;
  packet.payload.command.arguments[3] = 0;*/
}
