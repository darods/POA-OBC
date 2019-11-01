// Log a data record.
void logData() {//The original function can be found in SdFat datalogger example

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
  file.print(bmp.readAltitude(1013.25)-referencia);
  file.print(F(","));
  file.print(act.isFreeFall);
  file.print(F(","));
  file.print(bmp.readTemperature());
  file.print(F(","));
  file.print(bmp.readPressure());
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
