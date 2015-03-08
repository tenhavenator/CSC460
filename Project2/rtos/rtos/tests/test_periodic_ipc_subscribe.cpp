/*
 * Expected Results: The red error LED should be indicating error 6
 */

#include "../os.h"
#include <avr/io.h>

static SERVICE * service;

static void task_per_subscriber(void)
{
	int16_t value = 0;
	Service_Subscribe(service, &value);
}

/* To run this test, comment out the original function declaration and uncomment the r_main one. 
 * There can only be one r_main declared at any time.
 */
//int r_main(void)
int r_main_test_periodic_ipc_subscribe(void)
{    	
	service = Service_Init();
	Task_Create_Periodic(task_per_subscriber, 0, 5, 1, 10);
	
	Task_Terminate();
	
	return 0;
}
