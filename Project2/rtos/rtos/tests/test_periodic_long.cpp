#include "../os.h"
#include <avr/io.h>
#include <util/delay.h>

void periodic_task_long()
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
int r_main_test_periodic_long(void)
{    	
	DDRB = _BV(7);
	Task_Create_Periodic(periodic_task_long, 0, 200, 21, 0);
	return 0;
}
