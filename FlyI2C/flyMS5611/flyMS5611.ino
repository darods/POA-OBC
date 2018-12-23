#include <Wire.h>

long presion, temperatura;

void setup(){
  Serial.begin(9600);
  Wire.begin();
  setMS5611();
}

setMS5611(){
   Wire.beginTransmission(0x77); //esta es la direccion encontrada por el sketch y el fabricante
   Wire.write(); 
}
