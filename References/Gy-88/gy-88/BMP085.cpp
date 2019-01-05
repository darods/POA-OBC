
#include <Arduino.h> 
#include "BMP085.h"

Cal_values c_val = Cal_values();

// Read 2 bytes from the BMP085
// First byte will be from 'address'
// Second byte will be from 'address'+1
int BMP085::bmp085ReadInt(unsigned char address)
{
  unsigned char msb, lsb;

  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.write(address);
  Wire.endTransmission();

  Wire.requestFrom(BMP085_ADDRESS, 2);
  while(Wire.available()<2)
    ;
  msb = Wire.read();
  lsb = Wire.read();

  return (int) msb<<8 | lsb;
}

// Read the uncompensated temperature value
unsigned int BMP085::bmp085ReadUT()
{
  unsigned int ut;

  // Write 0x2E into Register 0xF4
  // This requests a temperature reading
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.write(0xF4);
  Wire.write(0x2E);
  Wire.endTransmission();

  // Wait at least 4.5ms
  delay(5);

  // Read two bytes from registers 0xF6 and 0xF7
  ut = bmp085ReadInt(0xF6);
  return ut;
}

// Read the uncompensated pressure value
unsigned long BMP085::bmp085ReadUP()
{
  unsigned char msb, lsb, xlsb;
  unsigned long up = 0;

  // Write 0x34+(OSS<<6) into register 0xF4
  // Request a pressure reading w/ oversampling setting
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.write(0xF4);
  Wire.write(0x34 + (OSS<<6));
  Wire.endTransmission();

  // Wait for conversion, delay time dependent on OSS
  delay(2 + (3<<OSS));

  // Read register 0xF6 (MSB), 0xF7 (LSB), and 0xF8 (XLSB)
  msb = bmp085Read(0xF6);
  lsb = bmp085Read(0xF7);
  xlsb = bmp085Read(0xF8);

  up = (((unsigned long) msb << 16) | ((unsigned long) lsb << 8) | (unsigned long) xlsb) >> (8-OSS);

  return up;
}

// Read 1 byte from the BMP085 at 'address'
char BMP085::bmp085Read(unsigned char address)
{
  unsigned char data;

  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.write(address);
  Wire.endTransmission();

  Wire.requestFrom(BMP085_ADDRESS, 1);
  while(!Wire.available())
    ;

  return Wire.read();
}

// Stores all of the bmp085's calibration values into global variables
// Calibration values are required to calculate temp and pressure
// This function should be called at the beginning of the program
void BMP085::bmp085Calibration()
{
  c_val.ac1 = bmp085ReadInt(0xAA);
  c_val.ac2 = bmp085ReadInt(0xAC);
  c_val.ac3 = bmp085ReadInt(0xAE);
  c_val.ac4 = bmp085ReadInt(0xB0);
  c_val.ac5 = bmp085ReadInt(0xB2);
  c_val.ac6 = bmp085ReadInt(0xB4);
  c_val.b1 = bmp085ReadInt(0xB6);
  c_val.b2 = bmp085ReadInt(0xB8);
  c_val.mb = bmp085ReadInt(0xBA);
  c_val.mc = bmp085ReadInt(0xBC);
  c_val.md = bmp085ReadInt(0xBE);
}

// Calculate temperature in deg C
float BMP085::bmp085GetTemperature()
{
  long x1, x2;
	unsigned int ut = bmp085ReadUT();
	
  x1 = (((long)ut - (long)c_val.ac6)*(long)c_val.ac5) >> 15;
  x2 = ((long)c_val.mc << 11)/(x1 + c_val.md);
  b5 = x1 + x2;

  float temp = ((b5 + 8)>>4);
  temp = temp /10;

  return temp;
}

// Calculate pressure given up
// calibration values must be known
// b5 is also required so bmp085GetTemperature(...) must be called first.
// Value returned will be pressure in units of Pa.
long BMP085::bmp085GetPressure()
{
  long x1, x2, x3, b3, b6, p;
  unsigned long b4, b7;
	unsigned long up = bmp085ReadUP();
  b6 = b5 - 4000;
  // Calculate B3
  x1 = (c_val.b2 * (b6 * b6)>>12)>>11;
  x2 = (c_val.ac2 * b6)>>11;
  x3 = x1 + x2;
  b3 = (((((long)c_val.ac1)*4 + x3)<<OSS) + 2)>>2;

  // Calculate B4
  x1 = (c_val.ac3 * b6)>>13;
  x2 = (c_val.b1 * ((b6 * b6)>>12))>>16;
  x3 = ((x1 + x2) + 2)>>2;
  b4 = (c_val.ac4 * (unsigned long)(x3 + 32768))>>15;

  b7 = ((unsigned long)(up - b3) * (50000>>OSS));
  if (b7 < 0x80000000)
    p = (b7<<1)/b4;
  else
    p = (b7/b4)<<1;

  x1 = (p>>8) * (p>>8);
  x1 = (x1 * 3038)>>16;
  x2 = (-7357 * p)>>16;
  p += (x1 + x2 + 3791)>>4;

  long temp = p;
  return temp;
}

//Uncompensated caculation - in Meters 
float BMP085::calcAltitude(float pressure)
{
  float A = pressure/101325;
  float B = 1/5.25588;
  float C = pow(A,B);
  C = 1 - C;
  C = C /0.0000225577;  // ==> C = C / (1/44330)
                        // ==> C = C * 44330
  return C;
}
