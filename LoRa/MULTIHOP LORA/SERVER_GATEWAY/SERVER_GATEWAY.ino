/*Last update : 22/05/2019
  Author      : roni.apriantoro@polines.ac.id
  Edited      : ariaji.prichi@gmail.com

  Description :
  This sketch is showing how to create a simple addressed on LoRa Gateway
   snd received float datas from multiple clients.
  
  Tested with Arduino Uno, Dragino LorA Shield V1.4 and Dragino LoRa MiniDev
*/
/****************************************************/
/*  Includes                                        */
/****************************************************/
#include <ESP8266WiFi.h>
#include <CayenneMQTTESP8266.h> //Library for connecting with cayenne cloud server
#define CAYENNE_DEBUG
#define CAYENNE_PRINT serial
#include <SPI.h> //Library to communicated between SPI LoRa and NodeMCU
#include <RH_RF95.h> //LoRa Library
#include <RHReliableDatagram.h> //ReliableDatagram Library
#include <String.h>


/****************************************************/
/*  Defines                                         */
/****************************************************/
#define RFM95_CS D2                                       //define slave select pin on NodeMCU
#define RFM95_RST D3                                      //define Reset or SS pin on NodeMCU
#define RFM95_INT D1                                      //define Interupt Pin on NodeMCU
#define LED D4                                            //set LED indicator
#define CLIENT      21
#define REPEATER    22
#define SERVER      23
#define TXPWR       13
/****************************************************/
/*  Packet structure                                */
/****************************************************/
#pragma pack(push, 2)
typedef struct DATA {
float temp1,
        rain1,
        level1;
};
#pragma pack(pop)
DATA data;

/****************************************************/
/*  Globals                                         */
/****************************************************/
const float FREQ = 920.0;
RH_RF95 rf95(RFM95_CS, RFM95_INT);                        // Singleton instance of the radio driver
RHDatagram terima(rf95, SERVER);                  // Class to manage message delivery and receipt, using the driver declared above
const char* ssid = "MST-III-HALL-TIMUR";                             //Set Wifi SSID
const char* password = "wifiharam";                      //Set Wifi Password

char username[]= "276c84c0-3894-11ea-b73d-1be39589c6b2";           //Username for cloud server login
char mqtt_password[] = "29a2c92f65779cc43536b33d77bdbce2fa20ecbb"; //Password for cloud server login
char Client_id[] = "3d7c8f30-3894-11ea-ba7c-716e7f5ba423";         //ClientID for cloud server login

/****************************************************/
/*  setup                                           */
/****************************************************/
void setup(void) 
{
  Serial.begin(115200); //Set Baudrate for serial communication
  SetupWifi();
  SetupLora();
 
}

/****************************************************/
/*  loop                                            */
/****************************************************/
void loop(void)
{
  //Cayenne.loop();
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

        //Cayenne.virtualWrite(1, data.rain1, "hujan/10mnt", "mm");  
        //Cayenne.virtualWrite(2, data.level1, "level/10mnt", "cm");      
      }
      else
        Serial.println("Incorrect response size");
    }
  }
  else
    Serial.println("Timed out waiting for response");
  Serial.println();
  delay(1000);
}

/****************************************************/
/*  SetupLora                                      */
/****************************************************/
void SetupLora(void)
{
  if (terima.init())//starting LoRa
  {
    if (!rf95.setFrequency(FREQ)) //Starting set frequency
    Serial.println("Unable to set RF95 frequency");
    rf95.setTxPower(TXPWR);
    Serial.println("RF95 radio initialized.");
  }
  else
    Serial.println("RF95 radio initialization failed.");
    Serial.print("RF95 max message length = ");
    Serial.println(rf95.maxMessageLength());
}

/****************************************************/
/*  SetupWifi                                      */
/****************************************************/
void SetupWifi(void)
{
  pinMode(LED, OUTPUT);                 //Set Pin LED as digital Output
  digitalWrite(LED,HIGH);               //Turn on Led
  pinMode(RFM95_RST, OUTPUT);           //Set pin Reset as Output
  digitalWrite(RFM95_RST, HIGH);        //Set pin Reset High
  delay(100);
  Serial.println();
  Serial.println();
  Serial.println("Gateway Module starting…");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  Cayenne.begin(username,mqtt_password,Client_id,ssid,password);
  
  Serial.println("");
  Serial.println("WiFi connected");
  digitalWrite(RFM95_RST, LOW); //Set pin reset Low
  delay(10);
  digitalWrite(RFM95_RST, HIGH); // Set pin reset High
  delay(10);
}
