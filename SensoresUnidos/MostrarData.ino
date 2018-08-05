void mostrarData(){
  Serial.print(" Pitch = ");
  Serial.print(pitch);
  Serial.print(" Roll = ");
  Serial.print(roll);  
  Serial.print(" Yaw = ");
  Serial.print(yaw);
  //Serial.print(" MPUTemp = ");
  //Serial.print(mpu.readTemperature());
  //Serial.print(" *C  ");
  //Serial.print(" rawTemp = ");
  //Serial.print(rawTemp);
  Serial.print(" Temp = ");
  Serial.print(realTemperature);
  Serial.print(" *C");
  //Serial.print(" rawPressure = ");
  //Serial.print(rawPressure);
  Serial.print(" Presion = ");
  Serial.print(realPressure);
  Serial.print(" Pa");
  //Serial.print(" absoluteAltitude = ");
  //Serial.print(absoluteAltitude);
  Serial.print(" Altura = ");
  Serial.print(relativeAltitude);    
  Serial.println(" m"); 
  //Serial.print(" CL = ");
  }

  void mostrarDataSD(){
    myFile.print(pitch);
    myFile.print(",");    
    myFile.println(roll);
    myFile.print(",");     
    myFile.println(yaw);
    myFile.print(","); 
    myFile.println(realTemperature);
    myFile.print(","); 
    myFile.println(realPressure);
    myFile.print(","); 
    myFile.println(relativeAltitude);
    myFile.close(); // close the file
    }
