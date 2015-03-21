/*
 * Project3.cpp
 *
 * Roomba control
 */ 


#include "OS/os.h"
#include <avr/io.h>

int r_main(void)
{
	DDRB = _BV(7);
	
    while(1)
    {
		uint16_t start = Now();
		PORTB = _BV(7);
		
		while(Now() - start < 1000)
		{
		
		}
		
		PORTB = 0;
		
		while(Now() - start < 2000)
		{
			
		}
    }
	
	return 0;
}