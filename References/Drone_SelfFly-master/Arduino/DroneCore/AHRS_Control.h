//
//  AHRS_Controller.h
//  SelfFly
//
//  Created by Maverick on 2016. 1. 21..
//

#ifndef __AHRS_CONTROL__
#define __AHRS_CONTROL__

void _Get_RollPitchYaw()
{
    //65.5 = 1 deg/sec (check the datasheet of the MPU-6050 for more information).
    _gEstimatedRPY[0] = (_gEstimatedRPY[0] * 0.7) + ((_gGyro_Roll / 65.5) * 0.3);   //Gyro pid input is deg/sec.
    _gEstimatedRPY[1] = (_gEstimatedRPY[1] * 0.7) + ((_gGyro_Pitch / 65.5) * 0.3);  //Gyro pid input is deg/sec.
    _gEstimatedRPY[2] = (_gEstimatedRPY[2] * 0.7) + ((_gGyro_Yaw / 65.5) * 0.3);    //Gyro pid input is deg/sec.
    
    // Gyro angle calculations
    _gAngleRoll  += ACCELGYRO_FS * _gGyro_Roll;
    _gAnglePitch += ACCELGYRO_FS * _gGyro_Pitch;

    // 0.00000133229 = ACCELGYRO_FS * (3.142(PI) / 180degr) The Arduino sin function is in radians
    _gAnglePitch -= _gAngleRoll * sin(_gGyro_Yaw * APPROX_SIN_SCALE);
    _gAngleRoll  += _gAnglePitch * sin(_gGyro_Yaw * APPROX_SIN_SCALE);
    
    // Accelerometer angle calculations
    // Calculate the total accelerometer vector.
    _gAccTotalVector = sqrt((_gAccel_X * _gAccel_X) + (_gAccel_Y * _gAccel_Y) + (_gAccel_Z * _gAccel_Z));

    if(abs(_gAccel_Y) < _gAccTotalVector)
        _gAnglePitchAcc = asin(_gAccel_Y / _gAccTotalVector) * RAD_TO_DEG_SCALE;

    if(abs(_gAccel_X) < _gAccTotalVector)    
        _gAngleRollAcc  = asin(_gAccel_X / _gAccTotalVector) * (-RAD_TO_DEG_SCALE);

    //Place the MPU-6050 spirit level and note the values in the following two lines for calibration.
    _gAnglePitchAcc -= 0.0;
    _gAngleRollAcc -= 0.0;
    
    _gAnglePitch = _gAnglePitch * 0.9996 + _gAnglePitchAcc * 0.0004;
    _gAngleRoll = _gAngleRoll * 0.9996 + _gAngleRollAcc * 0.0004;
      
    _gPitchLevelAdjust = _gAnglePitch * 15.0;
    _gRollLevelAdjust = _gAngleRoll * 15.0;
    
    if(0 == USE_AUTO_LEVEL)
    {
        _gPitchLevelAdjust = 0.0;
        _gRollLevelAdjust = 0.0;
    }
}

#endif /* AHRS_Controller_h */


















