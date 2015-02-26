/**
* Skeleton application containing definitions needed for compiling the OS
*/
#include "os.h"
#include <avr/io.h>
#include <util/delay.h>

extern const unsigned int PT = 1;
extern const unsigned char PPP[] = {IDLE, 10};

int r_main(void)
{    
	DDRB =_BV(7);
	
	uint16_t time = Now();
	
	for(;;)
	{	
		if(Now() - time > 10000)
		{
			PORTB = _BV(7);
			if(Now() - time > 20000)
			{
				time = Now();
			}
			
		}
		else {
			
			PORTB = 0;
		}				
	}

	return 0;
}
