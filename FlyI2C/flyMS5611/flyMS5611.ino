//Guide : https://github.com/ChristofSchmid/Baro-MS5611/blob/master/MS5611-I2C-Test.ino
#include <Wire.h>

//the addess of the MS5611
//int MS_sensor = 0x77;


void setup(){
  Serial.begin(9600);
  Wire.begin();
  MS5611SetUp();
  
}


void loop(){
    //recordPROM();
    MS5611SetUp();
    Serial.println(recordTemp());
    
    delay(500);
  }

void MS5611SetUp(){
    Wire.beginTransmission(0x77);
    Wire.write(0x1E);//the datasheet says that the first things you have to do after giving power to the arduino is running the "RESET" function
    //given by the registry 0x1E
    Wire.endTransmission();
  }

void recordADC(){
    Wire.beginTransmission(0x77);
    Wire.write(0x00);
    Wire.endTransmission();
    Wire.requestFrom(0x77,1);
    //Serial.println(Wire.read());
  }

void recordPROM(){
    Wire.beginTransmission(0x77);
    Wire.write(0xA2);
    Wire.endTransmission();
    Wire.requestFrom(0x77,1);
    //Serial.println(Wire.read());
  }

uint32_t recordTemp(){
    uint32_t value;
    Wire.beginTransmission(0x77);
    Wire.write(0x50);
    Wire.endTransmission();
    Wire.beginTransmission(0x77);
    Wire.write(0x00);
    Wire.endTransmission();
    Wire.requestFrom(0x77,3);

    uint8_t vxa = Wire.read();
    uint8_t vha = Wire.read();
    uint8_t vla = Wire.read();
    
    value = ((int32_t)vxa << 16) | ((int32_t)vha << 8) | vla;

    return value;
  }    
