#include <Wire.h>

//the addess of the MS5611
int MS_sensor = 0x77;


void setup(){
  Serial.begin(9600);
  Wire.begin();
  
}


void loop(){
  //Start request
  Wire.beginTransmission(MS_sensor);
  Wire.write(0x42);
  Wire.endTransmission();
  Wire.requestFrom(MS_sensor,24);
  while(Wire.available() == 0); //wait for response
  Serial.println(Wire.read());
  delay(3000);  
  }
