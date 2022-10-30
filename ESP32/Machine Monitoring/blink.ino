void kedip()
{
  if (millis() - tungguS >= 1000)
  {
    tungguS = millis();
    ledState = !ledState;
  }
  digitalWrite(led, ledState);
}
