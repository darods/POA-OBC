//
//  SR04_Controller.h
//  SelfFly
//
//  Created by Maverick on 2016. 1. 21..
//

#ifndef __SR04_CONTROL__
#define __SR04_CONTROL__

void _Sonar_GetData();
void _Sonar_DispStatus(int nCase);

void _Sonar_Initialize()
{
    int                     i = 0;

    _Sonar_DispStatus(0);
    
    // Set A1 as Digital Output Mode for Sonar Sensor (HC-SR04)
    DDRC |= B00000010;

    // Calibrate Sonar Sensor
    for(i=0 ; i<50 ; i++)
    {
        _Sonar_GetData();
        delay(20);
        
        if(0 == (i % 10))
            _Sonar_DispStatus(1);
    }

    SonarTimer.every(100, _Sonar_GetData);
    
    bIsSonarInitialized = 1;
    
    _Sonar_DispStatus(2);
}


void _Sonar_GetData()
{
    PORTC |= B00000010;
    delayMicroseconds(10);
    PORTC &= B11111101;

    // Get Raw Distance Value
    _gRawDist = pulseIn(PIN_SONAR_ECHO, HIGH, 100000);

    // Calculate Distance From Ground
    _gDistFromGnd = _gRawDist * 0.017; // (340(m/s) * 1000(mm) / 1000000(microsec) / 2(oneway))
}


void _Sonar_GetData_WithPeriod()
{
    const unsigned long     nCurrTime = micros();
    static unsigned long    nPrevTime = 0;
    
    if((nCurrTime - nPrevTime) > SONAR_GETDATA_PERIOD)
    {
        _Sonar_GetData();

        nPrevTime = nCurrTime;
    }
}


void _Sonar_DispStatus(int nCase)
{
    #if PRINT_SERIAL
        if(0 == nCase)
        {
            Serialprint(F(" *      ");
            Serialprint(_gDroneInitStep++);        
            Serialprintln(F(". Start Sonar Module Initialization   "));
        }
        else if(1 == nCase)
            Serialprint(F("."));
        else if(2 == nCase)
            Serialprintln(F(" Done!!"));
    #elif USE_LCD_DISPLAY
        {
            static int nCnt = 0;
            
            if(0 == nCase)
            {
                delay(500);
                _gLCDHndl.clear();
                
                _gLCDHndl.setCursor(0, 0);
                _gLCDHndl.print(_gDroneInitStep++);
                _gLCDHndl.setCursor(1, 0);
                _gLCDHndl.print(".Init Sonar");
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
            else if(2 == nCase)
            {
                _gLCDHndl.setCursor(nCnt, 1);
                _gLCDHndl.print("Done!");
                delay(1000);
            }
        }
    #endif
}

#endif /* SR04_Controller_h */

















