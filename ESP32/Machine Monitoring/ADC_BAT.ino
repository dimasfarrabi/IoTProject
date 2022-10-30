void batt_level()
{
  if ((millis() - tungguB) >= 1000)
  {
    pinMode(Analog_channel_pin, INPUT);
    ADC_VALUE = analogRead(Analog_channel_pin);
    if (ADC_VALUE <= 3054) ADC_VALUE_HIT = 3054;
    else if (ADC_VALUE >= 3340) ADC_VALUE_HIT = 3340;
    else ADC_VALUE_HIT = ADC_VALUE;
    persentase = (ADC_VALUE_HIT - 3054.00) * 100.00 / 286.00; // * (2 / 5);
    //  voltage_value = (ADC_VALUE * 2.81 ) / (3340);
    Serial.print((int)persentase); Serial.print(" ");
    if (!pre)last_pre = (int)persentase;
    else
    {
      if ((int)persentase < last_pre) last_pre -= 1.00;
      else if ((int)persentase > last_pre) last_pre += 1.00;
      else last_pre = (int)persentase;
    }
    Serial.println(last_pre);
    pre = true;
    tungguB = millis();
  }
}
