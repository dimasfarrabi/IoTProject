void sistem()
{
  trigger2 = true;
  batt_level();
}
void sistemF()
{
  if (millis() - tungguF >= 1000 * 30)
  {
    ProsesKirim();
    tungguF = millis();
  }
}
