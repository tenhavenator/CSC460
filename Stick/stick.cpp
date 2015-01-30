
/*
* stick.h
*
* Created by Jeff ten Have, Colin Knowles
* CSC 460 Project 1, Phase 2
* January 29, 2015
*
*/

#include "stick.h"
#include <Arduino.h> 

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
	int x = analogRead(mXPin) - 515;
	int y = analogRead(mYPin) - 515;
	STICK_STATE current_state;

	int speed = max(abs(x), abs(y));

	// ****************************************************************************
	// Calculate Speed value

	if (speed < SLOW_SPEED_THRES)
	{
		current_state.speed_s = STOP;
	}
	else if (speed >= SLOW_SPEED_THRES && speed < NORMAL_SPEED_THRES)
	{
		if (y > 0)
		{
			current_state.speed_s = SLOW_R;
		}
		else
		{
			current_state.speed_s = SLOW;
		}
	}
	else if (speed >= NORMAL_SPEED_THRES && speed < FAST_SPEED_THRES)
	{
		if (y > 0)
		{
			current_state.speed_s = NORMAL_R;
		}
		else
		{
			current_state.speed_s = NORMAL;
		}
	}
	else
	{
		if (y > 0)
		{
			current_state.speed_s = FAST_R;
		}
		else
		{
			current_state.speed_s = FAST;
		}
	}

	// ****************************************************************************
	// Calculate Radius value

	if (abs(x) < SLOW_SPEED_THRES)
	{
		current_state.radius_s = MAX;
	}
	else if (x >= SLOW_SPEED_THRES && x < NORMAL_SPEED_THRES)
	{
		current_state.radius_s = HIGH_R;
	}
	else if (x <= -SLOW_SPEED_THRES && x > -NORMAL_SPEED_THRES)
	{
		current_state.radius_s = HIGH_L;
	}
	else if (x >= NORMAL_SPEED_THRES && x < FAST_SPEED_THRES)
	{
		current_state.radius_s = MED_R;
	}
	else if (x <= -NORMAL_SPEED_THRES && x > -FAST_SPEED_THRES)
	{
		current_state.radius_s = MED_L;
	}
	else if (x >= FAST_SPEED_THRES)
	{
		current_state.radius_s = NONE_R;
	}
	else if (x <= FAST_SPEED_THRES)
	{
		current_state.radius_s = NONE_L;
	}

	if (abs(y) < SLOW_SPEED_THRES)
	{
		if (x >= 0)
		{
			current_state.radius_s = NONE_R;
		}
		else
		{
			current_state.radius_s = NONE_L;
		}
	}
	
	// ****************************************************************************
	// Calculate Switch Value

	if (digitalRead(mSwitchPin) == LOW)
	{
		current_state.switch_s = PRESSED;
	}
	else
	{
		current_state.switch_s = NOT_PRESSED;
	}

	return current_state;
}

