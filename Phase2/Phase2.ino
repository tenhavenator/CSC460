#include <radio.h>
#include <scheduler.h>
#include <stick.h>

#define STICK_XPIN 0
#define STICK_YPIN 1
#define SWITCH_PIN 7

#define RADIO_POWER_PIN 47

uint8_t send_addr[] = {0xAA,0xAA,0xAA,0xAA,0xAA};
uint8_t recv_addr[] = { 0xAA, 0xAB, 0xAC, 0xAD, 0xAE };

uint8_t velocity_high[] = {0, 0, 0, 0, 255, 255, 255};
uint8_t velocity_low[] = {200, 100, 50, 0, 206, 156, 56};
uint8_t radius_high[] = {0, 1, 3, 128, 252, 254, 255};
uint8_t radius_low[] = {1, 244, 232, 0, 24, 12, 255};

STICK_STATE mState;
volatile uint8_t rxflag = 0;

radiopacket_t radio_createPacket(uint8_t type, uint8_t speed_s, uint8_t radius, uint8_t data_byte)
{
  radiopacket_t pkt;
  pkt.type = type;
  memcpy(pkt.payload.command.sender_address, recv_addr, RADIO_ADDRESS_LENGTH);
  
  if (type == COMMAND) 
  {
	pkt.payload.command.command = 137;
	pkt.payload.command.num_arg_bytes = 4;
	pkt.payload.command.arguments[0] = velocity_high[speed_s];
	pkt.payload.command.arguments[1] = velocity_low[speed_s];
	pkt.payload.command.arguments[2] = radius_high[radius];
	pkt.payload.command.arguments[3] = radius_low[radius];
  }
  else if (type == IR_COMMAND) 
  {
	pkt.payload.ir_command.ir_command = SEND_BYTE;
	pkt.payload.ir_command.ir_data = data_byte;
	pkt.payload.ir_command.servo_angle = 0;
  }
  
  return pkt;
}

/*
 * Periodic task to send a radio packet based on the desired state of the Roomba
 */
void radio_sendPacket()
{
  radiopacket_t command_packet = radio_createPacket(COMMAND, mState.speed_s, mState.radius_s, 0);
  Radio_Set_Tx_Addr(send_addr);
  Radio_Transmit(&command_packet, RADIO_RETURN_ON_TX);
  
  if(mState.switch_s)
  {
    radiopacket_t ir_packet = radio_createPacket(IR_COMMAND, mState.speed_s, mState.radius_s, 65);
    Radio_Set_Tx_Addr(send_addr);
    Radio_Transmit(&ir_packet, RADIO_RETURN_ON_TX);
  }
}

/*
 * Periodic task to send a radio packet based on the desired state of the Roomba
 */
void poll_stick()
{
  mState = Stick_State_Current();  
}

/*
 * The idle task for the TTA scheduler.
 */
void idle(uint32_t idle_period)
{
  delay(idle_period);
}



void setup()
{  
  pinMode(13, OUTPUT);
  pinMode(RADIO_POWER_PIN, OUTPUT);
 
  digitalWrite(RADIO_POWER_PIN, LOW); 
  delay(100);
  digitalWrite(RADIO_POWER_PIN, HIGH);
  delay(100);
  
  // Configure Stick
  Stick_State_Init(STICK_XPIN, STICK_YPIN, SWITCH_PIN);
  
  // Configure scheduler
  Scheduler_Init();
  Scheduler_StartTask(0, 300, poll_stick);
  Scheduler_StartTask(5, 50, radio_sendPacket);
 
  // Configure Radio
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
  // Dispatch a scheduled event 
  uint32_t idle_period = Scheduler_Dispatch();
  idle(idle_period);
}
