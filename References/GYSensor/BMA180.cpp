#include <Arduino.h>
#include <wiring.h>

#include "bma180.H"

void BMA180::bma180Write(char add,char data)
{
	Wire.beginTransmission(BMA180_ADDRESS); 	// address of the accelerometer
	Wire.write(add);
	Wire.write(data);
	Wire.endTransmission();
}

void BMA180::bma180SoftReset()
{
	bma180Write(0x10,0xB6);
	delay(100);
}

void BMA180::bma180EnableWrite()
{	
	bma180Write(0x0D,0x10);
	delay(10);
}

void BMA180::bma180GetIDs(int *id, int *version)
{      
	Wire.beginTransmission(BMA180_ADDRESS); 	// address of the accelerometer
	Wire.write(0x00); 							// set read pointer to data
	Wire.endTransmission();
	Wire.requestFrom(BMA180_ADDRESS, 2);

	*id = Wire.read();
	*version= Wire.read();
  
}

void BMA180::bma180SetFilter(FILTER f)	// 10,20,40,75,150,300,600,1200, HP 1HZ,BP 0.2-300, higher values not authorized
{
	Wire.beginTransmission(BMA180_ADDRESS);		// address of the accelerometer
	Wire.write(0x20); 							// read from here
	Wire.endTransmission();
	Wire.requestFrom(BMA180_ADDRESS, 1);
	byte data = Wire.read();
	Wire.beginTransmission(BMA180_ADDRESS); 	// address of the accelerometer
	Wire.write(0x20);
	Wire.write((data & 0x0F) | f<<4); 			// low pass filter to 10 Hz
	Wire.endTransmission();
	delay(10);
}

void BMA180::bma180SetGSensitivty(GSENSITIVITY maxg) //1, 1.5 2 3 4 8 16
{
	Wire.beginTransmission(BMA180_ADDRESS); 	// address of the accelerometer
	Wire.write(0x35); 							// read from here
	Wire.endTransmission();
	Wire.requestFrom(BMA180_ADDRESS, 1);
	byte data = Wire.read();
	Wire.beginTransmission(BMA180_ADDRESS); 	// address of the accelerometer
	Wire.write(0x35);
	Wire.write((data & 0xF1) | maxg<<1);// range +/- 2g
	Wire.endTransmission();

	gSense = maxg;
}

void BMA180::bma180ReadAccel()
{  
	Wire.beginTransmission(BMA180_ADDRESS); 	// address of the accelerometer
	Wire.write(0x02); 							// set read pointer to data
	Wire.endTransmission();
	Wire.requestFrom(BMA180_ADDRESS,7);

	// read in the 3 axis data, each one is 16 bits
	// print the data to terminal
	//Serial.print("Accelerometer: X = ");
	x = Wire.read();
	x |= Wire.read() << 8;
	x = x >> 2;
	y = Wire.read();
	y |= Wire.read() << 8;
	y = y >> 2;
	z = Wire.read();
	z |= Wire.read() << 8;
	z = z >> 2;
	temp = Wire.read();
}

float BMA180::bma180GetgSense()
{
    switch(gSense)
    {
        case G1: 	return 1.0 * 0.0001250;
        case G15: 	return 1.5 * 0.0001875;
        case G2: 	return 2.0 * 0.0002500;
        case G3: 	return 3.0 * 0.0003750;
        case G4: 	return 4.0 * 0.0005000;
        case G8: 	return 8.0 * 0.0009900;
        case G16: 	return 16.0 * 0.0019800;
    }
}

short BMA180::bma180FloatX()
{
	return x;
}

short BMA180::bma180FloatY()
{
	return y;
}

short BMA180::bma180FloatZ()
{
	return z;
}

float BMA180::bma180GravityX()
{
	float data = bma180GetgSense();
	return x * data;
}

float BMA180::bma180GravityY()
{
	float data = bma180GetgSense();
	return y * data;
}

float BMA180::bma180GravityZ()
{
	float data = bma180GetgSense();
	return z * data;
}

float BMA180::bma180Temp()
{
	return map((int8_t)temp,-128,127,-400,875)/10.0;;
}


 