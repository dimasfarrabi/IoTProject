#include <SoftwareSerial.h>
#include <String.h>
#include <DFRobot_SIM7000.h>
#define PIN_TX     11
#define PIN_RX     10
SoftwareSerial     mySerial(PIN_RX,PIN_TX);
DFRobot_SIM7000    sim7000;
String payload = "";

void setup(){
  Serial.begin(115200);
  sim7000.begin(mySerial);
  sim7000.turnOFF();
  delay(5000);
  Serial.println("Turn ON SIM7000......");
  if (sim7000.turnON()) {                           //Turn ON SIM7000
    Serial.println("Turn ON !");
  }

  Serial.println("Set baud rate......");
  if (sim7000.setBaudRate(19200)) {                 //Set baud rate from 115200 to 19200
    Serial.println("Set baud rate:19200");
  } else {
    Serial.println("Faile to set baud rate");
    while (1);
  }
  
  // Setup Serial
  while (!Serial);
  delay(2000);
  Serial.begin(115200);
  
  mySerial.println("AT+CSCLK=0");
  delay(3000);
  ShowSerialData();
  Serial.println();
  
  mySerial.println("AT+CGATT?");
  delay(3000);
  ShowSerialData();
  Serial.println();

  mySerial.println("AT+CSTT=\"NB1INTERNET\"");      // Replace with your APN
  delay(3000);
  ShowSerialData();

  mySerial.println("AT+CSTT?");                     // Check APN
  delay(3000);
  ShowSerialData();
  Serial.println();

  mySerial.println("AT+CIICR");                     // Bring up the wireless connection
  delay(3000);
  ShowSerialData();
  Serial.println();

  mySerial.println("AT+CIFSR");                     // Get assigned IP adress
  delay(3000);
  ShowSerialData();
  Serial.println();

  mySerial.println("AT+CIPSPRT=0");
  delay(3000);
  ShowSerialData();
  Serial.println();
  
}

 
void loop() 
{   
  SendTo();
}

void SendTo()
{
  float dailyRain = 1.0;
  int centimeter  = 3;   
  Serial.println();
  Serial.println("----------- Read Sensors ------------");
  Serial.println("Rain is " + String(dailyRain,2) + "mm");
  Serial.println("Level is " + String(centimeter) + " cm");
  Serial.println("----------------------------------------");
  Serial.println();

  payload = "rain="+String(dailyRain,2) + "&level="+ String(centimeter);
  
  int num;
  String le;
  String var;
  //var="{\"rain\"="+ String(dailyRain,2) +"&\"level\"=" + String(centimeter)+"}";
  num=payload.length();
  le=String(num);
  
  mySerial.println("AT+CIPSTART=\"tcp\",\"103.147.154.61\",\"80\"");              // Start the connection to Ubidots
  delay(1000);
  ShowSerialData();
  Serial.println();
  
  mySerial.println("AT+CIPSEND ");                                              // Start to send data to remote server
  delay(100);
  ShowSerialData();
  mySerial.print("POST /data/input/node1.php HTTP/1.1\r\n");
  delay(100);
  ShowSerialData();
  mySerial.print("Host: banjir.polines-longsor.com\r\n");
  delay(100);
  ShowSerialData();
  mySerial.print("Content-Type: application/x-www-form-urlencoded;\r\n");
  delay(100);
  ShowSerialData();
  mySerial.print("Content-Length:"+le+"\r\n\r\n");
  delay(100);
  ShowSerialData();
  mySerial.print(payload);
  delay(100);
  ShowSerialData();
  mySerial.print("\r\n");
  delay(100);
  ShowSerialData();
  mySerial.println((char)26);                            // Sending the HTTP request
  delay(5000);                                           // Waiting for a reply. Important! tweak this time depending on the latency of your mobile Internet connection
  mySerial.println();
  ShowSerialData();

  mySerial.println("\r\n");
  mySerial.println("AT+CIPCLOSE");                      // Close the connection
  delay(100);
  ShowSerialData();
}

void ShowSerialData()
{
  while(mySerial.available()!=0)
  
    Serial.write(mySerial.read());
    
}
