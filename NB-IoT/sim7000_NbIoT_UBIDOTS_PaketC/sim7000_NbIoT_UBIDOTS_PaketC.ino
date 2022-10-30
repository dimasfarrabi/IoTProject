

#include <SoftwareSerial.h>
SoftwareSerial mySerial(12,13); // RX, TX
//--------------------------------------define SIM7000------------------------------------//
#include <String.h>
#include <DFRobot_SIM7000.h>
#define PIN_TX     10
#define PIN_RX     11
SoftwareSerial     SerialNb(PIN_RX,PIN_TX);
DFRobot_SIM7000    sim7000;

String token = "BBFF-4wiOdNEn1cr9fdE2wkCse0UqakyRfv";       
                                                    // Your Ubidots token (API credential)
String devicelabel = "misred1";                     //Your device label

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


//--------------------------------------define EC-----------------------------------------//
#include <OneWire.h>
#define StartConvert 0
#define ReadTemperature 1
const byte numReadings = 20;     //the number of sample times
byte ECsensorPin = A1;  //EC Meter analog output,pin on analog 1
byte DS18B20_Pin = 3; //DS18B20 signal, pin on digital 2
unsigned int AnalogSampleInterval=25,printInterval=700,tempSampleInterval=850;  //analog sample interval;serial print interval;temperature sample interval
unsigned int readings[numReadings];      // the readings from the analog input
byte index = 0;                  // the index of the current reading
unsigned long AnalogValueTotal = 0;                  // the running total
unsigned int AnalogAverage = 0,averageVoltage=0;                // the average
unsigned long AnalogSampleTime,printTime,tempSampleTime;
float temperature,ECcurrent;
OneWire ds(DS18B20_Pin);

//-------------------------------------define pH------------------------------------------//
#define SensorPin A2            //output dari modul sensor pH masuk pada pin analog 0board arduino 
#define Offset 0.45            //deviation compensate
#define LED 9                // led indikator
#define samplingInterval 20   // Tiap 20 milidetik  sensor akan mengukur atau mengambil data pada larutan
#define printInterval 500    //  Tiap 1000 milidetik akan menampilkan hasil pengukuran pada serial monitor 
#define ArrayLenth  40    //times of collection, untuk penyimpanan pengambilan sampel
int pHArray[ArrayLenth];   //Store the average value of the sensor feedback
int pHArrayIndex=0;
static float pHValue,voltage;



void setup() {
  SerialNb.listen();
  sim7000.begin(SerialNb);
  sim7000.turnOFF();
  Serial.println("Turn ON SIM7000......");
  if (sim7000.turnON()) {                           //Turn ON SIM7000
    Serial.println("Turn ON !");
  }

  Serial.println("Set baud rate......");
  if (sim7000.setBaudRate(19200)) {                 //Set baud rate from 115200 to 19200
    Serial.println("Set baud rate:19200");
  } else {
    Serial.println("Faile to set baud rate");
    while (1);
  }

  SerialNb.println("AT+CSCLK=0");
  delay(3000);
  ShowSerialData();
  Serial.println();
  
  SerialNb.println("AT+CGATT?");
  delay(2000);
  ShowSerialData();
  Serial.println();

  SerialNb.println("AT+CSTT=\"NB1INTERNET\"");      // Replace with your APN
  delay(2000);
  ShowSerialData();

  SerialNb.println("AT+CSTT?");                     // Check APN
  delay(1000);
  ShowSerialData();
  Serial.println();

  SerialNb.println("AT+CIICR");                     // Bring up the wireless connection
  delay(3000);
  ShowSerialData();
  Serial.println();

  SerialNb.println("AT+CIFSR");                     // Get assigned IP adress
  delay(2000);
  ShowSerialData();
  Serial.println();

  SerialNb.println("AT+CIPSPRT=0");
  delay(3000);
  ShowSerialData();
  Serial.println();

  
  Serial.begin(115200);
  
  
  rtc.begin();
  rtc.setTime(13, 9, 40);    // Set time (24Hr)
  rtc.setDate(7 , 11, 2021);   // Set date
  pinMode(RainPin, INPUT);                            // set the Rain Pin as input.
  Serial.println("Rain Gauge Ready !!");              // rain gauge measured per 1 hour      
  mySerial.begin(9600);

  for (byte thisReading = 0; thisReading < numReadings; thisReading++)
  readings[thisReading] = 0;
  TempProcess(StartConvert);   //let the DS18B20 start the convert
  AnalogSampleTime=millis();
  printTime=millis();
  tempSampleTime=millis();
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

void Sampling_EC_PH(){

//---------------------------------------------read EC--------------------------------------------//  
  if(millis()-AnalogSampleTime>=AnalogSampleInterval)
  {
    AnalogSampleTime=millis();                              // subtract the last reading:
    AnalogValueTotal = AnalogValueTotal - readings[index];  // read from the sensor:
    readings[index] = analogRead(ECsensorPin);              // add the reading to the total:
    AnalogValueTotal = AnalogValueTotal + readings[index];  // advance to the next position in the array:
    index = index + 1;                                      // if we're at the end of the array...
    if (index >= numReadings)                               // ...wrap around to the beginning:
    index = 0;                                              // calculate the average:
    AnalogAverage = AnalogValueTotal / numReadings;
  }
   if(millis()-tempSampleTime>=tempSampleInterval)
  {
    tempSampleTime=millis();
    temperature = TempProcess(ReadTemperature);  // read the current temperature from the  DS18B20
    TempProcess(StartConvert);                   //after the reading,start the convert for next reading
  }
  if(millis()-printTime>=printInterval)
  {
    printTime=millis();
    averageVoltage=AnalogAverage*(float)5000/1024;

    float TempCoefficient=1.0;    //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.0185*(fTP-25.0));
    float CoefficientVolatge=(float)averageVoltage/TempCoefficient;
    if(CoefficientVolatge<150)Serial.println("No solution!");   //25^C 1us/cm<-->about ...mv, <1uS/cm,no solution
    else if(CoefficientVolatge>3300)Serial.println("Out of the range!");  //>25000us/cm,out of the range
    else
    {
      if(CoefficientVolatge<=448)ECcurrent=6.84*CoefficientVolatge-64.32;   //84us/cm<-->about ...mv
      else if(CoefficientVolatge<=1457)ECcurrent=3.28*CoefficientVolatge-127;  //1413us/cm<-->about 485mv
      else ECcurrent=5.3*CoefficientVolatge+2278;                           //12880us/cm<-->about 2000mv
      ECcurrent/=1000;    //convert us/cm to ms/cm
    }
  }

//---------------------------------------------------read pH---------------------------------------// 
  static unsigned long samplingTime = millis();
  static unsigned long printTime = millis();
  if(millis()-samplingTime > samplingInterval)
  {
      pHArray[pHArrayIndex++]=analogRead(SensorPin); // mengambil data analog
      if(pHArrayIndex==ArrayLenth)pHArrayIndex=0;
      voltage = avergearray(pHArray, ArrayLenth)*5.0/1024; // mengubah data analog ke digital
      pHValue = 3.5*voltage+Offset;
      samplingTime=millis();
  }

  Serial.println();
  Serial.println("----------- Read Sensors ------------");
  Serial.println("EC is " + String(ECcurrent,2) + " ms/cm");
  Serial.println("Temp is " + String(temperature) + " C");
  Serial.println("pH is " + String(pHValue) + " ms/cm");
  Serial.println("----------------------------------------");
  Serial.println();

  delay(150);
  
}

void Kirim_Data(){

    int num;
  String le;
  String var;
  var="{\"Water_level\":"+ String(jarak) +",\"Rainfall\":" + String(dailyRain,2)+",\"Temperature\":"+ String(temperature)+",\"pH\":"+ String(pHValue)+",\"Conductivity\":"+ String(ECcurrent,2)+"}";
  num=var.length();
  le=String(num);
  
  mySerial.println("AT+CIPSTART=\"tcp\",\"50.23.124.68\",\"80\"");              // Start the connection to Ubidots
  delay(1000);
  ShowSerialData();
  Serial.println();
  
  mySerial.println("AT+CIPSEND ");                                              // Start to send data to remote server
  delay(100);
  ShowSerialData();
  mySerial.print("POST /api/v1.6/devices/misred1 HTTP/1.1\r\n");
  delay(100);
  ShowSerialData();
  mySerial.print("Host: things.ubidots.com\r\n");
  delay(100);
  ShowSerialData();
  mySerial.print("X-Auth-Token: "+token+"\r\n");
  delay(100);
  ShowSerialData();
  //mySerial.print(token+"\r\n");
  //delay(100);
  //ShowSerialData();
  mySerial.print("Content-Type: application/json\r\n");
  delay(100);
  ShowSerialData();
  mySerial.print("Content-Length:"+le+"\r\n\r\n");
  delay(100);
  ShowSerialData();
  mySerial.print(var);
  delay(100);
  ShowSerialData();
  mySerial.print("\r\n");
  delay(100);
  ShowSerialData();
  mySerial.println((char)26);                            // Sending the HTTP request
  delay(5000);                                           // Waiting for a reply. Important! tweak this time depending on the latency of your mobile Internet connection
  mySerial.println();
  ShowSerialData();

  mySerial.println("\r\n");
  mySerial.println("AT+CIPCLOSE");                      // Close the connection
  delay(100);
  ShowSerialData();
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
//  mySerial.listen();
//  BacaKetinggianAir();
  
if(mnt == 9||mnt== 19||mnt == 29||mnt == 39||mnt == 49||mnt == 59){
//   SerialAT.listen(); 
//   kirimData();
     Sampling_EC_PH();
     mySerial.listen();
     BacaKetinggianAir();
 }
else if(mnt == 10||mnt== 30||mnt == 30||mnt == 40||mnt == 50||mnt == 0){
SerialNb.listen();
Kirim_Data();
//     Sampling_EC_PH();
 }
else if((mnt == 11 && det == 1)||(mnt== 21 && det == 1)||(mnt == 31 && det == 1)||(mnt == 41 && det == 1)||(mnt == 51 && det == 1)||(mnt == 1 && det == 1)){
   dailyRain = 0.0; 
//   dailyRain_till_LastHour = 0.0;
//   digitalWrite (A5, HIGH);
 } 
}

void ShowSerialData()
{
  while(mySerial.available()!=0)
  
    Serial.write(mySerial.read());
    
}

float TempProcess(bool ch)
{
  //returns the temperature from one DS18B20 in DEG Celsius
  static byte data[12];
  static byte addr[8];
  static float TemperatureSum;
  if(!ch){
          if ( !ds.search(addr)) {
              Serial.println("no more sensors on chain, reset search!");
              ds.reset_search();
              return 0;
          }
          if ( OneWire::crc8( addr, 7) != addr[7]) {
              Serial.println("CRC is not valid!");
              return 0;
          }
          if ( addr[0] != 0x10 && addr[0] != 0x28) {
              Serial.print("Device is not recognized!");
              return 0;
          }
          ds.reset();
          ds.select(addr);
          ds.write(0x44,1); // start conversion, with parasite power on at the end
  }
  else{
          byte present = ds.reset();
          ds.select(addr);
          ds.write(0xBE); // Read Scratchpad
          for (int i = 0; i < 9; i++) { // we need 9 bytes
            data[i] = ds.read();
          }
          ds.reset_search();
          byte MSB = data[1];
          byte LSB = data[0];
          float tempRead = ((MSB << 8) | LSB); //using two's compliment
          TemperatureSum = tempRead / 15.25;
    }
          return TemperatureSum;
}
double avergearray(int* arr, int number){
  int i;
  int max,min;
  double avg;
  long amount=0;
  if(number<=0){
    return 0;
  }
  if(number<5){   //less than 5, calculated directly statistics
    for(i=0;i<number;i++){
      amount+=arr[i];
    }
    avg = amount/number;
    return avg;
  }else{
    if(arr[0]<arr[1]){
      min = arr[0];max=arr[1];
    }
    else{
      min=arr[1];max=arr[0];
    }
    for(i=2;i<number;i++){
      if(arr[i]<min){
        amount+=min;        //arr<min
        min=arr[i];
      }else {
        if(arr[i]>max){
          amount+=max;    //arr>max
          max=arr[i];
        }else{
          amount+=arr[i]; //min<=arr<=max
        }
      }//if
    }//for
    avg = (double)amount/(number-2);
  }//if
  return avg;
}
