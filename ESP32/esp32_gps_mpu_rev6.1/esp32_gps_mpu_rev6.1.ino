// 18 09 2020 // draft 1 //

//------------------------------------define GPS--------------------------------//
#include <TinyGPS++.h>
#include <HardwareSerial.h>
TinyGPSPlus gps;
HardwareSerial SerialGPS(1);
#define RXD2 16
#define TXD2 17
int year , month , date, hour , minute , second;
String date_str , time_str , lat_str , lng_str;
float latitude, longitude;
int pm;

//----------------------------------define WiFi--------------------------------//
#include <Wire.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
const char* ssid      = "Arifah";
const char* password  = "1234567890";
String ssid1 = "MST-III-HALL-TIMUR";
String ssid2 = "ARCHITA1";
String ssid3 = "silakanlogin";
float rssi1, rssi2, rssi3;
int i =0;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

//-----------------------------------define firebase---------------------------//
#include <IOXhop_FirebaseESP32.h>
#define FIREBASE_HOST "fireiot-313fc.firebaseio.com"
#define FIREBASE_AUTH "HdjPHaoLNyriL7qs4mg2dZWab7KBwqCt7QeyiaOh"

//-----------------------------------define pushbutton--------------------------//
struct Warning {
  const uint8_t pinWarning;
  uint32_t numberKeyPresses;
  bool pencetWarning;
};

const int tombolSwitchMode=4;
int Switch=0;
boolean modeState;


Warning tombolWarning = {13, 0, false};

void IRAM_ATTR isr() {
  tombolWarning.pencetWarning = true;
}

//--------------------------------define accelerometer--------------------------//
float ax=0, ay=0, az=0, gx=0, gy=0, gz=0;
#include "I2Cdev.h"
#include "MPU6050.h"
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif
MPU6050 accelgyro(0x68); // <-- use for AD0 high

int16_t AXx, AYy, AZz;
int16_t GXx, GYy, GZz;
#define OUTPUT_READABLE_ACCELGYRO

boolean fall = false; //stores if a fall has occurred
boolean trigger1=false; //stores if first trigger (lower threshold) has occurred
boolean trigger2=false; //stores if second trigger (upper threshold) has occurred
boolean trigger3=false; //stores if third trigger (orientation change) has occurred
byte trigger1count=0; //stores the counts past since trigger 1 was set true
byte trigger2count=0; //stores the counts past since trigger 2 was set true
byte trigger3count=0; //stores the counts past since trigger 3 was set true
int angleChange=0;

//---------------------------------define KNN-----------------------------------//
#include <Arduino_KNN.h>
KNNClassifier myKNN_pos(3);
KNNClassifier myKNN_loc(3);
String posisijatuh, lokasijatuh, CurrentTime, CurrentDate;

//-------------------------------------setup------------------------------------//
void setup() {
  Serial.begin(115200);             //set serial baudrate to 115200
  
  SerialGPS.begin(9600, SERIAL_8N1, RXD2, TXD2);          //set GPS serial baudrate to 9600 and initializing Tx Rx Pinout
  Serial.println("Serial Txd is on pin: "+String(TX));    //displaying Tx Pin on serial monitor
  Serial.println("Serial Rxd is on pin: "+String(RX));    //displaying Rx Pin on serial monitor
  
  Serial.print("Connecting to ");             
  Serial.println(ssid);
  WiFi.begin(ssid, password);         //start connecting to WiFi
  while (WiFi.status() != WL_CONNECTED)       //while wifi not connected
  {
    delay(500);
    Serial.print("."); //print "...."
  }
  Serial.println("");
  Serial.println("WiFi connected");   //displaying connection status
  Serial.println(WiFi.localIP());     //displaying local IP Address
  
  timeClient.begin();
  timeClient.setTimeOffset(25200);

  MPU_dataset();


    #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
        Wire.begin();
    #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
        Fastwire::setup(400, true);
    #endif
    Serial.println("Initializing I2C devices...");
    accelgyro.initialize();

    // verify connection
    Serial.println("Testing device connections...");
    Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);    //start connection to firebase
  Firebase.setString("sensor/detection", "-");     //displaying firebase status
  
  pinMode(tombolWarning.pinWarning, INPUT_PULLUP);
  attachInterrupt(tombolWarning.pinWarning, isr, FALLING);
  pinMode(tombolSwitchMode, INPUT_PULLUP);
}


void MPU_dataset (void)
{
  float example_loc1[] = {-69,-68, -76};
  float example_loc2[] = {-68, -64, -74 };
  float example_loc3[] = { -67, -65, -77 };
  float example_loc4[] = { -67, -55, -66 };
  float example_loc5[] = { -74, -58, -64};
  float example_loc6[] = { -70, -58, -74 };
  float example_loc7[] = {  -63, -52, -71 };
  float example_loc8[] = { -65,-56, -69 };
  float example_loc9[] = {-77, -48, -74 };
  float example_loc10[] = {-70, -63, -73 };
  float example_loc11[] = { -70, -70, -72 };
  float example_loc12[] = {  -80, -72 -82 };
  float example_loc13[] = {-74, -66, -61 };
  float example_loc14[] = { -61, -66, -62 };
  float example_loc15[] = {-60, -69, -64 };
  float example_loc16[] = {-53, -65, -58 };
  float example_loc17[] = {-58, -62, -58};
  float example_loc18[] = {-52, -63, -59};
  float example_loc19[] = {-45, -75, -44};
  float example_loc20[] = { -64, -62, -47 };
  float example_loc21[] = { -65, -62, -48};

  myKNN_loc.addExample(example_loc1, 1); // add example for class 7
  myKNN_loc.addExample(example_loc2, 1); // add example for class 5
  myKNN_loc.addExample(example_loc3, 1); // add example for class 9
  myKNN_loc.addExample(example_loc4, 2); // add example for class 5 (again)
  myKNN_loc.addExample(example_loc5, 2); // add example for class 7
  myKNN_loc.addExample(example_loc6, 2); // add example for class 5
  myKNN_loc.addExample(example_loc7, 3); // add example for class 9
  myKNN_loc.addExample(example_loc8, 3); // add example for class 5 (again)
  myKNN_loc.addExample(example_loc9, 3); // add example for class 7
  myKNN_loc.addExample(example_loc10, 4); // add example for class 5
  myKNN_loc.addExample(example_loc11, 4); // add example for class 9
  myKNN_loc.addExample(example_loc12, 4); // add example for class 5 (again)
  myKNN_loc.addExample(example_loc13, 5); // add example for class 5
  myKNN_loc.addExample(example_loc14, 5); // add example for class 9
  myKNN_loc.addExample(example_loc15, 5); // add example for class 5 (again)
  myKNN_loc.addExample(example_loc16, 6); // add example for class 5
  myKNN_loc.addExample(example_loc17, 6); // add example for class 9
  myKNN_loc.addExample(example_loc18, 6); // add example for class 5 (again)
  myKNN_loc.addExample(example_loc19, 7); // add example for class 5
  myKNN_loc.addExample(example_loc20, 7); // add example for class 9
  myKNN_loc.addExample(example_loc21, 7); // add example for class 5 (again)
  
  float example1[] = {-0.106323, 0.070007, -1,43988};
  float example2[] = { -0.441284, -0.128723, -1.350769 };
  float example3[] = { -0.418579, -0.133362, -1,367126 };
  float example4[] = {0.911011, 0.181824, -0.479675 };
  float example5[] = {  0.909546, 0.169617, -0.489685};
  float example6[] = { 0.906616,0.184265,-0.487976 };
  float example7[] = { 0.026489,1.008728, -0.453552};
  float example8[] = {  0.112183,  0.976013,-0.448425};
  float example9[] = {0.140991,  0.977234, 0.450378};
  float example10[] = {  0.278442,-0.937073, -0.411804 };
  float example11[] = { 0.355835,-0.904358, -0.43988};
  float example12[] = {  0.334595, -0.909973,  -0.446472};
  float example13[] = {-1.05481, -0.189026,  -0.444031};
  float example14[] = {-1.057495, -0.216858,  -0.453796};
  float example15[] = {-1.058228, -0.215637, -0.445251};

  myKNN_pos.addExample(example1, 1); // add example for class 7
  myKNN_pos.addExample(example2, 1); // add example for class 5
  myKNN_pos.addExample(example3, 1); // add example for class 9
  myKNN_pos.addExample(example4, 2); // add example for class 5 (again)
  myKNN_pos.addExample(example5, 2); // add example for class 7
  myKNN_pos.addExample(example6, 2); // add example for class 5
  myKNN_pos.addExample(example7, 3); // add example for class 9
  myKNN_pos.addExample(example8, 3); // add example for class 5 (again)
  myKNN_pos.addExample(example9, 3); // add example for class 7
  myKNN_pos.addExample(example10, 4); // add example for class 5
  myKNN_pos.addExample(example11, 4); // add example for class 9
  myKNN_pos.addExample(example12, 4); // add example for class 5 (again)
  myKNN_pos.addExample(example13, 5); // add example for class 5
  myKNN_pos.addExample(example14, 5); // add example for class 9
  myKNN_pos.addExample(example15, 5); // add example for class 5 (again)
}

void loop() {
//reading_MPU();
//delay(1000);
//reading_position();
//delay(100);
//reading_location();
//delay(100);
//reading_GPS();
//delay(100);
 
Switch = digitalRead(tombolSwitchMode);
 if(Switch == LOW){
    modeState = !modeState;
 }
  
 if (modeState == false) {
  // pattern 1
  Serial.println("Indoor mode");
  read_MPU();
  delay(1000);
  static uint32_t lastMillis = 0;
  if (millis() - lastMillis > 5000){
    lastMillis = millis();
    reading_location();
    CurrentTimeDate();
  }

    if (tombolWarning.pencetWarning) {
      Serial.println("kondisi darurat!!");
      tombolWarning.pencetWarning = false;
      Firebase.setString("acc/posisi", "darurat");
      reading_location();
      CurrentTimeDate();
      posisi_jatuh();
    }
}

else { // (if modeState == true)
  Serial.println("Outdoor mode");
  read_MPU();
  delay(1000);
  static uint32_t lastMillisec = 0;
  if (millis() - lastMillisec > 5000){
    lastMillisec = millis();
    read_GPS();
    CurrentTimeDate();
    posisi_jatuh();
  }
  
    if (tombolWarning.pencetWarning) {
      Serial.println("kondisi darurat!!");
      tombolWarning.pencetWarning = false;
      Firebase.setString("acc/posisi", "darurat");
      read_GPS();
      CurrentTimeDate();
    }
  }

}

void read_MPU()
{
    accelgyro.getMotion6(&AXx, &AYy, &AZz, &GXx, &GYy, &GZz);

    // these methods (and a few others) are also available
    //accelgyro.getAcceleration(&ax, &ay, &az);
    //accelgyro.getRotation(&gx, &gy, &gz);

    #ifdef OUTPUT_READABLE_ACCELGYRO
        // display tab-separated accel/gyro x/y/z values
        ax = (AXx-2050)/16384.00;
        ay = (AYy-77)/16384.00;
        az = (AZz-1947)/16384.00;

        //270, 351, 136 for gyroscope
        gx = (GXx+270)/131.07;
        gy = (GYy-351)/131.07;
        gz = (GZz+136)/131.07;
        Serial.print("a/g:\t");
        Serial.print(ax); Serial.print("\t");
        Serial.print(ay); Serial.print("\t");
        Serial.print(az); Serial.print("\t");
        Serial.print(gx); Serial.print("\t");
        Serial.print(gy); Serial.print("\t");
        Serial.println(gz);
    #endif

    #ifdef OUTPUT_BINARY_ACCELGYRO
        Serial.write((uint8_t)(AXx >> 8)); Serial.write((uint8_t)(AXx & 0xFF));
        Serial.write((uint8_t)(AYy >> 8)); Serial.write((uint8_t)(AYy & 0xFF));
        Serial.write((uint8_t)(AZz >> 8)); Serial.write((uint8_t)(AZz & 0xFF));
        Serial.write((uint8_t)(GXx >> 8)); Serial.write((uint8_t)(GXx & 0xFF));
        Serial.write((uint8_t)(GYy >> 8)); Serial.write((uint8_t)(GYy & 0xFF));
        Serial.write((uint8_t)(GZz >> 8)); Serial.write((uint8_t)(GZz & 0xFF));
    #endif

 float Raw_Amp = pow(pow(ax,2)+pow(ay,2)+pow(az,2),0.5);
 int Amp = Raw_Amp * 10;  // Mulitiplied by 10 bcz values are between 0 to 1
 angleChange = pow(pow(gx,2)+pow(gy,2)+pow(gz,2),0.5); 
 Serial.println(angleChange);
 //Firebase.setInt("/acc/amp", Amp);
 //Firebase.setInt("/acc/anglechange", angleChange);
 //Firebase.pushInt("/acc/ampcoba", Amp);
 //Firebase.pushInt("/acc/anglechangecoba", angleChange);
 Serial.println(Amp);

 if (angleChange>=80 && angleChange<=350){ //if orientation changes by between 80-100 degrees
     if(Amp>=7){
      trigger3=true; 
      Serial.println(angleChange);
      Serial.println("TRIGGER 3 ACTIVATED");
     }
   }
   if(trigger3==false){
    Firebase.setString("/acc/trigger3", "false");
    Serial.println("false");
   }
 if(trigger3==true){
   //Firebase.setString("/acc/trigger3", "true");
   Serial.println("true");
   trigger3count++;
   //Firebase.setInt("/acc/angla", trigger3count);
 if(trigger3count>=1 && trigger3count<=3 && angleChange<=5){
   //Firebase.setInt("/acc/angl", angleChange);
   Serial.println("angleChange");
   fall = true;
   //Firebase.setInt("/acc/ket", trigger3count);
   trigger3 = false;
   trigger3count=0;
 }
  if(trigger3count>3){
    trigger3=false;
    trigger3count=0;
  }
 }
 if (fall==true){ //in event of a fall detection
   Serial.println("FALL DETECTED");
   Firebase.setString("sensor/detection", "Fall");
   posisi_jatuh();
   fall=false;
 } 
}



void reading_location()
{
  int numNetworks = WiFi.scanNetworks();
  for (int i = 0; i < numNetworks; i++){
    if(String(WiFi.SSID(i)) == ssid1) {
         Serial.print(String(WiFi.SSID(i)));
         Serial.println(WiFi.RSSI(i));
         rssi1 = WiFi.RSSI(i);
         //Firebase.setFloat("falldetect/rssi1", float(rssi1));
      }
      if(String(WiFi.SSID(i)) == ssid2) {
         Serial.print(String(WiFi.SSID(i)));
         Serial.println(WiFi.RSSI(i));
        rssi2 = WiFi.RSSI(i);
        //Firebase.setFloat("falldetect/rssi2", float(rssi2));
      }
      if(String(WiFi.SSID(i)) == ssid3) {
         Serial.print(String(WiFi.SSID(i)));
         Serial.println(WiFi.RSSI(i));
         rssi3 = WiFi.RSSI(i);
         //Firebase.setFloat("falldetect/rssi3", float(rssi3));
      }
  }
   float input[] = { rssi1,rssi2,rssi3 };
   int classification_loc = myKNN_loc.classify(input, 4); // classify input with K=3
   float confidence = myKNN_loc.confidence();
//      Serial.print("\tclassification = ");
//      Serial.println(classification_loc);
//      Serial.print("\tconfidence     = ");
//      Serial.println(confidence);
      if(classification_loc == 1){
        lokasijatuh = "garasi";
        Serial.println("garasi"); 
      }
      if(classification_loc == 2){
        lokasijatuh = "kamar kanan bawah"; 
        Serial.println("kamar kanan bawah");
      }
      if(classification_loc == 3){
        lokasijatuh = "kamar tengah bawah";
        Serial.println("kamar tengah bawah"); 
      }
      if(classification_loc == 4){
        lokasijatuh = "dapur"; 
        Serial.println("dapur");
      }
      if(classification_loc == 5){
        lokasijatuh = "kamar kiri atas"; 
        Serial.println("kamar kiri atas");
      }
      if(classification_loc == 6){
        lokasijatuh = "kamar tengah atas";
        Serial.println("kamar tengah atas"); 
      }
      if(classification_loc == 7){
        lokasijatuh = "kamar kanan atas"; 
        Serial.println("kamar kanan atas");
      }
      Firebase.setString("falldetect/posisi", String(lokasijatuh));
}

void read_GPS(){
  while (SerialGPS.available() > 0) {
      gps.encode(SerialGPS.read());
      if(gps.encode(SerialGPS.read())){
        Serial.print("Sats=");  Serial.println(gps.satellites.value());
        int sat = gps.satellites.value();
        //Firebase.setInt("/lokasi/satelit", sat);
        if(gps.location.isValid()){
          Serial.print("LAT=");  Serial.println(gps.location.lat(), 6);
          Serial.print("LONG="); Serial.println(gps.location.lng(), 6);
          Serial.print("ALT=");  Serial.println(gps.altitude.meters());
          Serial.print("Sats=");  Serial.println(gps.satellites.value());
          latitude = gps.location.lat();
          longitude = gps.location.lng();
          String lat_str = String(gps.location.lat(), 6);
          String lng_str = String(gps.location.lng(), 6);
          Firebase.setString("/lokasi/latitude", lat_str);
          Firebase.setString("/lokasi/longitude", lng_str);
        }
        
      }
    }
}

void posisi_jatuh()
{
 if (ax>=0.8){             
Serial.println("jatuh belakang");
Firebase.setString("acc/posisi", "jatuh belakang");
   }
 else if (ax<=-1.0){ 
Serial.println("jatuh depan");
Firebase.setString("acc/posisi", "jatuh depan");
   }
 else if (ay>=0.9){ 
Serial.println("jatuh kanan");
Firebase.setString("acc/posisi", "jatuh kanan");
   }
 else if (ay<=-0.9){ 
Serial.println("jatuh kiri");
Firebase.setString("acc/posisi", "jatuh kiri");
   }
}

void CurrentTimeDate(){
   timeClient.update();
   unsigned long epochTime = timeClient.getEpochTime();
   CurrentTime = timeClient.getFormattedTime();
   Serial.println(CurrentTime);
   struct tm *ptm = gmtime ((time_t *)&epochTime); 
   int monthDay = ptm->tm_mday;
   int currentMonth = ptm->tm_mon+1;
   int currentYear = ptm->tm_year+1900;
   CurrentDate = String(monthDay) + " / " + String(currentMonth) + " / " + String(currentYear);
   Serial.println(CurrentDate);
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["day"] = CurrentDate;
    root["hour"] = CurrentTime;
    String history = Firebase.push("/history", root);
}
