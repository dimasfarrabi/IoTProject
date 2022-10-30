//--------------------------------------include library and defines sim7000
#include <SoftwareSerial.h>  
#include <String.h>
#include <DFRobot_SIM7000.h>
#define PIN_TX     11  //pin RX sim7000 to pin 11 arduino
#define PIN_RX     10  //pin TX sim7000 to pin 10 arduino
SoftwareSerial     mySerial(PIN_RX,PIN_TX);
DFRobot_SIM7000    sim7000;
String payload = "";

//--------------------------------------defines jsn srt04
const int trigPin = 2;  //jsn pin to arduino
const int echoPin = 3;
long duration;
int distance, level;
int ketinggian;
  
//--------------------------------------difines Tipping Bucket
#define RainPin 4         //at pin4
bool bucketPositionA = false;     // one of the two positions of tipping-bucket               
const double bucketAmount = 0.516;   // 0.516 mm of rain equivalent of mm to trip tipping-bucket  
double dailyRain = 0.0;   // rain accumulated for the day          
bool first;   // as we want read

//---------------------------------------include library and defines RTC DS3231
#include <DS3231.h>
DS3231  rtc(SDA, SCL);    // RTC pin
Time  t;
int mnt;
int det;

void setup()
{
  Serial.begin(115200);   //serial at 115200 baudrate
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
  Serial.begin(115200);
  
  mySerial.println("AT+CSCLK=0");  //make sure sim7000 is wake up
  delay(800);
  ShowSerialData();
  Serial.println();
  
  mySerial.println("AT+CGATT?");   //check to get connection
  delay(800);         
  ShowSerialData();       //if the value show 1 its mean got connection to network
  Serial.println();

  mySerial.println("AT+CSTT=\"NB1INTERNET\"");      // Replace with your APN
  delay(800);
  ShowSerialData();

  mySerial.println("AT+CSTT?");   // Check APN
  delay(800);
  ShowSerialData();
  Serial.println();

  mySerial.println("AT+CIICR");  // Bring up the wireless connection
  delay(800);
  ShowSerialData();
  Serial.println();

  mySerial.println("AT+CIFSR");  // Get assigned IP adress
  delay(800);
  ShowSerialData();
  Serial.println();

  mySerial.println("AT+CIPSPRT=0");  //send prompt
  delay(800);
  ShowSerialData();
  Serial.println();
  
  pinMode(RainPin, INPUT);     // set the Rain Pin as input.
  Serial.println("Rain Gauge Ready !!");
  
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input

  rtc.begin();
  rtc.setTime(12, 9, 20);    // Set time (24Hr)
  rtc.setDate(19 , 3, 2020);   // Set date
}
 
void loop() 
{
  t = rtc.getTime();
  mnt = t.min;
  det = t.sec;
 
 if ((bucketPositionA==false)&&(digitalRead(RainPin)==LOW)){  //read Tipping Bucket
      bucketPositionA=true;
      dailyRain+=bucketAmount;                        // update the daily rain
      }
  if ((bucketPositionA==true)&&(digitalRead(RainPin)==HIGH)){
      bucketPositionA=false;
      }

  Serial.print(rtc.getDateStr());  //print date
  Serial.print(" , ");
  Serial.print(rtc.getTimeStr());  //print time
  Serial.print(" , ");
  Serial.println(dailyRain,3);  //print Rain Gauge value
  
  if(mnt == 10|mnt == 20||mnt == 30||mnt == 40||mnt == 50||mnt == 0) 
  {
    SendTo();
  }
  if((mnt == 11 && det == 8)||(mnt== 21 && det == 8)||(mnt == 31 && det == 8)||(mnt == 41 && det == 8)||(mnt == 51 && det == 8)||(mnt == 1 && det == 8))
  {
    dailyRain = 0.0;
  }
}

void SendTo()
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);  
  digitalWrite(trigPin, HIGH); // Sets the trigPin on HIGH state for 10 micro seconds
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW); // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  distance = duration*0.034/2;  // Calculating the distance
  level = 67-distance;
  if (level < 67 && level > 0)
  {
   ketinggian = level;
  }
        
  Serial.println();
  Serial.println("----------- Read Sensors ------------");
  Serial.println("Rain is " + String(dailyRain,2) + "mm");
  Serial.println("Level is " + String(ketinggian) + " cm");
  Serial.println("----------------------------------------");
  Serial.println();

  payload = "rain="+String(dailyRain,2) + "&level="+ String(ketinggian);  //print data payload structure
  
  int num;
  String le;
  String var;
  num=payload.length();   //payload length
  le=String(num);
  
  mySerial.println("AT+CIPSTART=\"tcp\",\"103.147.154.61\",\"80\"");   // Start the connection to Web over TCP
  delay(1000);
  ShowSerialData();
  Serial.println();
  
  mySerial.println("AT+CIPSEND ");    // Start to send data to remote server
  delay(100);
  ShowSerialData();
  mySerial.print("POST /data/input/node1.php HTTP/1.1\r\n");  //HTTP POST to file target
  delay(100);
  ShowSerialData();
  mySerial.print("Host: banjir.polines-longsor.com\r\n");  //Server HOST
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
  mySerial.println((char)26);     // Sending the HTTP request
  delay(5000);                    // Waiting for a HTTP Response
  mySerial.println();
  ShowSerialData();

  mySerial.println("\r\n");
  mySerial.println("AT+CIPCLOSE");      // Close the connection
  delay(100);
  ShowSerialData();
}

void ShowSerialData()
{
  while(mySerial.available()!=0)
  
    Serial.write(mySerial.read());
    
}
