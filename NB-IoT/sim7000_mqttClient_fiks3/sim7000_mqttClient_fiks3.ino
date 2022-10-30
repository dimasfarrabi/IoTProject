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
const char* topicInit = "data2";

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

//------------------define EC & temp-----------------//
#include <OneWire.h>
#define StartConvert 0
#define ReadTemperature 1
const byte numReadings = 20;          //the number of sample times
byte EC_PIN = A0;                     //EC Meter analog output,pin on analog 1
byte TEMP_PIN = 4;                    //DS18B20 signal, pin on digital 2
unsigned int readings[numReadings];      // the readings from the analog input
byte index = 0;                          // the index of the current reading
unsigned long AnalogValueTotal = 0;                  // the running total
unsigned int AnalogAverage = 0,averageVoltage=0;                // the average
float temperature,ECcurrent;
OneWire ds(TEMP_PIN);                                // on digital pin 4
  
//-----------------------define pH-------------------//
const int PH_PIN  = A1;         //analog pin at A0
float pH = 0;
//pHStep = 0.171;               //for pH e201c sensor
float pHStep = 0.18;            //for pH dfrobot sensor

//---------------------------------------include library and defines RTC DS3231
#include <DS3231.h>
DS3231  rtc(SDA, SCL);    // RTC pin
Time  t;
int mnt;
int det;

float dailyRain = 0.0;
int centimeter = 7;

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
  rtc.setTime(13, 0, 20);    // Set time (24Hr)
  rtc.setDate(28 , 9, 2021);   // Set date
}

void readPH ()
{
   int readingADCph = analogRead(PH_PIN);
   Serial.print("ADC ph is = ");
   Serial.print(readingADCph);
   double pHVoltage = 5 / 1024.0 * readingADCph;
   Serial.print("       pH Voltage = ");
   Serial.print(pHVoltage, 3);
   pH = 7.00 + ((1.543 - pHVoltage) / pHStep);      //Po = 7.00 + ((voltagePh7 - pHVoltage) / PhStep);
   Serial.print("       the pH value = ");
   Serial.println(pH, 3);
}

void readEC()
{
    AnalogValueTotal = AnalogValueTotal - readings[index];    // read from the sensor:
    readings[index] = analogRead(EC_PIN);                // add the reading to the total:
    AnalogValueTotal = AnalogValueTotal + readings[index];     // advance to the next position in the array:
    index = index + 1;                                          // if we're at the end of the array...
    if (index >= numReadings)                                  // ...wrap around to the beginning:
    index = 0;                                                // calculate the average:
    AnalogAverage = AnalogValueTotal / numReadings;

    temperature = TempProcess(ReadTemperature);  // read the current temperature from the  DS18B20
    TempProcess(StartConvert);                   //after the reading,start the convert for next reading

    averageVoltage=AnalogAverage*(float)5000/1024;
    Serial.print("Analog value:");
    Serial.print(AnalogAverage);   //analog average,from 0 to 1023
    Serial.print("    Voltage:");
    Serial.print(averageVoltage);  //millivolt average,from 0mv to 4995mV
    Serial.print("mV    ");
    Serial.print("temp:");
    Serial.print(temperature);    //current temperature
    Serial.print("^C     EC:");

    float TempCoefficient=1.0+0.0185*(temperature-25.0);                //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.0185*(fTP-25.0));
    float CoefficientVolatge=(float)averageVoltage/TempCoefficient;
    if(CoefficientVolatge<150)Serial.println("No solution!");           //25^C 1413us/cm<-->about 216mv  if the voltage(compensate)<150,that is <1ms/cm,out of the range
    else if(CoefficientVolatge>3300)Serial.println("Out of the range!");        //>20ms/cm,out of the range
    else
    {
      if(CoefficientVolatge<=448)ECcurrent=6.84*CoefficientVolatge-64.32;         //1ms/cm<EC<=3ms/cm
      else if(CoefficientVolatge<=1457)ECcurrent=6.98*CoefficientVolatge-127;     //3ms/cm<EC<=10ms/cm
      else ECcurrent=5.3*CoefficientVolatge+2278;                                 //10ms/cm<EC<20ms/cm
      ECcurrent/=1000;                                                            //convert us/cm to ms/cm
      Serial.print(ECcurrent,2);                                                  //two decimal
      Serial.println("ms/cm");
    }
}

void kirimData(){
  
    if (!mqtt.connected()) {
    reconnect();
  }
    mqtt.loop();
  String packet = "";
 
  packet.concat(("{\"pH\":"));
  packet.concat(pH);
 
  packet.concat((",\"Conductivity\":"));
  packet.concat(ECcurrent);
  
  packet.concat((",\"Temperature\":"));
  packet.concat(temperature);

  packet.concat((",\"Rain_Gauge\":"));
  packet.concat(dailyRain);

  packet.concat((",\"Water_Level\":"));
  packet.concat(centimeter);
 
  packet.concat("}");
  Serial.println(packet);
  mqtt.publish(topicInit, packet.c_str());
  mqtt.subscribe(topicInit);
delay(8000);
}

void loop() {
readPH();
readEC();
  if(mnt == 10|mnt == 20||mnt == 30||mnt == 40||mnt == 50||mnt == 0) 
  {
    kirimData();
  }
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
          TemperatureSum = tempRead / 16;
    }
          return TemperatureSum;
}
