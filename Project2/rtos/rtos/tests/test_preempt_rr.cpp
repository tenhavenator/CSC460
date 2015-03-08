/*
	Expected results: 
	Look at the logic analyzer output
	- you should see a fast pulse (RR) on digital pin 13
	- you should also see a long pulse on digital pin 12 that stops the fast pulse (periodic)
	- the periodic task should have a period of 0.2s
*/

#include "../os.h"
#include <avr/io.h>
#include <util/delay.h>

void periodic_task_preempting()
{
	for (;;)
	{
		PORTB = _BV(6);
		_delay_ms(100);
		PORTB = 0;
		Task_Next();
	}
}

void rr_task_preempted()
{
	for (;;)
	{
		PORTB = _BV(7);
		_delay_ms(10);
		PORTB = 0;
		_delay_ms(10);
	}
}

/* To run this test, comment out the original function declaration and uncomment the r_main one. 
 * There can only be one r_main declared at any time.
 */
// int r_main(void)
int r_main_test_preempt_rr(void)
{    	
	DDRB = _BV(7) | _BV(6);
	Task_Create_RR(rr_task_preempted, 0);
	Task_Create_Periodic(periodic_task_preempting, 0, 40, 21, 0);
	return 0;
}
