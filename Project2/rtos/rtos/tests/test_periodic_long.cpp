#include "../os.h"
#include <avr/io.h>
#include <util/delay.h>

extern int8_t Task_Create_Periodic(void(*f)(void), int16_t arg, uint16_t period, uint16_t wcet, uint16_t start);

void periodic_task_long()
{
	for (;;)
	{
		DDRB = _BV(7);
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
	Task_Create_Periodic(periodic_task_long, 0, 200, 21, 0);
	return 0;
}
