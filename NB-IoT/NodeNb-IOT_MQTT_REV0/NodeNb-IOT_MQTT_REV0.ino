//--------------------------------------include library and defines sim7000
#include <SoftwareSerial.h>  
#include <String.h>
#include <DFRobot_SIM7000.h>
#define PIN_TX     11  //pin RX sim7000 to pin 11 arduino
#define PIN_RX     10  //pin TX sim7000 to pin 10 arduino
SoftwareSerial     mySerial(PIN_RX,PIN_TX);
DFRobot_SIM7000    sim7000;     
float dailyRain = 2.2;
int centimeter = 15;
void setup()
{
  Serial.begin(19200);   //serial at 115200 baudrate
  sim7000.begin(mySerial);  //Software serial setup to sim7000
  sim7000.turnOFF();  //sim7000 turned off at first
  delay(800);
  Serial.println("Turn ON SIM7000......");
  if (sim7000.turnON()) 
  {    //Turn ON SIM7000
    Serial.println("Turn ON !");
  }

  Serial.println("Set baud rate......");
  if (sim7000.setBaudRate(19200))
  {   //Set baud rate from 115200 to 19200
    Serial.println("Set baud rate:19200");
  } 
  else 
  {
    Serial.println("Faile to set baud rate");
    while (1);
  }
  
  // Setup Serial
  while (!Serial);
  delay(800);
  Serial.begin(19200);
  
  mySerial.println("AT+CSCLK=0");  //make sure sim7000 is wake up
  delay(3000);
  ShowSerialData();
  Serial.println();
  
  mySerial.println("AT+CGATT?");   //check to get connection
  delay(3000);         
  ShowSerialData();       //if the value show 1 its mean got connection to network
  Serial.println();

  mySerial.println("AT+CSTT=\"NB1INTERNET\"");      // Replace with your APN
  delay(3000);
  ShowSerialData();

  mySerial.println("AT+CSTT?");   // Check APN
  delay(3000);
  ShowSerialData();
  Serial.println();

  mySerial.println("AT+CIICR");  // Bring up the wireless connection
  delay(3000);
  ShowSerialData();
  Serial.println();

  mySerial.println("AT+CIFSR");  // Get assigned IP adress
  delay(3000);
  ShowSerialData();
  Serial.println();

  mySerial.println("AT+CIPSPRT=0");  //send prompt
  delay(3000);
  ShowSerialData();
  Serial.println();
  

}

 
void loop() 
{
  int num;
  String le;
  String var;
  var="{\"Rain_Gauge\":"+ String(dailyRain,2) +",\"Water_Level\":" + String(centimeter)+"}";
  num=var.length();
  le=String(num);

  mySerial.println("AT+CNACT=1,\"NB1INTERNET\"");    // Start to send data to remote server
  delay(3000);
  ShowSerialData();

  mySerial.println("AT+CNACT?");    // Start to send data to remote server
  delay(3000);
  ShowSerialData();
  
  mySerial.println("AT+SMCONF=\"URL\",\"103.163.139.48\",\"1883\"");   // Start the connection to Web over TCP
  delay(3000);
  ShowSerialData();
  Serial.println();

  mySerial.println("AT+SMCONF= \"KEEPTIME\" ,60");   // Start the connection to Web over TCP
  delay(3000);
  ShowSerialData();
  Serial.println();
  
  mySerial.println("AT+SMCONN");    // Start to send data to remote server
  delay(3000);
  ShowSerialData();

  mySerial.println("AT+SMPUB= \"data\","+le+",1,1");    // Start to send data to remote server
  delay(3000);
  ShowSerialData();

  mySerial.println(var);    // Start to send data to remote server
  delay(3000);
  ShowSerialData();

  mySerial.println("AT+SMDISC");      // Close the connection
  delay(3000);
  ShowSerialData();
  
  delay(8000);
}


void ShowSerialData()
{
  while(mySerial.available()!=0)
  
    Serial.write(mySerial.read());
    
}
