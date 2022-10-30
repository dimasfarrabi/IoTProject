#include <TinyGPS++.h>
#include <HardwareSerial.h>
float latitude, longitude;
String lat_str, lng_str;
static const int RXPin = 16, TXPin = 17;
static const uint32_t GPSBaud = 4800;
TinyGPSPlus gps;
HardwareSerial SerialGPS(1);
void setup()
{
  Serial.begin(115200);
  SerialGPS.begin(GPSBaud, SERIAL_8N1, RXPin, TXPin);
}

void loop()
{
  while (SerialGPS.available() > 0) {
    if (gps.encode(SerialGPS.read()))
    {
      if (gps.location.isValid())
      {
        latitude = gps.location.lat();
        lat_str = String(latitude , 6);
        longitude = gps.location.lng();
        lng_str = String(longitude , 6);
        Serial.print("Latitude = ");
        Serial.println(lat_str);
        Serial.print("Longitude = ");
        Serial.println(lng_str);
      }
     delay(1000);
     Serial.println();  
    }
  }  
}
