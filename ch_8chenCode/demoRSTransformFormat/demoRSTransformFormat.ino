
/*
  Serial Event example
 
 When new serial data arrives, this sketch adds it to a String.
 When a newline is received, the loop prints the string and 
 clears it.
 
 A good test for this is to try it with a GPS receiver 
 that sends out NMEA 0183 sentences. 
 
 Created 9 May 2011
 by Tom Igoe
 
 This example code is in the public domain.
 
 http://www.arduino.cc/en/Tutorial/SerialEvent
 
 */
 
#include<ADS1299Manager.h>  
ADS1299Manager ADSManager;

#define N_CHANNELS_PER_OPENBCI (8) 
#define MAX_N_CHANNELS (N_CHANNELS_PER_OPENBCI) 
int nActiveChannels = MAX_N_CHANNELS;

long int sampleCounter = 0 ;

String comdata="";
boolean startSend=false;

void setup() {
  // initialize serial:
  Serial.begin(115200);

}

void loop() {
  
  
  while (Serial.available() > 0)  
    {
        comdata += char(Serial.read());
        delay(2);
   }
   // if got the string data
    if (comdata.length() > 0)
    {
        // if it is the first time received with the data "A"
       // set the start data flag and the each step flag to true 
        if(comdata=="A")
        {
          //Serial.print("5");
          comdata = "";
          startSend=true;
        }
        // if received the data "S",stop to send data
        if(comdata=="S")
        {
        //Serial.println(comdata);
        comdata = "";
        startSend=false;
        }
    }
        
    if(startSend==true)
    {
 
      ADSManager.writeChannelDataAsBinaryToAndroid(MAX_N_CHANNELS,sampleCounter);
      sampleCounter++;
    }
    else
    {
      sampleCounter=0;
    }

  
}



