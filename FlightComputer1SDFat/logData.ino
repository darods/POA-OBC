// Log a data record.
void logData() {
  /*uint16_t data[ANALOG_COUNT];

  // Read all channels to avoid SD write latency between readings.
  for (uint8_t i = 0; i < ANALOG_COUNT; i++) {
    data[i] = analogRead(i);
  }
  // Write data to file.  Start with log time in micros.
  Serial.print(logTime);
  file.print(logTime);

  // Write ADC data to CSV record.
  for (uint8_t i = 0; i < ANALOG_COUNT; i++) {
    file.write(',');
    Serial.write(',');
    file.print(data[i]);
    Serial.print(data[i]);
  }
  file.println();
  Serial.println();
  */
    // Calcula los valores del MS5611
  // Read true temperature & Pressure
  double realTemperature = ms5611.readTemperature(true);
  long realPressure = ms5611.readPressure(true);
   // Calculate altitude
  //float absoluteAltitude = ms5611.getAltitude(realPressure);
  float relativeAltitude = ms5611.getAltitude(realPressure, referencePressure)/10;
  //if(relativeAltitude < -2){relativeAltitude = 0;}

  // Calcula los valores del MPU6050
  Vector normAccel = mpu.readNormalizeAccel();
  int pitch = -(atan2(normAccel.XAxis, sqrt(normAccel.YAxis*normAccel.YAxis + normAccel.ZAxis*normAccel.ZAxis))*180.0)/M_PI;
  int roll = (atan2(normAccel.YAxis, normAccel.ZAxis)*180.0)/M_PI;
  

  //Writing the data
  file.print(relativeAltitude);
  Serial.print(relativeAltitude);
  file.print(F(","));
  Serial.print(F(","));
  file.print(realTemperature);
  Serial.print(realTemperature);
  file.print(F(","));
  Serial.print(F(","));
  file.print(realPressure);
  Serial.print(realPressure);
  file.print(F(","));
  Serial.print(F(","));
  file.print(pitch);
  Serial.print(pitch);
  file.print(F(","));
  Serial.print(F(","));
  file.print(roll);
  Serial.print(roll);
  file.print(F(","));
  Serial.print(F(","));
  file.print(normAccel.XAxis);
  Serial.print(normAccel.XAxis);
  file.print(F(","));
  Serial.print(F(","));
  file.print(normAccel.YAxis);
  Serial.print(normAccel.YAxis);
  file.print(F(","));
  Serial.print(F(","));
  file.print(normAccel.ZAxis);
  Serial.print(normAccel.ZAxis);

  
  file.println();
  Serial.println();
}
