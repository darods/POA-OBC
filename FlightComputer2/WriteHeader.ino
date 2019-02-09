// Write data header.
void writeHeader() {
  //The original function can be found in SdFat datalogger example
  file.print(F("Time,Altitude,FreeFall,Temperature,Pressure,Pitch,Roll,Yaw,Xnorm,Ynorm,Znorm"));
  Serial.print(F("Time,Altitude,FreeFall,Temperature,Pressure,Pitch,Roll,Yaw,Xnorm,Ynorm,Znorm"));

  file.println();
  Serial.println();
}
