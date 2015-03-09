/*
 * Expected Results: The on board LED should turn on very soon after the program starts
 */

#include "../os.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define TEST_PORT_DDR DDRB
#define TEST_PORT PORTB
#define TEST_PIN _BV(7)

static SERVICE * service;
static int16_t expected_value = 274;

static void task_rr_subscriber(void)
{	
	int16_t actual_value = 0;
	TCCR3B |= (_BV(CS31));
	OCR3A = TCNT3 + 500;
	TIMSK3 |= _BV(OCIE3A);
		
	Service_Subscribe(service, &actual_value);
	
	if(actual_value == expected_value)
	{
		TEST_PORT = TEST_PIN;
	}
	
	Task_Terminate();
}


/*ISR(TIMER3_COMPA_vect)
{			
	Service_Publish(service, expected_value);
	OCR3A = TCNT3 + 500;
}*/

/* To run this test, comment out the original function declaration and uncomment the r_main one. 
 * There can only be one r_main declared at any time.
 */
//int r_main(void)
int r_main_test_ipc_INT_to_RR(void)
{    	
	TEST_PORT_DDR = TEST_PIN;
	TEST_PORT = 0;

	service = Service_Init();
	Task_Create_RR(task_rr_subscriber, 0);
	
	Task_Terminate();
	
	return 0;
}



