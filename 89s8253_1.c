#include "reg8253.h"

#define UpPEPressed					1
#define	DnPEPressed					2
#define UpONPressed					4
#define	DnONPressed					8
#define TimePressed					16
#define NoPressed					0

#define FoodTypeChangePressed		(UpPEPressed + UpONPressed)									//2 + 4
#define MotorCountPressed 			(DnPEPressed + DnONPressed + UpPEPressed)					//1 + 3 + 2
#define K1ChangePressed				(UpPEPressed + UpONPressed + DnPEPressed)					//2 + 4 + 1
#define K2ChangePressed				(UpPEPressed + UpONPressed + DnONPressed)					//2 + 4 + 3
#define ResetPressed				(DnPEPressed + UpPEPressed)									//1 + 2				//reset system and Year, Month and Date to 1, 1, 1
#define ClockUpPressed				(DnONPressed + UpONPressed)									//3 + 4
#define ClockOpenUpPressed			(DnPEPressed + UpONPressed)									//1 + 4
#define ClockCloseUpPressed			(UpPEPressed + DnONPressed)									//2 + 3
#define ClockStopUpPressed			(DnPEPressed + DnONPressed)									//1 + 3				// add clock to stop feeding
#define PeriodIncPressed			(UpPEPressed)												//2
#define PeriodDecPressed			(DnPEPressed)												//1
#define FoodIncPressed				(UpONPressed)												//4
#define FoodDecPressed				(DnONPressed)												//3

#define ScalePercentPressed			(DnPEPressed + DnONPressed + UpONPressed)					//1 + 3 + 4

//#define InitSetupRTCPressed     	(DnPEPressed + UpPEPressed + DnONPressed + UpONPressed)		//1 + 2 + 3 + 4			//reset system and Year, Month and Date to 1, 1, 1 with Hour, Min and Sec to 0, 0, 0
#define MotorFactorPressed     		(DnPEPressed + UpPEPressed + DnONPressed + UpONPressed)		//1 + 2 + 3 + 4			//reset system and Year, Month and Date to 1, 1, 1 with Hour, Min and Sec to 0, 0, 0

//#define DebugPressed				(DnPEPressed + DnONPressed)									//1 + 3

//define for old code compatible only, not use in this version
#define ClockDnPressed				(TimePressed + DnPEPressed)									//5 + 1
#define ClockOpenDnPressed			(TimePressed + UpPEPressed)									//5 + 2
#define ClockCloseDnPressed			(TimePressed + DnONPressed)									//5 + 3
#define BuzzerChangePressed			(TimePressed + UpONPressed)									//5 + 4
#define TimeMonitorPressed			(TimePressed + DnPEPressed + UpPEPressed)	

unsigned int	PeriodSet, PeriodShow, Period;		//keep Period setting time cycle, format HH:MM -> mins 23:59 = 1,439 mins
unsigned long	FoodSet, FoodShow, Food;			//keep amount of food setting, format xx.xx Kg --> 0 - 99990 gram(s)
unsigned int	PercentSet, PercentShow;
//, Percent;	//keep percentage of provide food in time off period
unsigned char	MotorSet, MotorShow;				//keep no. of motor in system, format 1 - 16
unsigned int	K1Set, K1Show;						//keep K1 parameter of Rough food, format x.00 g(s) --> 0 - 999 [/100 gram(s)]
unsigned int	K2Set, K2Show;						//keep K2 parameter of Fine  food, format x.00 g(s) --> 0 - 999 [/100 gram(s)]
unsigned int	           ClockOpenSet,  ClockCloseSet, ClockStopSet;		//keep current clock setting, Clock to open system and off, format same as PeriodSet
unsigned int 	ClockShow, ClockOpenShow, ClockCloseShow, ClockStopShow;
unsigned int	Clock;
idata unsigned long   ClockInSec, ClockInSecOld;
idata unsigned long   LastTimeReset;						//keep last time system has been reset in format of Days + Hours + Mins + Secs in sec format
idata unsigned long	PassTime;							// clock - open time
idata unsigned long	FoodLeftTempLong;	
//idata unsigned int	TempSec;						// 2022 edit NOT USE
idata unsigned char	BuzzerStyle;						//type of sound of buzzer
idata unsigned char	TimeType, OldTimeType;							//keep time day or night or Stop

//for next round setting
idata unsigned int	PeriodSetNext,K1SetNext, K2SetNext;	//for next round
idata unsigned long FoodSetNext; 
idata unsigned int  PercentSetNext;										//for next round
idata unsigned char	MotorSetNext;										//for next round

//working value
//unsigned int 	Period, Food;						//keep track of period and food left,  defined above
idata unsigned int	WorkingCount1mS;				//keep count 1 mS for dec period and food
idata unsigned int	Counter100mS;					//keep count 100 mS for update food show on screen
idata unsigned int	SecCounter;						//keep count 1 sec has pass for food show on screen
idata unsigned char	WorkingState;					//keep state of PE ON end or not
//idata unsigned int	NotWorkingCount1mS;				//keep count 1 mS for not work, just for update clock
bdata				FreshStart;		
idata unsigned char	SWStatus, OldSWStatus;				//keep status of switch
idata unsigned int	SWCounter;							//keep counting time of sw has been press
idata unsigned int	SWCounterSpeed;						//keep counting time of sw has been press for speed change
idata unsigned char	LastStatus;
unsigned char	MotorFactor;

//=========== Dot blinking
//idata unsigned int	DotCounter;							//keep track for blink dot every sec, On 500 mS, Off 500 mS	 	// 2022 edit NOT use

//food
idata unsigned long	FoodTemp, FoodDelivered;
//==========================================================================================================
#define IE 0x90 
#define Enable						1
#define Disable						0
#define	ScanTimeL					0x10				//1mS  0xFA00, but for error correction from reload value, count less 10H
#define	ScanTimeH					0xFA

#define StartFlag 						0x1A			//	Start
#define StopFlag 							0xB3			//	Stop
#define AckFlagFromMCU 				0x55   	// R_Ack
#define NAckFlagFromMCU 			0xAA  	// R_NAck
#define AckFlagFromWIFI 			0x55  	// S_Ack
#define NAckFlagFromWIFI			0xAA 	// S_RAck

//#define IE 0x90
unsigned char temp; //debug

bdata unsigned char Status_Flag;					//keep status of system
sbit 				BuzzerOpenSet 		= Status_Flag ^ 0;	// set buzzer on off from user
sbit				FoodTypeSet			= Status_Flag ^ 1;	// set type of food, Rough or fine
sbit				FoodTypeSetNext		= Status_Flag ^ 2;	// set type of food, Rough or fine, Next round

bdata unsigned char Working_Flag;
sbit 				BuzzerOpenShow		= Working_Flag ^ 0;	// setting buzzer on off
sbit				FoodTypeShow		= Working_Flag ^ 1;	// showing type of food, Rough or fine
sbit				ResetFlag			= Working_Flag ^ 2;	// set when press reset system, force to restart
sbit				SystemRunFlag		= Working_Flag ^ 3;	// set when system run, disable when pressing reset kb
sbit				DotOnFlag			= Working_Flag ^ 4;	// set when it has to on blinking dot
sbit				OwnerScreenFlag		= Working_Flag ^ 5; // set when start program and show owner screen
sbit				RTCReadFlag			= Working_Flag ^ 6;	// set when read data RTC every bytes are ok, not FF
sbit				NewRoundFlag		= Working_Flag ^ 7; // set when period is end, load next round

bdata unsigned char Operation_Flag;
sbit 				ClockDirtyFlag		= Operation_Flag ^ 0;


#define TimeoutReceiveLimit 3040
idata unsigned int CountDataByte;
idata unsigned char commandStatus;
idata unsigned int TimeoutTimer1ms;
idata unsigned char dataReceive;
idata unsigned char CheckSum_8_Bits;
idata unsigned char Command, DataByte0, DataByte1, DataByte2, DataByte3, DataByte4, DataByte5, DataByte6, DataByte7;
idata unsigned char StoreByteCommand, StoreByteData0, StoreByteData1, StoreByteData2, StoreByteData3;
idata unsigned char StoreByteChecksum8;
idata unsigned char DataTransmit;

bdata unsigned char WorkingSerial_Flag;
sbit ReceiveDataCompleteFlag  = WorkingSerial_Flag ^ 0;
sbit TimeOutRecCountFlag      = WorkingSerial_Flag ^ 1;
sbit SendNakTimeOutFlag       = WorkingSerial_Flag ^ 2;
sbit ReceiveDataNoTimeOutFlag = WorkingSerial_Flag ^ 3;
sbit TI_Flag  								= WorkingSerial_Flag ^ 4;
sbit ReceiveFlag 							= WorkingSerial_Flag ^ 5;
sbit ReadyTransmitFlag 				= WorkingSerial_Flag ^ 6;

idata unsigned int SignalModeSel;  	//Modified Sine wave
sbit ModifiedSineWaveFlag     = WorkingSerial_Flag ^ 6;	

void Init(void);
void InitTimer(void);

void UART_Init(void);
void resetTempDataVar(void);
void transmit_Data(unsigned char ch);
void MappingCommandAndData(unsigned char Command);
	
void Init(void)
{
		ReceiveDataCompleteFlag = Disable;
    CheckSum_8_Bits         = 0x00;
    TimeoutTimer1ms         = 0;
		CountDataByte           = 0;
		SignalModeSel						= 0;
		DataByte0 = 0;
		DataByte1 = 0;
		DataByte2 = 0;
		DataByte3 = 0;
		DataByte4 = 0;
		DataByte5 = 0;
		DataByte6 = 0;
		DataByte7 = 0;

		TimeOutRecCountFlag 			= Disable;
		SendNakTimeOutFlag				= Disable;
		ReceiveDataNoTimeOutFlag 	= Disable;

		ModifiedSineWaveFlag = 0;	
		TI_Flag = Disable;
		ReceiveFlag = Disable;
		ReadyTransmitFlag = Disable;
		temp = 0;
}

void InitTimer(void)
{
		TMOD &= 0xF0;         	// Setup timer/counter0 mode register 
		TMOD |= 0x01;         	// Set M0 for 16-bit  timer 
		TL0 = ScanTimeL;    	// Set value for timer 0 
		TH0 = ScanTimeH;
		TR0	= 1;			  	//start timer0
		ET0 = 1;
	
		EA = 1;
		PT0 = 1;				//Timer 0 is highest priority

		// Serial edit INT0 interrupt
		IT0=1;                 						// Set Falling Edge Trigger for INT0
		EX0 = 1; 
}

void UART_Init(void)
{
	//IE = 0x90; //Enabling Serial Interrupt
    //EA = 1;      /* Enable global interrupt */
    //ES = 1;      /* Enable serial interrupt */
		//TMOD 	= 0x0F;
	//PCON = 0x80;
  TMOD 	|= 0x20; // Timer1 in Mode2.
	SCON 	= 0x50; // Asynchronous mode, 8-bit data and 1-stop bit  
  TH1 	= 0xFD;
		//TL1 = 0xfd;
		//TH1 = 0xFB;
    // TH1 = 256 - (11059200UL)/(long)(32*12*baudrate); // Load timer value for baudrate generation
	ES 	= 1;      /* Enable serial interrupt */
	TR1 = 1; // Turn ON the timer for Baud rate generation
}

void ISR_T0 (void) interrupt 1 using 2 
{
		TL0 = ScanTimeL;    	// Set value for timer 0 
		TH0 = ScanTimeH;
		TimeoutTimer1ms++;

}

void ISR_Serial(void) interrupt 4 using 3
{
		if(RI == 1)
		{
			RI = 0;
			dataReceive = SBUF;
			//transmit_Data(dataReceive);
			CountDataByte++;
			ReceiveFlag = Enable;
			//RI = 0;
		}
		if(TI == 1)
		{
			SBUF = 'F';
			TI = 0;
		//	ReadyTransmitFlag = Enable;
			//if(ReadyTransmitFlag == 1)
			//{
				//DataTransmit = '1';
			//	ReadyTransmitFlag = 0;
		//	}
			//TI_Flag = 1;
		}
}


	
void transmit_Data(unsigned char ch)
{
		//ReadyTransmitFlag = 1;
		//DataTransmit = ch;
		//T1 = 1;;
		//ReadyTransmitFlag = Disable;
		//if(ReadyTransmitFlag == Enable)
		//{
		//SBUF = ch;
		//	ReadyTransmitFlag = Disable;
		//}
		//while(TI_Flag == 1); // it is done transmitting a character by setting the TI bit in SCON
    //TI_Flag = 0;
		//while (TI == 0); // 1 When fish transmit 
		//TI = 0;
}


void main(void)
{

	Init();
	InitTimer();
	UART_Init();
	/transmit_Data('1');
	/transmit_Data('2');
	/transmit_Data('3');
	/transmit_Data('4');
	//transmit_Data('5');
	while(1)
	{
		//transmit_Data('1');
		//transmit_Data('2');
		//transmit_Data(0x41);
		if(ReceiveFlag == Enable)
		{
				switch (CountDataByte)
        {
        case 1:
                DataByte0 = dataReceive;
                break;
        case 2:
                DataByte1 = dataReceive;
                break;
        case 3:
                DataByte2 = dataReceive;
                break;
        case 4:
                DataByte3 = dataReceive;
                break;
        case 5:
                DataByte4 = dataReceive;
                break;
        case 6:
                DataByte5 = dataReceive;
                break;
        case 7:
                DataByte6 = dataReceive;
                break;
        case 8:
                DataByte7 = dataReceive;
								CountDataByte = 0;
                ReceiveDataCompleteFlag = Enable;
                break;
        }
				ReceiveFlag = 0;
			}
		
			if(DataByte0 == StartFlag && DataByte7 == StopFlag && ReceiveDataCompleteFlag == Enable)
      { 
				CheckSum_8_Bits += DataByte2;
				CheckSum_8_Bits += DataByte3;
				CheckSum_8_Bits += DataByte4;
				CheckSum_8_Bits += DataByte5;
				CheckSum_8_Bits += DataByte6;
				if(DataByte1 == AckFlagFromWIFI)
				{
					//CheckCommand and Save To EEPROM
					MappingCommandAndData(StoreByteCommand);
					//sendAckFromMCU
					transmit_Data(AckFlagFromMCU);
				}
				else if(DataByte1 == NAckFlagFromWIFI)
				{
					resetTempDataVar();
				}
				else
				{
					if(CheckSum_8_Bits == 0) //No error if checksum is 0
					{
						//SendAckFromMCU
						transmit_Data(AckFlagFromMCU);
						//SendData : DataByte0 - DataByte7
						transmit_Data(DataByte0);
						transmit_Data(DataByte1);
						transmit_Data(DataByte2);
						transmit_Data(DataByte3);
						transmit_Data(DataByte4);
						transmit_Data(DataByte5);
						transmit_Data(DataByte6);
						transmit_Data(DataByte7);
						// Ack send
						//Store Data to TempVar that cant be re-writing in 1-cycle
						StoreByteCommand    = DataByte1;
						StoreByteData0      = DataByte2;
						StoreByteData1      = DataByte3;
						StoreByteData2      = DataByte4;
						StoreByteData3      = DataByte5;
						StoreByteChecksum8  = DataByte6;
					}
					else //error if checksum is not 0
					{
						//sendNack
						transmit_Data(NAckFlagFromMCU);
						resetTempDataVar();
					}
				
				}
							
					ReceiveDataCompleteFlag = Disable; 
     }
			if(CountDataByte != 0 && TimeOutRecCountFlag == Disable)
			{
				TimeoutTimer1ms = 0;
				TimeOutRecCountFlag = Enable;
			}
			if(TimeoutTimer1ms > 3040 && TimeOutRecCountFlag == Enable)
			{
					transmit_Data('0');
					CountDataByte = 0;
					TimeOutRecCountFlag = Disable;
					resetTempDataVar();
			}

				
	}
}
void MappingCommandAndData(unsigned char Command)
{
    //unsigned char   DataTypeChar = 0;
    unsigned int    DataTypeInt = 0;
    //unsigned long   DataTypeLong = 0;
    switch (Command)
    {
    case    0x01: LastStatus = FoodTypeChangePressed;
            FoodTypeShow = StoreByteData3;
            break;
    case 0x02: LastStatus = K1ChangePressed;
            DataTypeInt += StoreByteData2 << 8;
            DataTypeInt += StoreByteData3;
            K1Show = DataTypeInt;
            break;
    case 0x03: LastStatus = K2ChangePressed;
            DataTypeInt += StoreByteData2 << 8;
            DataTypeInt += StoreByteData3;
            K2Show = DataTypeInt;
            break;
    case 0x04: LastStatus = MotorCountPressed;
            MotorShow = StoreByteData3;
            break;
    case 0x05: LastStatus = MotorFactorPressed;
            MotorFactor = StoreByteData3; 
            break;
    case 0x06: LastStatus = PeriodIncPressed; // PeriodDecPressed is a same
            DataTypeInt   += StoreByteData2 << 8;
            DataTypeInt   += StoreByteData3;
            PeriodShow = DataTypeInt;
            break;
    case 0x07: LastStatus = FoodIncPressed; // FoodDecPressed is a same
            DataTypeInt     += StoreByteData2 << 8;
            DataTypeInt     += StoreByteData3;
            FoodShow = DataTypeInt;
            break;
    case 0x08: LastStatus = ClockUpPressed; // What
            DataTypeInt += StoreByteData2 << 8;
            DataTypeInt += StoreByteData3;
            ClockShow = DataTypeInt;
            break;
    case 0x09: LastStatus = ClockOpenUpPressed;
            DataTypeInt    += StoreByteData2 << 8;
            DataTypeInt    += StoreByteData3;
            ClockOpenShow = DataTypeInt;
            break;
    case 0x0A: LastStatus = ClockCloseUpPressed;
            DataTypeInt   += StoreByteData2 << 8;
            DataTypeInt   += StoreByteData3;
            ClockCloseShow = DataTypeInt;
            break;
    case 0x0B: LastStatus = ClockStopUpPressed;
            DataTypeInt   += StoreByteData2 << 8;
            DataTypeInt   += StoreByteData3;
            ClockStopShow = DataTypeInt;
            break;
    case 0x0C: LastStatus = ScalePercentPressed;
            DataTypeInt   += StoreByteData2 << 8;
            DataTypeInt   += StoreByteData3;
            PercentShow = DataTypeInt;
            break;
    case 0x0D: LastStatus = ResetPressed;
            // Ask recheck
            break;
    }
}

void resetTempDataVar(void)
{
    DataByte0           = 0;
    DataByte1           = 0;
    DataByte2           = 0;
    DataByte3           = 0;
    DataByte4           = 0;
    DataByte5           = 0;
    DataByte6           = 0;
    DataByte7           = 0;
    CountDataByte       = 0;
    StoreByteCommand    = 0;
    StoreByteData0      = 0;
    StoreByteData1      = 0;
    StoreByteData2      = 0;
    StoreByteData3      = 0;
    StoreByteChecksum8  = 0;
}
