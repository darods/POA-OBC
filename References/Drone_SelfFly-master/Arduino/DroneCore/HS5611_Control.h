//
//  HS5611_Controller.h
//  SelfFly
//
//  Created by Maverick on 2016. 1. 21..
//

#ifndef __HS5611_CONTROL__
#define __HS5611_CONTROL__

void _Barometer_GetData();
void _Barometer_CalculateData();

void _Barometer_Initialize()
{
    int                     i = 0;
    MS561101BA              *pBaroHndl = &(_gBaroHndl);

    Serialprint(F(" Initializing Barometer Sensor (MS5611)..."));

    pBaroHndl->init(MS561101BA_ADDR_CSB_LOW);

    for(i=0 ; i<50 ; i++)
    {
        _Barometer_GetData();
        delay(20);
    }

    // Get Average Pressure & Temperature
    _gAvgTemp = pBaroHndl->getAvgTemp();
    _gAvgPressure = pBaroHndl->getAvgPressure();

    // Get Reference Altitude
    _gRefAbsoluteAltitude = pBaroHndl->getAltitude(_gAvgPressure, _gAvgTemp);

    bIsBarometerInitialized = 1;
    
    Serialprintln(F(" Done"));
}


void _Barometer_GetData()
{
    MS561101BA              *pBaroHndl = &(_gBaroHndl);

    _gRawTemp = pBaroHndl->getTemperature(MS561101BA_OSR_512);
    _gRawPressure = pBaroHndl->getPressure(MS561101BA_OSR_512);

    // Push to Array to Get Average Pressure & Temperature
    pBaroHndl->pushTemp(_gRawTemp);
    pBaroHndl->pushPressure(_gRawPressure);

    // Calculate Altitude
    _Barometer_CalculateData();
}


void _Barometer_CalculateData()
{
    MS561101BA              *pBaroHndl = &(_gBaroHndl);

    // Get Average Pressure & Temperature
    _gAvgTemp = pBaroHndl->getAvgTemp();
    _gAvgPressure = pBaroHndl->getAvgPressure();

    // Get Altitude
    _gRawAbsoluteAltitude = pBaroHndl->getAltitude(_gAvgPressure, _gAvgTemp);

    // Push to Array to Get Average Altitude
    pBaroHndl->pushAltitude(_gRawAbsoluteAltitude);

    // Get Average Pressure & Temperature
    _gAvgAbsoluteAltitude = pBaroHndl->getAvgAltitude();

    // Get Vertical Speed
    _gVerticalSpeed = abs(_gAvgAbsoluteAltitude - _gPrevAvgAbsoluteAltitude) / (double)(_gDiffTime);
    _gRelativeAltitude = _gAvgAbsoluteAltitude - _gRefAbsoluteAltitude;

    _gPrevAvgAbsoluteAltitude = _gAvgAbsoluteAltitude;
}
#endif /* HS5611_Controller_h */


















