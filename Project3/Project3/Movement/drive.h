/*
 * drive.h
 *
 * Created: 2015-04-05 10:40:14 AM
 *  Author: Colin
 */ 


#ifndef DRIVE_H_
#define DRIVE_H_

#define TURN_90_DEGREES_MS 885
#define TURN_45_DEGREES_MS 442

enum
{
	DRIVE_STOP = 0,
	DRIVE_FORWARDS,
	DRIVE_BACKWARDS,
	DRIVE_TURN_CW_90,
	DRIVE_TURN_CCW_90,
	DRIVE_TURN_AROUND,
};


void drive_init(irobot * robot);
void drive(int16_t velocity);
void spin(bool cw);

#endif /* DRIVE_H_ */