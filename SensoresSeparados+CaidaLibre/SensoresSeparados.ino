/*
MPU6050 : https://github.com/jarzebski/Arduino-MPU6050 by Korneliusz Jarzebski
BMP280 :  https://github.com/adafruit/Adafruit_BMP280_Library by Adafruit Industries
*/

#include <Wire.h>
#include <MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>


// Timers
float timeStep = 0.1;

// Pitch, Roll and Yaw values
float pitch = 0;
float roll = 0;
float yaw = 0;

//variables caida libre
boolean ledState = false;
boolean freefallDetected = false;
int freefallBlinkCount = 0;
//algo del BMP280, creo que son direcciones de registro
#define BMP_SCK 13
#define BMP_MISO 12
#define BMP_MOSI 11 
#define BMP_CS 10

//conectados de manera I2C
MPU6050 mpu;
Adafruit_BMP280 bmp; 

void setup() 
{
  Serial.begin(115200);
  
  //inicio BMP280
  if (!bmp.begin()) {  
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
    while (1);
  }

  // Initialize MPU6050
  while(!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G))
  {
    Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
    delay(500);
  }

  
  // Calibrate gyroscope. The calibration must be at rest.
  // If you don't want calibrate, comment this line.
  mpu.calibrateGyro();

  // Set threshold sensivty. Default 3.
  // If you don't want use threshold, comment this line or set 0.
  mpu.setThreshold(3);


  mpu.setAccelPowerOnDelay(MPU6050_DELAY_3MS);
  
  mpu.setIntFreeFallEnabled(true);
  mpu.setIntZeroMotionEnabled(false);
  mpu.setIntMotionEnabled(false);
  
  mpu.setDHPFMode(MPU6050_DHPF_5HZ);

  mpu.setFreeFallDetectionThreshold(17);
  mpu.setFreeFallDetectionDuration(2);  
  
  checkSettings();

  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);
  
  attachInterrupt(0, doInt, RISING);
}

void loop()
{
  // Read normalized values gyro
  Vector norm = mpu.readNormalizeGyro();
 // lee la aceleracion
  Vector rawAccel = mpu.readRawAccel();
  //lee la actividad xd
  Activites act = mpu.readActivites();

  // Calculate Pitch, Roll and Yaw
  pitch = pitch + norm.YAxis * timeStep;
  roll = roll + norm.XAxis * timeStep;
  yaw = yaw + norm.ZAxis * timeStep;

  // Output raw
  mostrarData();
  Serial.println(act.isFreeFall);// 1 si es caida libre, cero si no
  

  if (freefallDetected)
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
  

  // Wait to full timeStep period
  delay(50);
}
