
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
//#include <WiFi.h>
const char* ssid      = "dims";
const char* password  = "mehngopo";

//-----------------------------------define firebase---------------------------//
#include <IOXhop_FirebaseESP32.h>
#define FIREBASE_HOST "fireiot-313fc.firebaseio.com"
#define FIREBASE_AUTH "HdjPHaoLNyriL7qs4mg2dZWab7KBwqCt7QeyiaOh"

//-----------------------------------define pushbutton--------------------------//
const int pinButton=4;
int GPS_status=0;
boolean modeState;
//-------------------------------------setup------------------------------------//
void setup() {
  Serial.begin(115200);             //set serial baudrate to 115200
  
  SerialGPS.begin(9600, SERIAL_8N1, RXD2, TXD2);          //set GPS serial baudrate to 9600 and initializing Tx Rx Pinout
  Serial.println("Serial Txd is on pin: "+String(TX));    //displaying Tx Pin on serial monitor
  Serial.println("Serial Rxd is on pin: "+String(RX));    //displaying Rx Pin on serial monitor
  
 /* Serial.print("Connecting to ");             
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
  
  pinMode(pinButton, INPUT_PULLUP);   //setup the ESP32 pinout for pushbutton
  
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);    //start connection to firebase
  Firebase.setString("sensor/detection", "-"); */    //displaying firebase status
}

void loop() {
 GPS_status = digitalRead(pinButton);
  if(GPS_status == LOW){
    modeState = !modeState; // switch the state of the variable
    //Serial.println("GPS");
  }
  if (modeState == false) {

  // pattern 1
  Serial.println("GPS off");
    /*while (SerialGPS.available() > 0) {
      Serial.println(gps.encode(SerialGPS.read()));
      if(gps.encode(SerialGPS.read())){
        Serial.print("Sats=");  Serial.println(gps.satellites.value());
        int sat = gps.satellites.value();
        Firebase.setInt("/lokasi/satelit", sat);
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
    }*/
}

else { // (if modeState == true)

  // pattern 2
while (SerialGPS.available() > 0) {
      Serial.println(gps.encode(SerialGPS.read()));
      if(gps.encode(SerialGPS.read())){
        Serial.print("Sats=");  Serial.println(gps.satellites.value());
        int sat = gps.satellites.value();
        Firebase.setInt("/lokasi/satelit", sat);
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

}
