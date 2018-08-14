/*
MPU6050 : https://github.com/jarzebski/Arduino-MPU6050 by Korneliusz Jarzebski
BMP280 :  https://github.com/adafruit/Adafruit_BMP280_Library by Adafruit Industries
*/

#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <MPU6050.h>
#include <Adafruit_BMP280.h>

//Variables memoria SD
File myFile;
int pinCS = 4; // Pin 4 on Arduino Uno




//algo del BMP280, creo que son direcciones de registro
#define BMP_SCK 13
#define BMP_MISO 12
#define BMP_MOSI 11 
#define BMP_CS 10

//conectados de manera I2C
MPU6050 mpu;
Adafruit_BMP280 bmp; 

void setup() 
{Serial.begin(115200);

  /*// SD Card Initialization
  pinMode(pinCS, OUTPUT);  
  
  if (SD.begin()) 
  {
    Serial.println("SD card is ready to use.");
  } else
  {
    Serial.println("SD card initialization failed");
    return;
  }*/

    // Initialize MPU6050
  while(!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G))
  {
    Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
    delay(500);
  }

  
  //inicio BMP280
  if (!bmp.begin()) {  
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
    while (1);
  }

  
}

void loop()
{
  // Calcula los valores del MPU6050
  Vector normAccel = mpu.readNormalizeAccel();
  int pitch = -(atan2(normAccel.XAxis, sqrt(normAccel.YAxis*normAccel.YAxis + normAccel.ZAxis*normAccel.ZAxis))*180.0)/M_PI;
  int roll = (atan2(normAccel.YAxis, normAccel.ZAxis)*180.0)/M_PI;
  float tempMPU6050 = mpu.readTemperature();

  // Calcula los valores del MPU6050
  // Read true temperature & Pressure
  double realTemperature = bmp.readTemperature();
  long realPressure = bmp.readPressure();

  // Calculate altitude
  float absoluteAltitude = bmp.readAltitude(745.24);
  float relativeAltitude = bmp.readAltitude(745.24);
 
 //Pone todos los valores como una cadena
  String coma = String(',');
  String Pitch = String(pitch);
  String Roll = String(roll);
  String Xnorm = String(normAccel.XAxis);
  String Ynorm = String(normAccel.YAxis);
  String Znorm = String(normAccel.ZAxis);
  String Temp1 = String(tempMPU6050);
  String Temp2 = String(realTemperature); 
  String Presion = String(realPressure);
  String Altura = String(relativeAltitude); 
  String timer = String (millis());
  
  String datos = String(Pitch+coma+Roll+coma+Xnorm+coma+Ynorm+coma+Znorm+coma+Temp1+coma+Temp2+coma+Presion+coma+Altura+coma+timer);
  Serial.println(datos);
  
  /*Guarda los datos en la tarjeta SD
  myFile = SD.open("test.txt", FILE_WRITE);
  if (myFile) {    
    myFile.println(datos);
    myFile.close();
  }
  // if the file didn't open, print an error:
  else {
    Serial.println("error opening test.txt");
  }*/
  // Wait to full timeStep period
  delay(50);
}
