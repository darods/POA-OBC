#include <Wire.h>
#include <MPU6050.h>
#include <MS5611.h>

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

MPU6050 mpu;
MS5611 ms5611;

double referencePressure;

void setup() 
{
  Serial.begin(115200);
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
  checkSettings1();
  
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

void checkSettings1()
{
  Serial.print("Oversampling: ");
  Serial.println(ms5611.getOversampling());
}

void loop()
{
  // Read raw values
  uint32_t rawTemp = ms5611.readRawTemperature();
  uint32_t rawPressure = ms5611.readRawPressure();

  // Read true temperature & Pressure
  double realTemperature = ms5611.readTemperature();
  long realPressure = ms5611.readPressure();

  // Calculate altitude
  float absoluteAltitude = ms5611.getAltitude(realPressure);
  float relativeAltitude = ms5611.getAltitude(realPressure, referencePressure);

  
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
  Serial.print(act.isFreeFall);// 1 si es caida libre, cero si no

  Serial.print(" rawTemp = ");
  Serial.print(rawTemp);
  Serial.print(", realTemp = ");
  Serial.print(realTemperature);
  Serial.print(" *C");

  Serial.print(" rawPressure = ");
  Serial.print(rawPressure);
  Serial.print(", realPressure = ");
  Serial.print(realPressure);
  Serial.print(" Pa");

  Serial.print(" absoluteAltitude = ");
  Serial.print(absoluteAltitude);
  Serial.print(" m, relativeAltitude = ");
  Serial.print(relativeAltitude);    
  Serial.println(" m");
  

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


