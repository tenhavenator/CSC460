/*
 * Expected Results: The on board LED should turn on very soon after the program starts
 */

#include "../os.h"
#include <avr/io.h>
#include <util/delay.h>

#define TEST_PORT_DDR DDRB
#define TEST_PORT PORTB
#define TEST_PIN _BV(7)

static SERVICE * service;
static int16_t expected_value = -23;
static int waiting_for_publish = 0;

static void task_sys_subscriber(void)
{
	int16_t actual_value = 0;
	waiting_for_publish = 1;
	Service_Subscribe(service, &actual_value);
	
	if(actual_value == expected_value)
	{
		TEST_PORT = TEST_PIN;
	}
	
	Task_Terminate();
}

static void task_per_publisher(void)
{
	for(;;)
	{
		if(waiting_for_publish)
		{
			Service_Publish(service, expected_value);
			Task_Terminate();
		}
	}
}

/* To run this test, comment out the original function declaration and uncomment the r_main one. 
 * There can only be one r_main declared at any time.
 */
int r_main(void)
//int r_main_test_ipc_PER_to_SYS(void)
{    	
	TEST_PORT_DDR = TEST_PIN;
	TEST_PORT = 0;
	
	service = Service_Init();
	Task_Create_System(task_sys_subscriber, 0);
	Task_Create_Periodic(task_per_publisher, 0, 5, 1, 10);
	
	Task_Terminate();
	
	return 0;
}
