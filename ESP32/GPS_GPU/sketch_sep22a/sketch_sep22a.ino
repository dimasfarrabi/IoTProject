struct Warning {
  const uint8_t pinWarning;
  uint32_t numberKeyPresses;
  bool pencetWarning;
};

const int tombolSwitchMode=4;
int Switch=0;
boolean modeState;


Warning tombolWarning = {2, 0, false};

void IRAM_ATTR isr() {
  tombolWarning.pencetWarning = true;
}

void setup() {
  Serial.begin(115200);
  pinMode(tombolWarning.pinWarning, INPUT_PULLUP);
  attachInterrupt(tombolWarning.pinWarning, isr, FALLING);
  pinMode(tombolSwitchMode, INPUT_PULLUP);   //setup the ESP32 pinout for pushbutton
}

void loop() {

 Switch = digitalRead(tombolSwitchMode);
  if(Switch == LOW){
    modeState = !modeState;
  }

  if (modeState == false) {
  // pattern 1
  Serial.println("Indoor mode 1");
  delay(1000);
  Serial.println("Indoor mode 2");
  delay(1000);
  Serial.println("Indoor mode 3");
  delay(1000);
  Serial.println("Indoor mode 4");
  delay(1000);
  
    if (tombolWarning.pencetWarning) {
      Serial.println("kondisi darurat!!");
      tombolWarning.pencetWarning = false;
    }
  }

  else  { // (if modeState == true)
  Serial.println("Outdoor mode 1");
   delay(1000);
  Serial.println("Outdoor mode 2");
   delay(1000);
  Serial.println("Outdoor mode 3");
   delay(1000);
  Serial.println("Outdoor mode 4");
   delay(1000);
    if (tombolWarning.pencetWarning) {
      Serial.println("kondisi darurat!!");
      tombolWarning.pencetWarning = false;
    }
  }
}
