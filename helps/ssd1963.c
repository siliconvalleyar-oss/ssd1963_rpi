//=============================================================================
// SSD1963 driver for STM32 microcontrollers
// (c) Radoslaw Kwiecien, radek@dxp.pl	
// http://en.radzio.dxp.pl/ssd1963/
//=============================================================================
#include "stm32f10x.h"
#include "ssd1963.h"
//=============================================================================
// Write command to SSD1963
//=============================================================================
void SSD1963_WriteCommand(unsigned int commandToWrite)
{
SSD1963_DATAPORT->ODR  = commandToWrite;
SSD1963_CTRLPORT->BSRR = SSD1963_PIN_RD;
SSD1963_CTRLPORT->BRR  = SSD1963_PIN_A0 | SSD1963_PIN_CS | SSD1963_PIN_WR;
asm("nop");
SSD1963_CTRLPORT->BSRR = SSD1963_PIN_A0 | SSD1963_PIN_CS | SSD1963_PIN_WR;
}
//=============================================================================
// Write data to SSD1963
//=============================================================================
void SSD1963_WriteData(unsigned int dataToWrite)
{
SSD1963_DATAPORT->ODR  = dataToWrite;
SSD1963_CTRLPORT->BSRR = SSD1963_PIN_RD | SSD1963_PIN_A0;
SSD1963_CTRLPORT->BRR  = SSD1963_PIN_CS | SSD1963_PIN_WR;
asm("nop");
SSD1963_CTRLPORT->BSRR = SSD1963_PIN_CS | SSD1963_PIN_WR;
}
//=============================================================================
// 
//=============================================================================
void SSD1963_Init (void)
{
volatile unsigned int dly;
SSD1963_CTRLPORT->BRR = SSD1963_PIN_RST;
for(dly = 0; dly < 1000; dly++);
SSD1963_CTRLPORT->BSRR = SSD1963_PIN_RST;
for(dly = 0; dly < 1000; dly++);

SSD1963_WriteCommand(SSD1963_SOFT_RESET);

SSD1963_WriteCommand(SSD1963_SET_PLL_MN);
SSD1963_WriteData(49);	// PLLclk = REFclk * 50 (500MHz)
SSD1963_WriteData(4);	// SYSclk = PLLclk / 5  (100MHz)
SSD1963_WriteData(4);  // dummy

SSD1963_WriteCommand(SSD1963_SET_PLL);
SSD1963_WriteData(0x01);
 
for(dly = 0; dly < 100000; dly++);

SSD1963_WriteCommand(SSD1963_SET_PLL);
SSD1963_WriteData(0x03);

SSD1963_WriteCommand(SSD1963_SET_LCD_MODE); 
SSD1963_WriteData(0x0C);			
SSD1963_WriteData(0x00);			
SSD1963_WriteData(mHIGH((TFT_WIDTH-1)));
SSD1963_WriteData(mLOW((TFT_WIDTH-1)));
SSD1963_WriteData(mHIGH((TFT_HEIGHT-1)));
SSD1963_WriteData(mLOW((TFT_HEIGHT-1)));
SSD1963_WriteData(0b000000);

SSD1963_WriteCommand(SSD1963_SET_PIXEL_DATA_INTERFACE);
SSD1963_WriteData(SSD1963_PDI_16BIT565);

SSD1963_WriteCommand(SSD1963_SET_LSHIFT_FREQ); 
SSD1963_WriteData((LCD_FPR >> 16) & 0xFF);
SSD1963_WriteData((LCD_FPR >> 8) & 0xFF);
SSD1963_WriteData(LCD_FPR & 0xFF);

SSD1963_WriteCommand(SSD1963_SET_HORI_PERIOD);
SSD1963_WriteData(mHIGH(TFT_HSYNC_PERIOD));
SSD1963_WriteData(mLOW(TFT_HSYNC_PERIOD));
SSD1963_WriteData(mHIGH((TFT_HSYNC_PULSE + TFT_HSYNC_BACK_PORCH)));
SSD1963_WriteData(mLOW((TFT_HSYNC_PULSE + TFT_HSYNC_BACK_PORCH)));
SSD1963_WriteData(TFT_HSYNC_PULSE);
SSD1963_WriteData(0x00);			
SSD1963_WriteData(0x00);
SSD1963_WriteData(0x00);			

SSD1963_WriteCommand(SSD1963_SET_VERT_PERIOD); 		
SSD1963_WriteData(mHIGH(TFT_VSYNC_PERIOD));
SSD1963_WriteData(mLOW(TFT_VSYNC_PERIOD));
SSD1963_WriteData(mHIGH((TFT_VSYNC_PULSE + TFT_VSYNC_BACK_PORCH)));
SSD1963_WriteData(mLOW((TFT_VSYNC_PULSE + TFT_VSYNC_BACK_PORCH)));
SSD1963_WriteData(TFT_VSYNC_PULSE);
SSD1963_WriteData(0x00);			
SSD1963_WriteData(0x00);

SSD1963_WriteCommand(SSD1963_SET_DISPLAY_ON);		//SET display on
}
//=============================================================================
//
//=============================================================================
void SSD1963_SetArea(unsigned int sx, unsigned int ex, unsigned int sy, unsigned int ey)
{
SSD1963_WriteCommand(SSD1963_SET_COLUMN_ADDRESS);	
SSD1963_WriteData((sx >> 8) & 0xFF);
SSD1963_WriteData((sx >> 0) & 0xFF);
SSD1963_WriteData((ex >> 8) & 0xFF);
SSD1963_WriteData((ex >> 0) & 0xFF);

SSD1963_WriteCommand(SSD1963_SET_PAGE_ADDRESS);	
SSD1963_WriteData((sy >> 8) & 0xFF);
SSD1963_WriteData((sy >> 0) & 0xFF);
SSD1963_WriteData((ey >> 8) & 0xFF);
SSD1963_WriteData((ey >> 0) & 0xFF);
}
//=============================================================================
// Fill area of specified color
//=============================================================================
void SSD1963_FillArea(unsigned int sx, unsigned int ex, unsigned int sy, unsigned int ey, int color)
{
int i;
SSD1963_SetArea(sx, ex, sy, ey);
SSD1963_WriteCommand(SSD1963_WRITE_MEMORY_START);
for(i = 0; i < ((ex-sx+1)*(ey-sy+1)); i++)
	{
	SendData(color);
	}
}
//=============================================================================
// Fills whole screen specified color
//=============================================================================
void SSD1963_ClearScreen(unsigned long color)
{
unsigned int x,y;
SSD1963_SetArea(0, TFT_WIDTH-1 , 0, TFT_HEIGHT-1);
SSD1963_WriteCommand(0x2c);
for(x=0;x<TFT_WIDTH;x++)
	{
        for(y= 0;y<TFT_HEIGHT;y++)
                {
                SSD1963_WriteData(color);
                }
	}
}
//=============================================================================
//
//=============================================================================
void GLCD_SetPixel(int x, int y, int color)
{
SSD1963_SetArea(x, x, y, y);
SSD1963_WriteCommand(0x2c);
SSD1963_WriteData(color);
SSD1963_WriteCommand(0x0);
}
//=============================================================================
//
//=============================================================================
