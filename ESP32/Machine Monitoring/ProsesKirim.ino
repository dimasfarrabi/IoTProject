void kirim30m()
{
  if((millis() - tunggu30m) >= 1000*60*30)
  {
    if(!trigger2)
    {
      ProsesKirim();
    }
    tunggu30m = millis();
  }
}
void ProsesKirim()
{
//  initWifi();
  digitalWrite(led, HIGH);
  makeIFTTTRequest();
  kirim_data();
  Serial.print("ADC VALUE = "); Serial.println(ADC_VALUE);
  Serial.print(persentase_2); Serial.println("%");
  digitalWrite(led, LOW);
  //#ifdef ESP32
  //  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * 1);
  //  Serial.println("Going to sleep now");
  //  esp_deep_sleep_start();
  //#else
  //  Serial.println("Going to sleep now");
  //  ESP.deepSleep(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  //#endif

}
