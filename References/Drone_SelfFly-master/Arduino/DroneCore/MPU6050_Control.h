//
//  MPU6050_Controller.h
//  SelfFly
//
//  Created by Maverick on 2016. 1. 21..
//

#ifndef __MPU6050_CONTROL__
#define __MPU6050_CONTROL__

void _AccelGyro_GetGyroData();
void _AccelGyro_GetAccelData();
void _AccelGyro_GetGyroAccelData();
void _AccelGyro_Calibration();
void _AccelGyro_DispStatus(int nCase);

void _AccelGyro_Initialize()
{
    int                 i = 0;
    
    _AccelGyro_DispStatus(0);

    Wire.beginTransmission(IMU_ADDRESS);
    Wire.write(0x6B);                    
    Wire.write(0x00);                    
    Wire.endTransmission();              
    
    Wire.beginTransmission(IMU_ADDRESS);
    Wire.write(0x1B);                    
    Wire.write(0x08);                    
    Wire.endTransmission();              
    
    Wire.beginTransmission(IMU_ADDRESS);
    Wire.write(0x1C);
    Wire.write(0x08);
    Wire.endTransmission();
    
    Wire.beginTransmission(IMU_ADDRESS);
    Wire.write(0x1B);                    
    Wire.endTransmission();              
    Wire.requestFrom(IMU_ADDRESS, 1);
    while(Wire.available() < 1);         
    if(Wire.read() != 0x08)
    {
        _LED_Blink(1, 0, 0, 200000);
        while(1)
            delay(10);
    }
    
    Wire.beginTransmission(IMU_ADDRESS);
    Wire.write(0x1A);                    
    Wire.write(0x03);                    
    Wire.endTransmission();

    // For Magnetic Sensor
    //I2Cdev::writeBit(IMU_ADDRESS, 0x6A, 5, false);
    //I2Cdev::writeBit(IMU_ADDRESS, 0x37, 1, true);
    //I2Cdev::writeBit(IMU_ADDRESS, 0x6B, 6, false);

    delay(100);

    _AccelGyro_Calibration();
    
    _AccelGyro_DispStatus(3);

    bIsMPUInitialized = 1;
    
    return;
}


void _AccelGryo_GetRawGyroData()
{
    Wire.beginTransmission(IMU_ADDRESS);
    Wire.write(0x43);                                                            // starting with register 0x43 (GYRO_XOUT_H)
    Wire.endTransmission();
    Wire.requestFrom(IMU_ADDRESS, 6);                                            // request a total of 6 registers
    
    _gRawGyro[X_AXIS]  = (float)(Wire.read()<<8 | Wire.read());                  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
    _gRawGyro[Y_AXIS]  = (float)(Wire.read()<<8 | Wire.read());                  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
    _gRawGyro[Z_AXIS]  = (float)(Wire.read()<<8 | Wire.read());                  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
}


void _AccelGyro_GetGyroData()
{
    int                     i = 0;
  
    _AccelGryo_GetRawGyroData();
    
    for(i=0 ; i<=Z_AXIS ; i++)
        _gRawGyro[i] -= _gCalibMeanGyro[i];
    
    _gGyro_Roll = _gRawGyro[_gGyroAccelAxis[0]];
    if(_gMPUAxisRvrsFlag[0]) _gGyro_Roll *= -1.0;
    _gGyro_Pitch = _gRawGyro[_gGyroAccelAxis[1]];
    if(_gMPUAxisRvrsFlag[1]) _gGyro_Pitch *= -1.0;
    _gGyro_Yaw = _gRawGyro[_gGyroAccelAxis[2]];
    if(_gMPUAxisRvrsFlag[2]) _gGyro_Yaw *= -1.0;
}


void _AccelGryo_GetRawAccelData()
{
    Wire.beginTransmission(IMU_ADDRESS);
    Wire.write(0x3B);                                                            // starting with register 0x3B (ACCEL_XOUT_H)
    Wire.endTransmission();
    Wire.requestFrom(IMU_ADDRESS, 6);                                            // request a total of 6 registers
    
    _gRawAccel[X_AXIS] = (float)(Wire.read()<<8 | Wire.read());                  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
    _gRawAccel[Y_AXIS] = (float)(Wire.read()<<8 | Wire.read());                  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
    _gRawAccel[Z_AXIS] = (float)(Wire.read()<<8 | Wire.read());                  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
}


void _AccelGyro_GetAccelData()
{
    int                     i = 0;

    _AccelGryo_GetRawAccelData();
    
    _gAccel_X = _gRawAccel[_gGyroAccelAxis[1]];
    if(_gMPUAxisRvrsFlag[1]) _gAccel_X *= -1.0;
    _gAccel_Y = _gRawAccel[_gGyroAccelAxis[0]];
    if(_gMPUAxisRvrsFlag[0]) _gAccel_Y*= -1.0;
    _gAccel_Z = _gRawAccel[_gGyroAccelAxis[2]];
    if(_gMPUAxisRvrsFlag[2]) _gAccel_Z*= -1.0;
}


void _AccelGryo_GetRawGryoAccelData()
{
    Wire.beginTransmission(IMU_ADDRESS);
    Wire.write(0x3B);                                                            // starting with register 0x3B (ACCEL_XOUT_H)
    Wire.endTransmission();
    Wire.requestFrom(IMU_ADDRESS, 14);                                           // request a total of 14 registers
    
    _gRawAccel[X_AXIS] = (float)(Wire.read()<<8 | Wire.read());                  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
    _gRawAccel[Y_AXIS] = (float)(Wire.read()<<8 | Wire.read());                  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
    _gRawAccel[Z_AXIS] = (float)(Wire.read()<<8 | Wire.read());                  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
    _gTemperature      = (float)(Wire.read()<<8 | Wire.read());
    _gRawGyro[X_AXIS]  = (float)(Wire.read()<<8 | Wire.read());                  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
    _gRawGyro[Y_AXIS]  = (float)(Wire.read()<<8 | Wire.read());                  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
    _gRawGyro[Z_AXIS]  = (float)(Wire.read()<<8 | Wire.read());                  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
}


void _AccelGyro_GetGyroAccelData()
{
    int                     i = 0;

    _AccelGryo_GetRawGryoAccelData();
    
    for(i=0 ; i<=Z_AXIS ; i++)
        _gRawGyro[i] -= _gCalibMeanGyro[i];

    _gGyro_Roll = _gRawGyro[_gGyroAccelAxis[0]];
    if(_gMPUAxisRvrsFlag[0]) _gGyro_Roll *= -1.0;
    _gGyro_Pitch = _gRawGyro[_gGyroAccelAxis[1]];
    if(_gMPUAxisRvrsFlag[1]) _gGyro_Pitch *= -1.0;
    _gGyro_Yaw = _gRawGyro[_gGyroAccelAxis[2]];
    if(_gMPUAxisRvrsFlag[2]) _gGyro_Yaw *= -1.0;

    _gAccel_X = _gRawAccel[_gGyroAccelAxis[1]];
    if(_gMPUAxisRvrsFlag[1]) _gAccel_X *= -1.0;
    _gAccel_Y = _gRawAccel[_gGyroAccelAxis[0]];
    if(_gMPUAxisRvrsFlag[0]) _gAccel_Y*= -1.0;
    _gAccel_Z = _gRawAccel[_gGyroAccelAxis[2]];
    if(_gMPUAxisRvrsFlag[2]) _gAccel_Z*= -1.0;
}


void _AccelGyro_Calibration()
{
    int                     i = 0, j = 0;
    float                   nAccGyro[3] = {0.0, 0.0, 0.0};
    float                   nAccAccel[3] = {0.0, 0.0, 0.0};
    float                   nAccTemp = 0.0;
    int                     nCount = 2000;
    
    Serialprintln(F(" * Please wait for 10 seconds"));
    Serialprintln(F(" "));
    
    _AccelGyro_DispStatus(1);
    
    for(i=0 ; i<=Z_AXIS ; i++)
        _gCalibMeanGyro[i] = 0.0;
    
    for(i=0 ; i<nCount ; i++)
    {
        float               nRawTemp= 0.0;

        // We don't want the esc's to be beeping annoyingly.
        // So let's give them a 1000us puls while calibrating the gyro.
        // Set Digital Port 8, 9, 10, and 11 as high.
        _ESC_SetDefault();
            
        _AccelGryo_GetRawGryoAccelData();
        
        for(j=0 ; j<3 ; j++)
        {
            nAccGyro[j] += _gRawGyro[j];
            nAccAccel[j] += _gRawAccel[j];
        }
        nAccTemp += _gTemperature;
        
        if(0 == (i % 20))
            _LED_Blink(1, 0, 0, 100000);                                        // Blink LED with Period as 100ms

        if(0 == (i % 400))
            _AccelGyro_DispStatus(1);
    }
    
    for(i=0 ; i<=Z_AXIS ; i++)
    {
        _gCalibMeanGyro[i]  = nAccGyro[i] / nCount;
        _gCalibMeanAccel[i] = nAccAccel[i] / nCount;
    }
    _gCalibMeanTemp = nAccTemp / nCount;
    
    _AccelGyro_DispStatus(2);
}


void _AccelGyro_CheckAxis(int nAxisIdx)
{
    unsigned long           nCurrTime;
    byte                    nTmpAxis = B11111111, i = B11111111;
    float                   nAngle[3] = {0.0, 0.0, 0.0};
    
    if(0 == nAxisIdx)
    {
       // Check a Roll Axis
        Serialprintln(F("1. Please Lift the Left Wing to 30 degree for 10 seconds"));
        Serialprint(F("             Roll (Y) Axis is ["));
    }
    else if(1 == nAxisIdx)
    {
        // Check a Pitch Axis
        Serialprintln(F("2. Please Lift the Nose to 30 degree for 10 seconds"));
        Serialprint(F("             Picth (X) Axis is ["));
    }
    else if(2 == nAxisIdx)
    {
        // Check a Yaw Axis
        Serialprintln(F("3. Please Rotate the Nose to the right to 30 degree for 10 seconds"));
        Serialprint(F("             Yaw (Z) Axis is ["));
    }
    
    nCurrTime = millis() + 10000;
    while(nCurrTime > millis() && (B11111111 == nTmpAxis))
    {
        const unsigned long     nStartTime = micros();
        int                     i = 0;
        
        // Get Sensor (Gyro / Accel / Megnetic / Baro / Temp)      
        _AccelGryo_GetRawGyroData();
        
        // Calculate Roll, Pitch, and Yaw by Quaternion
        nAngle[X_AXIS] += _gRawGyro[X_AXIS] * ACCELGYRO_FS;
        nAngle[Y_AXIS] += _gRawGyro[Y_AXIS] * ACCELGYRO_FS;
        nAngle[Z_AXIS] += _gRawGyro[Z_AXIS] * ACCELGYRO_FS;

        for(i=0 ; i<3 ; i++)
        {
            if((nAngle[i] < -15) || (nAngle[i] > 15))
            {
                nTmpAxis = i;
                if(nAngle[i] < 0)
                    nTmpAxis |= B10000000;
                break;
            }
        }
        
        _Wait(nStartTime, 4000);
    }
    
    _gGyroAccelAxis[nAxisIdx] = nTmpAxis;
    
    Serialprint(_gGyroAccelAxis[nAxisIdx] & B00000011);
    if(B10000000 & _gGyroAccelAxis[nAxisIdx])
        Serialprintln(F("] and Reversed"));
    else
        Serialprintln(F("] and Not Reversed"));

    Serialprintln(F("     Please Return to the Center"));
    Serialprintln(F(" "));

    nCurrTime = millis() + 10000;
    while(nCurrTime > millis())
    {
        const unsigned long     nStartTime = micros();
        
        _AccelGryo_GetRawGyroData();
        
        // Calculate Roll, Pitch, and Yaw by Quaternion
        nAngle[X_AXIS] += _gRawGyro[X_AXIS] * ACCELGYRO_FS;
        nAngle[Y_AXIS] += _gRawGyro[Y_AXIS] * ACCELGYRO_FS;
        nAngle[Z_AXIS] += _gRawGyro[Z_AXIS] * ACCELGYRO_FS;

        if(((-5 <= nAngle[X_AXIS]) && (nAngle[X_AXIS] <= 5)) &&
           ((-5 <= nAngle[Y_AXIS]) && (nAngle[Y_AXIS] <= 5)) &&
           ((-5 <= nAngle[Z_AXIS]) && (nAngle[Z_AXIS] <= 5)))
            break;

        _Wait(nStartTime, 4000);
    }
}


void _AccelGyro_DispStatus(int nCase)
{
    static int              nCnt = 0;
    
    #if PRINT_SERIAL
        if(0 == nCase)
        {
            Serialprint(F(" *      "));
            Serialprint(_gDroneInitStep++);        
            Serialprintln(F(". Start MPU6050 Module Initialization   "));
        }
        else if(1 == nCase)
        {
            if(0 == nCnt++)
                Serialprint(F(" *          Calibrating "));
            else
                Serialprint(F("."));
        }
        else if(2 == nCase)
        {
            Serialprintln(F("."));
            
            Serialprintln(F(""));
            Serialprintln(F("        Calibrated Gyro & Accel Value"));
            Serialprint(F("             Gyro:"));
            Serialprint(F("  X:")); Serialprint(_gCalibMeanGyro[X_AXIS]);
            Serialprint(F("  Y:")); Serialprint(_gCalibMeanGyro[Y_AXIS]);
            Serialprint(F("  Z:")); Serialprintln(_gCalibMeanGyro[Z_AXIS]);
            Serialprint(F("             Accel:"));
            Serialprint(F("  X:")); Serialprint(_gCalibMeanAccel[X_AXIS]);
            Serialprint(F("  Y:")); Serialprint(_gCalibMeanAccel[Y_AXIS]);
            Serialprint(F("  Z:")); Serialprintln(_gCalibMeanAccel[Z_AXIS]);
        }
        else if(3 == nCase)
            Serialprintln(F(" *          => Done!!   "));
    #elif USE_LCD_DISPLAY
    {
        if(0 == nCase)
        {
            delay(500);
            _gLCDHndl.clear();

            _gLCDHndl.setCursor(0, 0);
            _gLCDHndl.print(_gDroneInitStep++);
            _gLCDHndl.setCursor(1, 0);
            _gLCDHndl.print(".Init MPU6050");
        }
        else if(1 == nCase)
        {
            _gLCDHndl.setCursor(nCnt++, 1);
            if(1 == nCnt)
            {
                delay(500);              
                _gLCDHndl.print("Calib:");
                nCnt += 5;
            }
            else
                _gLCDHndl.print(".");
        }
        else if(3 == nCase)
        {
            _gLCDHndl.setCursor(nCnt, 1);
            _gLCDHndl.print("Done!");
            delay(1000);
        }
    }
    #endif
}

#endif /* MPU6050_Controller_h */















