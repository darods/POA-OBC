// ***********************************************************
// *******   ITG3205 Example Firmware by FLYTRON.COM    ******
// ***  Designed and Coded by Melih Karakelle on 2011      ***
// **       This Source code licensed under GPL             **
// ***********************************************************

#include <Wire.h>
#include <ITG3205.h>

ITG3205 itg3205;

void setup()
{
  Wire.begin(); 
  Serial.begin(9600);
  delay(100);
  itg3205.itg3205initGyro();
  delay(100);
  itg3205.itg3205CalGyro();
  delay(100);
}

void loop()
{
   itg3205.itg3205ReadGyro();
   Serial.print("Gyro(degree/s): ");
   Serial.print(itg3205.itg3205GyroX()); // Data to Degree conversation
   Serial.print(", ");
   Serial.print(itg3205.itg3205GyroY());
   Serial.print(", ");
   Serial.println(itg3205.itg3205GyroZ());
   
   Serial.print("Temperature: ");
   Serial.println(itg3205.itg3205Temp()) ;
   
   delay(100);
}
