/*
 * 
 * RF Send
 * https://www.youtube.com/watch?v=D40cgHyBLL4
 * http://educ8s.tv/nrf24l01/
 * 
 * https://www.instructables.com/id/Interface-Python-and-Arduino-with-pySerial/
 * 
 */

#include <SPI.h>  
#include "RF24.h"

RF24 myRadio (7, 8);

// address to be transmitted, make sure it is the same 
byte addresses[][6] = {"0"};
struct package
{
  /*
   * 0: backward
   * 1: stop
   * 2: forward
   */
  int id=0;
  int lH=1;
  int rH=1;
  int lT=1;
  int rT=1;
};


typedef struct package Package;
Package data;

int SIZE_DATA;


void setup() {
  Serial.begin(57600); // set the baud rate
  Serial.println("Ready"); // print "Ready" once

  myRadio.begin();  

  // set channel = 115
  myRadio.setChannel(115); 
  
  // use the maximum transmitting power that the module offers 
  // use more power but extends the range of the communication
  myRadio.setPALevel(RF24_PA_MAX); 

  // Data Rate : 250Kbps
  myRadio.setDataRate( RF24_250KBPS ) ; 

  // open the pipe in order to write to it later 
  myRadio.openWritingPipe( addresses[0]);

  SIZE_DATA = sizeof(data);
  
  delay(1000);
}

void loop() {
  if(Serial.available()){
    while(Serial.available()){ // only send data back if data has been sent
      char c = Serial.read(); // read the incoming data
      if(c == 'P'){
        bool isValid=true;
        while(Serial.available()){
          int lH = Serial.read() - '0';
          int rH = Serial.read() - '0';
          int lT = Serial.read() - '0';
          int rT = Serial.read() - '0';
          if(lH < 0 && lH >2) isValid=false;
          if(rH < 0 && rH >2) isValid=false;
          if(lT < 0 && lT >2) isValid=false;
          if(rT < 0 && rT >2) isValid=false;
          if(isValid){
            data.lH = lH;
            data.rH = rH;
            data.lT = lT;
            data.rT = rT;
            myRadio.write(&data, SIZE_DATA); 
            Serial.println("Valid Data. Sent");
            data.id += 1;
          }
        }
      }
    }
  }
  
  
  delay(100); // delay for 1/10 of a second
}
