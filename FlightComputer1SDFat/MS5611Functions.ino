void checkSettings()
{
  Serial.print(F("Oversampling: "));
  Serial.println(ms5611.getOversampling());
}
