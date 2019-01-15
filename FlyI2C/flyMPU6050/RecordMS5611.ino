void RecordMS5611(){
  realTemperature = ms5611.readTemperature(true);
  realPressure = ms5611.readPressure(true);

  // Calculate altitude
  //float absoluteAltitude = ms5611.getAltitude(realPressure);
  relativeAltitude = ms5611.getAltitude(realPressure, referencePressure)/10;
  
  }
