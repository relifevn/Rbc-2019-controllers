/*
 * Refs:
 * Simple web server: http://arduino-er.blogspot.com/2015/05/arduino-esp8266-simpe-web-server.html
 * 
 * [SETTING MODE]
 * ESP8266 | ARDUINO
 * RX      | RX
 * TX      | TX
 * GND     | GND
 * VCC     | 3.3
 * CH_PD(EN)| 3.3
 * GPIO 0  | None
 * GPIO 2  | None
 * 
 * ARDUINO | ARDUINO
 * Reset   | GND
 * 
 * 
 * [RUNNING MODE]
 * ESP8266 | ARDUINO
 * RX      | 3 
 * TX      | 2
 * GND     | GND
 * VCC     | 3.3
 * CH_PD(EN)| 3.3
 * GPIO 0  | None
 * GPIO 2  | None
 * 
 * AT Command : http://absaransari.com/2018/01/24/setting-up-esp8266-wifi-mofule-using-at-commands-set/
 * 
 * 0: backward
 * 1: stop
 * 2: forward
 * 
 * 
 * 
 */

#include <SoftwareSerial.h>
 
#define DEBUG true
#define TOKEN_ID "DyEnergy19"

#define MIN_SPEED 0
#define MAX_SPEED 255

#define LEFT_HEAD_PIN_D 7
#define LEFT_HEAD_PIN_A 6
#define RIGHT_HEAD_PIN_D 5
#define RIGHT_HEAD_PIN_A 4
 
SoftwareSerial esp8266(2,3); // make RX Arduino line is pin 2, make TX Arduino line is pin 3.
                             // This means that you need to connect the TX line from the esp to the Arduino's pin 2
                             // and the RX line from the esp to the Arduino's pin 3


//String strHTML = "<!doctype html>\
//<html>\
//<head>\
//<title>arduino-er</title>\
//</head>\
//<body>\
//<H1>arduino-er.blogspot.com</H1>\
//</body>\
//</html>";
String strJSONSuccess = "{\"status\":\"OK\"}";
String strJSONFailure = "{\"status\":\"InvalidData\"}";


void setup()
{
  Serial.begin(9600);
  esp8266.begin(9600); // your esp's baud rate might be different

  initializePin(LEFT_HEAD_PIN_D);
  initializePin(LEFT_HEAD_PIN_A);
  initializePin(RIGHT_HEAD_PIN_D);
  initializePin(RIGHT_HEAD_PIN_A);
   
  sendData("AT+RST\r\n",2000,DEBUG); // reset module
  sendData("AT+CWMODE=2\r\n",1000,DEBUG); // configure as access point
  sendData("AT+CIPAP=\"192.168.10.1\"\r\n",3000,DEBUG); // check
  String setWifi="AT+CWSAP_DEF=\"DYNAMIC_ENERGY\",\"tomorrow\",";
  setWifi += String(random(2,10));
  setWifi += ",3,4,0\r\n";
  sendData(setWifi,2000,DEBUG); // Note: where 1=channel, 3=WPA2_PSK, 4=Max Connections, 0=SSID is broadcasted
  sendData("AT+CWSAP?\r\n",3000,DEBUG); // check
  sendData("AT+CIFSR\r\n",1000,DEBUG); // get ip address
  sendData("AT+CIPMUX=1\r\n",1000,DEBUG); // configure for multiple connections
  sendData("AT+CIPSERVER=1,80\r\n",1000,DEBUG); // turn on server on port 80
  
  Serial.println("[INFO] Set up. Done!");
}

void loop()
{
  if(esp8266.available()) // check if the esp is sending a message 
  {
    Serial.println("[INFO] ESP8266 is available!");
    if(esp8266.find("+IPD,"))
    {
      Serial.println("[INFO] Found +IPD"); 
      delay(1000); // wait for the serial buffer to fill up (read all the serial data)
      // get the connection id so that we can then disconnect
      int connectionId = esp8266.read()-48; // subtract 48 because the read() function returns 
                                           // the ASCII decimal value and 0 (the first decimal number) starts at 48
      Serial.print("[DEBUG] Connect Id :"); Serial.println(connectionId);    
      
      // Read tokenId
      String tokenId=TOKEN_ID;
//      esp8266.find("tokenId="); 
//      for(int i=0; i<10; i++){
//        char c = esp8266.read(); // read 1 character.
//        tokenId+=c;
//      }

      //http://192.168.10.1/?tokenId=DyEnergy19&leftHead=0&rightHead=0&leftTail=0&rightTail=0&stopCommand=0
      //http://192.168.10.1/?lH=0&rH=0&lT=0&rT=0&sC=0
      // For debug
//      while (esp8266.available() > 0) {
//        char a = esp8266.read();
//        Serial.write(a);
//      }

      esp8266.find("lH="); 
      int leftHead = esp8266.read()-48; 

      esp8266.find("rH="); 
      int rightHead = esp8266.read()-48; 
      
      esp8266.find("lT="); 
      int leftTail = esp8266.read()-48; 

      esp8266.find("rT="); 
      int rightTail = esp8266.read()-48; 

      esp8266.find("sC="); 
      int stopCommand= esp8266.read()-48; 

      if(checkValidData(tokenId, leftHead, rightHead, leftTail, rightTail, stopCommand)){
        Serial.println("[DEBUG] Valid Data received !");
        motorController(LEFT_HEAD_PIN_D, LEFT_HEAD_PIN_A, MAX_SPEED, leftHead);
        motorController(RIGHT_HEAD_PIN_D, RIGHT_HEAD_PIN_A, MAX_SPEED, rightHead);
        sendHTTPResponse(connectionId, strJSONSuccess);
      }else{
        Serial.println("[DEBUG] Invalid Data received !");
        Serial.print("[INFO] tokenId = ");
        Serial.println(tokenId);
        Serial.print("[INFO] leftHead= ");
        Serial.println(leftHead);
        Serial.print("[INFO] rightHead= ");
        Serial.println(rightHead);
        Serial.print("[INFO] leftTail= ");
        Serial.println(leftTail);
        Serial.print("[INFO] rightTail= ");
        Serial.println(rightTail);
        Serial.print("[INFO] stopCommand= ");
        Serial.println(stopCommand);
        sendHTTPResponse(connectionId, strJSONFailure);
      }
           
      
    }
  }else{
    //Serial.println("ESP8266 is unavailable!"); 
  }
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
}

bool checkValidData(String tokenId, int leftHead, int rightHead, int leftTail, int rightTail, int stopCommand){
  bool isValid=true;
  if(tokenId!=TOKEN_ID) isValid=false;
  if(leftHead<0 || leftHead>=3) isValid=false;
  if(rightHead<0 || rightHead>=3) isValid=false;
  if(leftTail<0 || leftTail>=3) isValid=false;
  if(rightTail<0 || rightTail>=3) isValid=false;
  if(stopCommand!=0 && stopCommand!=1) isValid=false;
  return isValid;
}

int readDigits(int digit){
  int val=0;
  for(int i=0; i<digit; i++){
    val += (esp8266.read()-48)*10*(digit-i-1);
  }
//  int pinNumber = (esp8266.read()-48)*10; // get first number i.e. if the pin 13 then the 1st number is 1, then multiply to get 10
//  pinNumber += (esp8266.read()-48); // get second number, i.e. if the pin number is 13 then the 2nd number is 3, then add to the first number
  return val;
}

void togglePin(int pin){
  digitalWrite(pin, !digitalRead(pin)); // toggle pin 
}
 


//Send command to ESP8266, assume OK, no error check
//wait some time and display respond
void sendESP8266Cmdln(String cmd, int waitTime)
{
  esp8266.println(cmd);
  delay(waitTime);
  clearESP8266SerialBuffer();
}

//Basically same as sendESP8266Cmdln()
//But call ESP8266.print() instead of call ESP8266.println()
void sendESP8266Data(String data, int waitTime)
{
  esp8266.print(data);
  delay(waitTime);
  clearESP8266SerialBuffer();
}


//Clear and display Serial Buffer for ESP8266
void clearESP8266SerialBuffer()
{
  Serial.println("[DEBUG] Clearing Serial Buffers ...");
  while (esp8266.available() > 0) {
    char a = esp8266.read();
    Serial.write(a);
  }
  Serial.println("\n[DEBUG] Clear Serial Buffers. Done!");
}


void sendHTTPResponse(int id, String content)
{
  String response;
  response = "HTTP/1.1 200 OK\r\n";
  //response += "Content-Type: text/html; charset=UTF-8\r\n"; 
  response += "Content-Type: application/json; charset=UTF-8\r\n";
  response += "Content-Length: ";
  response += content.length();
  response += "\r\n";
  response +="Connection: close\r\n\r\n";
  response += content;

  String cmd = "AT+CIPSEND=";
  cmd += id;
  cmd += ",";
  cmd += response.length();
  
  Serial.println("[DEBUG] Send HTTP response: "+cmd);
  sendESP8266Cmdln(cmd, 2000);
  
  Serial.println("[DEBUG] Response from ESP8266: "+response);
  sendESP8266Data(response, 2000);
}

void initializePin(int pin){
  pinMode(pin,OUTPUT); // set to output 
  digitalWrite(pin,LOW); // stop working
}
 
/*
* Name: sendData
* Description: Function used to send data to ESP8266.
* Params: command - the data/command to send; timeout - the time to wait for a response; debug - print to Serial window?(true = yes, false = no)
* Returns: The response from the esp8266 (if there is a reponse)
*/
String sendData(String command, const int timeout, boolean debug)
{
    String response = "";
    
    esp8266.print(command); // send the read character to the esp8266
    
    long int time = millis();
    
    while( (time+timeout) > millis())
    {
      while(esp8266.available())
      {
        
        // The esp has data so display its output to the serial window 
        char c = esp8266.read(); // read the next character.
        response+=c;
      }  
    }
    
    if(debug)
    {
      Serial.print("[DEBUG] Response: ");
      Serial.println(response);
    }
    
    return response;
}
