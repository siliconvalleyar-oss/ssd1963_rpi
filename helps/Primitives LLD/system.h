
#ifndef __SYSTEM_H_
#define __SYSTEM_H_

#include <plib.h>
#include <GenericTypeDefs.h>

/*	Define the hardware platform to use	*/
//#define PIC32_STARTER_KIT			//use PIC32 STARTER Kit on MMEVK R1A
#define PIC32MX_EVK_RD4			//use PIC24/32 EVK RD4 with PIC32MX360F512L MCU option


// Specific to PIC32MX processor
#define	GetSystemClock()			(80000000ul)         	// 8 MHz/2 x 20 PLL -or- 12MHz/3 x 20 PLL
#define	GetPeripheralClock()		GetSystemClock()		//FPBDIV = DIV_1
#define	GetInstructionClock()		(GetSystemClock())

#endif
