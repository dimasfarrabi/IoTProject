#include <SD.h>
const int chipSelect = 10;// SD Card CS pin
File myFile;

float dailyRain;
int level;
int tanggal,bulan,tahun,jam,menit,det; 
void setup(){
  Serial.begin(9600);
  delay(2000);
  SetupSDCard();
 }

void SetupSDCard(void)
{
  pinMode(chipSelect, OUTPUT);
  if (SD.begin())
  {
    Serial.println("SD card is ready to use.");
  }
  else
  {
    Serial.println("SD card initialization failed");
    return;
  }
}

void loop() {
  if (Serial.available()) {
    char c = Serial.peek(); // just look at the next character on the serial buffer
    if (c == '\r' || c == '\n') // ignore the carriage return and newline characters
      Serial.read(); // remove the character from serial buffer by reading it
    else {
      tanggal = Serial.parseInt();
      bulan = Serial.parseInt();
      tahun = Serial.parseInt();
      jam = Serial.parseInt();
      menit = Serial.parseInt();
      det = Serial.parseInt();
      dailyRain = Serial.parseFloat(); // read and parse the string representing the float value
      level = Serial.parseInt();
      Serial.print(tanggal);
      Serial.print("/");
      Serial.print(bulan);
      Serial.print("/");
      Serial.print(tahun);
      Serial.print(" - ");
      Serial.print(jam);
      Serial.print(":");
      Serial.print(menit);
      Serial.print(":");
      Serial.print(det);
      Serial.print(" , ");
      Serial.print(dailyRain);
      Serial.print(" , ");
      Serial.println(level);
      myFile = SD.open("datalog.txt", FILE_WRITE);
          if (myFile) {
          myFile.print(tanggal);
          myFile.print("/");
          myFile.print(bulan);
          myFile.print("/");
          myFile.print(tahun);
          myFile.print(" - ");
          myFile.print(jam);
          myFile.print(":");
          myFile.print(menit);
          myFile.print(":");
          myFile.print(det);
          myFile.print(" , ");
          myFile.print(dailyRain);
          myFile.print(" , ");
          myFile.println(level);
          myFile.close(); // close the file
          }
      // if the file didn't open, print an error:
      else {
          Serial.println("error opening datalog.txt");
          }
    }
  }

}
