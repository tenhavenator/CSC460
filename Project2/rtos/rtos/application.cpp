/**
* Skeleton application containing definitions needed for compiling the OS
*/
#include "os.h"

#include <avr/io.h>
#include <util/delay.h>

extern const unsigned int PT = 1;
extern const unsigned char PPP[] = {IDLE, 10};

extern int Task_Create(void (*f)(void), int arg, unsigned int level, unsigned int name);


SERVICE * serv;

void rr_task()
{
	_delay_ms(5000);
	Service_Publish(serv, 4);
}

int r_main(void)
{    	
	Task_Create(rr_task, 0, RR, 1);
	
    serv = Service_Init();
	
	int retval;
	Service_Subscribe(serv, &retval);
		
	DDRB = _BV(7);
	int i;
	for(i = 0; i < retval; i++)
	{
		PORTB = _BV(7);
		_delay_ms(1000);
		PORTB = 0;
		_delay_ms(1000);
	}
	
}
