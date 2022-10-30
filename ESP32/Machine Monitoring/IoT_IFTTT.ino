#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif

#include <HTTPClient.h>

const int Analog_channel_pin = 32;
int ADC_VALUE = 0;
int ADC_VALUE_HIT = 0;
int ADC_MAX = 3340;
int ADC_MIN = 3054;
float voltage_value = 0.0;
float persentase = 0.0;
int last_pre = 0;
float persentase_2 = 0.0;
char ssid1[15];
char ssid2[15];
const int trigPin = 18;
const int led = 2;
int a = 0;
int b = 0;
int State = 0;
unsigned long tungguB = 0;
unsigned long tungguF = 0;
unsigned long tungguS = 0;
unsigned long tungguK = 0;
unsigned long tunggu30m = 0;
bool pre = false;
bool trigger2 = false;
bool trigger = false;
bool ledState = false;
String kirim;
int Bit;
const char* ssid = "FORMULATRIX_2.4GHz";
const char* password = "FmlxF@iry13";
const char* resource = "/trigger/IoTMonitoring_Trial/with/key/iV-DofTKfwBoeF-5RkO9SM6r5nz7BHHqizHAQISwkNS";
const char* server = "maker.ifttt.com";
uint64_t uS_TO_S_FACTOR = 1000000;  // Conversion factor for micro seconds to seconds
// sleep for 30 minutes = 1800 seconds
uint64_t TIME_TO_SLEEP = 1800;
void setup() {
  Serial.begin(115200);
  pinMode(Analog_channel_pin, INPUT);
  pinMode(led, OUTPUT);
  pinMode(trigPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(trigPin), sistem , CHANGE);
  uint64_t chipid = ESP.getEfuseMac(); // The chip ID is essentially its MAC address(length: 6 bytes).
  uint16_t chip = (uint16_t)(chipid >> 32);
  snprintf(ssid1, 15, "%04X", chip);
  snprintf(ssid2, 15, "%08X", (uint32_t)chipid);
  delay(2000);
  initWifi();

}
void loop()
{
  //  Serial.println(WiFi.localIP());
  unsigned long tunggu = millis();
  kedip();
  kirim30m();
  if (trigger2)
  {
    tungguK = tunggu;
    trigger = true;
    trigger2 = false;
  }
  if (trigger)
  {
    if ((unsigned long)(tunggu - tungguK) >= 100)
    {
      Serial.println(digitalRead(trigPin));
      if (digitalRead(trigPin) == 1) 
      {
        kirim = "FALSE";Bit = 0;
      }
      else 
      {
        kirim = "TRUE";Bit = 1;
      }
      ProsesKirim();
      trigger = false;
    }
  }

  //  Serial.println(WiFi.status());
}
void initWifi() {
  Serial.print("Connecting to: ");
  Serial.print(ssid);
  WiFi.begin(ssid, password);
  int timeout = 10 * 4; // 10 seconds
  while (WiFi.status() != WL_CONNECTED  ) { //&& (timeout-- > 0)) {
    delay(250);
    Serial.print(".");
  }

  Serial.println("");
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Failed to connect, going back to sleep");
  }
  else if ( WiFi.status() == WL_CONNECTED)
  {
    batt_level();
  }
  Serial.print("WiFi connected in: ");
  Serial.print(millis());
  Serial.print(", IP address: ");
  Serial.println(WiFi.localIP());

}
// Make an HTTP request to the IFTTT web service
void makeIFTTTRequest() {
  Serial.print("Connecting to ");
  Serial.print(server);

  WiFiClient client;
  int retries = 5;
  while (!!!client.connect(server, 80) && (retries-- > 0)) {
    Serial.print(".");
  }
  Serial.println();
  if (!!!client.connected()) {
    Serial.println("Failed to connect...");
  }

  Serial.print("Request resource: ");
  Serial.println(resource);
  // Temperature in Celsius
  String jsonObject = String("{\"value1\":\"") + kirim + "\",\"value2\":\"" + ssid1 + ssid2 + "\",\"value3\":\"" + last_pre + "%" + "\"}";

  client.println(String("POST ") + resource + " HTTP/1.1");
  client.println(String("Host: ") + server);
  client.println("Connection: close\r\nContent-Type: application/json");
  client.print("Content-Length: ");
  client.println(jsonObject.length());
  client.println();
  client.println(jsonObject);

  int timeout = 5 * 10; // 5 seconds
  while (!!!client.available() ) { //&& (timeout-- > 0)) {
    delay(100);
  }
  if (!!!client.available()) {
    Serial.println("No response...");
  }
  while (client.available()) {
    Serial.write(client.read());
  }

  Serial.println("\nclosing connection");
  client.stop();
}

void kirim_data() {
  String Machine, DevID;
  float Batt = last_pre;
  Machine = "YCM EV1020A";
  DevID = "ESP32-Y";

  String postData = (String)"Machine=" + Machine + "&Bit=" + Bit
                    + "&DevID=" + ssid1 + "&Batt=" + Batt;

  HTTPClient http;
  http.begin("HOSTNAME");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  auto httpCode = http.POST(postData);
  String payload = http.getString();

  Serial.println(postData);
  Serial.println(payload);

  http.end();
  Serial.println("udah ngirim HTTP");
}
