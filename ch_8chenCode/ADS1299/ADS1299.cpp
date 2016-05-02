//
//  ADS1299.cpp   ARDUINO LIBRARY FOR COMMUNICATING WITH ADS1299
//  
//  Created by Conor Russomanno, Luke Travis, and Joel Murphy. Summer, 2013
//


#include "pins_arduino.h"
#include "ADS1299.h"

void ADS1299::initialize(int _DRDY, int _RST, int _CS, int _FREQ, boolean _isDaisy)
{
	Serial.println(F("start ADS1299 initialize"));
	//初始化ADS1299::initialize(8,9,10,4);
	isDaisy = _isDaisy;
    DRDY = _DRDY;  //DRDY和CS已在.h文件定义
    CS = _CS;

	int FREQ = _FREQ;
	int RST = _RST;
	
	/////////////////reset,p29,p35;////////////////////////////////////////////////////////////////////////////////
		delay(50);				// recommended power up sequence requiers Tpor (~32mS)	
		pinMode(RST,OUTPUT);
	//	pinMode(RST,LOW);       //错误？
	    digitalWrite(RST,LOW);  //更正
		delayMicroseconds(4);	// toggle reset pin
	//	pinMode(RST,HIGH);      //错误？
	    digitalWrite(RST,HIGH); //更正
		delayMicroseconds(20);	// recommended to wait 18 Tclk before using device (~8uS);
	
	

    // **** ----- SPI Setup ----- **** //
    
    // Set direction register for SCK and MOSI pin.
    // MISO pin automatically overrides to INPUT.
    // When the SS pin is set as OUTPUT, it can be used as
    // a general purpose output port (it doesn't influence
    // SPI operations).
 
////初始化SPI总线////////   
    pinMode(SCK, OUTPUT);
    pinMode(MOSI, OUTPUT);
    pinMode(SS, OUTPUT);   
	
	digitalWrite(SCK, LOW);
    digitalWrite(MOSI, LOW);
    digitalWrite(SS, HIGH);
    
    // set as master and enable SPI 使能SPI并设置为主机
    SPCR |= _BV(MSTR); //设置Arduino为主机
    SPCR |= _BV(SPE);  //使能SPI

    //set bit order
    SPCR &= ~(_BV(DORD)); //MSB在前，SPI data format is MSB (pg. 25)
	// set data mode
    SPCR = (SPCR & ~SPI_MODE_MASK) | SPI_DATA_MODE;  //clock polarity = 0; clock phase = 1 (pg. 8)时钟极性0，时钟相位1

	
    // set clock divider，FREQ选择时钟频率，1MHZ,4MHZ,8MHZ
	switch (FREQ)  //选择的时钟频率，FREQ赋值为4，默认为四分频
	{
		case 8:
			DIVIDER = SPI_CLOCK_DIV_2;
			break;
		case 4:
			DIVIDER = SPI_CLOCK_DIV_4;
			break;
		case 1:
			DIVIDER = SPI_CLOCK_DIV_16;
			break;
		default:
			break;
	}
	
    SPCR = (SPCR & ~SPI_CLOCK_MASK) | (DIVIDER);  // set SCK frequency  
    SPSR = (SPSR & ~SPI_2XCLOCK_MASK) | (DIVIDER); // by dividing 16MHz system clock
    

    // **** ----- End of SPI Setup ----- **** //
 
 

 
    // initalize the  data ready chip select and reset pins:    初始化数据准备位、片选和复位引脚
    pinMode(DRDY, INPUT);
    pinMode(CS, OUTPUT);
	
	digitalWrite(CS,HIGH); 	
	digitalWrite(RST,HIGH);
	Serial.println(F("ADS1299 initialize finished"));
	Serial.println();
}











//System Commands           系统命令：开中断，传递各个系统命令的对应值，（延时）关中断（延时）
void ADS1299::WAKEUP() 
{
    digitalWrite(CS, LOW); 
    transfer(_WAKEUP);
    digitalWrite(CS, HIGH); 
    delayMicroseconds(3);  		//must wait 4 tCLK cycles before sending another command (Datasheet, pg. 35)
}

void ADS1299::STANDBY()			// only allowed to send WAKEUP after sending STANDBY
{		
    digitalWrite(CS, LOW);
    transfer(_STANDBY);
    digitalWrite(CS, HIGH);
}

void ADS1299::RESET() 			// reset all the registers to default settings
{			
    digitalWrite(CS, LOW);
    transfer(_RESET);
    delayMicroseconds(12);   	//must wait 18 tCLK cycles to execute this command (Datasheet, pg. 35)
    digitalWrite(CS, HIGH);
}

void ADS1299::START() 			//start data conversion 				
{			
    digitalWrite(CS, LOW);
    transfer(_START);
    digitalWrite(CS, HIGH);	
}

void ADS1299::STOP()            //stop data conversion
{			                
    digitalWrite(CS, LOW);
    transfer(_STOP);
    digitalWrite(CS, HIGH);
}


//Data Read Commands
void ADS1299::RDATAC()
{
    digitalWrite(CS, LOW);
    transfer(_RDATAC);
    digitalWrite(CS, HIGH);
	delayMicroseconds(3);   
}

void ADS1299::SDATAC() 
{
    digitalWrite(CS, LOW);
    transfer(_SDATAC);
    digitalWrite(CS, HIGH);
	delayMicroseconds(3);   //must wait 4 tCLK cycles after executing this command (Datasheet, pg. 37)
}

	
//read data
void ADS1299::RDATA() {				//  use in Stop Read Continuous mode when DRDY goes low
	byte inByte;
	stat_1 = 0;							//  clear the status registers
	stat_2 = 0;	
	int nchan = 8;	//assume 8 channel.  If needed, it automatically changes to 16 automatically in a later block.
	digitalWrite(CS, LOW);				//  open SPI
	transfer(_RDATA);
	
	// READ CHANNEL DATA FROM FIRST ADS IN DAISY LINE
	for(int i=0; i<3; i++){			//  read 3 byte status register (1100+LOFF_STATP+LOFF_STATN+GPIO[7:4])
		inByte = transfer(0x00);
		stat_1 = (stat_1<<8) | inByte;				
	}
	
	for(int i = 0; i<8; i++){
		for(int j=0; j<3; j++){		//  read 24 bits of channel data from 1st ADS in 8 3 byte chunks
			inByte = transfer(0x00);
			channelData[i] = (channelData[i]<<8) | inByte;
		}
	}
	
	if (isDaisy) {
		nchan = 16;
		
		// READ CHANNEL DATA FROM SECOND ADS IN DAISY LINE
		for(int i=0; i<3; i++){			//  read 3 byte status register (1100+LOFF_STATP+LOFF_STATN+GPIO[7:4])
			inByte = transfer(0x00);
			stat_2 = (stat_1<<8) | inByte;				
		}
		
		for(int i = 8; i<16; i++){
			for(int j=0; j<3; j++){		//  read 24 bits of channel data from 2nd ADS in 8 3 byte chunks
				inByte = transfer(0x00);
				channelData[i] = (channelData[i]<<8) | inByte;
			}
		}
	}
	
	for(int i=0; i<nchan; i++){			// convert 3 byte 2's compliment to 4 byte 2's compliment	
		if(bitRead(channelData[i],23) == 1){	
			channelData[i] |= 0xFF000000;
		}else{
			channelData[i] &= 0x00FFFFFF;
		}
	}   
}


/*  //8通道时的RDATA()
void ADS1299::RDATA() {					//  use in Stop Read Continuous mode when DRDY goes low
	byte inByte;						//  to read in one sample of the channels
    digitalWrite(CS, LOW);				//  open SPI
    transfer(_RDATA);					//  send the RDATA command
	stat = transfer(0x00);				//  read status register (1100 + LOFF_STATP + LOFF_STATN + GPIO[7:4])
	for(int i = 0; i<8; i++){
		for(int j=0; j<3; j++){		//  read in the status register and new channel data
			inByte = transfer(0x00);
			channelData[i] = (channelData[i]<<8) | inByte;
		}
	}
	digitalWrite(CS, HIGH);				//  close SPI
	
	for(int i=0; i<8; i++){
		if(bitRead(channelData[i],23) == 1){	// convert 3 byte 2's compliment to 4 byte 2's compliment
			channelData[i] |= 0xFF000000;
		}else{
			channelData[i] &= 0x00FFFFFF;
		}
	}
    
}
*/


// Register Read/Write Commands    寄存器读写命令
byte ADS1299::getDeviceID()               //
{			                              // simple hello world com check
	byte data = RREG(0x00);
	
	if(verbose)                           //verbose表示详细信息，verbose=FALSE，意思就是设置运行的时候不显示详细信息。
	{						              // verbose output
		Serial.print(F("Device ID "));   
		printHex(data);	                 //以十六进制格式打印出数据
	}
	return data;
}

byte ADS1299::RREG(byte _address)
{		                                //  reads ONE register at _address
    byte opcode1 = _address + 0x20; 	//  RREG expects 001rrrrr where rrrrr = _address
	
    digitalWrite(CS, LOW); 				//  open SPI
    transfer(opcode1); 					//  opcode1
    transfer(0x00); 					//  opcode2
    regData[_address] = transfer(0x00); //  update mirror location with returned byte
    digitalWrite(CS, HIGH); 			//  close SPI	
	
	
	if (verbose)
	{						            //  verbose output
		printRegisterName(_address);
		printHex(_address);
		Serial.print(", ");
		printHex(regData[_address]);
		Serial.print(", ");
		
		
		for(byte j = 0; j<8; j++)
		{
			Serial.print(bitRead(regData[_address], 7-j));
			if(j!=7) Serial.print(", ");
		}
		
		Serial.println();
	}
	return regData[_address];			// return requested register value
}

// Read more than one register starting at _address
void ADS1299::RREGS(byte _address, byte _numRegistersMinusOne) 
{
//	for(byte i = 0; i < 0x17; i++){
//		regData[i] = 0;					//  reset the regData array
//	}
    byte opcode1 = _address + 0x20; 	//  RREG expects 001rrrrr where rrrrr = _address
    digitalWrite(CS, LOW); 				//  open SPI
    transfer(opcode1); 					//  opcode1
    transfer(_numRegistersMinusOne);	//  opcode2
 
    for(int i = 0; i <= _numRegistersMinusOne; i++)
	{
        regData[_address + i] = transfer(0x00); 	//  add register byte to mirror array
	}
    digitalWrite(CS, HIGH); 			//  close SPI

//    Serial.print("start to print register value ");
//    Serial.println();

	if(verbose)
	{						//  verbose output
		for(int i = 0; i<= _numRegistersMinusOne; i++)  //打印出所有寄存器状态
		{
			printRegisterName(_address + i);  //打印寄存器名称
			printHex(_address + i);           //打印寄存器地址
			Serial.print(", ");
			printHex(regData[_address + i]);  //直接打印寄存器的值
			Serial.print(", ");

			for(int j = 0; j<8; j++)     //打印寄存器各个位的值
			{
				Serial.print(bitRead(regData[_address + i], 7-j));
				if(j!=7) Serial.print(", ");
			}
			Serial.println();
		}
    }
    
}

void ADS1299::WREG(byte _address, byte _value)   //  Write ONE register at _address
{	
    byte opcode1 = _address + 0x40; 	//  WREG expects 010rrrrr where rrrrr = _address
    digitalWrite(CS, LOW); 				//  open SPI
    transfer(opcode1);					//  Send WREG command & address
    transfer(0x00);						//	Send number of registers to read -1
    transfer(_value);					//  Write the value to the register 
    digitalWrite(CS, HIGH); 			//  close SPI
	regData[_address] = _value;			//  update the mirror array
	if(verbose){						//  verbose output
		Serial.print(F("Register "));
		printHex(_address);
		Serial.println(F(" modified."));
	}
}

void ADS1299::WREGS(byte _address, byte _numRegistersMinusOne) {
    byte opcode1 = _address + 0x40;		//  WREG expects 010rrrrr where rrrrr = _address
    digitalWrite(CS, LOW); 				//  open SPI
    transfer(opcode1);					//  Send WREG command & address
    transfer(_numRegistersMinusOne);	//	Send number of registers to read -1	
	for (int i=_address; i <=(_address + _numRegistersMinusOne); i++){
		transfer(regData[i]);			//  Write to the registers
	}	
	digitalWrite(CS,HIGH);				//  close SPI
	if(verbose){
		Serial.print(F("Registers "));
		printHex(_address); Serial.print(F(" to "));
		printHex(_address + _numRegistersMinusOne);
		Serial.println(F(" modified"));
	}
}


void ADS1299::updateChannelData(){
	byte inByte;
	int nchan=8;  //assume 8 channel.  If needed, it automatically changes to 16 automatically in a later block.
	digitalWrite(CS, LOW);				//  open SPI
	
	// READ CHANNEL DATA FROM FIRST ADS IN DAISY LINE
	for(int i=0; i<3; i++){			//  read 3 byte status register from ADS 1 (1100+LOFF_STATP+LOFF_STATN+GPIO[7:4])
		inByte = transfer(0x00);
		stat_1 = (stat_1<<8) | inByte;				
	}
	
	for(int i = 0; i<8; i++){
		for(int j=0; j<3; j++){		//  read 24 bits of channel data from 1st ADS in 8 3 byte chunks
			inByte = transfer(0x00);
			channelData[i] = (channelData[i]<<8) | inByte;
		}
	}
	
	if (isDaisy) {
		nchan = 16;
		// READ CHANNEL DATA FROM SECOND ADS IN DAISY LINE
		for(int i=0; i<3; i++){			//  read 3 byte status register from ADS 2 (1100+LOFF_STATP+LOFF_STATN+GPIO[7:4])
			inByte = transfer(0x00);
			stat_2 = (stat_1<<8) | inByte;				
		}
		
		for(int i = 8; i<16; i++){
			for(int j=0; j<3; j++){		//  read 24 bits of channel data from 2nd ADS in 8 3 byte chunks
				inByte = transfer(0x00);
				channelData[i] = (channelData[i]<<8) | inByte;
			}
		}
	}
	
	digitalWrite(CS, HIGH);				//  close SPI
	
	//reformat the numbers
	for(int i=0; i<nchan; i++){			// convert 3 byte 2's compliment to 4 byte 2's compliment	
		if(bitRead(channelData[i],23) == 1){	
			channelData[i] |= 0xFF000000;
		}else{
			channelData[i] &= 0x00FFFFFF;
		}
	}
}


/* //8通道时的updateChannelData
void ADS1299::updateChannelData()       
{
	byte inByte;                        //定义一个叫inByte的变量，它将要存储从XX寄存器中读到数值
	digitalWrite(CS, LOW);				//  open SPI
	stat = transfer(0x00);				//  read status register (1100 + LOFF_STATP + LOFF_STATN + GPIO[7:4])
	stat = transfer(0x00);				//  read status register (1100 + LOFF_STATP + LOFF_STATN + GPIO[7:4])
	stat = transfer(0x00);				//  read status register (1100 + LOFF_STATP + LOFF_STATN + GPIO[7:4])
 
    // 
	for(int i = 0; i<8; i++)
	{
		for(int j=0; j<3; j++)
		{		                       //  read 24 bits of channel data in 8 3 byte chunks
			inByte = transfer(0x00);   //什么也不送出，但是得到寄存器的返回值
			channelData[i] = (channelData[i]<<8) | inByte;
		}
	}
	digitalWrite(CS, HIGH);				//  close SPI

	for(int i=0; i<8; i++)               // convert 3 byte 2's compliment to 4 byte 2's compliment
	{				
		if(bitRead(channelData[i],23) == 1)
		{	
			channelData[i] |= 0xFF000000;
		}
		else
		{
			channelData[i] 
				&= 0x00FFFFFF;
		}
	}
	Serial.println("updateChannelData finished");
}
*/



//	if(verbose){
//		Serial.print(stat); Serial.print(", ");
//		for(int i=0; i<8; i++){
//			Serial.print(channelData[i]);
//			if(i<7){Serial.print(", ");}
//		}
//		Serial.println();
//	}



// String-Byte converters for RREG and WREG     字符串转换器
void ADS1299::printRegisterName(byte _address) 
{
    if(_address == ID)
	{
        Serial.print(F("ID, "));       //the "F" macro loads the string directly from Flash memory, thereby saving RAM
    }
    else if(_address == CONFIG1)
	{
        Serial.print(F("CONFIG1, "));
    }
    else if(_address == CONFIG2)
	{
        Serial.print(F("CONFIG2, "));
    }
    else if(_address == CONFIG3)
	{
        Serial.print(F("CONFIG3, "));
    }
    else if(_address == LOFF)
	{
        Serial.print(F("LOFF, "));
    }
    else if(_address == CH1SET)
	{
        Serial.print(F("CH1SET, "));
    }
    else if(_address == CH2SET)
	{
        Serial.print(F("CH2SET, "));
    }
    else if(_address == CH3SET)
	{
        Serial.print(F("CH3SET, "));
    }
    else if(_address == CH4SET)
	{
        Serial.print(F("CH4SET, "));
    }
    else if(_address == CH5SET)
	{
        Serial.print(F("CH5SET, "));
    }
    else if(_address == CH6SET)
	{
        Serial.print(F("CH6SET, "));
    }
    else if(_address == CH7SET)
	{
        Serial.print(F("CH7SET, "));
    }
    else if(_address == CH8SET)
	{
        Serial.print(F("CH8SET, "));
    }
    else if(_address == BIAS_SENSP)
	{
        Serial.print(F("BIAS_SENSP, "));
    }
    else if(_address == BIAS_SENSN)
	{
        Serial.print(F("BIAS_SENSN, "));
    }
    else if(_address == LOFF_SENSP)
	{
        Serial.print(F("LOFF_SENSP, "));
    }
    else if(_address == LOFF_SENSN)
	{
        Serial.print(F("LOFF_SENSN, "));
    }
    else if(_address == LOFF_FLIP)
	{
        Serial.print(F("LOFF_FLIP, "));
    }
    else if(_address == LOFF_STATP)
	{
        Serial.print(F("LOFF_STATP, "));
    }
    else if(_address == LOFF_STATN)
	{
        Serial.print(F("LOFF_STATN, "));
    }
    else if(_address == GPIO)
	{
        Serial.print(F("GPIO, "));
    }
    else if(_address == MISC1)
	{
        Serial.print(F("MISC1, "));
    }
    else if(_address == MISC2)
	{
        Serial.print(F("MISC2, "));
    }
    else if(_address == CONFIG4)
	{
        Serial.print(F("CONFIG4, "));
    }
}



//SPI communication methods                  SPI通讯方式，开始清零中断，数据赋给SPDR
//整个函数从主元件送出一个字节到从元件，等待通信完成，然后再通过从元件向主元件返回1个字节的值。
byte ADS1299::transfer(byte _data)           //transfer(byte _data) 函数，要传递一个字符给该函数，并返回一个字符，函数的形参是byte型                                            //BYTE是让人关注它的长度，而不需要关注它的类型，相当于unsigned char 
{
	cli();                                  //清零中断
    SPDR = _data;                         //函数传递的参数要传递给SPI的数据寄存器SPDR
    while (!(SPSR & _BV(SPIF)))           //等待，直到SPIF标志表明数据已成功送出
    {  };

	sei();                                //置位中断
    return SPDR;                          //置位中断
}




// Used for printing HEX in verbose feedback mode     
void ADS1299::printHex(byte _data)              //以十六进制格式打印出数据
{
	Serial.print("0x");
    if(_data < 0x10) Serial.print("0");
    Serial.print(_data, HEX);
}

//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//



