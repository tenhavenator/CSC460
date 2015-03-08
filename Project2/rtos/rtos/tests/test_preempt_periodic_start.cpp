/*
	Expected results: 
	Look at the logic analyzer output
	- system task should start right away (digital pin 12)
	- system task delays first run of periodic task for 50 ticks
	- periodic task's first run time starts immediately after system task
	- periodic task's second run time starts 50 ticks after first one starts instead of 100
	- periodic task's remaining run times go every 100 ticks as normal
*/

#include "../os.h"
#include <avr/io.h>
#include <util/delay.h>

static void system_task_preempting()
{
		PORTB = _BV(6);
		_delay_ms(500);
		PORTB = 0;
}


static void periodic_task_preempted()
{
	for (;;)
	{
		PORTB = _BV(7);
		_delay_ms(100);
		PORTB = 0;		
		Task_Next();
	}
}

/* To run this test, comment out the original function declaration and uncomment the r_main one. 
 * There can only be one r_main declared at any time.
 */
// int r_main(void)
int r_main_test_preempt_periodic_start(void)
{    	
	DDRB = _BV(7) | _BV(6) | _BV(5);
	PORTB = 0;
	Task_Create_Periodic(periodic_task_preempted, 0, 100, 21, 50);
	Task_Create_System(system_task_preempting, 0);
	return 0;
}
