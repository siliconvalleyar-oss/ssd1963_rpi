/*
 *  Microchip Graphics
 * Development: Leo A.
 * Author:  Leo A.
 * Device : PIC32MX360F512
 * touchscreen & wifi
 * Projects :Domotics House
 *Driver  Device Touch Screen :     TSC 2046 or ADS7846
 * Driver Graphics :    SSD1963
 */

#include "./SSD1963.h"
#include "./system.h"
#include "./TimeDelay.h" 
#include <math.h>	
#include "./domotics13_24bitsC32.h"
#include "HardwareProfile.h"
#include <plib.h>



#pragma config FPLLMUL = MUL_20, FPLLIDIV = DIV_2	
#pragma config FPLLODIV = DIV_1, FWDTEN = OFF
#pragma config POSCMOD = HS, FNOSC = PRIPLL



#define  CMD_X 0xD0
#define CMD_Y 0x90


unsigned char *bmp;
unsigned int rand1(int lim);
void DrawLine(int x1, int y1, int x2, int y2, int _color);
void putBMP();
void lawn(void);
void UARTInit(void);
void configINT1(void);

//void __ISR(_EXTERNAL_1_VECTOR,ipl4AUTO)  ExtInt1_ISR(void)   ;

int flag=0;

//////////////////////////////////////// MAIN ///////////////////////////////////////////////////////////
#if defined (__18CXX)
	void main (void)
#elif defined (__PIC32MX__)
int main(void)
#endif
{
unsigned int x1, y1, x2, y2, counter;
unsigned int x,y,r,i,g,b,z,count;
unsigned int valueX,valueY;
UINT16 Sensor_Value;//Sensor_Value = Get_Value_SSPI(ADDRESS);
UINT16 aux_TS_Y=0,aux_TS_X=0;
 UINT16   touchScreenX,touchScreenY;
UINT16 TS_Y=0,TS_X=0,TS_Z1=0,TS_Z2=0;
int div=10;
UINT16 pressure=0;
unsigned char value[20];

#if defined (PIC32MX_EVK_RD4)
DDPCONbits.JTAGEN = 0;	//disable the JTAG port
#elif defined (PIC32_STARTER_KIT)
DDPCONbits.JTAGEN = 1;	
//enable the JTAG port
#endif


configPinBackligth();
HardwareLedsOuputs();

BACKLIGTH=1;
ledRA0=ledRA1=ledRA2=ledRA3=0;

//#if defined (__PIC32MX__)
SYSTEMConfigPerformance(GetSystemClock());
//#endif
// Use PBCLK divider of 1:1 to calculate UART baud, timer tick etc.
UARTInit();
ResetDevice();
//HardwareConfigButtonInit();


//SetBacklight(255);
//_color=BLACK;
ClearDevice();
initHardware();
bmp=&domotics;
putBMP();
Init_MSPI();
configINT1();
INTConfigureSystem(INT_SYSTEM_CONFIG_MULT_VECTOR);
INTEnableInterrupts();

UARTPutString("Project graphics PIC32 Domotics\r\n");
UARTPutString("Connection Success!!!\r\n");


while(1){   

      if(flag==1) {
          
         //for (count=0;count<div;count++)
         {
            TS_X=Get_Value_SSPI(CMD_X);
            TS_Y=Get_Value_SSPI(CMD_Y);           
            TS_Z1=Get_Value_SSPI(CMD_Z1);           
            TS_Z2=Get_Value_SSPI(CMD_Z2);           
          // TS_X=   tsc2046(CMD_X); 
           //TS_Z1=   tsc2046(CMD_Z1); 
           //TS_Y=   tsc2046(CMD_Y);
           //TS_Z2=   tsc2046(CMD_Z2); 
        //   touchGetXY(TS_X,TS_Y);
          }
          //TS_X=TS_X/div;
          //TS_Y=TS_Y/div;
          //TS_Z1=TS_Z1/div;
        //  TS_Z2=TS_Z2/div;
   /*
                    pressure = 280 * TS_X;   // MMc: Was 40. This is x-plate resistance; Whats ours???
          pressure /= 4096;
          pressure *= TS_Z2 - TS_Z1;
          pressure /= TS_Z1;
*/
        Led1=HIGH;
        touchScreenX=((TS_X/10)*1.26);
       touchScreenY=((TS_Y/10)*0.76);

           
        if((aux_TS_X!=TS_X||aux_TS_Y!=TS_Y)&TS_Z1!=0){
            UARTPutString("\r\nTSC2046_X:  ");
            sprintf(value,"%u",touchScreenX);
            UARTPutString(value);
            
            UARTPutString("\r\nTSC2046_Y:  ");
            sprintf(value,"%u",touchScreenY);
            UARTPutString(value);    
            //UARTPutString("\r\n");  
            
            sprintf(value,"%u",TS_Z1);            
            UARTPutString("\r\n\t\t TSC2046_Z1:  ");
            UARTPutString(value);     

            sprintf(value,"%u",TS_Z2);            
            UARTPutString("\r\n\t\t TSC2046_Z2:  ");
            UARTPutString(value);    
           // UARTPutString("\r\n");  
        /*
                        sprintf(value,"%u",pressure);            
            UARTPutString("\r\n\t\t TSC2046_Pressure :  ");
     */
            UARTPutString(value);    
            UARTPutString("\r\n");  
            
            aux_TS_X=TS_X;
            aux_TS_Y=TS_Y;
            }              
        Led1=LOW;
        flag=0;
        }   
    }
return 1;
}


void putBMP(){
SHORT valueX,valueY;
int i,y,x;

valueX=(bmp[5]<<8)|bmp[4];
valueY=(bmp[3]<<8)|bmp[2];
i=6;
    for(y=0;y<valueY;y++)
    {
        for(x=0;x<valueX;x++)
        {
            _color= (bmp[i+1]<<8)|bmp[i];
            PutPixel(x,y);
           i=i+2;
        } 
    }
}

void configINT1(void){
 //   TRISEbits.TRISE8=1;
    IEC0bits.INT1IE = 0; // disable INT0 interrupt
    INTCONbits.INT1EP = 1; // edge polarity ( 1 rising, 0 falling edge )
    IPC1bits.INT1IP= 7; // interrupt priority
    IPC1bits.INT1IS = 0; // interrupt sub priority
    IFS0bits.INT1IF= 0; // clear INT0 flag
    IEC0bits.INT1IE = 1; // enable INT0 interrupt
}

 

void __ISR(_EXTERNAL_1_VECTOR,IPL7AUTO)  ExtInt1_ISR(void){
    flag=1;
    Led3=1;
    Led3=0;
    IFS0bits.INT1IF=0; //clear interrupt flag
}



