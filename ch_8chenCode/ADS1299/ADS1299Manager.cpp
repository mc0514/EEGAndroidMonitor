


#include <ADS1299Manager.h>

typedef long int32;
//typedef byte uint8_t;


void ADS1299Manager::initialize(void)
{
	Serial.println(F("start ADSManager initialize "));
	boolean isDaisy = false;
    initialize(a211bci_V2,isDaisy);
	Serial.println(F("All initialize finished!"));
	Serial.println();
}


//Initilize the ADS1299 controller...call this once
void ADS1299Manager::initialize(const int version,boolean isDaisy) 
{
  Serial.println(F("start initialize right now!"));
  setVersion211bci(version);
  ADS1299::initialize(PIN_DRDY,PIN_RST,PIN_CS,SCK_MHZ,isDaisy); // (DRDY pin, RST pin, CS pin, SCK frequency in MHz);
  //等价于ADS1299::initialize(8,9,10,4);
  delay(100);
  
  verbose = false;      // when verbose is true, there will be Serial feedback 
//verbose = true;       // 
  reset();
  
  n_chan_all_boards = a211bci_NCHAN; //默认8通道
  if (isDaisy) n_chan_all_boards = 2*a211bci_NCHAN;  //菊花链时2*8=16通道

  //set default state for internal test signal
  //ADS1299::WREG(CONFIG2,0b11010000);delay(1);   //set internal test signal, default amplitude, default speed, datasheet PDF Page 41
  //ADS1299::WREG(CONFIG2,0b11010001);delay(1);   //set internal test signal, default amplitude, 2x speed, datasheet PDF Page 41
  configureInternalTestSignal(ADSTESTSIG_AMP_1X,ADSTESTSIG_PULSE_FAST); //设置内部测试信号、默认振幅、2X速度、PDF41   
  //设置导通检测的默认状态（无阻抗检测，隐掉）
 // configureLeadOffDetection(LOFF_MAG_6NA,LOFF_FREQ_31p2HZ);

  Serial.println(F("ADSManager initialize finished"));  
};


//OpenBCI version:有两个版本（两个值：1和2），有三个影响
//影响使用positive or negative inputs;
//影响使用SRB1还是SRB2参考信号；
//影响lead_off信号翻转与否。
//自己设计的电路中使用的是negative input，SRB2参考
void ADS1299Manager::setVersion211bci(const int version)
{
  if (version == a211bci_V1) //如果version=1，则使用positive input
  {
  	  use_neg_inputs = false;
  	  for (int i=0; i < a211bci_NCHAN; i++) 
	  {
  	  	  use_SRB2[i] = false;
  	  }
  } 
  else                      //version=2，则使用negative input
  {
  	  use_neg_inputs = true;
  	  for (int i=0; i < a211bci_NCHAN; i++) 
	  {
  	  	  use_SRB2[i] = true;
  	  }
	  Serial.println(F("All use_SRB2 is true"));
  }
  //setSRB1(use_SRB1());  //set whether SRB1 is active or not
}



   //这里有一段有关正负输入以及翻转极性的代码，暂不添加





//reset all the ADS1299's settings.  Call however you'd like.  Stops all data acquisition
void ADS1299Manager::reset(void)
{
  ADS1299::RESET();             // send RESET command to default all registers
  ADS1299::SDATAC();            // exit Read Data Continuous mode to communicate with ADS  
  delay(100);
    
  // turn off all channels
  for (int chan=1; chan <= a211bci_NCHAN; chan++) 
  {
    deactivateChannel(chan);
	//关闭任何阻抗检测（无阻抗检测，隐掉）
	//changeChannelLeadOffDetection(chan,OFF,BOTHCHAN); 
  }
  
  setSRB1(use_SRB1());  //set whether SRB1 is active or not
  //configure ADS1299 so that bias is generated based on channel state(不明用途，暂时隐掉)
  //setAutoBiasGeneration(true); 
  
};


//deactivate the given channel...note: stops data colleciton to issue its commands
//N is the channel number: 1-8 
void ADS1299Manager::deactivateChannel(int N)
{
  byte reg, config;
	
  //check the inputs
  if ((N < 1) || (N > a211bci_NCHAN)) return;
  
  //proceed...first, disable any data collection
  ADS1299::SDATAC();  // exit Read Data Continuous mode to communicate with ADS
  delay(1);

  //shut down the channel
  N = constrain(N-1,0,a211bci_NCHAN-1);  //subtracts 1 so that we're counting from 0, not 1
  reg = CH1SET+(byte)N;
  config = ADS1299::RREG(reg); delay(1);
  bitSet(config,7);  //left-most bit (bit 7) = 1, so this shuts down the channel
  if (use_neg_inputs) bitClear(config,3);  //bit 3 = 0 disconnects SRB2
  ADS1299::WREG(reg,config); delay(1);
  
  //以下一段代码替换了原版的一个函数
  //remove the channel from the bias generation...
  reg = BIAS_SENSP; if (use_neg_inputs) reg = BIAS_SENSN;  //are we using the P inptus or the N inputs?
  config = ADS1299::RREG(reg); delay(1);//get the current bias settings
  bitClear(config,N);          //clear this channel's bit to remove from bias generation
  ADS1299::WREG(reg,config); delay(1);  //send the modified byte back to the ADS

  //set how this channel affects the bias generation...
  //alterBiasBasedOnChannelState(N);
}; 
    
        
//Active a channel in single-ended mode  
//  N is 1 through 8
//  gainCode is defined in the macros in the header file
//  inputCode is defined in the macros in the header file
void ADS1299Manager::activateChannel(int N,byte gainCode,byte inputCode) 
{
  Serial.println("start activate all channels");
  //check the inputs
  if ((N < 1) || (N > a211bci_NCHAN)) return;
  
  //proceed...first, disable any data collection
  ADS1299::SDATAC(); delay(1);      // exit Read Data Continuous mode to communicate with ADS

  //active the channel using the given gain.  Set MUX for normal operation
  //see ADS1299 datasheet, PDF p44
  N = constrain(N-1,0,a211bci_NCHAN-1);  //shift down by one
  byte configByte = 0b00000000;  //left-most zero (bit 7) is to activate the channel
  gainCode = gainCode & 0b01110000;  //bitwise AND to get just the bits we want and set the rest to zero
  configByte = configByte | gainCode; //bitwise OR to set just the gain bits high or low and leave the rest alone
  inputCode = inputCode & 0b00000111;  //bitwise AND to get just the bits we want and set the rest to zero
  configByte = configByte | inputCode; //bitwise OR to set just the gain bits high or low and leave the rest alone
  if (use_SRB2[N]) configByte |= 0b00001000;  //set the SRB2 flag...p44 in the data sheet
  ADS1299::WREG(CH1SET+(byte)N,configByte); delay(1);

  //add this channel to the bias generation
  //see ADS1299 datasheet, PDF p44
  byte reg = BIAS_SENSP; if (use_neg_inputs) reg = BIAS_SENSN;  //are we using the P inptus or the N inputs?
  byte biasSettings = ADS1299::RREG(reg); //get the current bias settings
  bitSet(biasSettings,N);                   //set this channel's bit
  ADS1299::WREG(reg,biasSettings); delay(1);  //send the modified byte back to the ADS
  
  //Now, these actions are necessary whenever there is at least one active channel
  //though they don't strictly need to be done EVERY time we activate a channel.
  //just once after the reset.
  
  //activate SRB1 as the Negative input for all channels, if needed
  setSRB1(use_SRB1());

  //Finalize the bias setup...activate buffer and use internal reference for center of bias creation, datasheet PDF p42
  ADS1299::WREG(CONFIG3,0b11101100); delay(1); 
  Serial.println("activate all channels");
  Serial.println();
};


void ADS1299Manager::setSRB1(boolean desired_state) {
	if (desired_state) {
		ADS1299::WREG(MISC1,0b00100000); delay(1);  //ADS1299 datasheet, PDF p46
	} else {
		ADS1299::WREG(MISC1,0b00000000); delay(1);  //ADS1299 datasheet, PDF p46
	}
}


//Configure the test signals that can be inernally generated by the ADS1299
void ADS1299Manager::configureInternalTestSignal(byte amplitudeCode, byte freqCode)
{
	if (amplitudeCode == ADSTESTSIG_NOCHANGE) amplitudeCode = (ADS1299::RREG(CONFIG2) & (0b00000100));
	if (freqCode == ADSTESTSIG_NOCHANGE) freqCode = (ADS1299::RREG(CONFIG2) & (0b00000011));
	freqCode &= 0b00000011;  //only the last two bits should be used
	amplitudeCode &= 0b00000100;  //only this bit should be used
	byte message = 0b11010000 | freqCode | amplitudeCode;  //compose the code
	
	ADS1299::WREG(CONFIG2,message); delay(1);
	
       //ADS1299::WREG(CONFIG2,0b11010000);delay(1);   //set internal test signal, default amplitude, default speed, datasheet PDF Page 41
      //ADS1299::WREG(CONFIG2,0b11010001);delay(1);   //set internal test signal, default amplitude, 2x speed, datasheet PDF Page 41
      //ADS1299::WREG(CONFIG2,0b11010101);delay(1);   //set internal test signal, 2x amplitude, 2x speed, datasheet PDF Page 41
      //ADS1299::WREG(CONFIG2,0b11010011); delay(1);  //set internal test signal, default amplitude, at DC, datasheet PDF Page 41
      //ADS1299::WREG(CONFIG3,0b01101100); delay(1);  //use internal reference for center of bias creation, datasheet PDF p42 
}
 
//Start continuous data acquisition
void ADS1299Manager::start(void)
{
    ADS1299::RDATAC(); delay(1);           // enter Read Data Continuous mode
    ADS1299::START();    //start the data acquisition
	Serial.println("enter Read Data Continuous mode and start the data acquisition");
}
  
//Query to see if data is available from the ADS1299...return TRUE is data is available
int ADS1299Manager::isDataAvailable(void)
{
  Serial.println("DRDY ready or not?");
  return (!(digitalRead(PIN_DRDY)));
 
}
  
//Stop the continuous data acquisition
void ADS1299Manager::stop(void)
{
    ADS1299::STOP(); delay(1);   //stop the data acquisition
    ADS1299::SDATAC(); delay(1);      // exit Read Data Continuous mode to communicate with ADS
    Serial.println("stop Read Data Continuous mode and stop the data acquisition");
}
  
//print as text each channel's data
//   print channels 1-N (where N is 1-8...anything else will return with no action)
//   sampleNumber is a number that, if greater than zero, will be printed at the start of the line
//以文本形式打印各通道数据，打印示例：1，-2059,623，-4185,，2067,631，-1540,789，-1467
void ADS1299Manager::printChannelDataAsText(int N, long int sampleNumber)
{
	Serial.println("print data ");
	Serial.println();
	//check the inputs
	if ((N < 1) || (N > n_chan_all_boards)) return; 
	
	//print the sample number, if not disabled
	if (sampleNumber > 0) {
		Serial.print(sampleNumber);
		Serial.print(", ");  
	}

	//print each channel
	for (int chan = 0; chan < N; chan++ )
	{
		Serial.print(channelData[chan]);
		Serial.print(", ");
	}
	
	//print end of line
	Serial.println();
    
	
};



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//write as binary each channel's data
//   print channels 1-N (where N is 1-8...anything else will return with no action)
//   sampleNumber is a number that, if greater than zero, will be printed at the start of the line
int32 val;
byte *val_ptr = (byte *)(&val);
void ADS1299Manager::writeChannelDataAsBinary(int N, long sampleNumber)
{
	ADS1299Manager::writeChannelDataAsBinary(N,sampleNumber,false);
}

void ADS1299Manager::writeChannelDataAsBinary(int N, long sampleNumber,boolean useSyntheticData)
{
	//check the inputs
	if ((N < 1) || (N > n_chan_all_boards)) return;
	
	// Write header
	Serial.write( (byte) PCKT_START);
	//byte byte_val = (1+8)*4;
	Serial.write((1+N)*4);  //length of data payload, bytes
	
	
		
	//print the sample number, if not disabled
	val = sampleNumber;
	//val = 1L;
	//ptr = (uint8_t *)(&val);  //pretend that it is a string buffer so that Serial.write works easier
	//if (sampleNumber >= 0) {
		Serial.write(val_ptr,4); //4 bytes long
	//}

	//print each channel
	for (int chan = 0; chan < N; chan++ )
	{
		//get this channel's data
		if (useSyntheticData) 
		{
			val = makeSyntheticSample(sampleNumber,chan);
			//val = sampleNumber;
		} 
		else 
		{
			//get the real EEG data for this channel
			val = channelData[chan];
		}
		Serial.write(val_ptr,4); //4 bytes long
	}
	
	// Write footer
	Serial.write((byte)PCKT_END);
	
	// force everything out
	//Serial.flush();	
   //Serial.println("what is the function of OUTPUT_BINARY?");
 
};


void ADS1299Manager::writeChannelDataAsBinaryToAndroid(int N, long int sampleNumber){
  	//check the inputs
	if ((N < 1) || (N > n_chan_all_boards)) return;
	
	// Write header
	Serial.write( (byte) (0xA5));
	Serial.write((byte)(0xA5));
	Serial.write((byte)(0x5A));
	Serial.write((byte)(0x5A));
	//byte byte_val = (1+8)*4;
	Serial.write((2+N)*4,2);  //length of data payload, bytes
	Serial.write(1,2);
	
	
		
	//print the sample number, if not disabled
	val = sampleNumber;
	//val = 1L;
	//ptr = (uint8_t *)(&val);  //pretend that it is a string buffer so that Serial.write works easier
	//if (sampleNumber >= 0) {
		Serial.write(val,4); //4 bytes long
	//}

	//print each channel
	for (int chan = 0; chan < N; chan++ )
	{
		//get this channel's data
		//if (useSyntheticData) 
		//{
	  //val = makeSyntheticSample(sampleNumber,chan);
			//val = sampleNumber;
	  //} 
	  //	else 
	  //	{
			//get the real EEG data for this channel
			val = channelData[chan];
			//		}
		Serial.write(val_ptr,4); //4 bytes long
	}
	
	// Write footer
	Serial.write((byte)(0x99));
	Serial.write((byte)(0x99));
	Serial.write((byte)(0x44));
	Serial.write((byte)(0x44));
	// force everything out
	//Serial.flush();	
   //Serial.println("what is the function of OUTPUT_BINARY?");
}

//write channel data using binary format of ModularEEG so that it can be used by BrainBay (P2 protocol)
//this only sends 6 channels of data, per the P2 protocol
//http://www.shifz.org/brainbay/manuals/brainbay_developer_manual.pdf
#define max_int16 (32767)
#define min_int16 (-32767)
void ADS1299Manager::writeChannelDataAsOpenEEG_P2(long sampleNumber) {
	ADS1299Manager::writeChannelDataAsOpenEEG_P2(sampleNumber,false);
}
void ADS1299Manager::writeChannelDataAsOpenEEG_P2(long sampleNumber,boolean useSyntheticData) {
	static int count = -1;
	
	byte sync0 = 0xA5;
	byte sync1 = 0x5A;
	byte version = 2;
	
	Serial.write(sync0);
	Serial.write(sync1);
	Serial.write(version);
	byte foo = (byte)sampleNumber;
	if (foo == sync0) foo--;
	Serial.write(foo);
	
	long val32; //32-bit
	int val_i16;  //16-bit
	unsigned int val_u16;  //16-bit
	byte *val16_ptr = (byte *)(&val_u16);  //points to the memory for the variable above
	for (int chan = 0; chan < 6; chan++ )
	{
		//get this channel's data
		if (useSyntheticData) {
			//generate XX uV pk-pk signal
			//long time_samp_255 = (long)((sampleNumber) & (0x000000FF));  //make an 8-bit ramp waveform
			//time_samp_255 = (long)((time_samp_255*(long)(chan+1)) & (0x000000FF)); //each channel is faster than the previous
			//time_samp_255 += 256L*2L;  //make zero mean...empirically tuned via BrainBay visualization
			//val32 = (synthetic_amplitude_counts * time_samp_255) / 255L; //scaled zero-mean ramp 
			val32 = makeSyntheticSample(sampleNumber,chan) + 127L + 256L*2L;  //make zero mean...empirically tuned via BrainBay visualization
		} else {
			//get the real EEG data for this channel
			val32 = channelData[chan];
		}			
						
		//prepare the value for transmission
		val32 = val32 / (32);  //shrink to fit within a 16-bit number
		val32 = constrain(val32,min_int16,max_int16);  //constrain to fit in 16 bits
		val_u16 = (unsigned int) (val32 & (0x0000FFFF));  //truncate and cast
		if (val_u16 > 1023) val_u16 = 1023;
	
		//Serial.write(val16_ptr,2); //low byte than high byte on Arduino
		//Serial.write((byte)((val_u16 >> 8) & 0x00FF)); //high byte
		//Serial.write((byte)(val_u16 & 0x00FF)); //low byte
		foo = (byte)((val_u16 >> 8) & 0x00FF); //high byte
		if (foo == sync0) foo--;
		Serial.write(foo);
		foo = (byte)(val_u16 & 0x00FF); //high byte
		if (foo == sync0) foo--;
		Serial.write(foo);


		
	}
	//byte switches = 0b00000000;  //the last thing required by the P2 data protocol
	byte switches = 0x07;
	count++; if (count >= 18) count=0;
	if (count >= 9) {
		switches = 0x0F;
	}	
	Serial.write(switches);
	Serial.println("what is the function?");
}



#define  synthetic_amplitude_counts (8950L)   //counts peak-to-peak...should be 200 uV pk-pk  2.0*(100e-6 / (4.5 / 24 / 2^24))
long int ADS1299Manager::makeSyntheticSample(long sampleNumber,int chan) {
	//generate XX uV pk-pk signal
	long time_samp_255 = (long)((sampleNumber) & (0x000000FF));  //make an 8-bit ramp waveform
	time_samp_255 = (long)((time_samp_255*(long)(chan+1)) & (0x000000FF)); //each channel is faster than the previous
	//time_samp_255 += 256L*2L;  //make zero mean...empirically tuned via BrainBay visualization
	time_samp_255 -= 127L;
	return (synthetic_amplitude_counts * time_samp_255) / 255L; //scaled zero-mean ramp 
};


//print out the state of all the control registers
void ADS1299Manager::printAllRegisters(void)   
{
	Serial.println(F("start printAllRegisters"));
	boolean prevVerboseState = verbose;      //verbose布尔型,verbose=1或0
	
        verbose = true;
        ADS1299::RREGS(0x00,0x10);     // write the first registers
        delay(100);  //stall to let all that data get read by the PC
        ADS1299::RREGS(0x11,0x17-0x11);     // write the rest
        verbose = prevVerboseState;
		Serial.println(F("printAllRegisters finished"));
		Serial.println();
}


//only use SRB1 if all use_SRB2 are set to false
boolean ADS1299Manager::use_SRB1(void) {
	for (int Ichan=0; Ichan < a211bci_NCHAN; Ichan++) {
		if (use_SRB2[Ichan]) {
			return false;
		}
	}
	return true;
}
			