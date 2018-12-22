
#include <MPU6050.h>
#include <MS5611.h>

//Declaramos los Sensores
MPU6050 mpu;
MS5611 ms5611;

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

//Variables MS5611
double referencePressure;

void setup() {
  Serial.begin(9600);
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
  attachInterrupt(0, doInt, RISING); 

    // Initialize MS5611 sensor
  Serial.println("Initialize MS5611 Sensor");

  while(!ms5611.begin(/*MS5611_ULTRA_HIGH_RES*/))
  {
    Serial.println("Could not find a valid MS5611 sensor, check wiring!");
    delay(500);
  }

  // Get reference pressure for relative altitude
  referencePressure = ms5611.readPressure();
}

void loop() {
  // Read normalized values gyro
  Vector norm = mpu.readNormalizeGyro();
 // lee la aceleracion
    Vector normAccel = mpu.readNormalizeAccel();
  //lee la actividad xd
  Activites act = mpu.readActivites();

  // Calculate Pitch, Roll and Yaw
  pitch = pitch + norm.YAxis * timeStep;
  roll = roll + norm.XAxis * timeStep;
  yaw = yaw + norm.ZAxis * timeStep;

  long realPressure = ms5611.readPressure(true);
  float relativeAltitude = ms5611.getAltitude(realPressure, referencePressure);
  
  
  String coma = String(',');
  String Pitch = String(pitch);
  String Roll = String(roll);
  String Yaw = String(yaw);
  String Xnorm = String(normAccel.XAxis);
  String Ynorm = String(normAccel.YAxis);
  String Znorm = String(normAccel.ZAxis-2);
  String Altura = String(relativeAltitude); 
  String Caida = String(act.isFreeFall);
  String timer = String (millis());

  String datos = String(Pitch+coma+Roll+coma+Yaw+coma+Xnorm+coma+Ynorm+coma+Znorm+coma+Altura+coma+Caida+coma+timer);
  Serial.println(datos);
  delay(300);
}
