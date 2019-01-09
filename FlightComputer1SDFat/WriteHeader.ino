// Write data header.
void writeHeader() {
  /*file.print(F("micros"));
  Serial.print(F("micros"));
  for (uint8_t i = 0; i < ANALOG_COUNT; i++) {
    file.print(F(",adc"));
    Serial.print(F(",adc"));
    file.print(i, DEC);
    Serial.print(i, DEC);
  }
  file.println();
  Serial.println();*/
  file.print(F("Time,Altitude,Temperature,Pressure,Pitch,Roll,Xnorm,Ynorm,Znorm"));
  Serial.print(F("Time,Altitude,Temperature,Pressure,Pitch,Roll,Xnorm,Ynorm,Znorm"));

  file.println();
  Serial.println();
}
