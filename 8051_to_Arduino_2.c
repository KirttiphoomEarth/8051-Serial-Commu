//====================================== Serial TEST #1===========================================//
// #include<reg51.h>
// sbit LED1 = P2^1;
// //unsigned char *receiveStrPointer;
// //unsigned char receiveStr;
// //receiveStr = *receiveStrPointer;

// void UART_Init()
// {
//     SCON=0X50;
//     TMOD=0X20;
//     TH1=0Xfd;//253
//     TR1=1;
// }
// char serial_receive(void){
// 		while(RI==0);     // Wait till the data is received
//     RI=0;             // Clear Receive Interrupt Flag for next cycle
//     return(SBUF);     // return the received char
// }
// void serial_transmit(unsigned char ch) //TX funtion
// {
//     SBUF=ch;
//     while(TI==0);// it is done transmitting a character by setting the TI bit in SCON
//     TI=0;
// }
// void send(unsigned char *str) // loop transmit char
// {
//     while(*str)
//     {
//         serial_transmit(*str);
//         str++;
//     }
// }
// void delay(int x){
//     int y;
//     for(y=0;y<x;y++);
// }

// main()
// {
//     UART_Init();
// 		send("89s8253 Hello\n\r");
//     while(1)
// 		{
// 			send("Echo from 89s8253 : ");
// 			delay(500);
// 		}
// }
//=========================================================================================//


// while(RI==0);
// RI=0;
// ch=SBUF;           //Received data is stored into the a variable
//  SBUF=ch;           //Send the character in the variable a 
//  while(TI==0);
//  TI=0;


//====================================== Serial TEST #2 ISR ===========================================//
#include<reg51.h>

char uart_data;
char line = '\n';
void UART_Init()
{ 
		EA  = 1;		/* Enable global interrupt */
		ES = 1;  		/* Enable serial interrupt */	
    SCON = 0x50;  // Asynchronous mode, 8-bit data and 1-stop bit
    TMOD = 0x20;  //Timer1 in Mode2.
		TH1 = 0xfd;
	//TH1 = 256 - (11059200UL)/(long)(32*12*baudrate); // Load timer value for baudrate generation
		TR1 = 1;      //Turn ON the timer for Baud rate generation
}


int main()
{
		UART_Init();
    while(1);

}

void serial_ISR(void) interrupt 4
{
	if(RI == 1)
	{
		RI = 0;
		uart_data = SBUF;
		SBUF = uart_data;
		//SBUF = line;
	}
	else TI = 0;
}
//==========================================================================================================//

//====================================== Serial TEST #3 ISR Chack===========================================//
// #include<reg51.h>

// char uart_data;
// char line = '\n';
// bit RecievedFlag = 0;
// bit BusyFlag = 0;

// void UART_Init()
// { 
// 		EA  = 1;		/* Enable global interrupt */
// 		ES = 1;  		/* Enable serial interrupt */	
//     SCON = 0x50;  // Asynchronous mode, 8-bit data and 1-stop bit
//     TMOD = 0x20;  //Timer1 in Mode2.
// 		TH1 = 0xfd;
// 	//TH1 = 256 - (11059200UL)/(long)(32*12*baudrate); // Load timer value for baudrate generation
// 		TR1 = 1;      //Turn ON the timer for Baud rate generation
// }


// int main()
// {
// 		UART_Init();
//     while(1);

// }

// void serial_ISR(void) interrupt 4
// {
// 	if(RI == 1)
// 	{
// 		RI = 0;
// 		uart_data = SBUF;
// 		RecievedFlag = 1;
// 	}
// 	if (RecievedFlag & (!BusyFlag))
//   {
//        SBUF = uart_data;
//        BusyFlag = 1;
//   }
// 	if (TI)
//   {
//        TI = 0;
//        BusyFlag = 0;
// 				RecievedFlag = 0;
//   }
// }
//==========================================================================================================//
