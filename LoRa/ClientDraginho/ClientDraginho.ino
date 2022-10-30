#include <SoftwareSerial.h>
#include <String.h>
#include <DFRobot_SIM7000.h>
#define PIN_TX     12
#define PIN_RX     11
SoftwareSerial     mySerial(PIN_RX,PIN_TX);
DFRobot_SIM7000    sim7000;
String payload = "";
#include <Wire.h>
#include <VL53L0X.h>
VL53L0X sensor;
long centimeter, distance;

#define RainPin 4
bool bucketPositionA = false;                       // one of the two positions of tipping-bucket               
const double bucketAmount = 0.516;                  // 0.516 mm of rain equivalent of ml to trip tipping-bucket  
double dailyRain = 11.0;                             // rain accumulated for the day
double hourlyRain = 0.0;                            // rain accumulated for one hour
double dailyRain_till_LastHour = 0.0;               // rain accumulated for the day till the last hour          
bool first;                                        // as we want read

#include <DS3231.h>
DS3231  rtc(SDA, SCL);// RTC pin
Time  t;
int mnt;
int det;

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
  
  pinMode(RainPin, INPUT);                            // set the Rain Pin as input.
  Serial.println("Rain Gauge Ready !!");

  /*SetupLevel();

  rtc.begin();
  rtc.setTime(12, 9, 20);// Set Jam (24jam)
  rtc.setDate(19 , 3, 2020);// Set Tanggal*/
}

/*void SetupLevel(void)
{
  Wire.begin();
  sensor.init();
  sensor.setTimeout(500);
  sensor.startContinuous();
}*/
 
void loop() {
  unsigned long startTimer,
                stopTimer;
  /*t = rtc.getTime();
  mnt = t.min;
  det = t.sec;*/
  
 if ((bucketPositionA==false)&&(digitalRead(RainPin)==LOW)){
      bucketPositionA=true;
      dailyRain+=bucketAmount;                        // update the daily rain
      }
  if ((bucketPositionA==true)&&(digitalRead(RainPin)==HIGH)){
      bucketPositionA=false;
      }

  //int distance =sensor.readRangeContinuousMillimeters();
  distance = distance/10;
  centimeter = 12; //68-distance;

  /*Serial.print(rtc.getDateStr());
  Serial.print(" , ");
  Serial.print(rtc.getTimeStr());
  Serial.print(" , ");*/
  Serial.print(dailyRain,3);
  Serial.print(" , ");
  Serial.println(centimeter);
  
  
  //if(mnt == 10|mnt == 20||mnt == 30||mnt == 40||mnt == 50||mnt == 0){
    SendTo();
delay(8000);
  //}
}

void SendTo()
{
        
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
