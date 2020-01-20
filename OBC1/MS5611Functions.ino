void checkSettings()
{
  file.print(F("Oversampling: "));
  file.print(ms5611.getOversampling());
}
