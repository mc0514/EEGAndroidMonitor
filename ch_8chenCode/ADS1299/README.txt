This Library opens communication between Arduino UNO and the ADS1299
SPI register fuses are directly set to match the ADS1299 interface requirements.  //库，Ardunio UNo 和ADS1299 SPI寄存器通讯，匹配ADS1299接口
SCK rates: 1MHz, 4MHz, 8MHz.                                                      //时钟频率
All registers and commands are fully accessable.                                  //所有的寄存器和命令都可用
Here is a list of the functions and parameters available from the Library.        //库中可用的功能和参数列表



ADS1299.Initialize(int _DRDY, int _RST, int _CS, int _FREQ)                      //ADS1299初始化：数据准备，片选，时钟频率
	Data Ready pin, Chip Select pin, and SCK frequency in MHz.
	the current shield uses DRDY = 8; RST 9 CS = 10.
	frequency options are 1MHz, 4MHz, 8MHz.
	bear in mind that at higher frequencies, appropriate delays                  //高频时，适当延迟
	must be taken when sending multiple bytes (DS p.37)                          //发送多个字节时，必须设置
	initialization performs recommended timing and toggle of /RESET pin

	
	
	
//Public Variables                                                             //公用变量

    int DRDY, CS; 		// pin numbers for DRDY and CS         //DRDY和CS，结构变量，当前库中未使用，在以后的成员函数中有用
	These are part of the constructor
	they are not implemented in the current library
	but could be useful in future member functions

    int stat;			// used to hold the status register       //stat：设置状态寄存器，当读取通道数据时，状态寄存器返回第一个字节？            
	the status register is the first byte returned when you read channel data
	it has 1100 + LOFF_STATP + LOFF_STATN + GPIO[7:4]             //P45

    byte regData [24];		// array is used to mirror register data            //regData数组：用来反映寄存器数据，该数组用WREGS（）写ADS寄存器数据的多个地址
	this array is used to write multiple locations of ADS register data using WREGS()
	user must first assign target values to the corresponding locations in regData array    //用户必须先分配目标值到regData数组相应的位置，调用RREGS（）将读取多个位置和更新regData数组
	a call to RREGS() will read multiple locations and update regData array
  
    long channelData [9];	// array used when reading channel data             //channelData数组：读取通道数据
    boolean verbose;		// turn on/off Serial feedback                      //verbose：开关串行反馈

	
	
	

//System Commands                                                          //系统命令

    void WAKEUP();	// wakeup from standby mode                          //从待机模式唤醒
    void STANDBY();	// go into standby mode                                 //进入待机模式
    void RESET();	// reset all registers to default values              //复位所有寄存器为默认值
    void START();	// start a data conversion session                    //开始数据转换
    void STOP();	// stop data conversion                                //停止数据转换

	timed delays are taken where required in the above funcitons.    //上面多有的功能都要注意时间延迟      
	bear in mind the timing of your byte to byte transmission.      //记住字节与字节传输的时间
	clk on ADS is ~2MHz, so that is a limit.
    
//Data Read Commands                                                  //数据读命令

    void RDATAC();	                 									//RDATAC（）
	enter Read Data Continuous mode                                    //进入读取数据连续模式，使能ADC和移位寄存器；该模式下，通过设置start引脚（或发送开始命令）开始采样
	Read Data Continuous mode enables the ADC and shift registers
	in this mode, you can start a sampling session by setting the start pin,
	or sending the start command

    void SDATAC();	                                                    //SDATAC（）
	enter Stop Read Data Continuous mode                                //进入停止读数据连续模式，该模式将让你写和读寄存器
	this mode will let you write and read registers

    void RDATA();	                                                           //RDATA（）
	Read Data command is used when you're in Stop Read Data Continuous mode    //读数据命令用于当你停止读数据连续模式
	if you send or set the 'start' command, this allows you to sample on DRDY   //如果你发送或设置start命令，这将允许你在DRDY采样并且可读写寄存器
	and also have access to read/write the registers. see DS p.37 
    
	
	
	
//Register Read/Write Commands	                                       //寄存器读/写命令

    byte getDeviceID();                                                //getDeviceID（）
	simple hello world to check that the device is working properly    //简单的hello world检查设备是否工作正常，在开发模块上返回0x3E
	returns 0x3E on the Dev Module I'm using.
	must be in Stop Data Continuous mode prior to sending this         //必须在停止读数据连续模式之前发送改命令

    byte RREG(byte _address);                                        //byte RREG(byte _address)
	returns the value of one register address, specified             //返回寄存器地址的值，也在地址中更新regData
	the funcion also updates regData at _address

    void RREGS(byte _address, byte _numRegistersMinusOne);            // void RREGS(byte _address, byte _numRegistersMinusOne)
	
	updates the public regData array with current values              //用当前值更新公共regData数组的值
	_address is the address to start reading at
	_numRegistersMinusOne is the number of registers to read past the _address
	NOTE: the public regData array behaves like a mirror of the ADS internal arrays, 
	however, the user must update the mirror. 

    void printRegisterName(byte _address);
	this is a look up table. part of the verbose feedback and not used external to the library    //查找表：部分详细的反馈和不使用外部库

    void WREG(byte _address, byte _value); 
	writes a single value specified by _value to the register address at _address.

	
    void WREGS(byte _address, byte _numRegistersMinusOne); 
	writes a number of values to sequential addresses starting at _address
	user must first set values in correct locations of the public byte array regData
	regData is a mirror of the ADS register values

	
    void printHex(byte _data);
	used to streamline verbose feedback 

    void updateChannelData();
	the public array, channelData[8] gets updated, along with the status register.
	there is bitwise conversion from 3 byte 2's compliment to 4 byte 2's compliment (long)
    
//SPI Transfer function                                                                           //SPI传输功能，数据放在SPI总线上，从SPI总线上返回一个字节

    byte transfer(byte _data);
	puts the byte _data on the SPI bus, and returns a byte from the SPI bus.


//KNOWN ISSUES                                                                                     //已知的问题：

	verbose feedback in the updateChannelData() function bumps against the DRDY signal
	this is because 115200 baud can't keep up?

	
//USING THIS LIBRARY
	
	The START pin is not broken out. You must send the START() command instead.                               
	For daisy-chaining in the future, we will route a GPIO to the START pin and                    //菊花链中，将。。一个GPIO给start
	header socket.

	You must remeber to get out of Read Data Continuous mode if you want                          //如果你想读/写或操作寄存器，你必须退出读数据连续模式。
	to read/write, and do other register things.


