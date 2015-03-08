#include "../os.h"
#include <avr/io.h>

#define TEST_PORT_DDR DDRB
#define TEST_PORT PORTB
#define TEST_PIN _BV(7)

SERVICE * service;
int16_t expected_value = 43;
int waiting_for_publish = 0;

void task_rr_subscriber(void)
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

void task_rr_publisher(void)
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
//int r_main_test_publish_system(void)
int r_main(void)
{    	
	TEST_PORT_DDR = TEST_PIN;
	TEST_PORT = 0;
	
	service = Service_Init();
	Task_Create_RR(task_rr_subscriber, 0);
	Task_Create_RR(task_rr_publisher, 0);

	Task_Terminate();
	
	return 0;
}
