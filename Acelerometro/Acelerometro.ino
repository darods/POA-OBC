#include <MPU6050_tockn.h>
#include <Wire.h>

MPU6050 mpu6050(Wire);

long timer = 0;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);
}

void loop() {
  mpu6050.update();
 
 

 if(mpu6050.getAccZ()>=1.9){
  Serial.println("CAIDA LIBRE DETECTADA");
 }

}
