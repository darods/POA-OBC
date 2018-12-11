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


float referencia;

void setup() 
{
  Serial.begin(115200);
  
  //inicio BMP280
  if (!bmp.begin()) {  
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
    while (1);
  }

  // Initialize MPU6050
  while(!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_16G))
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
  
  //checkSettings();

  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);
  
  attachInterrupt(0, doInt, RISING);
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
   
   
  /*Serial.print(" Yaw = ");
  Serial.print(yaw);
  Serial.print(" MPUTemp = ");
  Serial.print(mpu.readTemperature());
  Serial.print(" *C  ");
  Serial.print(" BMPTemp = ");
  Serial.print(bmp.readTemperature());
  Serial.print(" *C  ");
  Serial.print(" Presion = ");
  Serial.print(bmp.readPressure());
  Serial.print(" Pa ");
  Serial.print(" Altura = ");
  Serial.print(bmp.readAltitude(1013.25)-referencia);//https://keisan.casio.com/exec/system/1224579725
  Serial.print(" m ");//no he podido cuadra la altura, pues es un adato que para este sensor depende de las condiciones climaticas
  Serial.print(" CL = ");
  Serial.println(act.isFreeFall);// 1 si es caida libre, cero si no
  */
  String datos = String(Pitch+coma+Roll+coma+Yaw+coma+Xnorm+coma+Ynorm+coma+Znorm+coma+Temp1+coma+Temp2+coma+Presion+coma+Altura+coma+Caida+coma+timer);
  Serial.println(datos);
  
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
