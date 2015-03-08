#include "../os.h"
#include <avr/io.h>

extern int8_t Task_Create_Periodic(void(*f)(void), int16_t arg, uint16_t period, uint16_t wcet, uint16_t start);

void periodic_task_schedule_1()
{
	for (;;)
	{
		PORTB = _BV(5);
		PORTB = 0;
		Task_Next();
	}
}

void periodic_task_schedule_2()
{
	for (;;)
	{
		PORTB = _BV(6);
		PORTB = 0;
		Task_Next();
	}
}

void periodic_task_schedule_3()
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
int r_main_test_periodic_schedule_valid(void)
{    	
	DDRB = _BV(7) | _BV(6) | _BV(5);
	Task_Create_Periodic(periodic_task_schedule_1, 0, 3, 1, 0);
	Task_Create_Periodic(periodic_task_schedule_2, 0, 3, 1, 1);
	Task_Create_Periodic(periodic_task_schedule_3, 0, 3, 1, 2);
	return 0;
}
