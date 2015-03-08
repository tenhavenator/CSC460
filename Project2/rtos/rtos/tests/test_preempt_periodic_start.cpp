#include "../os.h"
#include <avr/io.h>
#include <util/delay.h>

static void system_task_preempting()
{
		PORTB = _BV(6);
		_delay_ms(400);
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
int r_main(void)
///int r_main_test_preempt_periodic_start(void)
{    	
	DDRB = _BV(7) | _BV(6);
	
	Task_Create_Periodic(periodic_task_preempted, 0, 100, 100, 0);
	Task_Create_System(system_task_preempting, 0);
	return 0;
}
