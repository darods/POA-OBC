/*
MPU6050 : https://github.com/jarzebski/Arduino-MPU6050 by Korneliusz Jarzebski
MS5611 :  https://github.com/jarzebski/Arduino-MS5611 by Korneliusz Jarzebski
Sensor que se usa : GY-86
*/

#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <MPU6050.h>
#include <MS5611.h>

//Variables memoria SD
File myFile;
int pinCS = 4; // Pin 4 on Arduino Uno

//Declaramos los Sensores
MPU6050 mpu;
MS5611 ms5611;

//Varaibles MS5611
double referencePressure;

void setup()
{ Serial.begin(115200);

// SD Card Initialization
  pinMode(pinCS, OUTPUT);  
  
  if (SD.begin()) 
  {
    Serial.println("SD card is ready to use.");
  } else
  {
    Serial.println("SD card initialization failed");
    return;
  }
  
  // Initialize MPU6050
  while(!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G))
  {
    Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
    delay(500);
  }

  
  // Initialize MS5611 sensor
  Serial.println("Initialize MS5611 Sensor");

  while(!ms5611.begin())
  {
    Serial.println("Could not find a valid MS5611 sensor, check wiring!");
    delay(500);
  }

  // Get reference pressure for relative altitude
  referencePressure = ms5611.readPressure();

  // Check settings
  checkSettings();
   
}

void checkSettings()
{
  Serial.print("Oversampling: ");
  Serial.println(ms5611.getOversampling());
}



void loop()
{  // Read normalized values 
  Vector normAccel = mpu.readNormalizeAccel();

  // Calculate Pitch & Roll
  int pitch = -(atan2(normAccel.XAxis, sqrt(normAccel.YAxis*normAccel.YAxis + normAccel.ZAxis*normAccel.ZAxis))*180.0)/M_PI;
  int roll = (atan2(normAccel.YAxis, normAccel.ZAxis)*180.0)/M_PI;
  
  uint32_t rawTemp = ms5611.readRawTemperature();
  uint32_t rawPressure = ms5611.readRawPressure();

  // Read true temperature & Pressure
  double realTemperature = ms5611.readTemperature();
  long realPressure = ms5611.readPressure();

  // Calculate altitude
  float absoluteAltitude = ms5611.getAltitude(realPressure);
  float relativeAltitude = ms5611.getAltitude(realPressure, referencePressure);
 
  // Output raw
  String coma = String(',');
  String Pitch = String(pitch);
  String Roll = String(roll);
  String Xnorm = String(normAccel.XAxis);
  String Ynorm = String(normAccel.YAxis);
  String Znorm = String(normAccel.ZAxis);
  String Altura = String(relativeAltitude); 
  
  String datos = String(Pitch+coma+Roll+coma+Xnorm+coma+Ynorm+coma+Znorm+coma+Altura);
  Serial.println(datos);
  

  myFile = SD.open("test.txt", FILE_WRITE);
  if (myFile) {    
    myFile.println(datos);
    myFile.close();
  }
  // if the file didn't open, print an error:
  else {
    Serial.println("error opening test.txt");
  }

  // Wait to full timeStep period
  delay(50);
}
