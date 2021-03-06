/*
	Expected results: 
	Look  at the logic analyzer output
	- you should see digital pin 13 go on and off for 100 ms at a time each forever
*/

#include "../os.h"
#include <avr/io.h>
#include <util/delay.h>

void rr_task_to_create()
{
	for(;;) {
		_delay_ms(100);
		PORTB = _BV(7);
		_delay_ms(100);
		PORTB = 0;
	}
}

/* To run this test, comment out the original function declaration and uncomment the r_main one. 
 * There can only be one r_main declared at any time.
 */
// int r_main(void)
int r_main_test_create_rr(void)
{    	
	DDRB = _BV(7);
	Task_Create_RR(rr_task_to_create, 0);
	return 0;
}
