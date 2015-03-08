#include "../os.h"
#include <avr/io.h>
#include <util/delay.h>

void system_task_to_create()
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
int r_main_test_create_system(void)
{    	
	return 0;
}
