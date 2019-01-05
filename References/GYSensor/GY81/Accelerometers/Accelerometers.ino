#include <Wire.h>
#include <BMA180.h>

BMA180 bma180;

void setup()
{
  int id,version;
  
  Wire.begin();
  Serial.begin(9600);
  
  bma180.bma180SoftReset();
  bma180.bma180EnableWrite();
  
  bma180.bma180GetIDs(&id,&version);
  Serial.print("Id = ");
  Serial.print(id,DEC);
  Serial.print(" v.");
  Serial.println(version,HEX);
  
  bma180.bma180SetFilter(bma180.F10HZ);
  bma180.bma180SetGSensitivty(bma180.G1);
  delay(100);
}

void loop()
{
    bma180.bma180ReadAccel(); 

    Serial.print("[");
    Serial.print(bma180.bma180FloatX());
    Serial.print(" ");
    Serial.print(bma180.bma180FloatY());
    Serial.print(" ");
    Serial.print(bma180.bma180FloatZ());
    Serial.print("] ");  
    
    Serial.print(" [");
    Serial.print(bma180.bma180GravityX());
    Serial.print("g ");
    Serial.print(bma180.bma180GravityY());
    Serial.print("g ");
    Serial.print(bma180.bma180GravityZ());
    Serial.println("g ]"); 
   
    Serial.print("T = ");
    Serial.println(bma180.bma180Temp(),1);
    
	Serial.println("\r\n"); 
	
    delay(1000);
}