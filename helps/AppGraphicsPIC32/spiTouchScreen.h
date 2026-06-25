/* 
 * File:   spiTouchScreen.h
 * Author: leo
 *
 * Created on June 17, 2017, 2:23 PM
 */


#include "HardwareProfile.h"
#include "system.h"
#include "TimeDelay.h"


#ifndef SPITOUCHSCREEN_H
#define	SPITOUCHSCREEN_H

#ifdef	__cplusplus
extern "C" {
#endif


#define TOUCH_SCREEN_CS_TRIS       TRISBbits.TRISB9
#define TOUCH_SCREEN_CS_LAT        LATBbits.LATB9
#define TOUCH_SCREEN_CS        PORTBbits.RB9    

    
#define TOUCH_SCREEN_SCK_TRIS      TRISGbits.TRISG6
#define TOUCH_SCREEN_SDO_TRIS      TRISGbits.TRISG8
#define TOUCH_SCREEN_SDI_TRIS      TRISGbits.TRISG7

#define TOUCH_SCREEN_SCK      PORTGbits.RG6
#define TOUCH_SCREEN_SDO      PORTGbits.RG8
#define TOUCH_SCREEN_SDI      PORTGbits.RG7    


#define TS_CS  TOUCH_SCREEN_CS    
#define TS_SCK      TOUCH_SCREEN_SCK
#define TS_SDO      TOUCH_SCREEN_SDO
#define TS_SDI      TOUCH_SCREEN_SDI
    
#define  CMD_RDX  0xD0	
#define  CMD_RDY  0x90
#define CMD_Z1 0xB0    
#define CMD_Z2 0xC0
    
    
#define READ_TIMES 15
#define LOST_VAL 5

#define INT1_TRIS    TRISEbits.TRISE8
#define INT1_RE8    PORTEbits.RE8
#define LAT_INT1_RE8    LATEbits.LATE8
#define initInterrupt1()     {INT1_TRIS=1;}
#define INT_PEN_IRQ_TSC2046 INT1_RE8
//#define INT_PEN_IRQ_TSC2046 LAT_INT1_RE8
#define TS_IRQ INT_PEN_IRQ_TSC2046

#define TSC_SCK_LO()     TS_SCK=0
#define TSC_SCK_HI()     TS_SCK=1

#define TSC_CS_LO() TS_CS=0
#define TSC_CS_HI() TS_CS=1
    
#define TSC_MOSI_HI() TS_SDO=1
#define TSC_MOSI_LO() TS_SDO=0

 #define TSC_TEMP0  (0x83 | 0x00 | 0x00)
#define TSC_Y      (0x83 | 0x10 | 0x04)
#define TSC_VBAT   (0x83 | 0x20 | 0x00)
#define TSC_Z1     (0x83 | 0x30 | 0x04)
#define TSC_Z2     (0x83 | 0x40 | 0x04)
#define TSC_X      (0x83 | 0x50 | 0x04)
#define TSC_AUX    (0x83 | 0x60 | 0x00)
#define TSC_TEMP1  (0x83 | 0x70 | 0x00)

#define TSC_SWAP_XY 1    
    
    
UINT16 Get_Value_SSPI(unsigned char ADDRESS);
void Init_MSPI(void);


#ifdef	__cplusplus
}
#endif

#endif	/* SPITOUCHSCREEN_H */

