
/*
 * stick.h
 *
 * Created by Jeff ten Have, Colin Knowles 
 * CSC 460 Project 1, Phase 2
 * January 29, 2015
 *
 */


#ifndef STICK_H_
#define STICK_H_

#define SLOW_SPEED_THRES 128
#define NORMAL_SPEED_THRES 256
#define FAST_SPEED_THRES 384

typedef enum _speed
{
	FAST,
	NORMAL,
	SLOW,
	STOP,
	SLOW_R,
	NORMAL_R,
	FAST_R
} SPEED;

typedef enum _radius
{
	NONE_L,
	MED_L,
	HIGH_L,
	MAX,
	HIGH_R,
	MED_R,
	NONE_R,
} RADIUS;

typedef enum _switch
{
	PRESSED,
	NOT_PRESSED

} SWITCH;

typedef struct _state
{
	RADIUS radius_s;
	SPEED speed_s;
	SWITCH switch_s;

} STICK_STATE;

void Stick_State_Init(int pXPin, int pYPin, int pSwitchPin);

STICK_STATE Stick_State_Current();


#endif

