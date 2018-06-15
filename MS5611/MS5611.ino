//Modificacion codigo de original de : Korneliusz Jarzebski
#include <Wire.h>
#include <MS5611.h>

MS5611 ms5611;

double referencePressure;

void setup() {
  Serial.begin(9600);
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
  long realPressure = ms5611.readPressure();
  float absoluteAltitude = ms5611.getAltitude(realPressure);
  float relativeAltitude = ms5611.getAltitude(realPressure, referencePressure);
  Serial.print(" absoluteAltitude = ");
  Serial.print(absoluteAltitude);
  Serial.print(" m, relativeAltitude = ");
  Serial.print(relativeAltitude);    
  Serial.println(" m");

  delay(1000);


}
