//
//  PID_Controller.h
//  SelfFly
//
//  Created by Maverick on 2016. 1. 21..
//

#ifndef __PID_CONTROL__
#define __PID_CONTROL__
        
void _Calculate_Altitude(float *pEstimatedThrottle);

void _CalculatePID()
{
    float                   nEstimatedRCVal[CH_TYPE_MAX] = {0.0, };
    float                   nCurrErrRate;
    int                     i = 0, j = 0;

    #if USE_EXT_SR_READ
    {
        if(Serial.available())
        {
            char ch = Serial.read();
            Serial.println(ch);
            if(ch == 'a')
            {
                _gPIDGainTable[0][0] -= 0.01;
                _gPIDGainTable[1][0] -= 0.01;
            }
            else if(ch == 'z')
            {
                _gPIDGainTable[0][0] += 0.01;
                _gPIDGainTable[1][0] += 0.01;
            }
            else if(ch == 's')
            {
                _gPIDGainTable[0][1] -= 0.01;
                _gPIDGainTable[1][1] -= 0.01;
            }
            else if(ch == 'x')
            {
                _gPIDGainTable[0][1] += 0.01;
                _gPIDGainTable[1][1] += 0.01;
            }
            else if(ch == 'd')
            {
                _gPIDGainTable[0][2] -= 0.01;
                _gPIDGainTable[1][2] -= 0.01;
            }
            else if(ch == 'c')
            {
                _gPIDGainTable[0][2] += 0.01;
                _gPIDGainTable[1][2] += 0.01;
            }
        }
    }
    #endif

    nEstimatedRCVal[CH_TYPE_ROLL] = 0.0;
    if(_gCompensatedRCVal[_gRCChMap[CH_TYPE_ROLL]] > 1508)
        nEstimatedRCVal[CH_TYPE_ROLL] = (_gCompensatedRCVal[_gRCChMap[CH_TYPE_ROLL]] - 1508);
    else if(_gCompensatedRCVal[_gRCChMap[CH_TYPE_ROLL]] < 1492)
        nEstimatedRCVal[CH_TYPE_ROLL] = (_gCompensatedRCVal[_gRCChMap[CH_TYPE_ROLL]] - 1492);
    
    nEstimatedRCVal[CH_TYPE_ROLL] -= _gRollLevelAdjust;
    nEstimatedRCVal[CH_TYPE_ROLL] /= 3.0;

    nEstimatedRCVal[CH_TYPE_PITCH] = 0.0;
    if(_gCompensatedRCVal[_gRCChMap[CH_TYPE_PITCH]] > 1508)
        nEstimatedRCVal[CH_TYPE_PITCH] = (_gCompensatedRCVal[_gRCChMap[CH_TYPE_PITCH]] - 1508);
    else if(_gCompensatedRCVal[_gRCChMap[CH_TYPE_PITCH]] < 1492)
        nEstimatedRCVal[CH_TYPE_PITCH] = (_gCompensatedRCVal[_gRCChMap[CH_TYPE_PITCH]] - 1492);

    nEstimatedRCVal[CH_TYPE_PITCH] -= _gPitchLevelAdjust;
    nEstimatedRCVal[CH_TYPE_PITCH] /= 3.0;

    nEstimatedRCVal[CH_TYPE_YAW] = 0.0;
    if(_gCompensatedRCVal[_gRCChMap[CH_TYPE_THROTTLE]] > 1050)
    {
        if(_gCompensatedRCVal[_gRCChMap[CH_TYPE_YAW]] > 1508)
            nEstimatedRCVal[CH_TYPE_YAW] = (_gCompensatedRCVal[_gRCChMap[CH_TYPE_YAW]] - 1508) / 3.0;
        else if(_gCompensatedRCVal[_gRCChMap[CH_TYPE_YAW]] < 1492)
            nEstimatedRCVal[CH_TYPE_YAW] = (_gCompensatedRCVal[_gRCChMap[CH_TYPE_YAW]] - 1492) / 3.0;
    }

    // PID configuration
    #if 1
    {
        // Roll
        nCurrErrRate = _gEstimatedRPY[0] - nEstimatedRCVal[CH_TYPE_ROLL];
        
        _gRPY_PID[0].nP_ErrRate = _gPIDGainTable[0][0] * nCurrErrRate;
        _gRPY_PID[0].nI_ErrRate += _gPIDGainTable[0][1] * nCurrErrRate;
        _gRPY_PID[0].nI_ErrRate = _Clip3Float(_gRPY_PID[0].nI_ErrRate, -_gPIDGainTable[0][3], _gPIDGainTable[0][3]);
        _gRPY_PID[0].nD_ErrRate = _gPIDGainTable[0][2] * (nCurrErrRate - _gRPY_PID[0].nPrevErrRate);
        
        _gRPY_PID[0].nBalance   = _gRPY_PID[0].nP_ErrRate + _gRPY_PID[0].nI_ErrRate + _gRPY_PID[0].nD_ErrRate;
        _gRPY_PID[0].nBalance   = _Clip3Float(_gRPY_PID[0].nBalance, -_gPIDGainTable[0][3], _gPIDGainTable[0][3]);
        
        _gRPY_PID[0].nPrevErrRate = nCurrErrRate;
    }

    {
        // Picth
        nCurrErrRate = _gEstimatedRPY[1] - nEstimatedRCVal[CH_TYPE_PITCH];
        
        _gRPY_PID[1].nP_ErrRate = _gPIDGainTable[1][0] * nCurrErrRate;
        _gRPY_PID[1].nI_ErrRate += _gPIDGainTable[1][1] * nCurrErrRate;
        _gRPY_PID[1].nI_ErrRate = _Clip3Float(_gRPY_PID[1].nI_ErrRate, -_gPIDGainTable[1][3], _gPIDGainTable[1][3]);
        _gRPY_PID[1].nD_ErrRate = _gPIDGainTable[1][2] * (nCurrErrRate - _gRPY_PID[1].nPrevErrRate);
        
        _gRPY_PID[1].nBalance   = _gRPY_PID[1].nP_ErrRate + _gRPY_PID[1].nI_ErrRate + _gRPY_PID[1].nD_ErrRate;
        _gRPY_PID[1].nBalance   = _Clip3Float(_gRPY_PID[1].nBalance, -_gPIDGainTable[1][3], _gPIDGainTable[1][3]);
        
        _gRPY_PID[1].nPrevErrRate = nCurrErrRate;
    }

    {
        // Yaw
        nCurrErrRate = _gEstimatedRPY[2] - nEstimatedRCVal[CH_TYPE_YAW];
        
        _gRPY_PID[2].nP_ErrRate = _gPIDGainTable[2][0] * nCurrErrRate;
        _gRPY_PID[2].nI_ErrRate += _gPIDGainTable[2][1] * nCurrErrRate;
        _gRPY_PID[2].nI_ErrRate = _Clip3Float(_gRPY_PID[2].nI_ErrRate, -_gPIDGainTable[2][3], _gPIDGainTable[2][3]);
        _gRPY_PID[2].nD_ErrRate = _gPIDGainTable[2][2] * (nCurrErrRate - _gRPY_PID[2].nPrevErrRate);
        
        _gRPY_PID[2].nBalance   = _gRPY_PID[2].nP_ErrRate + _gRPY_PID[2].nI_ErrRate + _gRPY_PID[2].nD_ErrRate;
        _gRPY_PID[2].nBalance   = _Clip3Float(_gRPY_PID[2].nBalance, -_gPIDGainTable[2][3], _gPIDGainTable[2][3]);
        
        _gRPY_PID[2].nPrevErrRate = nCurrErrRate;
    }
    #else
    for(i=0 ; i<3 ; i++)
    {
        int          nIdx = (2 != i) ? i : CH_TYPE_YAW;
        nCurrErrRate = _gEstimatedRPY[i] - nEstimatedRCVal[nIdx];
        
        _gRPY_PID[i].nP_ErrRate = _gPIDGainTable[i][0] * nCurrErrRate;
        _gRPY_PID[i].nI_ErrRate += _gPIDGainTable[i][1] * nCurrErrRate;
        _gRPY_PID[i].nI_ErrRate = _Clip3Float(_gRPY_PID[i].nI_ErrRate, -_gPIDGainTable[i][3], _gPIDGainTable[i][3]);
        _gRPY_PID[i].nD_ErrRate = _gPIDGainTable[i][2] * (nCurrErrRate - _gRPY_PID[i].nPrevErrRate);
        
        _gRPY_PID[i].nBalance   = _gRPY_PID[i].nP_ErrRate + _gRPY_PID[i].nI_ErrRate + _gRPY_PID[i].nD_ErrRate;
        _gRPY_PID[i].nBalance   = _Clip3Float(_gRPY_PID[i].nBalance, -_gPIDGainTable[i][3], _gPIDGainTable[i][3]);
        
        _gRPY_PID[i].nPrevErrRate = nCurrErrRate;
    }
    #endif
}


void _CalculateThrottleVal()
{
    int                     nThrottle = _gCompensatedRCVal[_gRCChMap[CH_TYPE_THROTTLE]];
    int                     i = 0;
    
    if(DRONESTATUS_START == _gDroneStatus)
    {
        const int           nRollBalance    = (int)(_gRPY_PID[0].nBalance);
        const int           nPitchBalance   = (int)(_gRPY_PID[1].nBalance);
        const int           nYawBalance     = (int)(_gRPY_PID[2].nBalance);

        if(1800 < nThrottle)
            nThrottle = 1800;
        
        _gESCOutput[0] = nThrottle - nRollBalance - nPitchBalance + nYawBalance;
        _gESCOutput[1] = nThrottle + nRollBalance - nPitchBalance - nYawBalance;
        _gESCOutput[2] = nThrottle + nRollBalance + nPitchBalance + nYawBalance;
        _gESCOutput[3] = nThrottle - nRollBalance + nPitchBalance - nYawBalance;

//        _gESCOutput[0] = nThrottle + nRollBalance + nPitchBalance + nYawBalance;
//        _gESCOutput[1] = nThrottle - nRollBalance + nPitchBalance - nYawBalance;
//        _gESCOutput[2] = nThrottle - nRollBalance - nPitchBalance + nYawBalance;
//        _gESCOutput[3] = nThrottle + nRollBalance - nPitchBalance - nYawBalance;

        for(i=0 ; i<4 ; i++)
            _gESCOutput[i] = _Clip3Float(_gESCOutput[i], ESC_ACTUAL_MIN, ESC_ACTUAL_MAX);
    }
    else
    {
        // Set Throttle Value as Min Value
        for(i=0 ; i<4 ; i++)
            _gESCOutput[i] = ESC_MIN;
    }
}


void _Calculate_Altitude(float *pEstimatedThrottle)
{
    if(1500 < _gCompensatedRCVal[_gRCChMap[CH_TYPE_TAKE_LAND]])
    {
        //(_gDistFromGnd - HOVERING_ALTITUDE)
    }
}

#endif /* PID_Controller_h */


















