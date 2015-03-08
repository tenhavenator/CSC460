/*
	Expected results: 
	Watch the ERROR LEDs
	- you should see the green (initialize-time) LED light up
	- one long pulse, followed by one short one
*/

#include "../os.h"
#include <avr/io.h>

void periodic_task_wcetgreater()
{
	for (;;)
	{
		Task_Next();
	}
}

/* To run this test, comment out the original function declaration and uncomment the r_main one. 
 * There can only be one r_main declared at any time.
 */
// int r_main(void)
int r_main_test_wcet_greater_than_period(void)
{    	
	Task_Create_Periodic(periodic_task_wcetgreater, 0, 1, 2, 0);
	return 0;
}
