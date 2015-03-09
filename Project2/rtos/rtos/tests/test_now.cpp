/*
	Expected results: 
	Look at the logic analyzer output
	- you should see digital pin 12 pulse every 25ms
	- should also see digital pin 11 pulse every time 12 does
	- if pin 11 pulses at the same time, that means Now() is calculating the 
	  time difference between two periods accurately to the millisecond
*/

#include "../os.h"
#include <avr/io.h>
#include <util/delay.h>

uint16_t now1 = 0;
uint16_t now2 = 0;
uint16_t period = 5;
uint16_t ms = period * 5;

static void task_periodic()
{
	for(;;) 
	{
		now1 = now2;
		now2 = Now();

		PORTB = _BV(6);
		PORTB = 0;

		uint16_t nowcompare = now2 - now1;
		if ((nowcompare == ms)) {
			PORTB = _BV(5);
			PORTB = 0;
		}
		Task_Next();
	}
}

/* To run this test, comment out the original function declaration and uncomment the r_main one. 
 * There can only be one r_main declared at any time.
 */
// int r_main(void)
int r_main_test_now(void)
{    	
	DDRB = _BV(7) | _BV(6) | _BV(5);
	
	Task_Create_Periodic(task_periodic, 0, period, 3, 5);
// 	for(;;) {
// 		uint8_t delay = 15;
// 		PORTB = 0;
// 		uint16_t now = Now();
// 		_delay_ms(delay);
// 		uint16_t now2 = Now();
// 		uint16_t now_diff = now2 - now;
// 		if ((now_diff - delay < 2)||(delay - now_diff < 2)) {
// 			PORTB = _BV(7);
// 		} 
// 		
// 	}
	return 0;
}
