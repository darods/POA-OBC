void checkSettingsMS5611()
{
  Serial.print("Oversampling: ");
  Serial.println(ms5611.getOversampling());
}

void MS5611Datos(){
  // Read raw values
  rawTemp = ms5611.readRawTemperature();
  rawPressure = ms5611.readRawPressure();

  // Read true temperature & Pressure
  realTemperature = ms5611.readTemperature();
  realPressure = ms5611.readPressure();

  // Calculate altitude
  absoluteAltitude = ms5611.getAltitude(realPressure);
  relativeAltitude = ms5611.getAltitude(realPressure, referencePressure);
  }
