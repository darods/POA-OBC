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
  file.print(logTime);
  file.print(F(","));
  file.print(relativeAltitude);
  file.print(F(","));
  file.print(act.isFreeFall);
  file.print(F(","));
  file.print(realTemperature);
  file.print(F(","));
  file.print(realPressure);
  file.print(F(","));
  file.print(pitch);
  file.print(F(","));
  file.print(roll);
  file.print(F(","));
  file.print(yaw);
  file.print(F(","));
  file.print(normAccel.XAxis);
  file.print(F(","));
  file.print(normAccel.YAxis);
  file.print(F(","));
  file.print(normAccel.ZAxis);
  file.println();
}
