/*
 * sensor.cpp
 *
 * Created: 2015-04-04 3:52:52 PM
 *  Author: Jeff
 */ 

#include "../irobot/irobot.hpp"
#include "sensor.h"

static irobot * m_robot = NULL;

void SensorInit(irobot * robot) {
	m_robot = robot;
}

uint16_t convert(uint8_t * light) {
	uint16_t high = light[0];
	uint16_t low = light[1];
	
	return (high << 8) | low;
}

SensorData SensorPoll() {
	
	// Jakob made this code
	static const int sensor_count = 8;
	uint8_t data[] = {
		sensor_count,
		irobot::sense_bumps_and_wheel_drops,
		irobot::sense_light_bump_left_signal,
		irobot::sense_light_bump_front_left_signal,
		irobot::sense_light_bump_center_left_signal,
		irobot::sense_light_bump_center_right_signal,
		irobot::sense_light_bump_front_right_signal,
		irobot::sense_light_bump_right_signal,
		irobot::sense_infrared_omni
	};
	
	
	 SensorData sensor_data;
	 m_robot->send(irobot::op_sensor_list, data, sensor_count + 1);
	 
	 uint8_t bumper_values = 0;
	 m_robot->receive(&bumper_values, 1);
	 sensor_data.bumper_right = bumper_values &0x1;
	 sensor_data.bumper_left = (bumper_values >> 1) &0x1;
	 sensor_data.wheel_drop_right = (bumper_values >> 2) &0x1;
	 sensor_data.wheel_drop_left = (bumper_values >> 3) &0x1;
	 
	 
	
	 m_robot->receive(sensor_data.light_left, 2);
	 
	 
	 m_robot->receive(sensor_data.light_front_left, 2);
	 m_robot->receive(sensor_data.light_center_left, 2);
	 m_robot->receive(sensor_data.light_center_right, 2);
	 m_robot->receive(sensor_data.light_front_right, 2);
	 m_robot->receive(sensor_data.light_right, 2);
	 m_robot->receive(&sensor_data.infrared_omni, 1);
	  
	 
	return sensor_data;
}