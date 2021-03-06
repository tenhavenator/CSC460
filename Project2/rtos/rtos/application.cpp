/**
* Skeleton application containing definitions needed for compiling the OS
*/
#include "os.h"
#include <avr/io.h>
#include <util/delay.h>

extern int Task_Create(void (*f)(void), int arg, unsigned int level, unsigned int name);
extern int8_t   Task_Create_Periodic(void(*f)(void), int16_t arg, uint16_t period, uint16_t wcet, uint16_t start);
int i1;
extern uint16_t clock;

SERVICE * serv;

void rr_task()
{
	_delay_ms(5000);
	Service_Publish(serv, 4);
}

void periodic_task()
{
	for (;;) 
	{
		i1++;
		DDRB = _BV(7);
		if (i1 % 2 == 0) {
			PORTB = _BV(7);
		}
		else {
			PORTB = 0;
		}
		
		Task_Next();
	}
}

int r_main_app(void)
{    	
	i1 = 0;
	//Task_Create(rr_task, 0, RR, 1);
	
    //serv = Service_Init();
	//int retval;
	
	//Service_Subscribe(serv, &retval);
		
	//DDRB = _BV(7);
	/*int i;
	for(i = 0; i < retval; i++)
	{
		PORTB = _BV(7);
		_delay_ms(100);
		PORTB = 0;
		_delay_ms(100);
	}*/
	
	//_delay_ms(000);
	
	Task_Create_Periodic(periodic_task, 1, 1, 1, 0);
	/*for (;;) 
	{
		DDRB = _BV(7);
		if (clock % 2 == 0) {
			PORTB = _BV(7);
		}
		else {
			PORTB = 0;
		}
	}*/
	
	
	/*
	int i;
	for(i = 0; i < 5; i++)
	{
		PORTB = _BV(7);
		_delay_ms(100);
		PORTB = 0;
		_delay_ms(100);
	}
	*/
	
	return 0;
}
