#include <ServoT4.h>

#define BUFFER_SIZE 36
#define SWITCH_PIN 4

volatile int mNumSendBits = 0;
volatile int mBitBuffer[BUFFER_SIZE];
volatile int bitBuffer[] = {0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0};
volatile int mByteIndex = 0;

ServoT4 mServo;

// Takes a byte and sets the bit buffer 
void setBitBuffer(byte pByte)
{
  mBitBuffer[0] = 0;
  mBitBuffer[1] = 0;
  for(int i = 2; i < BUFFER_SIZE; i++)
  {
    mBitBuffer[i] = (pByte >> (i-2)) & 0x1;
  }
}

void setup()
{  
  // Sets data direction for pin 13 to output (i.e. makes OC1C value visible)
  // We could use digital pins 11, 12, or 13 because these are the pins where timer 1 can generate a waveform
  pinMode(13, OUTPUT);
  
  // Sets digital pin 4 to the stick click
  pinMode(SWITCH_PIN, INPUT);
  
  // Enables pull-up resistors on switch stick
  digitalWrite(SWITCH_PIN, HIGH); 
  
  
  mServo.attach(9);
  
  // ======== Set up timer 3 to be the 500 us counter
  // Clear timer config.
  TCCR3A = 0;
  TCCR3B = 0;  
  
  //Set to CTC (mode 4)
  TCCR3B |= (1<<WGM32);
  
  // No Prescalar
  TCCR3B |= (1<<CS30);
  
  // Set TOP value (0.0005 seconds)
  OCR3A = 16000;
  
  // Enable interupt for when OCR3A matches TCNT3 (Timer register)
  TIMSK3 |= (1<<OCIE3A);
  
  //Set timer 3 to 0 (optional here).
  TCNT3 = 0;
  
  // ======== Set up timer 1 to be the 38 KHz timer =======================
  
  // Clear timer config
  TCCR1A = 0;
  TCCR1B = 0;
 
  // Set to Fast PWM (mode 15)
  TCCR1A |= (1<<WGM10) | (1<<WGM11);
  TCCR1B |= (1<<WGM12) | (1<<WGM13);
  	
  // Generate wave form in OC1C (digital 13, see pin mapping) using OCR1C as compare register. Set at bottom, clear at match.
  TCCR1A |= (1<<COM1C1);
  
  // No prescaler
  TCCR1B |= (1<<CS10);
  
  OCR1A = 421;  // 38 KHz (Top value)
  OCR1C = 0;  // Target
}

// See p. 101 for interrupts. Triggered when 500 us timer goes off
ISR(TIMER3_COMPA_vect)
{
  if(mNumSendBits > 0)
  {
    OCR1C = bitBuffer[BUFFER_SIZE - mNumSendBits] == 1 ? 105 : 0; // 421 * 25%
    mNumSendBits--;
  }
  else
  {
    OCR1C = 0;
  }
}

void loop() 
{       
   if(mNumSendBits == 0)
  {
    mNumSendBits = BUFFER_SIZE;    
  }
    
}
