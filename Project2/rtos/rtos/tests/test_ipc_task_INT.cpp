
/*
 * Expected Results: The on board LED should turn on very soon after the program starts
 */

#include "../os.h"
#include <avr/io.h>
#include <avr/interrupt.h>

static SERVICE * service;

static void task_sys_subscriber(void)
{
	int16_t actual_value = 0;
	
	Service_Subscribe(service, &actual_value);
	
	PORTB = _BV(6);
	PORTB = 0;
	
	Task_Terminate();
}


static void task_rr_interrupt(void)
{
	TCCR3B |= (_BV(CS31));
	OCR3A = TCNT3 + 500;
	TIMSK3 |= _BV(OCIE3A);

	for(;;)
	{}
}

ISR(TIMER3_COMPA_vect)
{			
	Service_Publish(service, 275);
	OCR3A = TCNT3 + 500;
	
	PORTB = _BV(5);
	PORTB = 0;
}

/* To run this test, comment out the original function declaration and uncomment the r_main one. 
 * There can only be one r_main declared at any time.
 */
int r_main(void)
//int r_main_test_ipc_task_INT(void)
{    
	DDRB = 	_BV(6) | _BV(5);
	PORTB = 0;
	
	service = Service_Init();
	
	Task_Create_System(task_sys_subscriber, 0);
	Task_Create_RR(task_rr_interrupt, 0);
	
	Task_Terminate();
	
	return 0;
}

