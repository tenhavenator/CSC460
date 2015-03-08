#include "../os.h"
#include <avr/io.h>

extern int8_t Task_Create_Periodic(void(*f)(void), int16_t arg, uint16_t period, uint16_t wcet, uint16_t start);

void periodic_task_to_create()
{
	for (;;)
	{
		DDRB = _BV(7);
		PORTB = _BV(7);
		PORTB = 0;
		Task_Next();
	}
}

/* To run this test, comment out the original function declaration and uncomment the r_main one. 
 * There can only be one r_main declared at any time.
 */
// int r_main(void)
int r_main_test_periodic_schedule_valid(void)
{    	
	return 0;
}
