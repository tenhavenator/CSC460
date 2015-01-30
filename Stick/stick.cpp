
/*
* stick.h
*
* Created by Jeff ten Have, Colin Knowles
* CSC 460 Project 1, Phase 2
* January 29, 2015
*
*/

#include "stick.h"

int mXPin;
int mYPin;
int mSwitchPin;


void Stick_State_Init(int pXPin, int pYPin, int pSwitchPin)
{
	mXPin = pXPin;
	mYPin = pYPin;
	mSwitchPin = pSwitchPin;

	pinMode(mSwitchPin, INPUT);
	digitalWrite(mSwitchPin, HIGH);
}

STICK_STATE Stick_State_Current()
{

	STICK_STATE current_state;
	// Calculate state

	return current_state;
}
