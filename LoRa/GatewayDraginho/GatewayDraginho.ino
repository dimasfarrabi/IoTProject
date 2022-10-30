

/*#include <Bridge.h>
#include <HttpClient.h>*/

#include <SPI.h>
#include <RH_RF95.h>
#include <RHReliableDatagram.h>
//#define BAUDRATE 115200
RH_RF95 rf95;
float frequency = 923.0;
#define CLIENT      22
#define SERVER      23
#pragma pack(push, 2)
typedef struct DATA {
float temp1,
        rain1,
        level1;
};
#pragma pack(pop)
DATA data;
RHDatagram terima(rf95, SERVER);

void setup() {
  // Bridge takes about two seconds to start up
  // it can be helpful to use the on-board LED
  // as an indicator for when it has initialized
  pinMode(A2, OUTPUT);
  digitalWrite(A2, LOW);
  Bridge.begin(BAUDRATE);
  digitalWrite(A2, HIGH);
  Console.begin();
  while (!Console); // wait for a serial connection

  if (!rf95.init())
    Serial.println("init failed");
  // Setup ISM frequency
  rf95.setFrequency(frequency);
  // Setup Power,dBm
  rf95.setTxPower(23);
  
}

void loop() {
unsigned long startTimer,
              stopTimer;         

  startTimer = millis();
  if (rf95.waitAvailableTimeout(3000))        // wait 100 mSec max for response
  { 
    uint8_t bufLen = sizeof(data);
    uint8_t from;
    if (terima.recvfrom((uint8_t *) &data, &bufLen, &from))// receive data
    {
      stopTimer = millis();
      Serial.print("Response time (mSec) = ");
      Serial.println(stopTimer-startTimer);
      Serial.print("Got mesaage from : 0x");
      Serial.println(from, HEX);
            
      if (bufLen == sizeof(data) && from == 22)// receive data from node address 101
      {
        Serial.println("<======= Received level =  " + String(data.level1));
        Serial.println("<======= Received rain = " + String(data.rain1,3));
        
        HttpClient http;
        http.begin("https://banjir.polines-longsor.com/data/input/node1.php");
        //http.addHeader("Content-Type", "application/x-www-form-urlencoded");
        String httpRequestData1 = "level="+String(data.level1);
        String httpRequestData2 = "rain="+String(data.rain1,3);
        String Payload = (httpRequestData1 + httpRequestData2);
        Serial.print("httpRequestData: ");
        Serial.println(Payload);
        int httpResponseCode = http.get(Payload);
          if (httpResponseCode>0) {
            Serial.print("HTTP Response code: ");
            Serial.println(httpResponseCode);
          }
          else {
          Serial.print("Error code: ");
          Serial.println(httpResponseCode);
          }     
      }
      else
        Serial.println("Incorrect response size");
    }
  }
}
