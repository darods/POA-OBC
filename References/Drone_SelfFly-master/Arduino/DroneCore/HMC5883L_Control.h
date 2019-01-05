//
//  HMC5883L_Controller.h
//  SelfFly
//
//  Created by Maverick on 2016. 1. 21..
//

#ifndef __HMC5883L_CONTROL__
#define __HMC5883L_CONTROL__

void _Mag_Initialize()
{
    HMC5883L            *pMagHndl = NULL;

    _gMagHndl = HMC5883L();
    pMagHndl = &_gMagHndl;

    // initialize Magnetic
    Serialprintln(F(" Initializing Magnetic..."));
    pMagHndl->initialize();

    // Verify Vonnection
    Serialprint(F("    Testing device connections..."));
    Serialprintln(pMagHndl->testConnection() ? F("  HMC5883L connection successful") : F("  HMC5883L connection failed"));

    // Calibrate Magnetic
    //Serialprint(F("    Start Calibration of Magnetic Sensor (HMC5883L) "));
    //pMagHndl->calibrate();
    //pMagHndl->calibration_offset(1);
    //Serialprintln(F("Done"));

    pMagHndl->setMode(HMC5883L_MODE_CONTINUOUS);
    pMagHndl->setGain(HMC5883L_GAIN_1090);
    pMagHndl->setDataRate(HMC5883L_RATE_75);
    pMagHndl->setSampleAveraging(HMC5883L_AVERAGING_8);

    // Date: 2015-11-05
    // Location: Seoul, South Korea
    // Latitude: 37.0000° North
    // Longitude: 126.0000° East
    // Magnetic declination: 7° 59.76' West
    // Annual Change (minutes/year): 3.9 '/y West
    // http://www.geomag.nrcan.gc.ca/calc/mdcal-en.php
    // http://www.magnetic-declination.com/
    _gDeclinationAngle = (7.0 + (59.76 / 60.0)) * DEG_TO_RAD_SCALE;

    bIsMagnitudeInitialized = 1;
    
    Serialprintln(F(" Done"));

    // Reference WebSite
    // http://www.meccanismocomplesso.org/en/arduino-magnetic-magnetic-magnetometer-hmc5883l/
}


void _Mag_GetData()
{
    _gMagHndl.getScaledHeading(&(_gRawMag[X_AXIS]), &(_gRawMag[Y_AXIS]), &(_gRawMag[Z_AXIS]));

    // Calculate Heading
    //_Mag_CalculateDirection();
}


void _Mag_CalculateDirection()
{
    int                     i = 0;
    
    _gMagHeadingRad = atan2(_gRawMag[Y_AXIS], _gRawMag[X_AXIS]);
    _gMagHeadingRad -= _gDeclinationAngle;      // If East, then Change Operation to PLUS

    if(_gMagHeadingRad < 0)
        _gMagHeadingRad += DOUBLE_RADIAN;

    if(_gMagHeadingRad > DOUBLE_RADIAN)
        _gMagHeadingRad -= DOUBLE_RADIAN;

    _gMagHeadingDeg = _gMagHeadingRad * RAD_TO_DEG_SCALE;

    if(_gMagHeadingDeg >= 1 && _gMagHeadingDeg < 240)
        _gMagHeadingDeg = map(_gMagHeadingDeg, 0, 239, 0, 179);
    else if(_gMagHeadingDeg >= 240)
        _gMagHeadingDeg = map(_gMagHeadingDeg, 240, 360, 180, 360);

    // Smooth angles rotation for +/- 3deg
    _gSmoothHeadingDegrees = round(_gMagHeadingDeg);

    if((_gSmoothHeadingDegrees < (_gPrevHeadingDegrees + 3)) &&
       (_gSmoothHeadingDegrees > (_gPrevHeadingDegrees - 3)))
        _gSmoothHeadingDegrees = _gPrevHeadingDegrees;

    _gPrevHeadingDegrees = _gSmoothHeadingDegrees;
}

#endif /* HMC5883L_Controller_h */


















