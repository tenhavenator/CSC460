/*
 * Project3.cpp
 *
 * Roomba control
 */ 


#include "OS/os.h"
#include "roomba/roomba.h"
#include "roomba/roomba_sci.h"

#include <avr/io.h>

int r_main(void)
{
	Roomba_Init();
	
	uint16_t start = Now();
	
	Roomba_Drive(50, 0x8000);
	
	while(Now() - start < 5000)
	{
	}

	Roomba_Drive(500, 0x8000);
	
	start = Now();
	while((Now() - start < 5000)
	{
		
	}
	
	Roomba_Drive(0, 0x8000);
	
	Task_Terminate();
	return 0;
}