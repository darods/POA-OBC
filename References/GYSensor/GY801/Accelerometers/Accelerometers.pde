/*
ADXL345_Example.pde - Example sketch for integration with an ADXL345 triple axis accelerometer.
Copyright (C) 2011 Love Electronics (loveelectronics.co.uk)

This program is free software: you can redistribute it and/or modify
it under the terms of the version 3 GNU General Public License as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

*/

// Include the Wire library so we can start using I2C.
#include <Wire.h>
// Include the Love Electronics ADXL345 library so we can use the accelerometer.
#include <ADXL345.h>

// Declare a global instance of the accelerometer.
ADXL345 accel;

void setup()
{
  // Begin by setting up the Serial Port so we can output our results.
  Serial.begin(9600);
  // Start the I2C Wire library so we can use I2C to talk to the accelerometer.
  Wire.begin();
  
  // Create an instance of the accelerometer on the default address (0x1D)
  accel = ADXL345();
  
  // Check that the accelerometer is infact connected.
  if(accel.EnsureConnected())
  {
    Serial.println("Connected to ADXL345.");
  }
  else 
  {
    Serial.println("Could not connect to ADXL345.");
  }
  
  // Set the range of the accelerometer to a maximum of 2G.
  accel.SetRange(2, true);
  // Tell the accelerometer to start taking measurements.
  accel.EnableMeasurements();
}

void loop()
{
  if(accel.IsConnected) // If we are connected to the accelerometer.
  {
    // Read the raw data from the accelerometer.
    AccelerometerRaw raw = accel.ReadRawAxis();
    //This data can be accessed like so:
    int xAxisRawData = raw.XAxis;
    
    // Read the *scaled* data from the accelerometer (this does it's own read from the accelerometer
    // so you don't have to ReadRawAxis before you use this method).
    // This useful method gives you the value in G thanks to the Love Electronics library.
    AccelerometerScaled scaled = accel.ReadScaledAxis();
    // This data can be accessed like so:
    float xAxisGs = scaled.XAxis;
    
    // We output our received data.
    Output(raw, scaled);
  }
}

// Output the data down the serial port.
void Output(AccelerometerRaw raw, AccelerometerScaled scaled)
{
   // Tell us about the raw values coming from the accelerometer.
   Serial.print("Raw:\t");
   Serial.print(raw.XAxis);
   Serial.print("   ");   
   Serial.print(raw.YAxis);
   Serial.print("   ");   
   Serial.print(raw.ZAxis);
   
   // Tell us about the this data, but scale it into useful units (G).
   Serial.print("   \tScaled:\t");
   Serial.print(scaled.XAxis);
   Serial.print("G   ");   
   Serial.print(scaled.YAxis);
   Serial.print("G   ");   
   Serial.print(scaled.ZAxis);
   Serial.println("G");
}

