void mostrarData(){
  Serial.print(" Pitch = ");
  Serial.print(pitch);
  Serial.print(" Roll = ");
  Serial.print(roll);  
  Serial.print(" Yaw = ");
  Serial.print(yaw);
  Serial.print(" MPUTemp = ");
  Serial.print(mpu.readTemperature());
  Serial.print(" *C  ");
  Serial.print(" BMPTemp = ");
  Serial.print(bmp.readTemperature());
  Serial.print(" *C  ");
  Serial.print(" Presion = ");
  Serial.print(bmp.readPressure());
  Serial.print(" Pa ");
  Serial.print(" Altura = ");
  Serial.print(bmp.readAltitude(745.24));//https://keisan.casio.com/exec/system/1224579725
  Serial.print(" m ");//no he podido cuadra la altura, pues es un adato que para este sensor depende de las condiciones climaticas
  Serial.print(" CL = ");
  }
