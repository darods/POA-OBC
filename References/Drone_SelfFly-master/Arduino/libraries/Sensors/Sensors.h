#ifndef __SENSORS_H__
#define __SENSORS_H__


#if ARDUINO >= 100
    #include "Arduino.h"
#else
    #include "WProgram.h"
#endif

#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include <Wire.h>
#endif

#if __GYROSCOPE_ENABLED__
    //#include <MPU6050.h>
    #include <MPU6050_6Axis_MotionApps20.h>
#endif

#if __COMPASS_ENABLED__
    #include <HMC5883L.h>
#endif

#if __BAROMETER_ENABLED__
    #include <MS561101BA.h>
#endif




#endif /* Sensors_h */
