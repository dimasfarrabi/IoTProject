
//----------------define bridge and HTTP Client------------//
/*#include <Bridge.h>
#include <HttpClient.h>
#include <YunClient.h>
YunClient client;
#define BAUDRATE 115200   //Bridge Baudrate
IPAddress server (103,147,154,61);  //Server IP address
String payload =""; //initializing payload
*/
//---------------define LoRa------------------//
#include <SPI.h>
#include <RH_RF95.h>
RH_RF95 rf95;
float frequency = 923.0;  //set the frequncy

void setup() {
//---------------setup LoRa------------------//
  Serial.begin(9600);
  while (!Serial) ; // Wait for serial port to be available
  Serial.println("Start Sketch");
  if (!rf95.init())
    Serial.println("init failed");
  // Setup ISM frequency
  rf95.setFrequency(frequency);
  // Setup Power,dBm
  rf95.setTxPower(13);
  // Defaults BW Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
  Serial.print("Listening on frequency: ");
  Serial.println(frequency);
  
//--------------setup bridge------------------//
  /*pinMode(A2, OUTPUT);
  digitalWrite(A2, LOW);
  Bridge.begin(BAUDRATE);
  digitalWrite(A2, HIGH);
  Console.begin();
  while (!Console); // wait for a serial connection
*/
}

void loop() {
  if (rf95.available())
  {
    // Should be a message for us now   
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (rf95.recv(buf, &len))
    {
      RH_RF95::printBuffer("request: ", buf, len);
      Serial.print("got request: ");
      Serial.println((char*)buf);
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);
      
        /*
                 if (client.connect(server, 80)) {
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

      uint8_t data[] = "And hello back to you";
      rf95.send(data, sizeof(data));
      rf95.waitPacketSent();
      Serial.println("Sent a reply");
    }
    else
    {
      Serial.println("recv failed");
    }
  }

 
}
