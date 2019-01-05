//Arduino 1.0+ Only
//Arduino 1.0+ Only

/*Based largely on code by  Jim Lindblom

 Get pressure, altitude, and temperature from the BMP085.
 Serial.print it out at 9600 baud to serial monitor.
 */

#include <Wire.h>
#include <BMP085.h>

BMP085 pressure_m;
void setup(){
  Serial.begin(9600);
  Wire.begin();

  pressure_m.bmp085Calibration();
}

void loop()
{
  float temperature = pressure_m.bmp085GetTemperature(); //MUST be called first
  float pressure = pressure_m.bmp085GetPressure();
  float altitude = pressure_m.calcAltitude(pressure);

  Serial.print("Temperature: ");
  Serial.print(temperature, 2); //display 2 decimal places
  Serial.println("deg C");

  Serial.print("Pressure: ");
  Serial.print(pressure, 0); //whole number only.
  Serial.println(" Pa");

  Serial.print("Altitude: ");
  Serial.print(altitude, 2); //display 2 decimal places
  Serial.println(" M");

  Serial.println();//line break

  delay(1000); //wait a second and get values again.
}
