/*
 * sensor.h
 *
 * Created: 2015-04-04 3:52:28 PM
 *  Author: Jeff
 */ 

#ifndef SENSOR_H_
#define SENSOR_H_

typedef struct {
	
	uint8_t  bumper_right;
	uint8_t  bumper_left;
	uint8_t light_center_left[2];
	uint8_t light_center_right[2];
	uint8_t light_front_right[2];
	uint8_t light_front_left[2];
	uint8_t light_right[2];
	uint8_t light_left[2];
	uint8_t infrared_omni[2];
} SensorData;

void SensorInit(irobot * robot);
SensorData SensorPoll();

int16_t GetAngle();

#endif /* SENSOR_H_ */