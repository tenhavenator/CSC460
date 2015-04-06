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
	uint8_t  wheel_drop_left;
	uint8_t  wheel_drop_right;
	uint8_t light_center_left[2];
	uint8_t light_center_right[2];
	uint8_t light_front_right[2];
	uint8_t light_front_left[2];
	uint8_t light_right[2];
	uint8_t light_left[2];
	uint8_t infrared_omni;
} SensorData;

void SensorInit(irobot * robot);
SensorData SensorPoll();
uint16_t convert(uint8_t * light);


#endif /* SENSOR_H_ */