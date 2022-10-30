//---------------------------------------------defines GSM and Serial Modem
#define TINY_GSM_MODEM_SIM7000

#include <SoftwareSerial.h>
SoftwareSerial SerialAT(11, 10); // RX, TX
SoftwareSerial mySerial(12,13); // RX, TX
#define SerialMon Serial  // Set serial for debug console (to the Serial Monitor, default speed 115200)

#define TINY_GSM_DEBUG SerialMon // Define the serial console for debug prints, if needed

#define GSM_AUTOBAUD_MIN 9600  // Range to attempt to autobaud
#define GSM_AUTOBAUD_MAX 115200

#define TINY_GSM_USE_GPRS true
#define TINY_GSM_USE_WIFI false  // Define how you're planning to connect to the internet

#define GSM_PIN ""  // set GSM PIN, if any

const char apn[] = "NB1INTERNET";  // Your GPRS credentials, if any

#include <TinyGsmClient.h>

#if TINY_GSM_USE_GPRS && not defined TINY_GSM_MODEM_HAS_GPRS   // Just in case someone defined the wrong thing..
#undef TINY_GSM_USE_GPRS
#undef TINY_GSM_USE_WIFI
#define TINY_GSM_USE_GPRS false
#define TINY_GSM_USE_WIFI true
#endif
#if TINY_GSM_USE_WIFI && not defined TINY_GSM_MODEM_HAS_WIFI
#undef TINY_GSM_USE_GPRS
#undef TINY_GSM_USE_WIFI
#define TINY_GSM_USE_GPRS true
#define TINY_GSM_USE_WIFI false
#endif
//---------------------------------------------MQTT details
const char* broker = "103.163.139.48";
const char* clientID = "";
const char* username = "";
const char* passwd = "";

//const char* topicLed = "GsmClientTest/led";
const char* topicInit = "data";

#include <PubSubClient.h>
#ifdef DUMP_AT_COMMANDS
  #include <StreamDebugger.h>
  StreamDebugger debugger(SerialAT, SerialMon);
  TinyGsm modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif
TinyGsmClient client(modem);
PubSubClient mqtt(client);


#include <DS3231.h>
DS3231  rtc(SDA, SCL);// RTC pin
Time  t;
int mnt;
int det;

unsigned char data[4]={};
float distance;
float jarak;
uint32_t lastReconnectAttempt = 0;

#define RainPin 2
bool bucketPositionA = false;                       // one of the two positions of tipping-bucket               
const double bucketAmount = 0.516;                  // 0.516 mm of rain equivalent of ml to trip tipping-bucket  
double dailyRain = 0.0;                             // rain accumulated for the day
double hourlyRain = 0.0;                            // rain accumulated for one hour
double dailyRain_till_LastHour = 0.0;               // rain accumulated for the day till the last hour          
bool first;                                        // as we want 


boolean mqttConnect() {
  SerialMon.print("Connecting to ");
  SerialMon.print(broker);
  boolean status = mqtt.connect("URL",broker,"1883");  // Connect to MQTT Broker
  if (status == false) {
    SerialMon.println(" fail");
    return false;
  }
  SerialMon.println(" success");
  return mqtt.connected();
}

void reconnect() {
  while (!mqtt.connected()) {
    Serial.print("Attempting MQTT connection...");
      if (mqttConnect()) {
        lastReconnectAttempt = 0;
      }
    delay(100);
  }
}

void setup() {
  // Set console baud rate
  SerialMon.begin(115200);
  delay(10);
  SerialMon.println("Wait...");
  SerialAT.begin(19200);
  delay(1000);
  SerialAT.listen();

  SerialMon.println("Initializing modem...");
  modem.restart();
  // modem.init();

  String modemInfo = modem.getModemInfo();
  SerialMon.print("Modem Info: ");
  SerialMon.println(modemInfo);

  SerialMon.println(" success");

  SerialMon.print("Waiting for network...");
  if (!modem.waitForNetwork()) {
    SerialMon.println(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(" success");

  if (modem.isNetworkConnected()) {
    SerialMon.println("Network connected");
  }

#if TINY_GSM_USE_GPRS
  // GPRS connection parameters are usually set after network registration
    SerialMon.print(F("Connecting to "));
    SerialMon.print(apn);
    if (!modem.gprsConnect(apn)) {
      SerialMon.println(" fail");
      delay(10000);
      return;
    }
    SerialMon.println(" success");

  if (modem.isGprsConnected()) {
    SerialMon.println("GPRS connected");
  }
#endif

  // MQTT Broker setup
  mqtt.setServer(broker, 1883);


  rtc.begin();
  rtc.setTime(13, 9, 40);    // Set time (24Hr)
  rtc.setDate(7 , 11, 2021);   // Set date
  pinMode(RainPin, INPUT);                            // set the Rain Pin as input.
  Serial.println("Rain Gauge Ready !!");              // rain gauge measured per 1 hour      
  mySerial.begin(9600);
}


void BacaKetinggianAir(){
  mySerial.listen();
      do{
     for(int i=0;i<4;i++)
     {
       data[i]=mySerial.read();
     }
  }while(mySerial.read()==0xff);

  mySerial.flush();

  if(data[0]==0xff)
    {
      int sum;
      sum=(data[0]+data[1]+data[2])&0x00FF;
      if(sum==data[3])
      {
        distance=(data[1]<<8)+data[2];
        if(distance>280)
          {
           Serial.print("distance=");
           jarak = distance/10;
           Serial.print(jarak);
           Serial.println("cm");
          }else 
              {
                Serial.println("Below the lower limit");        
              }
      }else Serial.println("ERROR");
     }
     delay(150);
  
}


void kirimData(){
    SerialAT.listen(); 
    if (!mqtt.connected()) {
    reconnect();
  }
  mqtt.loop();
  String packet = "";
 
  packet.concat(("{\"Rain_Gauge\":"));
  packet.concat(dailyRain);
 
  packet.concat((",\"Water_Level\":"));
  packet.concat(jarak);
 
  packet.concat("}");
  Serial.println(packet);
  mqtt.publish(topicInit, packet.c_str());
  mqtt.subscribe(topicInit);
delay(5000);
}

void loop() {
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
      
  Serial.print(rtc.getDateStr());
  Serial.print(" , ");
  Serial.print(rtc.getTimeStr());
  Serial.print(" , ");
  Serial.println(dailyRain,2);
  mySerial.listen();
  BacaKetinggianAir();
  
if(mnt == 9||mnt== 19||mnt == 29||mnt == 39||mnt == 49||mnt == 59){
//   SerialAT.listen(); 
//   kirimData();
     Sampling_EC_PH();
 }


}
