#include "../os.h"
#include <avr/io.h>

extern int8_t Task_Create_Periodic(void(*f)(void), int16_t arg, uint16_t period, uint16_t wcet, uint16_t start);

void periodic_task_zerowcet()
{
	for (;;)
	{
		Task_Next();
	}
}

/* To run this test, comment out the original function declaration and uncomment the r_main one. 
 * There can only be one r_main declared at any time.
 */
// int r_main(void)
int r_main_test_periodic_wcet_zero(void)
{    	
	Task_Create_Periodic(periodic_task_zerowcet, 0, 2, 0, 0);
	return 0;
}