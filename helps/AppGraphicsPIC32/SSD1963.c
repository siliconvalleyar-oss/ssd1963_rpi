

#include "./SSD1963.h"

WORD  _color;

static void SetArea(SHORT start_x, SHORT start_y, SHORT end_x, SHORT end_y);


//BYTE _gpioStatus = 0;


#if defined (PIC18F24JXXX)
	#define WriteCommand(cmd) {RS_LAT_BIT = 0; DAT = cmd; CS_LAT_BIT = 0; WR_LAT_BIT = 0; WR_LAT_BIT = 1; CS_LAT_BIT = 1;};
#elif defined (__PIC32MX__)
	#define WriteCommand(cmd) {RS_LAT_BIT = 0; PMDIN = cmd; CS_LAT_BIT = 0; WR_LAT_BIT = 0; WR_LAT_BIT = 1; CS_LAT_BIT = 1;};
#endif

#if defined (PIC18F24JXXX)

	#define WriteColor(color) { RS_LAT_BIT = 1; \
								DAT = (color>>8); WR_LAT_BIT = 0; WR_LAT_BIT = 1; \
								DAT = (color>>3); WR_LAT_BIT = 0; WR_LAT_BIT = 1; \
								DAT = (color<<3); WR_LAT_BIT = 0; WR_LAT_BIT = 1; \
								}

	#define WriteData(data) {RS_LAT_BIT = 1; DAT = data; WR_LAT_BIT = 0;  WR_LAT_BIT = 1;}
#elif defined (__PIC32MX__)
	#define WriteData(data) {RS_LAT_BIT = 1; PMDIN = data; WR_LAT_BIT = 0;  WR_LAT_BIT = 1;}
#endif


void SetArea(SHORT start_x, SHORT start_y, SHORT end_x, SHORT end_y)
{
	WriteCommand(CMD_SET_COLUMN);
	CS_LAT_BIT = 0;
	WriteData(start_x>>8);
	WriteData(start_x);
	WriteData(end_x>>8);
	WriteData(end_x);
	CS_LAT_BIT = 1;
	WriteCommand(CMD_SET_PAGE);
	CS_LAT_BIT = 0;
	WriteData(start_y>>8);
	WriteData(start_y);
	WriteData(end_y>>8);
	WriteData(end_y);
	CS_LAT_BIT = 1;
}


static void GPIO_WR(BYTE pin, BOOL state)
{
	BYTE _gpioStatus = 0;

	if(state==1)
		_gpioStatus = _gpioStatus|pin;
	else
		_gpioStatus = _gpioStatus&(~pin);

	WriteCommand(CMD_SET_GPIO_VAL);	// Set GPIO value
	CS_LAT_BIT = 0;
	WriteData(_gpioStatus);
	CS_LAT_BIT = 1;
}


void EnterSleepMode (void)
{
	WriteCommand(CMD_ENT_SLEEP);
}


void ExitSleepMode (void)
{
	WriteCommand(CMD_EXIT_SLEEP);
}


void DisplayOff(void)
{
	WriteCommand(CMD_BLANK_DISPLAY);
}


void DisplayOn(void)
{
	WriteCommand(CMD_ON_DISPLAY);
}


void EnterDeepSleep(void)
{
	WriteCommand(CMD_ENT_DEEP_SLEEP);
}


void SetBacklight(BYTE intensity)
{
	WriteCommand(CMD_SET_PWM_CONF);// Set PWM configuration for backlight control
	CS_LAT_BIT = 0;
	WriteData(0x0E);			// PWMF[7:0] = 2, PWM base freq = PLL/(256*(1+5))/256 = 
								// 300Hz for a PLL freq = 120MHz
	WriteData(intensity);		// Set duty cycle, from 0x00 (total pull-down) to 0xFF 
								// (99% pull-up , 255/256)
	WriteData(0x01);			// PWM enabled and controlled by host (mcu)
	WriteData(0x00);
	WriteData(0x00);
	WriteData(0x00);

	CS_LAT_BIT = 1;
}


void SetTearingCfg(BOOL state, BOOL mode)
{
	if(state == 1)
	{
		WriteCommand(CMD_SET_TEAR_ON);
		CS_LAT_BIT = 0;
		WriteData(mode&0x01);
		CS_LAT_BIT = 1;
	}
	else
	{
		WriteCommand(0x34);
	}

}


void ResetDevice(void)
{
	#if defined (PIC32_STARTER_KIT)
	//Set digital for AN/digital multiplex pins, only required if ADC pins are used for digital IO
	AD1PCFGbits.PCFG4 	= 1;//set TE_PORT_BIT digital
	AD1PCFGbits.PCFG15 	= 1;//set CS_LAT_BIT digital
	#endif


	RST_LAT_BIT  = 1;		
	RST_TRIS_BIT = 0;      	// enable RESET line

	RS_TRIS_BIT = 0;       	// enable RS line 

	CS_LAT_BIT  = 1;        	// SSD1963 is not selected by default
	CS_TRIS_BIT = 0;       	// enable SSD1963 CS line    

	RD_LAT_BIT  = 1;
	RD_TRIS_BIT = 0;

	WR_LAT_BIT  = 1;
	WR_TRIS_BIT = 0;

	TE_TRIS_BIT	= 1;

//#if defined (__PIC32MX__)
	// PMP setup 
	PMMODE = 0; PMAEN = 0; PMCON = 0;
	PMMODEbits.MODE   = 2;  // Intel 80 master interface
	// Before PLL is set and locked, the reference clock = crystal freq.
	// Set the largest data setup time for 10MHz clock
	PMMODEbits.WAITB  = 3;
	PMMODEbits.WAITM  = 15;
	PMMODEbits.WAITE  = 3;

	PMMODEbits.MODE16 = 1;  // 16 bit mode

	PMCONbits.PTRDEN = 0;  	// disable RD line
	PMCONbits.PTWREN = 0; 	// disable WR line
	PMCONbits.PMPEN  = 1;  	// enable PMP
/*#elif defined (__18CXX)
	DAT_TRIS = 0x00;		//set data port to output
#endif*/

	RST_LAT_BIT = 0;
	DelayMs(1);
	RST_LAT_BIT = 1;       // release from reset state to sleep state

	//Set MN(multipliers) of PLL, VCO = crystal freq * (N+1)
	//PLL freq = VCO/M with 250MHz < VCO < 800MHz
	//The max PLL freq is around 120MHz. To obtain 120MHz as the PLL freq

	WriteCommand(CMD_SET_PLL_MN);	// Set PLL with OSC = 10MHz (hardware), command is 0xE3
									// Multiplier N = 35, VCO (>250MHz)= OSC*(N+1), VCO = 360MHz
	CS_LAT_BIT = 0;
	WriteData(0x23);
	WriteData(0x02);				// Divider M = 2, PLL = 360/(M+1) = 120MHz
	WriteData(0x54);				// Validate M and N values
	CS_LAT_BIT = 1;	

	WriteCommand(CMD_PLL_START);	// Start PLL command
	CS_LAT_BIT = 0;
	WriteData(0x01);				// enable PLL
	CS_LAT_BIT = 1;	

	DelayMs(1);						// wait stablize
	WriteCommand(CMD_PLL_START);	// Start PLL command again
	CS_LAT_BIT = 0;
	WriteData(0x03);				// now, use PLL output as system clock	
	CS_LAT_BIT = 1;	
	//once PLL locked (at 120MHz), the data hold time set shortest
	#if defined (__PIC32MX__)
	PMMODEbits.WAITB  = 0;
	PMMODEbits.WAITM  = 0;
	PMMODEbits.WAITE  = 0;	
	#endif

	WriteCommand(CMD_SOFT_RESET);	// Soft reset
	DelayMs(10);
	/*
	*************************************************
	* 4.3" TFT panel model # TY430TFT480272
	*************************************************
	*/
//#if (DISPLAY_PANEL==TY430TFT480272)	
	//Set LSHIFT freq, i.e. the DCLK with PLL freq 120MHz set previously
	//Typical DCLK for TY430TFT480272 is 9MHz
	//9MHz = 120MHz*(LCDC_FPR+1)/2^20
	//LCDC_FPR = 78642 (0x13332)
	WriteCommand(CMD_SET_PCLK);		//set pixel clock (LSHIFT signal) frequency
	CS_LAT_BIT = 0;
	WriteData(0x01);
	WriteData(0x33);
	WriteData(0x32);
	CS_LAT_BIT = 1;
	
	//Set panel mode, varies from individual manufacturer
	WriteCommand(CMD_SET_PANEL_MODE);

//#if ((DISPLAY_PANEL==TY430TFT480272)||(DISPLAY_PANEL==LVC75Z779V1S)) || (DISPLAY_PANEL==TY700TFT800480_R3)
	CS_LAT_BIT = 0;
	WriteData(0x10);				// set 24-bit for TY430TF480272 4.3" panel, or LVC75Z779V1S 3.5" TFT Panel, data latch in rising edge for LSHIFT
	WriteData(0x00);				// set Hsync+Vsync mode
	WriteData((DISP_HOR_RESOLUTION-1)>>8); //Set panel size
	WriteData(DISP_HOR_RESOLUTION-1);
	WriteData((DISP_VER_RESOLUTION-1)>>8);
	WriteData(DISP_VER_RESOLUTION-1);
	WriteData(0x00);				//RGB sequence
	CS_LAT_BIT = 1;

	//Set horizontal period
	WriteCommand(CMD_SET_HOR_PERIOD);
	#define HT (DISP_HOR_RESOLUTION+DISP_HOR_PULSE_WIDTH+DISP_HOR_BACK_PORCH+DISP_HOR_FRONT_PORCH)
	CS_LAT_BIT = 0;
	WriteData((HT-1)>>8);	
	WriteData(HT-1);
	#define HPS (DISP_HOR_PULSE_WIDTH+DISP_HOR_BACK_PORCH)
	WriteData((HPS-1)>>8);
	WriteData(HPS-1);
	WriteData(DISP_HOR_PULSE_WIDTH-1);
	WriteData(0x00);
	WriteData(0x00);
	WriteData(0x00);
	CS_LAT_BIT = 1;
	//Set vertical period
	WriteCommand(CMD_SET_VER_PERIOD);
	#define VT (DISP_VER_PULSE_WIDTH+DISP_VER_BACK_PORCH+DISP_VER_FRONT_PORCH+DISP_VER_RESOLUTION)
	CS_LAT_BIT = 0;
	WriteData((VT-1)>>8);
	WriteData(VT-1);
	#define VSP (DISP_VER_PULSE_WIDTH+DISP_VER_BACK_PORCH)
	WriteData((VSP-1)>>8);
	WriteData(VSP-1);
	WriteData(DISP_VER_PULSE_WIDTH-1);
	WriteData(0x00);
	WriteData(0x00);
	CS_LAT_BIT = 1;
	
	//Set pixel format, i.e. the bpp
	WriteCommand(CMD_SET_PIXEL_FORMAT);
	CS_LAT_BIT = 0;
WriteData(0x55); 				// set 16bpp
        //WriteData(0x01100000); 	//18bit
//        WriteData(0b01110000); 	//24bit
	CS_LAT_BIT = 1;

	//Set pixel data interface
	WriteCommand(CMD_SET_DATA_INTERFACE);
	CS_LAT_BIT = 0;
#if (BUS_WIDTH == 8)
	WriteData(0x00);				// 8-bit data for low pin count MCU (e.g. PIC18F24J10)
#elif (BUS_WIDTH == 16)
	WriteData(0x03);				// 16-bit(565 format) data for 16bpp PIC32MX only
#endif
	CS_LAT_BIT = 1;



//#if ((DISPLAY_PANEL==TY700TFT800480_R3)||(DISPLAY_PANEL==TY500TFT800480)||(DISPLAY_PANEL==TY430TFT480272)||(DISPLAY_PANEL==LVC75Z779V1S))
	WriteCommand(CMD_SET_GPIO_CONF);// Set all GPIOs to output, controlled by host
	CS_LAT_BIT = 0;
	WriteData(0x0f);				// Set GPIO0 as output
	WriteData(0x01);				// GPIO[3:0] used as normal GPIOs
	CS_LAT_BIT = 1;
	// LL Reset to LCD!!!
	GPIO_WR(LCD_SPENA, 1);
	GPIO_WR(LCD_SPCLK, 1);
	GPIO_WR(LCD_SPDAT,1);
	GPIO_WR(LCD_RESET,1);
	GPIO_WR(LCD_RESET,0);
	DelayMs(1);
	GPIO_WR(LCD_RESET,1);
//#endif

	WriteCommand(CMD_ON_DISPLAY);	// Turn on display; show the image on display	
}

/*********************************************************************
* Function: void PutPixel(SHORT x, SHORT y)
*
* PreCondition: none
*
* Input: x,y - pixel coordinates
*
* Output: none
*
* Side Effects: none
*
* Overview: puts pixel
*
* Note: 
********************************************************************/
void PutPixel(SHORT x, SHORT y)
{
	SetArea(x,y,DISP_HOR_RESOLUTION-1,DISP_VER_RESOLUTION-1);
	WriteCommand(CMD_WR_MEMSTART);
	CS_LAT_BIT = 0;
			#if (BUS_WIDTH == 8)
				WriteColor(_color);
			#elif (BUS_WIDTH == 16)
				WriteData(_color);
			#endif
	CS_LAT_BIT = 1;
}


/*********************************************************************
* Function: void ClearDevice(void)
*
* PreCondition: none
*
* Input: none
*
* Output: none
*
* Side Effects: none
*
* Overview: clears screen with current color 
*
* Note: none
*
********************************************************************/
void ClearDevice(void)
{
	DWORD	counter;
	DWORD 	xcounter, ycounter;

	SetArea(0,0,DISP_HOR_RESOLUTION-1,DISP_VER_RESOLUTION-1);

	WriteCommand(CMD_WR_MEMSTART);

	CS_LAT_BIT = 0;
	for(ycounter=0;ycounter<DISP_VER_RESOLUTION;ycounter++)
	{
		for(xcounter=0;xcounter<DISP_HOR_RESOLUTION;xcounter++)
		{
			#if (BUS_WIDTH == 8)
				WriteColor(_color);
			#elif (BUS_WIDTH == 16)
				WriteData(_color);
			#endif
		}
	}

	CS_LAT_BIT = 1;
}



