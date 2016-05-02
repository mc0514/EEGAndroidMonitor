/*

 Developed by Chip Audette (Fall 2013) for use with OpenBCI
 Builds upon work by Joel Murphy and Conor Russomanno (Summer 2013)
 
 Modified January 2014.
 
 This example uses the ADS1299 Arduino Library, a software bridge between the ADS1299 TI chip and 
 Arduino. See http://www.ti.com/product/ads1299 for more information about the device and the README
 folder in the ADS1299 directory for more information about the library.
 
 */
 
typedef long int int32;
boolean flag = true ;
char trigger ;


#include<ADS1299Manager.h>  //单个OPENBCI的ADS1299管理库
ADS1299Manager ADSManager; //SPI通讯，数据传输准备就位，引脚号：13，12，11，10，9，8，4

#define N_CHANNELS_PER_OPENBCI (8) 
#define MAX_N_CHANNELS (2*N_CHANNELS_PER_OPENBCI) 
int nActiveChannels = MAX_N_CHANNELS;   //最大导联数,以上三句等价于：int nActiveChannels = MAX_N_CHANNELS = 16; 

//OPENBCI的另外一些设置
byte gainCode = ADS_GAIN24 ; // 可编程增益：24,在头文件中定义#define ADS_GAIN24 (0b01100000)
byte inputType = ADSINPUT_NORMAL ; //常规方式设置采集通道；还提供另外两种设置方式

//其他变量设置
long sampleCounter = 0 ; //定义采样计数
boolean is_running = false ; // 该标志在serialEvent()函数中设置
//#define PIN_STARTBINARY (7)  //拉低该引脚开始二进制转换,改为通过蓝牙发送和接受字符来控制

boolean startBecauseOfPin = false ;  //布尔型变量 startBecauseOfPin表明所处状态:是否传输数据
boolean startBecauseOfSerial = false ; //
//////////////////////////////////////////////////////////////////////////////////////
//1)serialEvent()函数的调用是运行在loop()函数之后的。
//2）serialEvent()函数只有在串口接收缓存器中有数据时才会进入，也就是Serial.available() >0。


//analog input
#define PIN_ANALOGINPUT (A0)
int analogVal = 0;
/*
#define OUTPUT_NOTHING (0)
#define OUTPUT_TEXT (1)
#define OUTPUT_BINARY (2)
#define OUTPUT_BINARY_SYNTHETIC (3)
#define OUTPUT_BINARY_4CHAN (4)
#define OUTPUT_BINARY_OPENEEG (6)
#define OUTPUT_BINARY_OPENEEG_SYNTHETIC (7)
#define OUTPUT_BINARY_WITH_AUX (8)
*/

int outputType;    //定义整形变量outputType，


/*
//Design filters  (This BIQUAD class requires ~6K of program space!  Ouch.)
//For frequency response of these filters: http://www.earlevel.com/main/2010/12/20/biquad-calculator/
#include <Biquad_multiChan.h>   //modified from this source code:  http://www.earlevel.com/main/2012/11/26/biquad-c-source-code/
#define SAMPLE_RATE_HZ (250.0)  //default setting for OpenBCI
#define FILTER_Q (0.5)        //critically damped is 0.707 (Butterworth)
#define FILTER_PEAK_GAIN_DB (0.0) //we don't want any gain in the passband
#define HP_CUTOFF_HZ (0.5)  //set the desired cutoff for the highpass filter

Biquad_multiChan stopDC_filter(MAX_N_CHANNELS,bq_type_highpass,HP_CUTOFF_HZ / SAMPLE_RATE_HZ, FILTER_Q, FILTER_PEAK_GAIN_DB); //one for each channel because the object maintains the filter states
//Biquad_multiChan stopDC_filter(MAX_N_CHANNELS,bq_type_bandpass,10.0 / SAMPLE_RATE_HZ, 6.0, FILTER_PEAK_GAIN_DB); //one for each channel because the object maintains the filter states
#define NOTCH_FREQ_HZ (60.0)
#define NOTCH_Q (4.0)              //pretty sharp notch
#define NOTCH_PEAK_GAIN_DB (0.0)  //doesn't matter for this filter type
Biquad_multiChan notch_filter1(MAX_N_CHANNELS,bq_type_notch,NOTCH_FREQ_HZ / SAMPLE_RATE_HZ, NOTCH_Q, NOTCH_PEAK_GAIN_DB); //one for each channel because the object maintains the filter states
Biquad_multiChan notch_filter2(MAX_N_CHANNELS,bq_type_notch,NOTCH_FREQ_HZ / SAMPLE_RATE_HZ, NOTCH_Q, NOTCH_PEAK_GAIN_DB); //one for each channel because the object maintains the filter states
boolean useFilters = false;  //enable or disable as you'd like...turn off if you're daisy chaining!
*/


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//setup函数只在程序的开头运行一次。在这个函数里可以在主循环开始前为程序设定一些通用的规则，如设置引脚形式、设置波特率等。
void setup() 
{
  Serial.begin(115200);   //设置波特率
  Serial.println("start setup"); 
  int OpenBCI_version = a211bci_V2 ; //等价于OpenBCI_version = 2，使用negative input，SRB2参考
  boolean isDaisy = false;     //默认为单个设备
  if (MAX_N_CHANNELS > 8) isDaisy = true;   //当通道数大于8时，开启菊花链配置模式
  ADSManager.initialize(OpenBCI_version,isDaisy);  //must do this VERY early in the setup...preferably first

  Serial.println(F("ADS1299-Arduino UNO - Stream Raw Data")); //read the string from Flash to save RAM
  Serial.print(F("Configured as OpenBCI_Version code = "));
  Serial.println(OpenBCI_version); 
  Serial.print(F(", isDaisy = "));Serial.println(ADSManager.isDaisy);
  Serial.print(F("Configured for "));Serial.print(MAX_N_CHANNELS); Serial.println(F(" Channels"));
  Serial.flush();
  
 //设置ads1299，设置通道、增益、输入类型、参考（SRB1）和病人的偏置信号
  for (int chan=1; chan <= nActiveChannels; chan++) 
  {
    ADSManager.activateChannel(chan, gainCode, inputType);
  }
  
  //ADSManager.configureLeadOffDetection(LOFF_MAG_6NA, LOFF_FREQ_31p2HZ);  //设置了导联脱落检测参数
  ADSManager.printAllRegisters(); //打印所有寄存器的状态
  Serial.flush();  
  
  
} 
// end of setup
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop()
{  
 
   if(flag)    //提示语句，循环开始时执行一次
   {
     Serial.println("Waiting for trigger......"); 
     Serial.println("Please enter 'o' to start running ;enter 'c' to stop running ."); 
     flag = false ;
   }
 
   trigger=Serial.read();         //通过串口读入字符，如果读入“o”则启动，如果读入“c”则停止
   
   if(trigger=='o')
   {
     Serial.println("Start running!"); 
     startBecauseOfPin = true;             //布尔型变量 startBecauseOfPin表明所处状态
//     startRunning(OUTPUT_NOTHING);
//     startRunning(OUTPUT_BINARY);
//     startRunning(OUTPUT_BINARY_OPENEEG);  //?????????????为什么开始就是6？？？？ 
     startRunning(9); 
   }
   else if(trigger=='c')
   {
      Serial.println("Stop running!");
      startBecauseOfPin = false;
      stopRunning();
   }   
 
 
  
  if (is_running)
  { 
    
     //判断数据准备是否就绪：DRDY的下降沿信号通知主控模块数据准备完毕
     //当DRDY低电平时，ADSManager.isDataAvailable()函数返回1，否则返回0；执行while的结果是：当DRDY=0时跳出while循环，即数据准备就绪，继续往下执行
    while(!(ADSManager.isDataAvailable()))
    {           
      delayMicroseconds(100);
      Serial.println("DRDY not ready"); //调试显示:8BCI会跳出while循环,16BCI一直在while循环，DRDY未准备就绪
    }
    Serial.println("out of while,continue to run"); 
    unsigned long start_micros = micros();
  
  
    //获取数据
    analogVal = analogRead(PIN_ANALOGINPUT);   // 获取模拟量
    ADSManager.updateChannelData();            // 更新数据数组，双重for循环将16通道数据更新到ChannelData[]数组中
    sampleCounter++;                           // 增加采样计数
    Serial.println(sampleCounter); 


    //打印数据
    switch (outputType)
    {
        case OUTPUT_NOTHING:  //不输出任何东西，Arduino仍然是来自openbci板采集数据…收集数据只是做它，只是不做任何操作
        break ;
        
        case OUTPUT_BINARY:  //打印所有通道，无论是否激活
        ADSManager.writeChannelDataAsBinary(MAX_N_CHANNELS,sampleCounter); 
        break ;
       
        case OUTPUT_BINARY_OPENEEG:   //这个格式接受6个通道
        ADSManager.writeChannelDataAsOpenEEG_P2(sampleCounter) ;
        break ;
       
       default:        //打印所有通道，无论是否激活
       ADSManager.printChannelDataAsText(MAX_N_CHANNELS,sampleCounter) ;              
    } 
  
  }
  
} 
// end of loop
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//调用函数
boolean startRunning(int OUT_TYPE)    //开始运行函数，一个整型形参
{
  outputType = OUT_TYPE;
  Serial.print("outputType=");
  Serial.println(OUT_TYPE);
  ADSManager.start();       //开始获取数据
  is_running = true;        //is_running是个布尔型变量
  return is_running;       //返回值0或1
}

boolean stopRunning(void)      //停止运行函数，无返回值
{
  ADSManager.stop() ;    //停止获取数据
  is_running = false ;
  return is_running ;
}














/*
#define ACTIVATE_SHORTED (2)
#define ACTIVATE (1)
#define DEACTIVATE (0)
void serialEvent(){            // send an 'x' on the serial line to trigger ADStest()
  while(Serial.available()){      
    char inChar = (char)Serial.read();
    switch (inChar)
    {
      //turn channels on and off
      case '1':
        changeChannelState_maintainRunningState(1,DEACTIVATE); break;
      case '2':
        changeChannelState_maintainRunningState(2,DEACTIVATE); break;
      case '3':
        changeChannelState_maintainRunningState(3,DEACTIVATE); break;
      case '4':
        changeChannelState_maintainRunningState(4,DEACTIVATE); break;
      case '5':
        changeChannelState_maintainRunningState(5,DEACTIVATE); break;
      case '6':
        changeChannelState_maintainRunningState(6,DEACTIVATE); break;
      case '7':
        changeChannelState_maintainRunningState(7,DEACTIVATE); break;
      case '8':
        changeChannelState_maintainRunningState(8,DEACTIVATE); break;
      case 'q':
        changeChannelState_maintainRunningState(1,ACTIVATE); break;
      case 'w':
        changeChannelState_maintainRunningState(2,ACTIVATE); break;
      case 'e':
        changeChannelState_maintainRunningState(3,ACTIVATE); break;
      case 'r':
        changeChannelState_maintainRunningState(4,ACTIVATE); break;
      case 't':
        changeChannelState_maintainRunningState(5,ACTIVATE); break;
      case 'y':
        changeChannelState_maintainRunningState(6,ACTIVATE); break;
      case 'u':
        changeChannelState_maintainRunningState(7,ACTIVATE); break;
      case 'i':
        changeChannelState_maintainRunningState(8,ACTIVATE); break;
        
      //turn lead-off detection on and off
      case '!':
        changeChannelLeadOffDetection_maintainRunningState(1,ACTIVATE,PCHAN); break;
      case '@':
        changeChannelLeadOffDetection_maintainRunningState(2,ACTIVATE,PCHAN); break;
      case '#':
        changeChannelLeadOffDetection_maintainRunningState(3,ACTIVATE,PCHAN); break;
      case '$':
        changeChannelLeadOffDetection_maintainRunningState(4,ACTIVATE,PCHAN); break;
      case '%':
        changeChannelLeadOffDetection_maintainRunningState(5,ACTIVATE,PCHAN); break;
      case '^':
        changeChannelLeadOffDetection_maintainRunningState(6,ACTIVATE,PCHAN); break;
      case '&':
        changeChannelLeadOffDetection_maintainRunningState(7,ACTIVATE,PCHAN); break;
      case '*':
        changeChannelLeadOffDetection_maintainRunningState(8,ACTIVATE,PCHAN); break;
      case 'Q':
        changeChannelLeadOffDetection_maintainRunningState(1,DEACTIVATE,PCHAN); break;
      case 'W':
        changeChannelLeadOffDetection_maintainRunningState(2,DEACTIVATE,PCHAN); break;
      case 'E':
        changeChannelLeadOffDetection_maintainRunningState(3,DEACTIVATE,PCHAN); break;
      case 'R':
        changeChannelLeadOffDetection_maintainRunningState(4,DEACTIVATE,PCHAN); break;
      case 'T':
        changeChannelLeadOffDetection_maintainRunningState(5,DEACTIVATE,PCHAN); break;
      case 'Y':
        changeChannelLeadOffDetection_maintainRunningState(6,DEACTIVATE,PCHAN); break;
      case 'U':
        changeChannelLeadOffDetection_maintainRunningState(7,DEACTIVATE,PCHAN); break;
      case 'I':
        changeChannelLeadOffDetection_maintainRunningState(8,DEACTIVATE,PCHAN); break;
       case 'A':
        changeChannelLeadOffDetection_maintainRunningState(1,ACTIVATE,NCHAN); break;
      case 'S':
        changeChannelLeadOffDetection_maintainRunningState(2,ACTIVATE,NCHAN); break;
      case 'D':
        changeChannelLeadOffDetection_maintainRunningState(3,ACTIVATE,NCHAN); break;
      case 'F':
        changeChannelLeadOffDetection_maintainRunningState(4,ACTIVATE,NCHAN); break;
      case 'G':
        changeChannelLeadOffDetection_maintainRunningState(5,ACTIVATE,NCHAN); break;
      case 'H':
        changeChannelLeadOffDetection_maintainRunningState(6,ACTIVATE,NCHAN); break;
      case 'J':
        changeChannelLeadOffDetection_maintainRunningState(7,ACTIVATE,NCHAN); break;
      case 'K':
        changeChannelLeadOffDetection_maintainRunningState(8,ACTIVATE,NCHAN); break;
      case 'Z':
        changeChannelLeadOffDetection_maintainRunningState(1,DEACTIVATE,NCHAN); break;
      case 'X':
        changeChannelLeadOffDetection_maintainRunningState(2,DEACTIVATE,NCHAN); break;
      case 'C':
        changeChannelLeadOffDetection_maintainRunningState(3,DEACTIVATE,NCHAN); break;
      case 'V':
        changeChannelLeadOffDetection_maintainRunningState(4,DEACTIVATE,NCHAN); break;
      case 'B':
        changeChannelLeadOffDetection_maintainRunningState(5,DEACTIVATE,NCHAN); break;
      case 'N':
        changeChannelLeadOffDetection_maintainRunningState(6,DEACTIVATE,NCHAN); break;
      case 'M':
        changeChannelLeadOffDetection_maintainRunningState(7,DEACTIVATE,NCHAN); break;
      case '<':
        changeChannelLeadOffDetection_maintainRunningState(8,DEACTIVATE,NCHAN); break; 
        
      //control the bias generation
      case '`':
        ADSManager.setAutoBiasGeneration(true); break;
      case '~': 
        ADSManager.setAutoBiasGeneration(false); break; 
        
      //control test signals
      case '0':
        activateAllChannelsToTestCondition(ADSINPUT_SHORTED,ADSTESTSIG_NOCHANGE,ADSTESTSIG_NOCHANGE); break;
      case '-':
        activateAllChannelsToTestCondition(ADSINPUT_TESTSIG,ADSTESTSIG_AMP_1X,ADSTESTSIG_PULSE_SLOW); break;
      case '+':
        activateAllChannelsToTestCondition(ADSINPUT_TESTSIG,ADSTESTSIG_AMP_1X,ADSTESTSIG_PULSE_FAST); break;
      case '=':
        //repeat the line above...just for human convenience
        activateAllChannelsToTestCondition(ADSINPUT_TESTSIG,ADSTESTSIG_AMP_1X,ADSTESTSIG_PULSE_FAST); break;
      case 'p':
        activateAllChannelsToTestCondition(ADSINPUT_TESTSIG,ADSTESTSIG_AMP_2X,ADSTESTSIG_DCSIG); break;
      case '[':
        activateAllChannelsToTestCondition(ADSINPUT_TESTSIG,ADSTESTSIG_AMP_2X,ADSTESTSIG_PULSE_SLOW); break;
      case ']':
        activateAllChannelsToTestCondition(ADSINPUT_TESTSIG,ADSTESTSIG_AMP_2X,ADSTESTSIG_PULSE_FAST); break;
        
      //other commands
      case 'n':
        toggleRunState(OUTPUT_BINARY_WITH_AUX);
        startBecauseOfSerial = is_running;
        if (is_running) Serial.println(F("Arduino: Starting binary (including AUX value)..."));
        break;
      case 'b':
        toggleRunState(OUTPUT_BINARY);
        //toggleRunState(OUTPUT_BINARY_SYNTHETIC);
        startBecauseOfSerial = is_running;
        if (is_running) Serial.println(F("Arduino: Starting binary..."));
        break;
      case 'v':
        toggleRunState(OUTPUT_BINARY_4CHAN);
        startBecauseOfSerial = is_running;
        if (is_running) Serial.println(F("Arduino: Starting binary 4-chan..."));
        break;
     case 's':
        stopRunning();
        startBecauseOfSerial = is_running;
        break;
     case 'x':
        toggleRunState(OUTPUT_TEXT);
        startBecauseOfSerial = is_running;
        if (is_running) Serial.println(F("Arduino: Starting text..."));
        break;
     case 'f':
        useFilters = true;
        Serial.println(F("Arduino: enabaling filters"));
        break;
     case 'g':
        useFilters = false;
        Serial.println(F("Arduino: disabling filters"));
        break;
     case '?':
        //print state of all registers
        ADSManager.printAllRegisters();
        break;
      default:
        break;
    }
  }
}

boolean toggleRunState(int OUT_TYPE)   //切换运行状态
{
  if (is_running) {
    return stopRunning();
  } else {
    return startRunning(OUT_TYPE);
  }
}

boolean stopRunning(void) {
  ADSManager.stop();                    // stop the data acquisition
  is_running = false;
  return is_running;
}



int changeChannelState_maintainRunningState(int chan, int start)
{
  boolean is_running_when_called = is_running;
  int cur_outputType = outputType;
  
  //must stop running to change channel settings
  stopRunning();
  if (start == true) {
    Serial.print(F("Activating channel "));
    Serial.println(chan);
    ADSManager.activateChannel(chan,gainCode,inputType);
  } else {
    Serial.print(F("Deactivating channel "));
    Serial.println(chan);
    ADSManager.deactivateChannel(chan);
  }
  
  //restart, if it was running before
  if (is_running_when_called == true) {
    startRunning(cur_outputType);
  }
}

int changeChannelLeadOffDetection_maintainRunningState(int chan, int start, int code_P_N_Both)
{
  boolean is_running_when_called = is_running;
  int cur_outputType = outputType;
  
  //must stop running to change channel settings
  stopRunning();
  if (start == true) {
    Serial.print(F("Activating channel "));
    Serial.print(chan);
    Serial.println(F(" Lead-Off Detection"));
    ADSManager.changeChannelLeadOffDetection(chan,ON,code_P_N_Both);
  } else {
    Serial.print(F("Deactivating channel "));
    Serial.print(chan);
    Serial.println(F(" Lead-Off Detection"));
    ADSManager.changeChannelLeadOffDetection(chan,OFF,code_P_N_Both);
  }
  
  //restart, if it was running before
  if (is_running_when_called == true) {
    startRunning(cur_outputType);
  }
}


int activateAllChannelsToTestCondition(int testInputCode, byte amplitudeCode, byte freqCode)
{
  boolean is_running_when_called = is_running;
  int cur_outputType = outputType;
  
  //set the test signal to the desired state
  ADSManager.configureInternalTestSignal(amplitudeCode,freqCode);
  
  //must stop running to change channel settings
  stopRunning();
    
  //loop over all channels to change their state
  for (int Ichan=1; Ichan <= 8; Ichan++) {
    ADSManager.activateChannel(Ichan,gainCode,testInputCode);  //Ichan must be [1 8]...it does not start counting from zero
  }
      
  //restart, if it was running before
  if (is_running_when_called == true) {
    startRunning(cur_outputType);
  }
}

long int runningAve[MAX_N_CHANNELS];
int applyFilters(void) {
  //scale factor for these coefficients was 32768 = 2^15
  const static long int a0 = 32360L; //16 bit shift?
  const static long int a1 = -2L*a0;
  const static long int a2 = a0;
  const static long int b1 = -64718L; //this is a shift of 17 bits!
  const static long int b2 = 31955L;
  static long int z1[MAX_N_CHANNELS], z2[MAX_N_CHANNELS];
  long int val_int, val_in_down9, val_out, val_out_down9;
  float val;
  for (int Ichan=0; Ichan < MAX_N_CHANNELS; Ichan++) {
    switch (1) {
      case 1:
        //use BiQuad
        val = (float) ADSManager.channelData[Ichan]; //get the stored value for this sample
        val = stopDC_filter.process(val,Ichan);    //apply DC-blocking filter
        break;
      case 2:
        //do fixed point, 1st order running ave
        val_int = ADSManager.channelData[Ichan]; //get the stored value for this sample
        //runningAve[Ichan]=( ((512-1)*(runningAve[Ichan]>>2)) + (val_int>>2) )>>7;  // fs/0.5Hz = ~512 points..9 bits
        //runningAve[Ichan]=( ((256-1)*(runningAve[Ichan]>>2)) + (val_int>>2) )>>6;  // fs/1.0Hz = ~256 points...8 bits
        runningAve[Ichan]=( ((128-1)*(runningAve[Ichan]>>1)) + (val_int>>1) )>>6;  // fs/2.0Hz = ~128 points...7 bits
        val = (float)(val_int - runningAve[Ichan]);  //remove the DC
        break;
//      case 3:
//        val_in_down9 = ADSManager.channelData[Ichan] >> 9; //get the stored value for this sample...bring 24-bit value down to 16-bit
//        val_out = (val_in_down9 * a0  + (z1[Ichan]>>9)) >> (16-9);  //8bits were already removed...results in 24-bit value
//        val_out_down9 = val_out >> 9;  //remove eight bits to go from 24-bit down to 16 bit
//        z1[Ichan] = (val_in_down9 * a1 + (z2[Ichan] >> 9) - b1 * val_out_down9  ) >> (16-9);  //8-bits were pre-removed..end in 24 bit number
//        z2[Ichan] = (val_in_down9 * a2  - b2 * val_out_down9) >> (16-9); //8-bits were pre-removed...end in 24-bit number
//        val = (float)val_out;
//        break;
    }
    val = notch_filter1.process(val,Ichan);     //apply 60Hz notch filter
    val = notch_filter2.process(val,Ichan);     //apply it again
    ADSManager.channelData[Ichan] = (long) val;  //save the value back into the main data-holding object
  }
  return 0;
}

int freeRam() 
{
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}
*/

