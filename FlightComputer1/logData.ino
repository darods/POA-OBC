// Log a data record.
void logData() {
  //The original function can be found in SdFat datalogger example
   
  // Read the MS5611 values, true temperature & Pressure
  double realTemperature = ms5611.readTemperature(true);
  long realPressure = ms5611.readPressure(true);
  
  // Calculate altitude
  float relativeAltitude = ms5611.getAltitude(realPressure, referencePressure)/10;
  //if(relativeAltitude < -2){relativeAltitude = 0;}

  // Getting MPU6050 values
  
  // Read normalized values gyro
  Vector norm = mpu.readNormalizeGyro();
  Vector normAccel = mpu.readNormalizeAccel();
  Activites act = mpu.readActivites();
  pitch = pitch + norm.YAxis * timeStep;
  roll = roll + norm.XAxis * timeStep;
  yaw = yaw + norm.ZAxis * timeStep;


  //Writing the data
  Serial.print(logTime);
  file.print(logTime);
  file.print(F(","));
  Serial.print(F(","));
  file.print(relativeAltitude);
  Serial.print(relativeAltitude);
  file.print(F(","));
  Serial.print(F(","));
  file.print(act.isFreeFall);
  Serial.print(act.isFreeFall);
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
  file.print(yaw);
  Serial.print(yaw);
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
