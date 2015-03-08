
/*
 * Expected Results: The on board LED should turn on very soon after the program starts
 */

#include "../os.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#define TEST_PORT_DDR DDRB
#define TEST_PORT PORTB
#define TEST_PIN _BV(7)

static SERVICE * service;
static int16_t expected_value = 274;
extern volatile int publish_flag;


static void task_sys_subscriber(void)
{
	int16_t actual_value = 0;
	Service_Subscribe(service, &actual_value);
	
	if(actual_value == expected_value)
	{
		TEST_PORT = TEST_PIN;
	}
	
	Task_Terminate();
}

static void task_rr_flag_checker()
{
	TCCR3B |= (_BV(CS31));
	OCR3A = TCNT3 + 64000;
	TIMSK3 |= _BV(OCIE3A);
	
	for(;;)
	{
		if(publish_flag == 1)
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
//int r_main_test_ipc_INT_to_SYS(void)
{    	
	TEST_PORT_DDR = TEST_PIN;
	TEST_PORT = 0;
	
	service = Service_Init();
	
	Task_Create_System(task_sys_subscriber, 0);
	Task_Create_RR(task_rr_flag_checker, 0);
	
	Task_Terminate();
	
	return 0;
}
