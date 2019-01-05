/*
===Contact & Support===
Website: http://eeenthusiast.com/
Youtube: https://www.youtube.com/EEEnthusiast
Facebook: https://www.facebook.com/EEEnthusiast/
Patreon: https://www.patreon.com/EE_Enthusiast
Revision: 1.0 (July 13th, 2016)
===Hardware===
- Arduino Uno R3
- MPU-6050 (Available from: http://eeenthusiast.com/product/6dof-mpu-6050-accelerometer-gyroscope-temperature/)
===Software===
- Latest Software: https://github.com/VRomanov89/EEEnthusiast/tree/master/MPU-6050%20Implementation/MPU6050_Implementation
- Arduino IDE v1.6.9
- Arduino Wire library
===Terms of use===
The software is provided by EEEnthusiast without warranty of any kind. In no event shall the authors or 
copyright holders be liable for any claim, damages or other liability, whether in an action of contract, 
tort or otherwise, arising from, out of or in connection with the software or the use or other dealings in 
the software.
*/

#include <Wire.h>
//#include <math.h>
#include <SPI.h>
#include <SD.h>
#include <MS5611.h>


//Variables SD micro memory
File myFile;
int pinCS = 4; // Pin 4 on Arduino Uno
//String data;

//Variables for MPU6050
long accelX, accelY, accelZ;
float gForceX, gForceY, gForceZ;

long gyroX, gyroY, gyroZ;
float rotX, rotY, rotZ;

//float pitch ,roll, yaw;

//Variables for MS5611
MS5611 ms5611;
double referencePressure;
double realTemperature;
long realPressure;
float relativeAltitude;

void setup() {
  Serial.begin(9600);
  Wire.begin();

  /*if (SD.begin()) 
  {
    Serial.println("SD card is ready to use.");
  } else
  {
    Serial.println("SD card initialization failed");
    return;
  }
*/
  pinMode(pinCS, OUTPUT);  
  
  if (SD.begin()) 
  {
    Serial.println("SD card is ready to use.");
  } else
  {
    Serial.println("SD card initialization failed");
    return;
  }
  
  setupMPU();
    // Initialize MS5611 sensor
  Serial.println("Initialize MS5611 Sensor");

  while(!ms5611.begin(MS5611_ULTRA_HIGH_RES))
  {
    Serial.println("Could not find a valid MS5611 sensor, check wiring!");
    delay(500);
  }
  referencePressure = ms5611.readPressure();
  // Check settings
  checkSettings();
}


void loop() {
  recordAccelRegisters();
  recordGyroRegisters();
  RecordMS5611();
  
  String coma = String(',');
  String data = String(rotX+coma+rotY+coma+rotZ+coma+gForceX+coma+gForceY+coma+gForceZ+coma+realTemperature+coma+realPressure+coma+relativeAltitude);
  Serial.println(data);

  /*Save files in the micro SD card
  myFile = SD.open("flight.txt",FILE_WRITE);
  if(myFile){
      myFile.println(data);
      myFile.close();
    }
    //error file did not open
    Serial.println("error trying to open flight.txt");*/
  delay(100);
}

/*
void printData() {
  String coma = String(',');
  data = (rotX+coma+rotY+coma+rotZ+coma+gForceX+coma+gForceY+coma+gForceZ+coma+realTemperature+coma+realPressure+coma+relativeAltitude);
  /*Serial.print("Gyro (deg)");
  Serial.print(" X=");
  Serial.print(rotX);
  Serial.print(" Y=");
  Serial.print(rotY);
  Serial.print(" Z=");
  Serial.print(rotZ);
  Serial.print(" Accel (m/s)");
  Serial.print(" X=");
  Serial.print(gForceX);
  Serial.print(" Y=");
  Serial.print(gForceY);
  Serial.print(" Z=");
  Serial.print(gForceZ);
  Serial.print(" Altura = ");
  Serial.println(relativeAltitude);
  /*
  Serial.print (" pitch = ");
  Serial.print(pitch);
  Serial.print (" roll = ");
  Serial.print(roll);
  Serial.print (" yaw = ");
  Serial.println(yaw);
  
  
  Serial.println(data);
}*/
