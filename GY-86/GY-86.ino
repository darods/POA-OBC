/*Modificacion codigo de original del MS5611 : Korneliusz Jarzebski
 * *Modificacion codigo de original del MPU6050 : Tockn, un genio
 * Nota 1 : las variables estan en ingles, pues seria una perdida de tiempo
 * pasar cosas como realPressure a PresionReal, al final los que modifican el codigo
 * por lo general saben ingles.
 * 
 * Nota 2 : El sensor GY-86 tambien tiene un magnetometro, por lo que podria incluirse que 
 * tambien se pudiera conocer si el rocket esta en direccion norte y asi
 * tocaria incluirlo despues
 */

 //Librerias
#include <Wire.h>//Lib comun
#include <MPU6050_tockn.h>//Acelerometro y giroscopio, hecha por un genio
#include <MS5611.h>//Barometro

//Variables 
//Acelerometro/Giroscopio
MPU6050 mpu6050(Wire);
//Barometro
MS5611 ms5611;
double referencePressure;


void setup() {
  Serial.begin(9600);
  Wire.begin();
  //Acelerometro/Giroscopio
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);
  //Barometro
  while(!ms5611.begin()){
    Serial.println("Could not find a valid MS5611 sensor, check wiring!");
    delay(500);
  }referencePressure = ms5611.readPressure();
  checkSettings();

}

void checkSettings(){
  Serial.print("Oversampling: ");
  Serial.println(ms5611.getOversampling());
}

void loop() {
  mpu6050.update();
  long realPressure = ms5611.readPressure();
  float absoluteAltitude = ms5611.getAltitude(realPressure);
  float relativeAltitude = ms5611.getAltitude(realPressure, referencePressure);

  Serial.print("=================================================================\n\n "); 
  Serial.print("Pitch : "); 
  Serial.print(mpu6050.getAngleX());
  Serial.print("\tRoll : ");
  Serial.print(mpu6050.getAngleY());
  Serial.print("\tYaw : ");
  Serial.print(mpu6050.getAngleZ());
  Serial.println("\n");
  Serial.print(" absoluteAltitude = ");
  Serial.print(absoluteAltitude);
  Serial.print(" m, \trelativeAltitude = ");
  Serial.print(relativeAltitude);    
  Serial.println(" m");
  

  

}
