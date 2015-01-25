#include <ServoT4.h>

#define BUFFER_SIZE 10
#define SWITCH_PIN 4

volatile int mNumSendBits = 0;
volatile int mBitBuffer[BUFFER_SIZE];
volatile byte mBytes[] = {B01000001, B01000010, B01000011, B01000100}; // A, B, C, D
volatile int mByteIndex = 0;

ServoT4 mServo;

// Takes a byte and sets the bit buffer 
void setBitBuffer(byte pByte)
{
  mBitBuffer[0] = 1;
  mBitBuffer[1] = 0;
  for(int i = 2; i < BUFFER_SIZE; i++)
  {
    mBitBuffer[i] = (pByte >> (i-2)) & 0x1;
  }
}

void setup()
{  
  //Serial.begin(9600);
  
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
  OCR3A = 8000;
  
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
    OCR1C = mBitBuffer[BUFFER_SIZE - mNumSendBits] == 1 ? 105 : 0; // 421 * 25%
    mNumSendBits--;
  }
  else
  {
    OCR1C = 0;
  }
}

void loop() 
{     

  int stickPos = analogRead(A0);
  stickPos = map(stickPos, 0, 1023, 0, 180);
  mServo.write(stickPos);
  
  if(digitalRead(SWITCH_PIN) == LOW)
  {
     if(mNumSendBits == 0)
    {
      setBitBuffer(mBytes[mByteIndex]);
      mNumSendBits = BUFFER_SIZE;
      mByteIndex = mByteIndex == 3 ? 0 : mByteIndex + 1;
      
    }
    
    while(digitalRead(SWITCH_PIN) == LOW)
    {     
      // Nothing
    }
  }
}
