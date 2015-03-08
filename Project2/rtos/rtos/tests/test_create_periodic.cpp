/*
	Expected results: 
	Look  at the logic analyzer output
	- you should see digital pin 13 go on and off, with a period of 10 ms
*/

#include "../os.h"
#include <avr/io.h>

void periodic_task_to_create()
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
int r_main_test_create_periodic(void)
{    	
	DDRB = _BV(7);
	Task_Create_Periodic(periodic_task_to_create, 0, 2, 1, 0);
	return 0;
}
