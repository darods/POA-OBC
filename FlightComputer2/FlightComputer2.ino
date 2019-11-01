/* Written by el-nasa (Daniel Alejandro Rodriguez) 2019
 *  GitHub : https://github.com/el-NASA
 *  Project : https://github.com/el-NASA/POA
 *  
* The main purpose of this arduino sketch is to provide an useful code to make the data logger for a cansat or a model rocket.
* The necessary libraries to execute it are:
*
*SdFat : https://github.com/greiman/SdFat by Bill Greiman
*MPU6050 : https://github.com/jarzebski/Arduino-MPU6050 by Korneliusz Jarzebski
*MS5611 :  https://github.com/jarzebski/Arduino-MS5611 by Korneliusz Jarzebski
*
* Reference Project : 
* Arduino Rocket Data Logger : https://www.instructables.com/id/Arduino-Rocket-Data-Logger/ by calmac_projects
*/

 //Declaring libraries
#include "SdFat.h"//It is a better version of the SD library that comes with arduino IDE, requires less memory overall and can do more cool stuff
#include <MPU6050.h>//Library for the  6 axis accelerometer and gyro
#include <Adafruit_BMP280.h>

const int buzzer = 5;
//Declaring the Sensors
MPU6050 mpu;
Adafruit_BMP280 bmp;

//MPU6050 variables
float timeStep = 0.1;//It is used for calculating pitch, roll and yaw, you can find the original example
//in the MPU6050 examples (MPU6050_gyro_pitch_roll_yaw)

// Pitch, Roll and Yaw values
float pitch = 0;
float roll = 0;
float yaw = 0;

//Free fall variables
boolean ledState = false;
boolean freefallDetected = false;
int freefallBlinkCount = 0;

//MS5611 Variables
double referencePressure;// It is used for calculating the reference alitude

//Variables BMP280
float referencia;

//SD fat configuration given by the SDFat datalogger example, modify it for  your porpose
//For making your data logger go to the WriteHeader and logData tabs
// SD chip select pin.  Be sure to disable any other SPI devices such as Enet.
const uint8_t chipSelect = 4;

// Interval between data records in milliseconds.
// The interval must be greater than the maximum SD write latency plus the
// time to acquire and write data to the SD to avoid overrun errors.
// Run the bench example to check the quality of your SD card.
const uint32_t SAMPLE_INTERVAL_MS = 500;

// Log file base name.  Must be six characters or less.
#define FILE_BASE_NAME "Data"
//------------------------------------------------------------------------------
// File system object.
SdFat sd;

// Log file.
SdFile file;

// Time in micros for next data record.
uint32_t logTime;

//==============================================================================
// User functions.  Edit writeHeader() and logData() for your requirements.

const uint8_t ANALOG_COUNT = 4;
//------------------------------------------------------------------------------
//WriteHeader function used to be here
//------------------------------------------------------------------------------
//logData function used to be here
//==============================================================================
// Error messages stored in flash.
#define error(msg) sd.errorHalt(F(msg))
//------------------------------------------------------------------------------
void setup() {
  const uint8_t BASE_NAME_SIZE = sizeof(FILE_BASE_NAME) - 1;
  char fileName[13] = FILE_BASE_NAME "00.csv";

  Serial.begin(9600);

  
  // Initializing MPU6050 according to MPU6050_free_fall example
  file.println(F("Initialize MPU6050 Sensor"));
  while(!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_16G))
    verificacion(buzzer);
  alertaInicio(buzzer);
  // Calibrate gyroscope. The calibration must be at rest.
  mpu.calibrateGyro();

  // Set threshold sensivty. Default 3.
  // If you don't want use threshold, comment this line or set 0.
  mpu.setThreshold(3);
  mpu.setDHPFMode(MPU6050_DHPF_5HZ);

  mpu.setFreeFallDetectionThreshold(17);
  mpu.setFreeFallDetectionDuration(2); 
  attachInterrupt(0, doInt, RISING); 

  //Initialize BMP280 sensor
  file.println(F("Initialize BMP280 Sensor"));   
  if (!bmp.begin())
    verificacion(buzzer);
  alertaInicio(buzzer);
  // Get reference pressure for relative altitude
  referencia = bmp.readAltitude(1013.25);
  

  
  // Initialize at the highest speed supported by the board that is
  // not over 50 MHz. Try a lower speed if SPI errors occur.
  if (!sd.begin(chipSelect, SD_SCK_MHZ(50))) {
    sd.initErrorHalt();
  }

  // Find an unused file name.
  if (BASE_NAME_SIZE > 6) {
    error("FILE_BASE_NAME too long");
  }
  while (sd.exists(fileName)) {
    if (fileName[BASE_NAME_SIZE + 1] != '9') {
      fileName[BASE_NAME_SIZE + 1]++;
    } else if (fileName[BASE_NAME_SIZE] != '9') {
      fileName[BASE_NAME_SIZE + 1] = '0';
      fileName[BASE_NAME_SIZE]++;
    } else {
      error("Can't create file name");
    }
  }
  if (!file.open(fileName, O_WRONLY | O_CREAT | O_EXCL)) {
    error("file.open");
  }
  // Read any Serial data.
  do {
    delay(10);
  } while (Serial.available() && Serial.read() >= 0);

  Serial.print(F("Logging to: "));
  Serial.println(fileName);

  // Write data header.
  writeHeader();

  // Start on a multiple of the sample interval.
  logTime = micros()/(1000UL*SAMPLE_INTERVAL_MS) + 1;
  logTime *= 1000UL*SAMPLE_INTERVAL_MS;
  alertaInicio(buzzer);
}


//------------------------------------------------------------------------------
void loop() {
  // Time for next record.
  logTime += 1000UL*SAMPLE_INTERVAL_MS;

  // Wait for log time.
  int32_t diff;
  do {
    diff = micros() - logTime;
  } while (diff < 0);

  // Check for data rate too high.
  if (diff > 10) {
    error("Missed data record");
  }

  logData();

  // Force data to SD and update the directory entry to avoid data loss.
  if (!file.sync() || file.getWriteError()) {
    error("write error");
  }

}
