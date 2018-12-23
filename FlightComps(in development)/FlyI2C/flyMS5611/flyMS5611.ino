#include <Wire.h>

long presion, temperatura;

void setup(){
  Serial.begin(9600);
  Wire.begin();
  //setMS5611();
}

void setMS5611(){
   Wire.beginTransmission(0x77); //esta es la direccion encontrada por el sketch y el fabricante
   Wire.write(0x1E);
   Wire.endTransmission();  
   Wire.beginTransmission(0x77); //esta es la direccion encontrada por el sketch y el fabricante
   Wire.write(0x48);
   Wire.endTransmission();  
   
}

void loop(){
    Wire.beginTransmission(0x77);
    //Wire.write(0x1E);
    Wire.endTransmission();
    Wire.requestFrom(0x46,0);
    presion = Wire.read() <<8 | Wire.read();
    //temperatura = Wire.read() <<8 | Wire.read();

    Serial.print("presion = ");
    Serial.println(presion);
    //Serial.print("temp = ");
    //Serial.println(temperatura);

    delay(1000);
    
  }
