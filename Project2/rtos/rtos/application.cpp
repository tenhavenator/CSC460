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
	    
		for(;;){
			PORTB = 0;
			_delay_ms(1000);
			PORTB = _BV(7);
			_delay_ms(1000);
		}
		
		return 0;
}
