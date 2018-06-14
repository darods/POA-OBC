
#include <MPU6050_tockn.h>//un genio
#include <Wire.h>

MPU6050 mpu6050(Wire);

void setup() {
  Serial.begin(9600);
  Wire.begin();
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);
}

void loop() {
  mpu6050.update();
  //bastante sencillo no hay mucho misterio, solo toma y muestra los datos
  Serial.print("Pitch : "); 
  Serial.print(mpu6050.getAngleX());
  Serial.print("\tRoll : ");
  Serial.print(mpu6050.getAngleY());
  Serial.print("\tYaw : ");
  Serial.print(mpu6050.getAngleZ());
  Serial.println("\n");
  
  
}


