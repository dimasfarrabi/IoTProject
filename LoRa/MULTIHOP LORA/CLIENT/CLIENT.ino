
#include <SPI.h>
#include <RH_RF95.h>
#include <RHReliableDatagram.h>
#include <String.h>
#include <EEPROM.h>

int alert1;
int alert2;
String alert3;
int led1      =3;
String gedung ="A";
#define CLIENT      'A'
#define REPEATER    'B'
#define SERVER      33
#define TXPWR       13
#define ReceivedBufferLength 20
char receivedBuffer[ReceivedBufferLength+1];   // store the serial command
byte receivedBufferIndex = 0;
boolean enterCalibrationFlag = 0;
#define compensationFactorAddress 8
float compensationFactor;
#define EEPROM_write(address, p) {int i = 0; byte *pp = (byte*)&(p);for(; i < sizeof(p); i++) EEPROM.write(address+i, pp[i]);}
#define EEPROM_read(address, p)  {int i = 0; byte *pp = (byte*)&(p);for(; i < sizeof(p); i++) pp[i]=EEPROM.read(address+i);}
RH_RF95 rf95;
#pragma pack(push, 2)
typedef struct DATA {
int data1,
        data2,
        data3;
};
#pragma pack(pop)
DATA data;
const float FREQ = 433.0;
RHDatagram kirim(rf95, CLIENT);
                                        // as we want read

void setup(){
  Serial.begin(9600);
  SetupLora();
 }
 
void SetupLora(void)
{
  if (kirim.init()) 
  {
    if (!rf95.setFrequency(FREQ))
      Serial.println("Unable to set RF95 frequency");
      //if (!rf95.setModemConfig(RH_RF95::Bw500Cr45Sf128))
      //Serial.println("Invalid setModemConfig() option");
      rf95.setTxPower(TXPWR);
      Serial.println("RF radio initialized.");
  }
  else 
    Serial.println("RF radio initialization failed.");
    //Serial.print("RF95 max message length = ");
    //Serial.println(rf95.maxMessageLength());  
}

void loop() {
  unsigned long startTimer,
                stopTimer;
      String input = "";
    if (Serial.available() > 0){
     input += (char) Serial.read();
     delay(5);
 
  if (input == "1"){
    digitalWrite(led1, HIGH);
    alert1 =1;
  }
  else if (input == "0"){
    digitalWrite(led1, LOW);
    alert1 = 1;
  }
  data.data1 = (alert1);
  data.data2 = (alert2);
  //data.data2 = (temp);
  Serial.println("=======> Sending alert1 =  " + String(data.data1) + String(gedung));
  Serial.println("=======> Sending alert2 = " + String(data.data2));
  startTimer = millis();
  if (!kirim.sendto((uint8_t *) &data, sizeof(data), REPEATER))
  Serial.print("Transmit failed");
  rf95.waitPacketSent(100);
  stopTimer = millis();
  Serial.print("Transmission time (mSec) = ");
  Serial.println(stopTimer-startTimer);
  Serial.println();
    }
  }

boolean serialDataAvailable(void)
{
  char receivedChar;
  static unsigned long receivedTimeOut = millis();
  while (Serial.available()>0) 
  {   
    if (millis() - receivedTimeOut > 500U) 
    {
      receivedBufferIndex = 0;
      memset(receivedBuffer,0,(ReceivedBufferLength+1));
    }
    receivedTimeOut = millis();
    receivedChar = Serial.read();
    if (receivedChar == '\n' || receivedBufferIndex==ReceivedBufferLength){
    receivedBufferIndex = 0;
    strupr(receivedBuffer);
    return true;
    }else{
      receivedBuffer[receivedBufferIndex] = receivedChar;
      receivedBufferIndex++;
    }
  }
  return false;
}

byte uartParse()
{
  byte modeIndex = 0;
  if(strstr(receivedBuffer, "CALIBRATION") != NULL) 
      modeIndex = 1;
  else if(strstr(receivedBuffer, "EXIT") != NULL) 
      modeIndex = 3;
  else if(strstr(receivedBuffer, "CONFIRM") != NULL)   
      modeIndex = 2;
  return modeIndex;
}


int getMedianNum(int bArray[], int iFilterLen) 
{
      int bTab[iFilterLen];
      for (byte i = 0; i<iFilterLen; i++)
      {
    bTab[i] = bArray[i];
      }
      int i, j, bTemp;
      for (j = 0; j < iFilterLen - 1; j++) 
      {
    for (i = 0; i < iFilterLen - j - 1; i++) 
          {
      if (bTab[i] > bTab[i + 1]) 
            {
    bTemp = bTab[i];
          bTab[i] = bTab[i + 1];
    bTab[i + 1] = bTemp;
       }
    }
      }
      if ((iFilterLen & 1) > 0)
  bTemp = bTab[(iFilterLen - 1) / 2];
      else
  bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
      return bTemp;
}

void readCharacteristicValues()
{
    EEPROM_read(compensationFactorAddress, compensationFactor);  
    if(EEPROM.read(compensationFactorAddress)==0xFF && EEPROM.read(compensationFactorAddress+1)==0xFF && EEPROM.read(compensationFactorAddress+2)==0xFF && EEPROM.read(compensationFactorAddress+3)==0xFF)
    {
      compensationFactor = 1.0;   // If the EEPROM is new, the compensationFactorAddress is 1.0(default).
      EEPROM_write(compensationFactorAddress, compensationFactor);
    }
}
