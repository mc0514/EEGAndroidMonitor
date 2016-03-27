/*
  Serial transform  example
 
 When new serial data arrives, this sketch adds it to a String.
 When a newline is received, the loop prints the string and 
 clears it.
 
 A good test for this is to try it with a android EEG monitor by
 bluetooth. 
 
 Created 27 Match 2016
 by Chao Mi
 

 */

boolean beginStep=false;
boolean startSend=false;
String comdata = "";

void setup() {
 
  Serial.begin(115200);

   
}

void loop() {
  
    //generate a random interger num to simulate the EEG data
    int sendData=random(20, 60);
    //trans to string
    String sendStr=String(sendData);
    // while the serial has data arrived, got all the data in a string  
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
          beginStep=true;
        }
        // if received the data "S",stop to send data
        if(comdata=="S")
        {
        //Serial.println(comdata);
        comdata = "";
        startSend=false;
        }
        // if received the data "*", each step start to send
        if(comdata=="*")
        {
          comdata="";
          beginStep=true;
        }
    }
    //when the start send flag and the each step send flag was true
    // start to send 
    if(startSend==true)
    {
        if(beginStep==true) 
        {
          Serial.println(sendStr);
          //comdata = "";
          beginStep=false;
        }
    }
     
}


