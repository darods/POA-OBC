//
//  Misc.h
//  SelfFly
//
//  Created by Maverick on 2016. 1. 21..
//

#ifndef __MISC__
#define __MISC__


void _Check_Drone_Status()
{
    if(((_gRCSignal_L[_gRCChMap[CH_TYPE_THROTTLE]] + 50) > _gCompensatedRCVal[_gRCChMap[CH_TYPE_THROTTLE]]) && 
       ((_gRCSignal_L[_gRCChMap[CH_TYPE_YAW]] + 50) > _gCompensatedRCVal[_gRCChMap[CH_TYPE_YAW]]))
        _gDroneStatus = DRONESTATUS_READY;
    
    //When yaw stick is back in the center position start the motors (step 2).
    if((DRONESTATUS_READY == _gDroneStatus) &&
       ((_gRCSignal_L[_gRCChMap[CH_TYPE_THROTTLE]] + 50) < _gCompensatedRCVal[_gRCChMap[CH_TYPE_THROTTLE]]) &&
       ((_gRCSignal_M[_gRCChMap[CH_TYPE_YAW]] - 50) < _gCompensatedRCVal[_gRCChMap[CH_TYPE_YAW]]) &&
       ((_gRCSignal_M[_gRCChMap[CH_TYPE_YAW]] + 50) > _gCompensatedRCVal[_gRCChMap[CH_TYPE_YAW]]))
    {
        int         i = 0;
        
        _gEstimatedRPY[0]   = 0.0;
        _gEstimatedRPY[1]   = 0.0;
        _gEstimatedRPY[2]   = 0.0;
        _gAngleRoll         = 0.0;
        _gAnglePitch        = 0.0;
        _gAngleYaw          = 0.0;
        _gAnglePitchOut     = 0.0;
        _gAngleRollOut      = 0.0;
        _gAngleYawOut       = 0.0;
        _gbAngleSet         = false;

        //Reset the pid controllers for a bumpless start.
        for(i=0 ; i<3 ; i++)
        {
            _gRPY_PID[i].nP_ErrRate = 0.0;
            _gRPY_PID[i].nI_ErrRate = 0.0;
            _gRPY_PID[i].nD_ErrRate = 0.0;
            _gRPY_PID[i].nPrevErrRate = 0.0;
            _gRPY_PID[i].nBalance = 0.0;
        }
        
        _gDroneStatus = DRONESTATUS_START;
    }
    else if((DRONESTATUS_START == _gDroneStatus) &&
            ((_gRCSignal_L[_gRCChMap[CH_TYPE_THROTTLE]] + 50) > _gCompensatedRCVal[_gRCChMap[CH_TYPE_THROTTLE]]))
    {
        _gDroneStatus = DRONESTATUS_READY;
    }
    
    //Stopping the motors: throttle low and yaw right.
    if(((DRONESTATUS_START == _gDroneStatus) || (DRONESTATUS_READY == _gDroneStatus)) &&
       ((_gRCSignal_L[_gRCChMap[CH_TYPE_THROTTLE]] + 50) > _gCompensatedRCVal[_gRCChMap[CH_TYPE_THROTTLE]]) && 
       ((_gRCSignal_H[_gRCChMap[CH_TYPE_YAW]] - 50) < _gCompensatedRCVal[_gRCChMap[CH_TYPE_YAW]]))
        _gDroneStatus = DRONESTATUS_STOP;
    
    // Set LED Period as 200ms When Low Voltage
    //if((1030 > _gCurrBatteryVolt) && (600 < _gCurrBatteryVolt))
    //    nLEDPeriod = 200000;

    if(DRONESTATUS_STOP == _gDroneStatus)
        _LED_SetDroneStatusColor(1, 1, 0);                                 // Turn On a Red Light
    else if(DRONESTATUS_READY == _gDroneStatus)
        _LED_SetDroneStatusColor(0, 1, 1);                                 // Turn On a Green Light
    else if(DRONESTATUS_START == _gDroneStatus)
        _LED_SetDroneStatusColor(0, 0, 1);                                 // Turn On a Blue Light
    
    return;
}


void _GetRawSensorData()
{
    _gPrevSensorCapTime = _gCurrSensorCapTime;
    _gCurrSensorCapTime = micros();
    
    _gDiffTime = (_gCurrSensorCapTime - _gPrevSensorCapTime) / 1000000.0;
    
    // Get Gyro Raw Data && Accel Raw Data
    // Read Gyro & Accel: 0.67ms -+0.01ms
    if(1 == bIsMPUInitialized)
        _AccelGyro_GetGyroAccelData();
    
    // Get Magnetic Raw Data
    // Read Magnitude   : 0.44ms -+0.01ms
    if(1 == bIsMagnitudeInitialized)
        _Mag_GetData();
    
    // Get Barometer Raw Data
    if(1 == bIsBarometerInitialized)
        //_Barometer_GetData();
        
        // Get Sonar Raw Data
        if(1 == bIsSonarInitialized)
        {
            //_Sonar_GetData();
            //_Sonar_GetData_WithPeriod();
        }
}


void _Check_BatteryVolt()
{
    _gCurrBatteryVolt = (0.92 * _gCurrBatteryVolt) + (analogRead(PIN_CHECK_POWER_STAT) + 65) * 0.09853;
    
    //Serialprintln(_gCurrBatteryVolt);
}


int _EEPROM_Read(int nStartAddress, int nValidMode)
{
    byte                nEEPRomData[EEPROM_SIZE];                // EEPROM Data
    int                 i = 0;
    int                 nValidationChk = 0;
    int                 nEEPRomAddress = 0;
    
    // Write Range of Transmitter
    if(EEPROM_DATA_SIGN == nStartAddress)
    {
        Serialprintln(F(" *            => Read Drone Signiture   "));
        
        nEEPRomAddress = EEPROM_DATA_SIGN;
        
        if('M' != EEPROM.read(nEEPRomAddress++)) nValidationChk = -1;
        if('a' != EEPROM.read(nEEPRomAddress++)) nValidationChk = -1;
        if('v' != EEPROM.read(nEEPRomAddress++)) nValidationChk = -1;
        if('e' != EEPROM.read(nEEPRomAddress++)) nValidationChk = -1;
        if('r' != EEPROM.read(nEEPRomAddress++)) nValidationChk = -1;
        if('i' != EEPROM.read(nEEPRomAddress++)) nValidationChk = -1;
        if('c' != EEPROM.read(nEEPRomAddress++)) nValidationChk = -1;
        if('k' != EEPROM.read(nEEPRomAddress++)) nValidationChk = -1;
        
        if(0 == nValidationChk)
            Serialprintln(F(" *                Verified!! You can Fly, Maverick!!     "));
        else
            Serialprintln(F(" *                Verification Failed!!"));
    }
    else if(EEPROM_DATA_MPU_AXIS == nStartAddress)
    {
        Serialprint(F(" *            => Read MPU Type   "));
        
        nEEPRomAddress = EEPROM_DATA_MPU_AXIS0_TYPE;
        
        for(i=0 ; i<=Z_AXIS ; i++)
        {
            byte            nTmpAccelGyroAxis;

            nTmpAccelGyroAxis = EEPROM.read(nEEPRomAddress++);
            if((1 == nValidMode) && (_gGyroAccelAxis[i] != nTmpAccelGyroAxis))
                nValidationChk = -1;
            else
            {
                _gGyroAccelAxis[i] = nTmpAccelGyroAxis;
                _gMPUAxisRvrsFlag[i] = (!(!(_gGyroAccelAxis[i] & B10000000)));
                _gGyroAccelAxis[i] &= B01111111;
                
                if((0 > _gGyroAccelAxis[i]) || (3 <= _gGyroAccelAxis[i]))
                    nValidationChk = -1;
                
                Serialprintln(F(" "));
                Serialprint(F("                  "));
                Serialprint(_gGyroAccelAxis[i]);
                Serialprint(F(" / "));
                Serialprint(_gMPUAxisRvrsFlag[i]);
            }
        }
        Serialprintln(F(" ")); Serialprintln(F(" "));
    }
    else if(EEPROM_DATA_MPU_CALIMEAN == nStartAddress)
    {
        Serialprintln(F(" *            => Read Calibrated MPU   "));
        
        nEEPRomAddress = EEPROM_DATA_MPU_CALIMEAN;

        Serialprint(F("                  "));
        for(i=0 ; i<=Z_AXIS ; i++)
        {
            float nTmpVal;
            EEPROM.get(nEEPRomAddress, nTmpVal);
            nEEPRomAddress += sizeof(float);
        
            if((1 == nValidMode) && (_gCalibMeanGyro[i] != nTmpVal))
                nValidationChk = -1;
            else
            {
                _gCalibMeanGyro[i] = nTmpVal;
                
                Serialprint(_gCalibMeanGyro[i]);
                Serialprint(F(" / "));
            }
        }
        Serialprintln(F(" "));

        Serialprint(F("                  "));
        for(i=0 ; i<=Z_AXIS ; i++)
        {
            float nTmpVal;
            EEPROM.get(nEEPRomAddress, nTmpVal);
            nEEPRomAddress += sizeof(float);
            
            if((1 == nValidMode) && (_gCalibMeanAccel[i] != nTmpVal))
                nValidationChk = -1;
            else
            {
                _gCalibMeanAccel[i] = nTmpVal;
                
                Serialprint(_gCalibMeanAccel[i]);
                Serialprint(F(" / "));
            }
        }
        Serialprintln(F(" ")); Serialprintln(F(" "));
    }
    else if(EEPROM_DATA_RC_TYPE == nStartAddress)
    {
        Serialprint(F(" *            => Read Transmitter Type   "));
        
        nEEPRomAddress = EEPROM_DATA_RC_CH0_TYPE;
        
        for(i=0 ; i<5 ; i++)
        {
            byte            nTmpRCChAxis;

            nTmpRCChAxis = EEPROM.read(nEEPRomAddress++);
            if((1 == nValidMode) && (_gRCChMap[i] != nTmpRCChAxis))
                nValidationChk = -1;
            else
            {
                _gRCChMap[i] = nTmpRCChAxis;
                _gRCRvrsFlag[i] = (!(!(_gRCChMap[i] & B10000000)));
                _gRCChMap[i] &= B01111111;

                if(i < 2)
                    _gRCRvrsFlag[i] = !(_gRCRvrsFlag[i]);
                    
                if((0 > _gRCChMap[i]) || (5 <= _gRCChMap[i]))
                    nValidationChk = -1;

                Serialprintln(F(" "));
                Serialprint(F("                  "));
                Serialprint(_gRCChMap[i]);
                Serialprint(F(" / "));
                Serialprint(_gRCRvrsFlag[i]);
            }
        }
        Serialprintln(F(" ")); Serialprintln(F(" "));
    }
    else if(EEPROM_DATA_RC_RANGE == nStartAddress)
    {
        Serialprint(F(" *            => Read Transmitter Range   "));
        
        nEEPRomAddress = EEPROM_DATA_RC_CH0_LOW_H;
        
        for(i=0 ; i<4 ; i++)
        {
            int             nTmpL, nTmpM, nTmpH;
            
            EEPROM.get(nEEPRomAddress, nTmpL);     nEEPRomAddress += sizeof(int);
            EEPROM.get(nEEPRomAddress, nTmpM);     nEEPRomAddress += sizeof(int);
            EEPROM.get(nEEPRomAddress, nTmpH);     nEEPRomAddress += sizeof(int);
            
            if((1 == nValidMode) &&
               ((_gRCSignal_L[i] != nTmpL) || (_gRCSignal_M[i] != nTmpM) || (_gRCSignal_H[i] != nTmpH)))
                nValidationChk = -1;
            else
            {
                _gRCSignal_L[i] = nTmpL;
                _gRCSignal_M[i] = nTmpM;
                _gRCSignal_H[i] = nTmpH;

                if((500 > _gRCSignal_L[i]) || (2100 <= _gRCSignal_L[i])) nValidationChk = -1;
                if((500 > _gRCSignal_M[i]) || (2100 <= _gRCSignal_M[i])) nValidationChk = -1;
                if((500 > _gRCSignal_H[i]) || (2100 <= _gRCSignal_H[i])) nValidationChk = -1;

                Serialprintln(F(" "));
                Serialprint(F("                  "));
                Serialprint(_gRCSignal_L[i]);
                Serialprint(F(" / "));
                Serialprint(_gRCSignal_M[i]);
                Serialprint(F(" / "));
                Serialprint(_gRCSignal_H[i]);
            }
        }
        Serialprintln(F(" ")); Serialprintln(F(" "));
    }
    
    delay(300);
    
    if(0 != nValidationChk)
        Serialprintln(F(" *              => Error! Invalid ROM Data!!   "));
    
    return nValidationChk;
}


void _EEPROM_Write(int nStartAddress)
{
    int                 i = 0;
    int                 nEEPRomAddress = 0;
    
    // Write Range of Transmitter
    if(EEPROM_DATA_SIGN == nStartAddress)
    {
        Serialprintln(F(" *            => Write Drone Signiture   "));
        
        nEEPRomAddress = EEPROM_DATA_SIGN;
        EEPROM.write(nEEPRomAddress++, 'M');
        EEPROM.write(nEEPRomAddress++, 'a');
        EEPROM.write(nEEPRomAddress++, 'v');
        EEPROM.write(nEEPRomAddress++, 'e');
        EEPROM.write(nEEPRomAddress++, 'r');
        EEPROM.write(nEEPRomAddress++, 'i');
        EEPROM.write(nEEPRomAddress++, 'c');
        EEPROM.write(nEEPRomAddress++, 'k');
    }
    else if(EEPROM_DATA_MPU_AXIS == nStartAddress)
    {
        Serialprintln(F(" *            => Write MPU Type   "));
        
        nEEPRomAddress = EEPROM_DATA_MPU_AXIS0_TYPE;

        Serialprint(F("                  "));
        for(i=0 ; i<=Z_AXIS ; i++)
        {
            EEPROM.write(nEEPRomAddress++, _gGyroAccelAxis[i]);
            
            Serialprint(_gGyroAccelAxis[i]);
            Serialprint(F(" / "));
        }
        Serialprintln(F(" "));
    }
    else if(EEPROM_DATA_MPU_CALIMEAN == nStartAddress)
    {
        Serialprintln(F(" *            => Write Calibrated MPU   "));
        
        nEEPRomAddress = EEPROM_DATA_MPU_CALIMEAN;
        
        Serialprint(F("                  "));
        for(i=0 ; i<=Z_AXIS ; i++)
        {
            EEPROM.put(nEEPRomAddress, _gCalibMeanGyro[i]);
            nEEPRomAddress += sizeof(float);
            
            Serialprint(_gCalibMeanGyro[i]);
            Serialprint(F(" / "));
        }
        Serialprintln(F(""));
        
        Serialprint(F("                  "));
        for(i=0 ; i<=Z_AXIS ; i++)
        {
            EEPROM.put(nEEPRomAddress, _gCalibMeanAccel[i]);
            nEEPRomAddress += sizeof(float);
            
            Serialprint(_gCalibMeanAccel[i]);
            Serialprint(F(" / "));
        }
        Serialprintln(F(" "));
    }
    else if(EEPROM_DATA_RC_TYPE == nStartAddress)
    {
        Serialprintln(F(" *            => Write Transmitter Type   "));
        
        nEEPRomAddress = EEPROM_DATA_RC_CH0_TYPE;
        
        Serialprint(F("                  "));
        for(i=0 ; i<5 ; i++)
        {
            EEPROM.write(nEEPRomAddress++, _gRCChMap[i]);
            
            Serialprint(_gRCChMap[i]);
            Serialprint(F(" / "));
        }
        Serialprintln(F(" "));
    }
    else if(EEPROM_DATA_RC_RANGE == nStartAddress)
    {
        Serialprintln(F(" *            => Write Transmitter Range   "));
        
        nEEPRomAddress = EEPROM_DATA_RC_CH0_LOW_H;
        
        for(i=0 ; i<4 ; i++)
        {
            EEPROM.put(nEEPRomAddress, _gRCSignal_L[i]);  nEEPRomAddress += sizeof(int);  
            EEPROM.put(nEEPRomAddress, _gRCSignal_M[i]);  nEEPRomAddress += sizeof(int);  
            EEPROM.put(nEEPRomAddress, _gRCSignal_H[i]);  nEEPRomAddress += sizeof(int);  

            Serialprint(F("                  "));
            Serialprint(_gRCSignal_L[i]);
            Serialprint(F(" / "));
            Serialprint(_gRCSignal_M[i]);
            Serialprint(F(" / "));
            Serialprintln(_gRCSignal_H[i]);
        }
        Serialprintln(F(" "));
    }
    
    delay(300);
    
    Serialprintln(F(" *                => Done!!   "));
}


void _EEPROM_Clear()
{
    int                 i = 0;
    
    for(i=EEPROM_DATA_RESERVED ; i<EEPROM_DATA_MAX ; i++)
        EEPROM.write(i, 0);
}

#endif /* Misc_h */

















