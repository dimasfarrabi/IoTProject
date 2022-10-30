
#include <SPI.h>
#include <RH_RF95.h>
#include <RHReliableDatagram.h>
#include <String.h>
#include <EEPROM.h>

#define CLIENT      31
#define REPEATER    32
#define SERVER      33
#define TXPWR       13
float frequency = 920.0;
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
float temp1,
        rain1,
        level1;
};
#pragma pack(pop)
DATA data;
const float FREQ = 920.0;
RHDatagram kirim(rf95, REPEATER);
RHDatagram terima(rf95, REPEATER); 

void setup(){
  Serial.begin(9600);
  SetupLora();
 }

void SetupLora(void)
{
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
    Serial.print("RF95 max message length = ");
    Serial.println(rf95.maxMessageLength());  
  }
  {
    if (terima.init()) 
    {
    if (!rf95.setFrequency(FREQ))
      Serial.println("Unable to set RF95 frequency");
      //if (!rf95.setModemConfig(RH_RF95::Bw500Cr45Sf128))
      //Serial.println("Invalid setModemConfig() option");
      rf95.setTxPower(TXPWR);
      Serial.println("RF95 radio initialized.");
    }
    else 
    Serial.println("RF95 radio initialization failed.");
    Serial.print("RF95 max message length = ");
    Serial.println(rf95.maxMessageLength());
  }
}

void loop(void)
{
  unsigned long startTimer,
                stopTimer;

  {
    startTimer = millis();
    if (rf95.waitAvailableTimeout(1000))        // wait 100 mSec max for response
    { 
    uint8_t bufLen = sizeof(data);
    uint8_t from;
    if (terima.recvfrom((uint8_t *) &data, &bufLen, &from))
    {
      stopTimer = millis();
      Serial.print("Response time (mSec) = ");
      Serial.println(stopTimer-startTimer);
      Serial.print("Got mesaage from : 0x");
      Serial.println(from, HEX);
            
      if (bufLen == sizeof(data) && from == 31)
      {
        Serial.println("<======= Received level =  " + String(data.level1));
        Serial.println("<======= Received rain = " + String(data.rain1,3));
      
        Serial.println("=======> Sending level =  " + String(data.level1));
        Serial.println("=======> Sending rain = " + String(data.rain1,3));
        startTimer = millis();
        if (!kirim.sendto((uint8_t *) &data, sizeof(data), SERVER))
        Serial.print("Transmit failed");
        rf95.waitPacketSent(100);
        stopTimer = millis();
        Serial.print("Transmission time (mSec) = ");
        Serial.println(stopTimer-startTimer);
        Serial.println();
      }
       else
        Serial.println("Incorrect response size");
     }
     }
      else
      Serial.println("Timed out waiting for response");
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
