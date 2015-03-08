/*
	Expected results: 
	Look at the logic analyzer output
	- while periodic task is running, digital pin 13 is set to on
	- system task gets created in periodic task after 5ms delay
	- while system task is on (5ms), periodic task is preempted
	- while system task is on, digital pin 12 is on instead of 13
	- after system task ends, periodic task continues for 5ms
	- total periodic run time = 10ms + 5ms preemption = 15ms
*/

#include "../os.h"
#include <avr/io.h>
#include <util/delay.h>

void system_task_preempting()
{
		PORTB = PORTB | _BV(6);
		_delay_ms(5);
		PORTB = PORTB & ~_BV(6);
}

void periodic_task_preempted()
{
	for (;;)
	{
		PORTB = _BV(7);
		_delay_ms(5);
		Task_Create_System(system_task_preempting, 0);
		_delay_ms(5);
		PORTB = 0;
		
		Task_Next();
	}
}

/* To run this test, comment out the original function declaration and uncomment the r_main one. 
 * There can only be one r_main declared at any time.
 */
// int r_main(void)
int r_main_test_preempt_periodic_middle(void)
{    	
	DDRB = _BV(7) | _BV(6);
	Task_Create_Periodic(periodic_task_preempted, 0, 4, 4, 0);
	return 0;
}
