/*
MPU6050 : https://github.com/jarzebski/Arduino-MPU6050 by Korneliusz Jarzebski
MS5611 :  https://github.com/jarzebski/Arduino-MS5611 by Korneliusz Jarzebski
Sensor que se usa : GY-86
*/

#include <Wire.h>
#include <MPU6050.h>
#include <MS5611.h>
#include <SD.h>
#include <SPI.h>
#include "I2Cdev.h"

File myFile;
int pinCS = 4; // Pin 4 on Arduino Uno

// Timers
float timeStep = 0.1;

// Pitch, Roll and Yaw values
float pitch = 0;
float roll = 0;
float yaw = 0;


//Variables MS5611

double realTemperature;
long realPressure;
float absoluteAltitude;
float relativeAltitude;
double referencePressure;

MPU6050 mpu;
MS5611 ms5611;

void setup() 
{
    // join I2C bus (I2Cdev library doesn't do this automatically)
    #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
        Wire.begin();
    #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
        Fastwire::setup(400, true);
    #endif
  Serial.begin(9600);

  pinMode(pinCS, OUTPUT);
  
  // SD Card Initialization
  if (SD.begin())
  {
    Serial.println("SD card is ready to use.");
  } else
  {
    Serial.println("SD card initialization failed");
    return;
  }
  
  // Initialize MS5611
  while(!ms5611.begin())
  {
    Serial.println("Could not find a valid MS5611 sensor, check wiring!");
    delay(500);
  }
  
  // Initialize MPU6050
  while(!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G))
  {
    Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
    delay(500);
  }
   
   
   // Get reference pressure for relative altitude
  referencePressure = ms5611.readPressure();
  // Check settings
  checkSettingsMS5611();
  
  // Calibrate gyroscope. The calibration must be at rest.
  // If you don't want calibrate, comment this line.
  mpu.calibrateGyro();

  // Set threshold sensivty. Default 3.
  // If you don't want use threshold, comment this line or set 0.
  mpu.setThreshold(3);

  /*
  mpu.setAccelPowerOnDelay(MPU6050_DELAY_3MS);
  
  mpu.setIntFreeFallEnabled(true);
  mpu.setIntZeroMotionEnabled(false);
  mpu.setIntMotionEnabled(false);
  
  mpu.setDHPFMode(MPU6050_DHPF_5HZ);
  mpu.setFreeFallDetectionThreshold(17);
  mpu.setFreeFallDetectionDuration(2);  
  
  checkSettingsMPU6050();
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);
  
  attachInterrupt(0, doInt, RISING);*/
}


void loop()
{  
 //Read normalized values gyro
 Vector norm = mpu.readNormalizeGyro();
 /* lee la aceleracion
  Vector rawAccel = mpu.readRawAccel();
  //lee la actividad xd
  Activites act = mpu.readActivites();
  */
  // Calculate Pitch, Roll and Yaw
  pitch = pitch + norm.YAxis * timeStep;
  roll = roll + norm.XAxis * timeStep;
  yaw = yaw + norm.ZAxis * timeStep;

  MS5611Datos();
  // Output raw
  String coma = String(',');
  String Pitch = String(pitch);
  String Roll = String(roll);
  String Yaw = String(yaw);
  String Temp = String(realTemperature);
  String Presion = String(realPressure);
  String Altura = String(relativeAltitude);
  String datos = String(Pitch+coma+Roll+coma+Yaw+coma+Temp+coma+Presion+coma+Altura);
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

  /*if (freefallDetected)
  {
    ledState = !ledState;
    digitalWrite(4, ledState);
    freefallBlinkCount++;
    if (freefallBlinkCount == 20)
    {
      freefallDetected = false;
      ledState = false;
      digitalWrite(4, ledState);
    }
  }
  */

  // Wait to full timeStep period
  delay(50);
}
