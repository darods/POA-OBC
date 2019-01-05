
#ifndef BMP085_h
#define BMP085_h

#include <Arduino.h>
#include <Wire.h>

#define BMP085_ADDRESS 0x77  	// I2C address of BMP08
#define OSS	3  					// Oversampling Setting

struct Cal_values
{
	// Calibration values
	int ac1;
	int ac2;
	int ac3;
	unsigned int ac4;
	unsigned int ac5;
	unsigned int ac6;
	int b1;
	int b2;
	int mb;
	int mc;
	int md;
};

class BMP085
{
	public:
		void bmp085Calibration();
		float bmp085GetTemperature();
		long bmp085GetPressure();
		float calcAltitude(float pressure);
		
	protected:
		int bmp085ReadInt(unsigned char address);
		unsigned int bmp085ReadUT();
		unsigned long bmp085ReadUP();
		char bmp085Read(unsigned char address);
	private:
	
		// b5 is calculated in bmp085GetTemperature(...), this variable is also used in bmp085GetPressure(...)
		// so ...Temperature(...) must be called before ...Pressure(...).
		long b5;
};
#endif