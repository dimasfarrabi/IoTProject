//---------------------------------------------defines GSM and Serial Modem
#define TINY_GSM_MODEM_SIM7000

#include <SoftwareSerial.h>
SoftwareSerial SerialAT(10, 11); // RX, TX
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
const char* topicInit = "data1";

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

float ECcurrent = 4.45;
int temperature = 28;
float pHValue = 7.8;



uint32_t lastReconnectAttempt = 0;


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
  delay(6000);

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
  rtc.setTime(13, 9, 0);    // Set time (24Hr)
  rtc.setDate(28 , 9, 2021);   // Set date
}

void kirimData(){
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
  
    if (!mqtt.connected()) {
    reconnect();
  }
    mqtt.loop();
  String packet = "";
 
  packet.concat(("{\"Rain_Gauge\":"));
  packet.concat(dailyRain);
 
  packet.concat((",\"Water_Level\":"));
  packet.concat(ketinggian);

  packet.concat((",\"Temperature\":"));
  packet.concat(temperature);

  packet.concat((",\"pH\":"));
  packet.concat(pHValue);

  packet.concat((",\"Conductivity\":"));
  packet.concat(ECcurrent);
  
  packet.concat("}");
  Serial.println(packet);
  mqtt.publish(topicInit, packet.c_str());
  mqtt.subscribe(topicInit);
delay(8000);
}

void loop() {
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
    kirimData();
  }
  if((mnt == 11 && det == 8)||(mnt== 21 && det == 8)||(mnt == 31 && det == 8)||(mnt == 41 && det == 8)||(mnt == 51 && det == 8)||(mnt == 1 && det == 8))
  {
    dailyRain = 0.0;
  }
 

}
