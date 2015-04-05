#include "irobot/irobot.hpp"
#include "Sensors/sensor.h"
#include <Arduino.h>

irobot g_robot(&Serial1, 5);

static void drive_straight(int16_t velocity)
{
	
    uint8_t data[4];
    data[0] = (velocity >> 8) & 0xFF;
    data[1] = velocity & 0xFF;
    data[2] = 0x80;
    data[3] = 0x00;
    g_robot.send(irobot::op_drive, data, 4);

}




void pdrive(int16_t velocity)
{
	uint8_t send_bytes[4];
	send_bytes[0] = (velocity >> 8) & 0xFF;
	send_bytes[1] = velocity & 0xFF;
	send_bytes[2] = 0x80;
	send_bytes[3] = 0x00;
	g_robot.send(irobot::op_drive, send_bytes, 4);
}

void pspin(int16_t angle, bool cw)
{
	uint8_t send_bytes[4];
	int16_t velocity = -300;
	if (cw) {
		velocity = 300;
	}
	
	send_bytes[0] = (velocity >> 8) & 0xFF;
	send_bytes[1] = velocity & 0xFF;
	send_bytes[2] = 0;
	send_bytes[3] = 1;
	g_robot.send(irobot::op_drive, send_bytes, 4);
}



int p_main()
{
    init();

    pinMode(13, OUTPUT);

    digitalWrite(13, LOW);

    g_robot.begin();

	//g_robot.send(irobot::op_reset);
    g_robot.send(irobot::op_full_mode);

    digitalWrite(13, HIGH);
	
	pspin(2, true);
	
	int rotated = 0;
	for(;;) {
		int16_t angle = GetAngle();
		rotated += angle;
		if (rotated >= 360) {
			pdrive(0);
		}
		
		
		_delay_ms(100);
	}

    /*for(;;)
    {
        g_robot.send(irobot::op_sensor, irobot::sense_bumps_and_wheel_drops);

        uint8_t raw_bumps_and_wheel_drops;
        g_robot.receive(&raw_bumps_and_wheel_drops, 1);

        bool wheel_drop = raw_bumps_and_wheel_drops & (_BV(3) | _BV(2));

        if (wheel_drop)
        {
            g_robot.stop();
            return 0;
        }
    }*/
}
