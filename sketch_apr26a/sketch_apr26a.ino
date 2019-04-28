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
 * 
 */

#include <SoftwareSerial.h>
 
#define DEBUG true
 
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
String strJSON = "{\"status\":\"OK\"}";

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

void setup()
{
  Serial.begin(9600);
  esp8266.begin(9600); // your esp's baud rate might be different
  
  pinMode(11,OUTPUT);
  digitalWrite(11,LOW);
  
  pinMode(12,OUTPUT);
  digitalWrite(12,LOW);
  
  pinMode(13,OUTPUT);
  digitalWrite(13,LOW);
   
  sendData("AT+RST\r\n",2000,DEBUG); // reset module
  sendData("AT+CWMODE=2\r\n",1000,DEBUG); // configure as access point
  sendData("AT+CIPAP=\"192.168.10.1\"\r\n",3000,DEBUG); // check
  sendData("AT+CWSAP_DEF=\"DYNAMIC_ENERGY\",\"tomorrow\",1,3,4,0\r\n",2000,DEBUG); // Note: where 1=channel, 3=WPA2_PSK, 4=Max Connections, 0=SSID is broadcasted
  sendData("AT+CWSAP?\r\n",3000,DEBUG); // check
  sendData("AT+CIFSR\r\n",1000,DEBUG); // get ip address
  sendData("AT+CIPMUX=1\r\n",1000,DEBUG); // configure for multiple connections
  sendData("AT+CIPSERVER=1,80\r\n",1000,DEBUG); // turn on server on port 80
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
     esp8266.find("pin="); // advance cursor to "pin="
     
     int pinNumber = (esp8266.read()-48)*10; // get first number i.e. if the pin 13 then the 1st number is 1, then multiply to get 10
     pinNumber += (esp8266.read()-48); // get second number, i.e. if the pin number is 13 then the 2nd number is 3, then add to the first number

     Serial.print("[DEBUG] Pin number :"); Serial.println(pinNumber);
     digitalWrite(pinNumber, !digitalRead(pinNumber)); // toggle pin    
     
     // make close command
     String closeCommand = "AT+CIPCLOSE="; 
     closeCommand+=connectionId; // append connection id
     closeCommand+="\r\n";

     String res = "AT+CIPSEND=";
     res += connectionId;
     res += ",30";
     res += "\r\nHTTP/1.1 200 OK\nContent-Type: application/json\n{\"title\":\"ok\"}\r\n";
     //sendData(res,1000,DEBUG); // send json response
     //sendHTTPResponse(connectionId, strHTML);
     sendHTTPResponse(connectionId, strJSON);
     //sendData(closeCommand,1000,DEBUG); // close connection
     //sendESP8266Cmdln(closeCommand, 1000);
    }
  }else{
    //Serial.println("ESP8266 is unavailable!"); 
  }
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
