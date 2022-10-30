
//----------------define bridge and HTTP Client------------//
/*#include <Bridge.h>
#include <HttpClient.h>
#include <YunClient.h>
YunClient client;
#define BAUDRATE 115200   //Bridge Baudrate
IPAddress server (103,147,154,61);  //Server IP address
String payload =""; //initializing payload*/

//---------------define LoRa------------------//
#include <SPI.h>
#include <Console.h>
#include <RH_RF95.h>
#include <RHReliableDatagram.h>
RH_RF95 rf95;
float frequency = 923.0;  //set the frequncy
#define CLIENT      22
#define SERVER      23
#pragma pack(push, 2)
typedef struct DATA {
float rain,
      level;
};
#pragma pack(pop)
DATA data;
RHDatagram terima(rf95, SERVER);

void setup() {
//--------------setup bridge------------------//
  /*pinMode(A2, OUTPUT);
  digitalWrite(A2, LOW);
  Bridge.begin(BAUDRATE);
  digitalWrite(A2, HIGH);*/
  Console.begin();
  while (!Console); // wait for a serial connection

//---------------setup LoRa------------------//
if (!rf95.init())
    Console.println("init failed");
    rf95.setFrequency(frequency);   // Setup ISM frequency
    rf95.setTxPower(13);   // Setup Power,dBm Defaults BW Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
    Console.print("Listening on frequency: ");
    Console.println(frequency);
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
      Console.print("Response time (mSec) = ");
      Console.println(stopTimer-startTimer);
      Console.print("Got mesaage from : 0x");
      Console.println(from, HEX);
      if (bufLen == sizeof(data) && from == 22)// receive data from node address 101
      {
        Serial.println("<======= Received level =  " + String(data.level));
        Serial.println("<======= Received rain = " + String(data.rain,3));
        
                 /*if (client.connect(server, 80)) {
                          Console.println("connected");
                          delay(2500);
                          payload = "rain="+String(data.rain,3) + "&level="+ String(data.level);
                          client.println("POST /data/input/node1.php HTTP/1.1");
                          Console.println("POST /data/input/node1.php HTTP/1.1");
                          client.println("Host: banjir.polines-longsor.com");
                          Console.println("Host: banjir.polines-longsor.com");
                          client.print("Content-length:");
                          Console.println("Content-length:");
                          client.println(payload.length());
                          Console.println(payload);
                          client.println("Connection: Close");
                          client.println("Content-Type: application/x-www-form-urlencoded;");
                          client.println();
                          client.println(payload);
                 }
                 else{
                        Console.println("connection failed");
                        delay(1000);
                 }
                 if(client.connected()){
                             client.stop();   //disconnect from server
                 }*/

      }

    }
  }
}
