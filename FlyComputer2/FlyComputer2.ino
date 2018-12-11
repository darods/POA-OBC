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
int pinCS = 4;


//Declaramos los Sensores
MPU6050 mpu;
Adafruit_BMP280 bmp; 


//Variables MPU6050
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


//Variables BMP280
float referencia;


void setup() 

{ Serial.begin(38400);

  //SD Card Initialization
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
  while(!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_16G))
  {
    Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
    delay(500);
  }

  
  // Calibrate gyroscope. The calibration must be at rest.
  mpu.calibrateGyro();

  // Set threshold sensivty. Default 3.
  // If you don't want use threshold, comment this line or set 0.
  mpu.setThreshold(3);

  

  mpu.setDHPFMode(MPU6050_DHPF_5HZ);

  mpu.setFreeFallDetectionThreshold(17);
  mpu.setFreeFallDetectionDuration(2);  

  //pinMode(5, OUTPUT);
  //digitalWrite(5, LOW);
  
  attachInterrupt(0, doInt, RISING);

    //inicio BMP280
  if (!bmp.begin()) {  
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
    while (1);
  }

  referencia = bmp.readAltitude(1013.25);
}

void loop()
{
  // Read normalized values gyro
  Vector norm = mpu.readNormalizeGyro();
 // lee la aceleracion
  Vector rawAccel = mpu.readRawAccel();
  Vector normAccel = mpu.readNormalizeAccel();
  //lee la actividad xd
  Activites act = mpu.readActivites();

  // Calculate Pitch, Roll and Yaw
  pitch = pitch + norm.YAxis * timeStep;
  roll = roll + norm.XAxis * timeStep;
  yaw = yaw + norm.ZAxis * timeStep;

  // Output raw
  String coma = String(',');
  String Pitch = String(pitch);
  String Roll = String(roll);
  String Yaw = String(yaw);
  String Xnorm = String(normAccel.XAxis);
  String Ynorm = String(normAccel.YAxis);
  String Znorm = String(normAccel.ZAxis-2);
  String Temp1 = String(mpu.readTemperature());
  String Temp2 = String(bmp.readTemperature()); 
  String Presion = String(bmp.readPressure());
  String Altura = String(bmp.readAltitude(1013.25)-referencia);
  String Caida = String(act.isFreeFall); 
  String timer = String (millis());


  String datos = String(Pitch+coma+Roll+coma+Yaw+coma+Xnorm+coma+Ynorm+coma+Znorm+coma+Temp1+coma+Temp2+coma+Presion+coma+Altura+coma+Caida+coma+timer);
  Serial.println(datos);
  
  if (freefallDetected)
  {
    ledState = !ledState;

    digitalWrite(5, ledState);

    freefallBlinkCount++;

    if (freefallBlinkCount == 20)
    {
      freefallDetected = false;
      ledState = false;
      digitalWrite(5, ledState);
    }
  }
   

  // Wait to full timeStep period
  delay(50);
}
