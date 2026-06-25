/*
******************************************************************************************************
* Program description: 	This is a demo program to access SSD1963 in 16-bit 8080 addressing mode.
* 						No complex functions, no GUI, no touch panel, but the basic IO command for
*						addressing SSD1963 is provided here.
*						Although the Microchip Graphics Library has been employed intensively throughout our
*						list of examples, it is not restricted to use Microchip.
*						Any microcontroller or processor that is able to generate the required control
*						signal (CS#, DC, RD#, WR#, and D[23:0]) will be able to drive SSD1963.
*						User may refer to the original application note released by Solomon Systech at Doc 04
*						under this hyperlink to download the document.
*						http://www.techtoys.com.hk/Displays/SSD1963EvalRev3B/SSD1963%20Eval%20Board%20Rev3B.htm
*
*						I sincerely hope this program is able to serve as the starting point
*						for porting SSD1963 driver to your own platform.
*
* How to use this demo:	(1) Define the hardware platform from system.h file
*							comment one of the following sentances to define either PIC24/32 EVK RD4
*							or MMEVK R1A
*							#define PIC32_STARTER_KIT
*							//#define PIC32MX_EVK_RD4
*						(2) Define the TFT panel to use.
*							Under TFT.h, select only one of the TFT panels available from us.
*							#define DISPLAY_PANEL TY430TFT480272
*							//#define DISPLAY_PANEL TY500TFT800480
*							//#define DISPLAY_PANEL TY700TFT800480
*							//#define DISPLAY_PANEL YOUR_PANEL
*						(3) Finally, rebuild the project.
*
* FileName:				MainDemo.c
* Hardware:				(1) PIC24/32 EVK RD4 with PIC32MX360F512L MCU option -or-
*						Multimedia Evaluation platform for PIC32SK (MMEVK R1A)
*						(2) SSD1963EVK R3B
*						(3) 4.3", 5", or 7" TFT panels
* Compiler:        		Microchip C32 v1.12 or above, MPLAB v8.63
* Programmer:			John Leung
* Company:				www.TechToys.com.hk
* Date:					20th June 2011
******************************************************************************************************
*/

#include "SSD1963.h"
#include "system.h"
#include "TimeDelay.h" 

// Configuration Bit settings
// SYSCLK = 80 MHz (8MHz Crystal/ FPLLIDIV * FPLLMUL / FPLLODIV)
// PBCLK = 80 MHz
// Primary Osc w/PLL (XT+,HS+,EC+PLL)
// WDT OFF
// Other options are don't care
//
#if defined (PIC32_STARTER_KIT)
#pragma config FPLLMUL = MUL_20, FPLLIDIV = DIV_2	//because it is 8MHz builtin PIC32_STARTER_KIT
#elif defined (PIC32MX_EVK_RD4)
#pragma config FPLLMUL = MUL_20, FPLLIDIV = DIV_3	//because it is 12MHz builtin PIC24/32 EVK RD4
#endif
#pragma config FPLLODIV = DIV_1, FWDTEN = OFF
#pragma config POSCMOD = HS, FNOSC = PRIPLL


int main(void)
{
	BYTE x, y;

	#if defined (PIC32MX_EVK_RD4)
		DDPCONbits.JTAGEN = 0;	//disable the JTAG port
	#elif defined (PIC32_STARTER_KIT)
		DDPCONbits.JTAGEN = 1;	//enable the JTAG port
	#endif

	/*
	Configure the device for maximum performance.
	This macro sets flash wait states, PBCLK divider and DRM wait states
	based on the specified clock frequency. It also turns on the cache
	mode if avaialble.
	Based on the current frequency, the PBCLK divider will be set at 1:1.
	This knowledge is required to correctly set UART baud rate, timer
	reload value and other time sensitive setting.
	*/
	SYSTEMConfigPerformance(GetSystemClock());
	// Use PBCLK divider of 1:1 to calculate UART baud, timer tick etc.

	ResetDevice();

	SetBacklight(255);	//full brightness
	
	while(1)
	{
		// Show a few colors as an opening screen
		_color = WHITE;
		ClearDevice();
		DelayMs(1000);
	
		_color = BLUE;
		ClearDevice();
		DelayMs(1000);
	
		_color = BRIGHTYELLOW;
		for(x=100; x<200; x++)
		{
			for(y=50; y<120; y++) {PutPixel(x,y);}
		}
		DelayMs(1000);			
		
		_color = LIGHTRED;
		ClearDevice();
		DelayMs(1000);

		_color = BRIGHTBLUE;
		for(x=50; x<90; x++)
		{
			for(y=30; y<200; y++) {PutPixel(x,y);}
		}
		DelayMs(1000);	

		_color = BRIGHTGREEN;
		ClearDevice();
		DelayMs(1000);
	
		_color = BLACK;
		ClearDevice();
		DelayMs(1000);
	}

}
