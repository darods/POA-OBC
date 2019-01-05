//
//  ESC_Controller.h
//  SelfFly
//
//  Created by Maverick on 2016. 1. 21..
//

#ifndef __ESC_CONTROL__
#define __ESC_CONTROL__

void _ESC_Update();
void _ESC_SetDefault();
void _ESC_DispStatus(int nCase);

void _ESC_Initialize()
{
    int                     i = 0;

    _ESC_DispStatus(0);

    // Set Digital Port 8, 9, 10, and 11 as Output
    DDRB |= B00001111;

    delay(100);

    // Set Value of Digital Port 8, 9, 10, and 11 as Low
    PORTB &= B11110000;

    // Set Value of Digital Port 8, 9, 10, and 11 as Minimun ESC to Initialize ESC For Two Seconds
    for(i=0 ; i<1250 ; i++)
        _ESC_SetDefault();
    
    delay(100);
    
    bIsThrottleInitialized = 1;
    
    _ESC_DispStatus(2);
}


void _ESC_Update()
{
    unsigned long           nESCOut[4] = {0, };
    unsigned long           nCurrTime = micros();
    
    // Set Digital Port 8, 9, 10, and 11 as high.
    PORTB |= B00001111;
    
    // Set Relative Throttle Value by Adding Current Time
    for(int i=0 ; i<4 ; i++)
        nESCOut[i] = _gESCOutput[i] + nCurrTime;
    
    while(PORTB & B00001111)
    {
        nCurrTime = micros();
        
        if(nESCOut[0] <= nCurrTime) PORTB &= B11111110;
        if(nESCOut[1] <= nCurrTime) PORTB &= B11111101;
        if(nESCOut[2] <= nCurrTime) PORTB &= B11111011;
        if(nESCOut[3] <= nCurrTime) PORTB &= B11110111;
    }
}


void _ESC_SetDefault()
{
    // Set Digital Port 8, 9, 10, and 11 as high.
    PORTB |= B00001111;
    delayMicroseconds(1000);
    
    //Set digital poort 8, 9, 10, and 11 low.
    PORTB &= B11110000;
    delayMicroseconds(3000);
}


void _ESC_DispStatus(int nCase)
{
    #if PRINT_SERIAL
        if(0 == nCase)
        {
            Serialprint(F(" *      "));
            Serialprint(_gDroneInitStep++);
            Serialprintln(F(". Start ESC Module Initialization   "));
        }
        else if(1 == nCase)
          Serialprint(F("."));
        else if(2 == nCase)
          Serialprintln(F(" *          => Done!!   "));
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
            _gLCDHndl.print(".Init ESC");
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
#endif /* ESC_Controller_h */

















