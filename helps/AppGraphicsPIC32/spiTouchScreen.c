/* 
 * Driver Device :
 *                              ADS7846 or TSC2046
 */

//#include <GenericTypeDefs.h>
#include "spiTouchScreen.h"
#include <plib.h>
#include <p32xxxx.h>


/*Config Hardware SPI Master  */
void Init_MSPI(void){
TOUCH_SCREEN_SCK_TRIS=0;
TOUCH_SCREEN_SDI_TRIS=1;
TOUCH_SCREEN_SDO_TRIS=0;
TOUCH_SCREEN_CS_TRIS=0;
//look in your MCU h file to get the actual TRIS struct parameters
TS_CS=1;
TS_SCK=1;
}



UINT16 Get_Value_SSPI2(unsigned char ADDRESS){
unsigned char temp; //used to serialize data
UINT16 in=0x0000; //will hold received 16bit data, initialize with 0
UINT8 i,j;
int time=5;

TS_CS=0;
Delay10us(1);//__Delay10us(50);
//CS Line set to logic low
//begin serializing ADDRESS and sending it via TS_SDO from MSB to LSB

for(i=0;i<8;i++){
temp=ADDRESS&0x80; //Keep the MSB, ignore the rest of the bits
    if(temp){ // if MSB is 1 put 1 on TS_SDO and cycle clock
    TS_SDO = 1;
    }
    else {
    TS_SDO = 0;
    }
TS_SCK = 0;
Delay10us(time);
TS_SCK = 1;
Delay10us(time);
ADDRESS <<=1; // Shift data to the right to get the next bit in MSB position
}

//cycle clock again, prepare to receive
TS_SCK = 0;
Delay10us(time);
TS_SCK = 1;
Delay10us(time);

    //receiving the data
    for(i=0;i<16;i++){
    TS_SCK = 0; //activate clock
    if(TS_SDI) // if on TS_SDI line we have 1 we put 1 in LSB position of ?in? variable
    in|=0x0001;
    Delay10us(time);
    TS_SCK = 1;
    Delay10us(time);

    //if TS_SDI 0 leave variable unchanged, cycle clock
    in<<=1; // shift LSB to right and prepare to receive the next bit
    }

    //reception of last bit needs to be out of for loop so it wont get shifted to the right

    if(TS_SDI){
    in|=0x0001;
    }
TS_SCK = 0;
Delay10us(time);//__Delay10us(50);
TS_SCK = 1;
Delay10us(time);//__Delay10us(50);

//cycle clock and close communication by putting TS_CS line to logic high

TS_CS=1;
Delay10us(1);//__Delay10us(50);

return in;
}



/* touch panel interface define */


void WriteCharTo7843(unsigned char num)         
{
unsigned char count=0;
unsigned char temp=0;
TS_SCK=0;
    for(count=0;count<8;count++)
    {

temp=num&0x80; //Keep the MSB, ignore the rest of the bits
        if(temp){ // if MSB is 1 put 1 on TS_SDO and cycle clock
            TS_SDO = 1;
        }
        else {
            TS_SDO = 0;
        }
    TS_SCK=0;  
    Delay10us(5);
    TS_SCK=1;
    Delay10us(5);
                
    num<<=1;
    }
}

UINT16 ReadFromCharFrom7843(){    
    	UINT8 count=0;
	UINT16 Num=0;
	for(count=0;count<12;count++)
	{
		Num<<=1;		
		TS_SCK=1;
            Delay10us(5);
		TS_SCK=0; 
        Delay10us(5);
		if(TS_SDI)
		{
		Num|=1;
		}
		
	}

return(Num);
}

UINT16 ADS_Read_AD(unsigned char CMD)          
{
UINT16 i;
    TS_CS=0;
    WriteCharTo7843(CMD);
    TS_SCK=1; 
     Delay10us(5);
    TS_SCK=0;
     Delay10us(5);
    i=ReadFromCharFrom7843();
    TS_CS=1;
return i;
}	

UINT16 ADS_Read_XY(UINT8 xy){
UINT16 i, j;
UINT16 buf[READ_TIMES];
UINT16 sum=0;
UINT16 temp;
	for(i=0;i<READ_TIMES;i++)
	{				 
		buf[i]=ADS_Read_AD(xy);	    
	}				    
	for(i=0;i<READ_TIMES-1; i++)
	{
		for(j=i+1;j<READ_TIMES;j++)
		{
			if(buf[i]>buf[j])
			{
				temp=buf[i];
				buf[i]=buf[j];
				buf[j]=temp;
			}
		}
	}	  
sum=0;
for(i=LOST_VAL;i<READ_TIMES-LOST_VAL;i++)sum+=buf[i];
temp=sum/(READ_TIMES-2*LOST_VAL);
return temp;   
} 

#define ERR_RANGE 20 

UINT8 Read_ADS(UINT16 *x,UINT16 *y){
UINT16 xtemp,ytemp;			 	 		  
	xtemp=ADS_Read_XY(CMD_RDX);
	ytemp=ADS_Read_XY(CMD_RDY);	 									   
	if(xtemp<100||ytemp<100)return 0;
	*x=xtemp;
	*y=ytemp;
	return 1;
}


UINT16 Get_Value_SSPI(unsigned char ADDRESS){
unsigned char temp; //used to serialize data
UINT16 in=0x0000; //will hold received 16bit data, initialize with 0
UINT8 i,j;
int time=5;

TS_CS=0;
Delay10us(1);
//CS Line set to logic low
//begin serializing ADDRESS and sending it via TS_SDO from MSB to LSB

for(i=0;i<8;i++){
temp=ADDRESS&0x80; //Keep the MSB, ignore the rest of the bits
    if(temp){ // if MSB is 1 put 1 on TS_SDO and cycle clock
    TS_SDO = 1;
    }
    else {
    TS_SDO = 0;
    }
TS_SCK = 0;
Delay10us(time);
TS_SCK = 1;
Delay10us(time);
ADDRESS <<=1; // Shift data to the right to get the next bit in MSB position
}

//cycle clock again, prepare to receive
TS_SCK = 0;
Delay10us(time);
TS_SCK = 1;
Delay10us(time);

    //receiving the data
    for(i=0;i<15;i++){
    TS_SCK = 0; //activate clock
    Delay10us(time);
    // if on TS_SDI line we have 1 we put 1 in LSB position of ?in? variable
    in|=TS_SDI;
    TS_SCK = 1;
    Delay10us(time);

    //if TS_SDI 0 leave variable unchanged, cycle clock
    in<<=1; // shift LSB to right and prepare to receive the next bit
    }

    //reception of last bit needs to be out of for loop so it wont get shifted to the right


//cycle clock and close communication by putting TS_CS line to logic high

TS_CS=1;
Delay10us(1);
in >>= 3;
return (in&0x0FFF);
}


UINT16 tsc2046(unsigned char cmd)
{
unsigned char i;
UINT16 tmp16;
int time=5;

// TODO: make defines for these ports/bits...
//  TRISESET = 0x000C;   // Normally these are D2 & D3 LCD outputs; Set as inputs
// for use with the TSC: D2=BUSY; D3=SDI (serial data from TSC)
TSC_SCK_LO();        // Init clock line low
Delay10us(time);
TSC_CS_LO();        // Activate TSC (chip select)
Delay10us(time);


    // Send the command byte
    for(i = 0x80; i; i>>=1){   // Shift a bit-mask from left to right    
                if(i & cmd)  TSC_MOSI_HI();
        else         TSC_MOSI_LO();
        Delay10us(time);    
                // Set up the data line
        // Clock the data
        TSC_SCK_HI();
        Delay10us(time);
        TSC_SCK_LO();
    }

    Delay10us(time);
    // Clock / Read-back 16 bits. Only 12 bits are significant
    tmp16 = 0;
    for(i=0; i<16; i++){
        TSC_SCK_HI();
        Delay10us(time);
        tmp16 |= TS_SDI;
        TSC_SCK_LO();
        Delay10us(time);
        tmp16 <<= 1;
        //tmp16 |= TS_SDI;   // TODO define for port assignment
    }

    Delay10us(time);
    TSC_CS_HI();        // De-Activate TSC (chip select)

  //  TRISECLR = 0x000C;   // Set the LCDs data lines (D2 & D3) as outputs.

    // Shift the 12 significant bits (currently in the MS bits) down 4 bits.
    tmp16 >>= 3;

    return tmp16 & 0x0FFF;
}




unsigned char touchGetXY(UINT16 *x, UINT16 *y)
{
    UINT16 tmpX, tmpY;

    // If no touch  detect, return false.
    if(tsc2046(TSC_Z1) == 0)  return FALSE;

    tmpX = tsc2046(TSC_X);      // Read X position
    tmpY = tsc2046(TSC_Y);      // Read Y position

    // Check the touch still active now, and again after
    // some delay (to debounce the touch)
    if(tsc2046(TSC_Z1) == 0) return FALSE;
    DelayMs(20);
    if(tsc2046(TSC_Z1) == 0) return FALSE;
    
    // This seems to be a valid touch. Return true, and the x,y values.
    if(TSC_SWAP_XY)
    {
        *x = tmpY;
        *y = tmpX;
    }
    else
    {
        *x = tmpX;
        *y = tmpY;
    }
    return TRUE;
}


