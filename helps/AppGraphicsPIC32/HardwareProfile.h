/*********************************************************************
 *
 *	Hardware specific definitions
 *
 *********************************************************************
 * FileName:        HardwareProfile.h
 * Dependencies:    None
 * Processor:       PIC24, dsPIC, PIC32
 * Compiler:        Microchip C30 and C32
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * Copyright (c) 2011 Microchip Technology Inc.  All rights 
 * reserved.
 *
 * Microchip licenses to you the right to use, modify, copy, and 
 * distribute: 
 * (i)  the Software when embedded on a Microchip microcontroller or 
 *      digital signal controller product ("Device") which is 
 *      integrated into Licensee�s product; or
 * (ii) ONLY the Software driver source files ENC28J60.c and 
 *      ENC28J60.h ported to a non-Microchip device used in 
 *      conjunction with a Microchip ethernet controller for the 
 *      sole purpose of interfacing with the ethernet controller. 
 *
 * You should refer to the license agreement accompanying this 
 * Software for additional information regarding your rights and 
 * obligations.
 *
 * THE SOFTWARE AND DOCUMENTATION ARE PROVIDED �AS IS� WITHOUT 
 * WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT 
 * LIMITATION, ANY WARRANTY OF MERCHANTABILITY, FITNESS FOR A 
 * PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL 
 * MICROCHIP BE LIABLE FOR ANY INCIDENTAL, SPECIAL, INDIRECT OR 
 * CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF 
 * PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY OR SERVICES, ANY CLAIMS 
 * BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE 
 * THEREOF), ANY CLAIMS FOR INDEMNITY OR CONTRIBUTION, OR OTHER 
 * SIMILAR COSTS, WHETHER ASSERTED ON THE BASIS OF CONTRACT, TORT 
 * (INCLUDING NEGLIGENCE), BREACH OF WARRANTY, OR OTHERWISE.
 *
 *
 * Date	                Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 10/03/06             Original, copied from Compiler.h
 * 06/25/09             dsPIC & PIC24H support 
 * 09/15/09             Added PIC24FJ256DA210 Development Board Support
 * 06/02/11             Added MPLAB X Support
 ********************************************************************/

#ifndef HARDWAREPROFILE_H
#define	HARDWAREPROFILE_H
#include <plib.h>
#include <p32xxxx.h>

#define HardwareConfigButtonInit()		{TRISAbits.TRISA7=1;TRISDbits.TRISD6=1;TRISDbits.TRISD7=1;TRISDbits.TRISD13=1;}

#define GetHWButtonProgram1()        PORTAbits.RA7

#define GetHWButtonProgram2()        PORTDbits.RD6

#define GetHWButtonProgram3()        PORTDbits.RD7

#define GetHWButtonProgram4()        PORTDbits.RD13

//#define HardwareLedsOuputs() {TRISAbits.TRISA0=0;TRISAbits.TRISA1=0;TRISAbits.TRISA2=0;TRISAbits.TRISA3=0;TRISAbits.TRISA4=0;TRISAbits.TRISA5=0;TRISAbits.TRISA6=0;TRISAbits.TRISA7=0;}

#define HardwareLedsOuputs() {TRISAbits.TRISA0=0;TRISAbits.TRISA1=0;TRISAbits.TRISA2=0;TRISAbits.TRISA3=0;}

/*Config  pin backligth */

#define BACKLIGTH PORTDbits.RD0 

#define configPinBackligth() {TRISDbits.TRISD0=0;}


/*Config Hardware  chip select memory 24LC256*/

#define initHardware() {TRISDbits.TRISD1=0;LATDbits.LATD1=1;}

/*Config Led  */
#define Led0        PORTAbits.RA0
#define Led1        PORTAbits.RA1
#define Led2        PORTAbits.RA2
#define Led3        PORTAbits.RA3
//#define Led4        PORTAbits.RA4
//#define Led5        PORTAbits.RA5
//#define Led6        PORTAbits.RA6
//#define Led7        PORTAbits.RA7

#define ledRA0 PORTAbits.RA0
#define ledRA1 PORTAbits.RA1
#define ledRA2 PORTAbits.RA2
#define ledRA3 PORTAbits.RA3
//#define ledRA6 PORTAbits.RA6
//#define ledRA7 PORTAbits.RA7


#define  HIGH 1
#define LOW 0 
#ifdef	__cplusplus
extern "C" {
#endif

#ifdef	__cplusplus
}
#endif

#endif	/* HARDWAREPROFILE_H */

