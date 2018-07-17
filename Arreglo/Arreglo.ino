//Librerias 
#include <Wire.h>//Lib comun 
#include <MPU6050_tockn.h>//Acelerometro y giroscopio, hecha por un genio
#include <Adafruit_BMP280.h>//Barometro
#include <Adafruit_Sensor.h>//Barometro

//Variables 
//Acelerometro/Giroscopio
MPU6050 mpu6050(Wire);
//Barometro
  
float alturaReferencia = 0; 
float alturaTotal = 0;
float alturaActual = 0;
                      
float presion=560; //la presion local, bogota esta fucking arriba
Adafruit_BMP280 bme;//I2C


void setup() {
  Serial.begin(9600);
  Wire.begin();
  //Acelerometro/Giroscopio
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);
  //Barometro
  if(!bme.begin()){
    Serial.println("error");
  } delay(100);//una pequeña espera para darle tiempo de iniciar
  alturaReferencia=bme.readAltitude(presion);//la diferencia para que el valor inicial sea cero
  
}

void loop() {
  mpu6050.update();

  if(!bme.begin()){
    Serial.println("error Barometro");
    while(!bme.begin());
  }
  alturaTotal=bme.readAltitude(presion);//la diferencia para que el valor inicial sea cero
  alturaActual = (alturaTotal-alturaReferencia);
  
  
 
  //bastante sencillo no hay mucho misterio, solo toma y muestra los datos
  Serial.print("Pitch : "); 
  Serial.print(mpu6050.getAngleX());
  Serial.print("\tRoll : ");
  Serial.print(mpu6050.getAngleY());
  Serial.print("\tYaw : ");
  Serial.print(mpu6050.getAngleZ());
  Serial.print("\nAlturas : ");
  Serial.print("\tActual : ");
  Serial.print(alturaActual);
  Serial.print("\tTotal : ");
  Serial.print(alturaTotal);
  Serial.println("\n");
 
}





