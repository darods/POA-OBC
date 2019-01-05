#include <Arduino.h> 
#include "ITG3205.h"

void ITG3205::itg3205initGyro()
{
   Wire.beginTransmission(ITG3205_Address); 
   Wire.write(0x3E);  
   Wire.write(0x00);   
   Wire.endTransmission(); 
   
   Wire.beginTransmission(ITG3205_Address); 
   Wire.write(0x15);  
   Wire.write(0x07);   
   Wire.endTransmission(); 
   
   Wire.beginTransmission(ITG3205_Address); 
   Wire.write(0x16);  
   Wire.write(0x1E);   // +/- 2000 dgrs/sec, 1KHz, 1E, 19
   Wire.endTransmission(); 
   
   Wire.beginTransmission(ITG3205_Address); 
   Wire.write(0x17);  
   Wire.write(0x00);   
   Wire.endTransmission(); 
}

void ITG3205::itg3205ReadGyro()
{  
	Wire.beginTransmission(ITG3205_Address); 	// address of the accelerometer
	Wire.write(0x1B); 							// set read pointer to data
	Wire.endTransmission();
	Wire.requestFrom(ITG3205_Address,8);

	temp = Wire.read() << 8;
	temp |= Wire.read();
	x = Wire.read() << 8;
	x |= Wire.read();
	y = Wire.read() << 8;
	y |= Wire.read();
	z = Wire.read() << 8;
	z |= Wire.read();
	
	x = x - g_offx;
	y = y - g_offy;
	z = z - g_offz;
}

void ITG3205::itg3205CalGyro()
{
	int tmpx = 0;
	int tmpy = 0;
	int tmpz = 0; 

	g_offx = 0;
	g_offy = 0;
	g_offz = 0;
 
	for (char i = 0;i<10;i++)
	{
		delay(10);  
		itg3205ReadGyro();
		tmpx += x;
		tmpy += y;
		tmpz += z; 
    }  
	g_offx = tmpx/10;
	g_offy = tmpy/10;
	g_offz = tmpz/10;
}

float ITG3205::itg3205GyroX()
{
	//float data = bma180GetgSense();
	return x / 14.375;
}

float ITG3205::itg3205GyroY()
{
	//float data = bma180GetgSense();
	return y / 14.375;
}

float ITG3205::itg3205GyroZ()
{
	//float data = bma180GetgSense();
	return z / 14.375;
}

float ITG3205::itg3205Temp()
{
	return (35+((temp+13200) / 280));
}

