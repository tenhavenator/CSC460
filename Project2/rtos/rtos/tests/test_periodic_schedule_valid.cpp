/*
	Expected results: 
	Look at the logic analyzer output
	- you should see digital pin 13 go on and off, then pin 12, pin 11, repeatedly
	- each should have a period of 15ms
*/

#include "../os.h"
#include <avr/io.h>

void periodic_task_schedule_1()
{
	for (;;)
	{
		PORTB = _BV(5);
		PORTB = 0;
		Task_Next();
	}
}

void periodic_task_schedule_2()
{
	for (;;)
	{
		PORTB = _BV(6);
		PORTB = 0;
		Task_Next();
	}
}

void periodic_task_schedule_3()
{
	for (;;)
	{
		PORTB = _BV(7);
		PORTB = 0;
		Task_Next();
	}
}

/* To run this test, comment out the original function declaration and uncomment the r_main one. 
 * There can only be one r_main declared at any time.
 */
// int r_main(void)
int r_main_test_periodic_schedule_valid(void)
{    	
	DDRB = _BV(7) | _BV(6) | _BV(5);
	Task_Create_Periodic(periodic_task_schedule_1, 0, 3, 1, 0);
	Task_Create_Periodic(periodic_task_schedule_2, 0, 3, 1, 1);
	Task_Create_Periodic(periodic_task_schedule_3, 0, 3, 1, 2);
	return 0;
}
