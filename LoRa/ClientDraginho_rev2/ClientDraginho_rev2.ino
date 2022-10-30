

#include <SPI.h>
#include <RH_RF95.h>
RH_RF95 rf95;
const float FREQ = 923.0;

String payload="";
char databuf[10];
uint8_t dataoutgoing[10];
String payload2="";
char databuf2[10];
uint8_t dataoutgoing2[10];


#include <Wire.h>
#include <VL53L0X.h>
VL53L0X sensor;
//long centimeter, distance;
long centimeter = 12;

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
  Serial.begin(9600);

  SetupLora();
  
  /*pinMode(RainPin, INPUT);                            // set the Rain Pin as input.
  Serial.println("Rain Gauge Ready !!");

  SetupLevel();

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
 
void SetupLora(void)
{
  //while (!Serial) ; // Wait for serial port to be available
  Serial.println("Start LoRa Client");
  if (!rf95.init())
    Serial.println("init failed");
  // Setup ISM frequency
  rf95.setFrequency(FREQ);
  // Setup Power,dBm
  rf95.setTxPower(20); 
}

void loop() {
/*  unsigned long startTimer,
                stopTimer;
  t = rtc.getTime();
  mnt = t.min;
  det = t.sec;
  
 if ((bucketPositionA==false)&&(digitalRead(RainPin)==LOW)){
      bucketPositionA=true;
      dailyRain+=bucketAmount;                        // update the daily rain
      }
  if ((bucketPositionA==true)&&(digitalRead(RainPin)==HIGH)){
      bucketPositionA=false;
      }

  int distance =sensor.readRangeContinuousMillimeters();
  distance = distance/10;
  centimeter = 68-distance;

  Serial.print(rtc.getDateStr());
  Serial.print(" , ");
  Serial.print(rtc.getTimeStr());
  Serial.print(" , ");*/
  Serial.print(dailyRain,3);
  Serial.print(" , ");
  Serial.println(centimeter);
  payload = "rain" + String(dailyRain) + "level" + String(centimeter);
  //if(mnt == 10|mnt == 20||mnt == 30||mnt == 40||mnt == 50||mnt == 0){
  
  Serial.println("Sending to LoRa Server");
  // Send a message to LoRa Server
  payload +=dtostrf (centimeter, 4, 2, databuf);
  strcpy((char *)dataoutgoing, databuf);
  Serial.println(databuf);
  rf95.send(dataoutgoing, sizeof(dataoutgoing));
  rf95.waitPacketSent(); 

  payload2 +=dtostrf (dailyRain, 4, 2, databuf2);
  strcpy((char *)dataoutgoing2, databuf2);
  Serial.println(databuf2);
  rf95.send(dataoutgoing2, sizeof(dataoutgoing2));
  rf95.waitPacketSent();


  /*uint8_t data[5];
   data[0] = dailyRain;
  data[1] = centimeter;
  rf95.send(data, sizeof(data));*/
  
  // Now wait for a reply
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);
    if (rf95.waitAvailableTimeout(3000))
    { 
    // Should be a reply message for us now   
    if (rf95.recv(buf, &len))
    {
      Serial.print("got reply: ");
      Serial.println((char*)buf);
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);    
    }
    else
    {
      Serial.println("recv failed");
    }
  }
  else
  {
    Serial.println("No reply, is LoRa server running?");
  }
delay(5000);
//}
}
