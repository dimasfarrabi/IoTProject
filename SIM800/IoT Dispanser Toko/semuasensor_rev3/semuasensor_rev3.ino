//----------------inisiasi library--------------//
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd (0x27,16,2);
#include <Wire.h>
#include <SoftwareSerial.h>
SoftwareSerial gprsSerial(10,11); 
#include <String.h>

//----------------inisiasi payload--------------//
String payload; //payload internet yang akan dikirim 
String payload_sms; //payload sms yang akan dikirim

//----------------inisiasi syntax keypad--------------//
#include <Keypad.h>
const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
//define the cymbols on the buttons of the keypads
char Keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {A7, A6, A5, A4}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {A3, A2, A1, A0}; //connect to the column pinouts of the keypad
Keypad keypad = Keypad( makeKeymap(Keys), rowPins, colPins, ROWS, COLS );

//----------------inisiasi tampilan menu--------------//
struct daftarMenu {
  char strMenu[17];
  uint32_t nomor_pilihan;  
  bool pilihan;            
};
char menuUtama[][17] = {
  "0..9 - Pilih    ",
  "* - Pilih       ",
  "# - Batal       ",
  "A - bahan_minum ",
  "B - bahan_alat  ",
  "C - Total/Pesan ",
  "D - Batal       ",
};
daftarMenu bahan_minum[] = {
  {"20KG bubuk milo          ",1L,  false}, 
  {"5KG jeruk peras        ",2L,  false},
  {"10Pcs teh tarik          ",3L,  false},
  {"20KG gula               ",4L,  false},
  {"20Galon air                ",5L,  false},
  {"10Bungkus es                 ",6L,  false},
};
daftarMenu bahan_alat[] = {
  {"10Pack sedotan              ",1L, false},
  {"30Pack gelas plastik          ",2L,  false},
  {"30Pack tutup gelas          ",3L,  false},
};

 
int8_t indexMenu = -1;
byte menuLevel = 0;
byte menuLevelSebelumnya = -1;
 
enum ModeMenu {
  modeMenuBahan,
  modeMenuAlat,
};
ModeMenu modeMenu;
#define standarWaktuTampil  1000L
#define LihatWaktuTampil    5000L
#define jumlahBahan       sizeof(bahan_minum)/sizeof(bahan_minum[0])
#define jumlahAlat       sizeof(bahan_alat)/sizeof(bahan_alat[0])
 
uint16_t waktuTampil;
unsigned long millisMulai;

//----------------inisiasi pin pushbutton dan status awal inputan flow--------------//
byte tombol_1= 5;
byte tombol_2=6;
byte tombol_3=7;
int nilai1;
int nilai2;
int nilai3;
int rate1=0;
int rate2=0;
int rate3=0;
boolean execute=false;
void(*saya_reset) (void) = 0;

void setup(){
  lcd.init();
  lcd.backlight();
  gprsSerial.begin(9600);               // the GPRS baud rate   
  Serial.begin(9600);    // the GPRS baud rate 
  delay(1000);

   
//--setup pin pushbutton sbg input---//
  pinMode(tombol_1, INPUT);
  pinMode(tombol_2, INPUT);
  pinMode(tombol_3, INPUT);
  millisMulai = millis();
  resetPilihan();
}

//----------------syntax untuk kirim internet SIM800L--------------//
void kirimInternet() {
  if (gprsSerial.available())
    Serial.write(gprsSerial.read());
 
  gprsSerial.println("AT");
  delay(700);
 ShowSerialData();
  gprsSerial.println("AT+CPIN?");
  delay(700);
 ShowSerialData();
  gprsSerial.println("AT+CREG?");
  delay(700);
 ShowSerialData();
  gprsSerial.println("AT+CGATT?");
  delay(700);
 ShowSerialData();
  gprsSerial.println("AT+CIPSHUT");
  delay(700);
 ShowSerialData();
  gprsSerial.println("AT+CIPSTATUS");
  delay(1000);
 ShowSerialData();
  gprsSerial.println("AT+CIPMUX=0");
  delay(1000);
 ShowSerialData();
 
  gprsSerial.println("AT+CSTT=\"airtelgprs.com\"");//start task and setting the APN,
  delay(1000);
 
  ShowSerialData();
 
  gprsSerial.println("AT+CIICR");//bring up wireless connection
  delay(1000);
 
  ShowSerialData();
 
  gprsSerial.println("AT+CIFSR");//get local IP adress
  delay(1000);
 
  ShowSerialData();
 
  gprsSerial.println("AT+CIPSPRT=0");
  delay(1000);
 
  ShowSerialData();
  
  gprsSerial.println("AT+CIPSTART=\"TCP\",\"meminum.xyz\",\"80\"");//start up the connection
  delay(2000);

  ShowSerialData();
 
  gprsSerial.println("AT+CIPSEND");//begin send data to remote server
  delay(2000);
  ShowSerialData();
  
  String str="GET https://meminum.xyz/simpan/insert?" + (payload);
  Serial.println(str);
  gprsSerial.println(str);//begin send data to remote server 
  
  delay(2000);
  ShowSerialData();
 
  gprsSerial.println((char)26);//sending
  delay(2000);//waitting for reply, important! the time is base on the condition of internet 
  gprsSerial.println();
 
  ShowSerialData();

  gprsSerial.println("AT+CIPSHUT");//close the connection
  delay(100);
  ShowSerialData(); 
}

//----------------syntax untuk kirim SMS SIM800L--------------//
void kirimSMS() {
  Serial.println("Set format SMS ke ASCII"); //menset format SMS
  gprsSerial.println("AT+CMGF=1\r\n"); //Prosedur penulisan oleh module sim800l
  delay(1000); //waktu jeda 1 detik
 
  Serial.println("SIM800 Set SMS ke Nomor Tujuan");
  //silahkan ganti no hp nya menjadi no hp ANDA
  gprsSerial.println("AT+CMGS=\"089607763594\"\r\n");
  delay(1000);
   
  Serial.println("Module mengirimkan SMS ke no tujuan");
  //isi SMS yang ditujukan ke no hp diatas
  gprsSerial.println("beli bahan "+(payload_sms)); 
  delay(1000);
   
  Serial.println("Ketik pada keyboard Ctrl+Z atau ESC > keluar menu ini");
  gprsSerial.println((char)26);
  delay(1000);
  
  lcd.clear();
  lcd.setCursor(0,0); 
  lcd.print("SMS Dikirim!");
  lcd.setCursor(0,1); 
  lcd.print("tekan 'D'");
  delay(3000);
}
void ShowSerialData()
{
  while(gprsSerial.available()!=0)
  Serial.write(gprsSerial.read());
  delay(5000); 
}
void loop(){
//----------------program pushbutton--------------//
  nilai1= digitalRead(tombol_1);
  nilai2= digitalRead(tombol_2);
  nilai3= digitalRead(tombol_3);

//----------------inisiasi pin flow meter--------------//
  const int pinFlow_1 = 2;
  const int pinFlow_2 = 3;
  const int pinFlow_3 = 4;

//--------------syntax utk pin 1 flowmeter-------//
  if(nilai1 == 1){ //jika tombol akan masuk ke flow meter menggunakan mode input 
    pinMode(pinFlow_1, INPUT);
    if (digitalRead(pinFlow_1)==HIGH){ //kalau flow terdeteksi, counter akan on 
        lcd.clear();
        lcd.setCursor(0,0); //penempatan teks di lcd 
        lcd.print("milo");
        lcd.setCursor(0,1);
        lcd.print(rate1++); //nampilin tulisan milo
        delay(500);
        if (rate1 > 20 && execute == false){ //jika rate > 20, masuk ke fungsi kirim inet
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("kirim data");  
        payload = "&item_id=1&qty=1&jml_uang=6000&user_id=2";
        kirimInternet(); 
        execute = true;
        saya_reset();
        }
    }
  }

//--------------syntax utk pin 2 flowmeter-------//   
  if(nilai2 == 1){
    pinMode(pinFlow_2, INPUT);
    if (digitalRead(pinFlow_2)==HIGH){
        lcd.clear(); 
        lcd.setCursor(0,0);
        lcd.print("teh tarik");
        lcd.setCursor(0,1);
        lcd.print(rate2++);
        delay(500);
        if (rate2 > 20 && execute == false){
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("kirim data");
        payload="&item_id=3&qty=1&jml_uang=5000&user_id=2";
        kirimInternet();
        execute = true;
        saya_reset();
        }
    }
  }

//--------------syntax utk pin 3 flowmeter-------//  
  if(nilai3 == 1){
    pinMode(pinFlow_3, INPUT);
    if (digitalRead(pinFlow_3)==HIGH){
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("jeruk peras");
        lcd.setCursor(0,1);
        lcd.print(rate3++);
        delay(500);
        if (rate3 > 20 && execute == false){
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("kirim data");
        payload="&item_id=4&qty=1&jml_uang=7000&user_id=2";
        kirimInternet(); 
        execute = true;
        saya_reset();
        }
    }
  }

//-----------------syntax untuk menu pada keypad----------------//
  char key = keypad.getKey(); 
 
  if (key) {
    Serial.println(key);
    switch (key)
    {
      case 'A':
        menuLevel = 1;
        indexMenu = -1;
        modeMenu = modeMenuBahan;
        break;
      case 'B':
        menuLevel = 1;
        indexMenu = -1;
        modeMenu = modeMenuAlat;
        break;
      case 'C':
        menuLevel = 2;
        updateMenu();
        break;
      case 'D':
        resetPilihan();
        menuLevel = 0;
        indexMenu = -1;
        updateMenu();
        break;
      case '*':
        if (menuLevel == 2)
        {
          pesananMasuk();
        }
        else
        {
          if (waktuTampil == LihatWaktuTampil)
          {
            if (modeMenu == modeMenuBahan)
            {
              bahan_minum[indexMenu].pilihan = true;
            }
            else if (modeMenu == modeMenuAlat)
            {
              bahan_alat[indexMenu].pilihan = true;
            }
            updateMenu();
          }
          else
          {
            waktuTampil = LihatWaktuTampil;
            millisMulai = millis();
          }
        }
        break;
      case '#':
        if (menuLevel == 2)
        {
          menuLevel = 1;
        }
        else
        {
          if (waktuTampil == LihatWaktuTampil)
          {
            if (modeMenu == modeMenuBahan)
            {
              bahan_minum[indexMenu].pilihan = false;
            }
            else if (modeMenu == modeMenuAlat)
            {
              bahan_minum[indexMenu].pilihan = false;
            }
            updateMenu();
          }
          else
          {
            waktuTampil = LihatWaktuTampil;
            millisMulai = millis();
          }
        }
        break;
      default:
        indexMenu = key - '1';
        updateMenu();
        millisMulai = millis();
        waktuTampil = LihatWaktuTampil;
        break;
    }
  }
 
  if (millis() - millisMulai > waktuTampil)
  {
    millisMulai = millis();
    waktuTampil = standarWaktuTampil;
    indexMenu++;
    updateMenu();
    menuLevelSebelumnya = menuLevel;
  }
}

//------------------syntax untuk pesanan masuk----------------//
void pesananMasuk()
{
  //Aksi pesanan masuk
  lcd.setCursor(0, 0);
  lcd.println("  Terima Kasih  ");
  lcd.setCursor(0, 1);
  lcd.println("Silahkan tunggu ");
  delay(3000);
  Serial.println("Pesanan masuk!!!"); 
  menuLevel = 0;
  indexMenu = -1;
}

//------------------syntax untuk update menu----------------//
void updateMenu()
{
  if (menuLevel == 0) //menu utama
  {
    if (indexMenu == sizeof(menuUtama) / sizeof(menuUtama[0]))
    {
      indexMenu = 0;
    }
    if (menuLevelSebelumnya != menuLevel)
    {
      lcd.clear();
      lcd.print("Selamat Datang");
    }
    tampilMenuUtama(indexMenu);
  }
  else if (menuLevel == 2) 
  {
    lcd.setCursor(0, 0);
    lcd.print(" ");
    formatStrnomor_pilihan(totalPilihan());
    lcd.setCursor(0, 1);
    lcd.print("* Ya   # kembali");
  }
  else if (modeMenu == modeMenuBahan)
  {
    if (indexMenu >= jumlahBahan)
    {
      indexMenu = 0;
    }
    tampilbahan_minum(indexMenu);
  }
  else if (modeMenu == modeMenuAlat)
  {
    if (indexMenu >= jumlahAlat)
    {
      indexMenu = 0;
    }
    tampilbahan_alat(indexMenu);
  }
}

//------------------syntax untuk reset pilihan----------------//
void resetPilihan()
{
  for (byte i = 0; i < jumlahBahan; i++)
  {
    bahan_minum[i].pilihan = false;
  }
  for (byte i = 0; i < jumlahAlat; i++)
  {
    bahan_alat[i].pilihan = false;
  }
}

//------------------syntax untuk reset pilihan----------------//
uint32_t totalPilihan()
{
  uint32_t total = 0;
  for (byte i = 0; i < jumlahBahan; i++)
  {
    if (bahan_minum[i].pilihan)
    {
      total += bahan_minum[i].nomor_pilihan;
    }
  }
  for (byte i = 0; i < jumlahAlat; i++)
  {
    if (bahan_alat[i].pilihan)
    {
      total += bahan_alat[i].nomor_pilihan;
    }
  }
  return total;
}


void tampilMenuUtama(byte index)
{
  lcd.setCursor(0, 1);
  lcd.print(menuUtama[index]);
}
void tampilbahan_minum(byte index)
{
  lcd.setCursor(0, 0);
  lcd.print(bahan_minum[index].strMenu);
  lcd.setCursor(0, 1);
  lcd.print("");
  formatStrnomor_pilihan(bahan_minum[index].nomor_pilihan);
  if (bahan_minum[index].pilihan)
  {
    lcd.setCursor(15, 1);
    lcd.print("*");
    payload_sms=(bahan_minum[index].strMenu);
    kirimSMS();
  }
}
void tampilbahan_alat(byte index)
{
  lcd.setCursor(0, 0);
  lcd.print(bahan_alat[index].strMenu);
  lcd.setCursor(0, 1);
  lcd.print("");
  formatStrnomor_pilihan(bahan_alat[index].nomor_pilihan);
  if (bahan_alat[index].pilihan)
  {
    lcd.setCursor(15, 1);
    lcd.print("*");
    payload_sms=(bahan_alat[index].strMenu);
    kirimSMS();
  }
}
void formatStrnomor_pilihan(uint32_t nomor_pilihan)
{
  String strnomor_pilihan = String(nomor_pilihan);
  uint8_t panjangStr = strnomor_pilihan.length();
  uint8_t offset = 3 - (panjangStr % 3);
  for (byte i = 0; i < strnomor_pilihan.length(); i++)
  {
    lcd.print(strnomor_pilihan[i]);
    if (!((strnomor_pilihan.length() + i - offset + 1) % 3))
    {
      if (i != strnomor_pilihan.length() - 1)
      {
        lcd.print('.');
      }
    }
  }
  for (byte i = 0; i < 16 - 5 - strnomor_pilihan.length(); i++)
  {
    lcd.print(' ');
  }
}
