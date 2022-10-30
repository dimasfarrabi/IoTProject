
#include <Console.h>
#define BAUDRATE 115200
#include <SPI.h>
#include <LoRa.h>

void setup() 
{
  // Rocket Scream Mini Ultra Pro with the RFM95W only:
  // Ensure serial flash is not interfering with radio communication on SPI bus
//  pinMode(4, OUTPUT);
//  digitalWrite(4, HIGH);
  
  Bridge.begin(BAUDRATE);
  Console.begin();
  while (!Console) ; // Wait for console port to be available
  Console.println("Start Sketch");
  if (!LoRa.begin(915E6)) {
    Console.println("Starting LoRa failed!");
    while (1);
  }
}



void loop()
{
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    Console.print("Received packet '");
    String data = "";
    // read packet
    while (LoRa.available()) {
      //Console.print((char)LoRa.read());
      //Console.print((char)LoRa.read());
      data += (char)LoRa.read();
      Console.print(data);
    }

    // print RSSI of packet
    Console.print("' with RSSI ");
    Console.println(LoRa.packetRssi());
  }
}
