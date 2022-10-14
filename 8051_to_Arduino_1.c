#include<reg51.h>

#define FOSC 25000000

//========// New Setup By EARTH //================//
//SCON = 0x40; 	//Serail Mode 0x40 NO stop start 0x50 start stop
//TH1 = 0xf4;		// Baud Rate set
//TMOD = 0x20; 	//Time 1 Mode 2 
//TR1 = 1;			//Time 1 start
//================================================//

//======// Prototype //======//
void serial_init(void);
void serail_initialize();
char serial_receive();
void serial_transmit(unsigned char);
void delay(int);
//==========================//

//======// Global Variable //======//
unsigned rec;
sbit LED1 = P2^1;
//================================//


void main(){
		serial_init();
    while(1){
			
        serial_transmit('A');
        delay(1000);
				LED1 = 1;
				delay(1000);
				LED1 = 0;
				delay(1000);
    }
}

void serial_init(void)
{
unsigned long int divisor = 256 - ((FOSC/384)/2400); // Baud Rate Calculation 56 - (FOSC/(384*Baud Rate)
PCON = 0x80;  
SCON = 0x50; 	//Serail Mode 0x40 NO stop start 0x50 start stop
TH1 = 0xf3;		// Baud Rate set 2400
TMOD = 0x20; 	//Time 1 Mode 2 
TR1 = 1;			//Time 1 start
}

char serial_receive(void){
    while(RI == 0);
    rec = SBUF;
    RI = 0;
    return rec;
}

void serial_transmit(unsigned char datat){
    SBUF = datat;
    while(TI==0);
    TI=0;
}

void delay(int x){
    int i;
    for(i=0;i<x;i++);
}
