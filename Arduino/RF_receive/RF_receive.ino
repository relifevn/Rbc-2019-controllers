
/*
 * 
 * RF Receive 
 * https://www.youtube.com/watch?v=D40cgHyBLL4
 * http://educ8s.tv/nrf24l01/
 * 
 */

#include <SPI.h>  
#include "RF24.h" 

#define MIN_SPEED 0
#define MAX_SPEED 255

#define LEFT_HEAD_PIN_D 6
#define LEFT_HEAD_PIN_A 5
#define RIGHT_HEAD_PIN_D 4
#define RIGHT_HEAD_PIN_A 3
 
RF24 myRadio (7, 8); 
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

//byte addresses[][6] = {"0"}; 
const byte addresses[6] = {'R','E','L','I','F','E'};



typedef struct package Package;
Package data;


void initializePin(int pin){
  pinMode(pin,OUTPUT); // set to output 
  digitalWrite(pin,LOW); // stop working
}
 
void motorController(int pinD, int pinA, int speed, int state){
  if(state==0){
      // backward
      speed=constrain(speed, MIN_SPEED, MAX_SPEED); // make sure value is from MIN_SPEED to MAX_SPEED
      digitalWrite(pinD, LOW);
      analogWrite(pinA, speed);
  }else if(state==2){
      // forward
      speed=constrain(speed, MIN_SPEED, MAX_SPEED); // make sure value is from MIN_SPEED to MAX_SPEED
      digitalWrite(pinD, HIGH);
      analogWrite(pinA, 255-speed);
  }else{
      // stop
      digitalWrite(pinD, LOW);
      analogWrite(pinA, LOW);
  }
  Serial.println("[] Controlled!");
}

void setup() 
{
  Serial.begin(57600);
  delay(1000);
  
  myRadio.begin(); 
  myRadio.setChannel(115); 
  myRadio.setPALevel(RF24_PA_MAX);
  myRadio.setDataRate( RF24_250KBPS ) ; 
  myRadio.openReadingPipe(1, addresses[0]);
  myRadio.startListening();
  
  initializePin(LEFT_HEAD_PIN_D);
  initializePin(LEFT_HEAD_PIN_A);
  initializePin(RIGHT_HEAD_PIN_D);
  initializePin(RIGHT_HEAD_PIN_A);

  Serial.println("Ready");
  
}


void loop()  
{

  if ( myRadio.available()) 
  {
    while (myRadio.available())
    {
      myRadio.read( &data, sizeof(data) );
    }
    Serial.print("\nPackage:");
    Serial.println(data.id);
    
    Serial.print("lH = ");
    Serial.println(data.lH);
    Serial.print("rH = ");
    Serial.println(data.rH);
    Serial.print("lT = ");
    Serial.println(data.lT);
    Serial.print("rT = ");
    Serial.println(data.rT);

    motorController(LEFT_HEAD_PIN_D, LEFT_HEAD_PIN_A, MAX_SPEED, data.lH);
    motorController(RIGHT_HEAD_PIN_D, RIGHT_HEAD_PIN_A, MAX_SPEED, data.rH);
  }

}
