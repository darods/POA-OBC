//
//  RC_Controller.h
//  SelfFly
//
//  Created by Maverick on 2016. 1. 21..
//

#ifndef __RC_CONTROL__
#define __RC_CONTROL__

void _RC_Wait_Signal();
void _RC_GetRCRange();
void _RC_DispStatus(int nCase);

void _RC_Initialize()
{
    _RC_DispStatus(0);

    PCICR |= (1 << PCIE2);                // Set PCIE2 to Enable Scan ISR
    PCMSK2 |= (1 << PCINT18);             // Set Digital Input 2 as RC Input (Roll)
    PCMSK2 |= (1 << PCINT19);             // Set Digital Input 3 as RC Input (Pitch)
    PCMSK2 |= (1 << PCINT20);             // Set Digital Input 4 as RC Input (Throttle)
    PCMSK2 |= (1 << PCINT21);             // Set Digital Input 5 as RC Input (Yaw)
    PCMSK2 |= (1 << PCINT22);             // Set Digital Input 6 as RC Input (Landing & TakeOff)

    _RC_Wait_Signal();
    
    delay(100); _RC_DispStatus(1);
    delay(100); _RC_DispStatus(1);
    delay(100); _RC_DispStatus(1);
    delay(100); _RC_DispStatus(1);
    delay(100); _RC_DispStatus(1);
    
    _RC_DispStatus(2);
}


void _Read_RCData_From_EEPROM()
{
    int                 i = 0;
    int                 nRCType = 0;
    int                 nEEPRomAddress = 0;
    
    Serialprintln(F("   ")); Serialprintln(F("   ")); Serialprintln(F("   ")); Serialprintln(F("   "));
    Serialprintln(F("********************************************************************"));
    Serialprintln(F(" *       Reading Drone Setting from EEPROM          "));
    Serialprintln(F("********************************************************************"));
    
    // Read Range of Transmitter
    for(i=EEPROM_DATA_RC_CH0_TYPE ; i<=EEPROM_DATA_RC_CH4_TYPE ; i++)
        _gEEPROMData[i] = EEPROM.read(i);
    
    for(i=EEPROM_DATA_RC_CH0_LOW_H ; i<=EEPROM_DATA_RC_CH4_HIG_L ; i++)
        _gEEPROMData[i] = EEPROM.read(i);
    
    delay(300);
    
    Serialprintln(F(" *            => Done!!   "));
}


//This part converts the actual receiver signals to a standardized 1000 – 1500 – 2000 microsecond value.
//The stored data in the EEPROM is used.
void _RC_Compensate(byte nRCCh)
{
    int                     nDiff = 0;
    int                     nCompensatedRC = 0;
    const int               nLow        = _gRCSignal_L[nRCCh];
    const int               nCenter     = _gRCSignal_M[nRCCh];
    const int               nHigh       = _gRCSignal_H[nRCCh];
    const byte              nReverse    = _gRCRvrsFlag[nRCCh];
    int                     nActualRC   = _gRCSignalVal[nRCCh];
    
    if(nActualRC < nCenter)
    {
        //The actual receiver value is lower than the center value
        // Limit the lowest value to the value that was detected during setup
        if(nActualRC < nLow)
            nActualRC = nLow;
        
        // Calculate and scale the actual value to a 1000 - 2000us value
        nDiff = ((long)(nCenter - nActualRC) * (long)500) / (nCenter - nLow);
        
        // If the channel is reversed
        if(nReverse == 1)
            nCompensatedRC = 1500 + nDiff;
        else
            nCompensatedRC = 1500 - nDiff;
    }
    else if(nActualRC > nCenter)
    {
        //The actual receiver value is higher than the center value
        //Limit the lowest value to the value that was detected during setup
        if(nActualRC > nHigh)
            nActualRC = nHigh;
        
        //Calculate and scale the actual value to a 1000 - 2000us value
        nDiff = ((long)(nActualRC - nCenter) * (long)500) / (nHigh - nCenter);
        
        //If the channel is reversed
        if(nReverse == 1)
            nCompensatedRC = 1500 - nDiff;
        else
            nCompensatedRC = 1500 + nDiff;
    }
    else
        nCompensatedRC = 1500;
    
    _gCompensatedRCVal[nRCCh] = nCompensatedRC;
}


void _RC_Wait_Signal()
{
    byte                nFlag = 0;
    const int           nOffsetH = 2100;
    const int           nOffsetL = 900;
    
    while(nFlag < 15)
    {
        if((_gRCSignalVal[0] > nOffsetL) && (_gRCSignalVal[0] < nOffsetH))
            nFlag |= B00000001;
        
        if((_gRCSignalVal[1] > nOffsetL) && (_gRCSignalVal[1] < nOffsetH))
            nFlag |= B00000010;
        
        if((_gRCSignalVal[2] > nOffsetL) && (_gRCSignalVal[2] < nOffsetH))
            nFlag |= B00000100;
        
        if((_gRCSignalVal[3] > nOffsetL) && (_gRCSignalVal[3] < nOffsetH))
            nFlag |= B00001000;

        _ESC_SetDefault();
        
        delay(500);
    }
}


void _RC_Wait_CenterPos()
{
    byte                nFlag = 0;
    const int           nCneterOffset = 100;
    const int           nCneterOffsetH = 1500 + nCneterOffset;
    const int           nCneterOffsetL = 1500 - nCneterOffset;
    
    while(nFlag < 15)
    {
        if((_gRCSignalVal[0] > nCneterOffsetL) && (_gRCSignalVal[0] < nCneterOffsetH))
            nFlag |= B00000001;
        
        if((_gRCSignalVal[1] > nCneterOffsetL) && (_gRCSignalVal[1] < nCneterOffsetH))
            nFlag |= B00000010;
        
        if((_gRCSignalVal[2] > nCneterOffsetL) && (_gRCSignalVal[2] < nCneterOffsetH))
            nFlag |= B00000100;
        
        if((_gRCSignalVal[3] > nCneterOffsetL) && (_gRCSignalVal[3] < nCneterOffsetH))
            nFlag |= B00001000;

        delay(500);
    }
    
    Serialprintln(F(" *    Sticks are Aligned to Center"));
}


void _RC_CheckAxis(int nAxisIdx)
{
    unsigned long           nCurrTime;
    int                     nChNum = -1;
    
    if(0 == nAxisIdx)
    {
        Serialprintln(F(" *      Please Move Roll Stick to Left Wing Up and Back to Center"));
        Serialprint(F("                Roll Channel is ["));
    }
    else if(1 == nAxisIdx)
    {
        Serialprintln(F(" *      Please Move Pitch Stick to Nose Up and Back to Center"));
        Serialprint(F("                Pitch Channel is ["));
    }
    else if(2 == nAxisIdx)
    {
        Serialprintln(F(" *      Please Move Throttle Stick Up and Back to Center"));
        Serialprint(F("                Throttle Channel is ["));
    }
    else if(3 == nAxisIdx)
    {
        Serialprintln(F(" *      Please Move Yaw Stick to Nose Right and Back to Center"));
        Serialprint(F("                Yaw Channel is ["));
    }
    else if(4 == nAxisIdx)
    {
        Serialprintln(F("        Please Move Gear Stick to Up and Back to Center"));
        Serialprint(F("                Gear Channel is ["));
    }
    
    delay(2000);
    
    nCurrTime = millis() + 10000;
    while(nCurrTime > millis() && (-1 == nChNum))
    {
        if((_gRCSignalVal[0] < 1200) || (_gRCSignalVal[0] > 1700)) nChNum = B00000000;
        else if((_gRCSignalVal[1] < 1200) || (_gRCSignalVal[1] > 1700)) nChNum = B00000001;
        else if((_gRCSignalVal[2] < 1200) || (_gRCSignalVal[2] > 1700)) nChNum = B00000010;
        else if((_gRCSignalVal[3] < 1200) || (_gRCSignalVal[3] > 1700)) nChNum = B00000011;
        else if((_gRCSignalVal[4] > 1700)) nChNum = B00000100;
    }
    _gRCChMap[nAxisIdx] = nChNum;
    
    // Check RC Signal is Inverse of Not
    Serialprint(nChNum);
    if((_gRCSignalVal[0] < 1200) || (_gRCSignalVal[1] < 1200) ||
       (_gRCSignalVal[2] < 1200) || (_gRCSignalVal[3] < 1200))
    {
        _gRCChMap[nAxisIdx] |= B10000000;
        Serialprint(F("] and Reversed"));
    }
    else
        Serialprint(F("] and Not Reversed"));
    
    Serialprintln(F(" "));
    
    _RC_Wait_CenterPos();
}


void _RC_GetRCRange()
{
    int                     i = 0;
    const int               nOffset = 100;
    byte                    nFlag = 0;
    unsigned long           nCurrTime;
    
    for(i=0 ; i<5 ; i++)
    {
        _gRCSignal_L[i] = 9999;
        _gRCSignal_M[i] = 1500;
        _gRCSignal_H[i] = 0;
    }
    
    Serialprintln(F(" *      Start Receiver Range Check   "));
    Serialprintln(F(" *      Please Keep Moving Remote Controller   "));
    Serialprint(F("          "));
    
    i = 0;
    nCurrTime = millis() + 20000;
    while(nCurrTime > millis())
    {
        // Set Min & Max Range
        if(_gRCSignalVal[0] < _gRCSignal_L[0]) _gRCSignal_L[0] = _gRCSignalVal[0];
        if(_gRCSignalVal[0] > _gRCSignal_H[0]) _gRCSignal_H[0] = _gRCSignalVal[0];
        if(_gRCSignalVal[1] < _gRCSignal_L[1]) _gRCSignal_L[1] = _gRCSignalVal[1];
        if(_gRCSignalVal[1] > _gRCSignal_H[1]) _gRCSignal_H[1] = _gRCSignalVal[1];
        if(_gRCSignalVal[2] < _gRCSignal_L[2]) _gRCSignal_L[2] = _gRCSignalVal[2];
        if(_gRCSignalVal[2] > _gRCSignal_H[2]) _gRCSignal_H[2] = _gRCSignalVal[2];
        if(_gRCSignalVal[3] < _gRCSignal_L[3]) _gRCSignal_L[3] = _gRCSignalVal[3];
        if(_gRCSignalVal[3] > _gRCSignal_H[3]) _gRCSignal_H[3] = _gRCSignalVal[3];
        if(_gRCSignalVal[4] < _gRCSignal_L[4]) _gRCSignal_L[4] = _gRCSignalVal[4];
        if(_gRCSignalVal[4] > _gRCSignal_H[4]) _gRCSignal_H[4] = _gRCSignalVal[4];
        
        if(0 == (i++ % 10))
            Serialprint(F("."));
        
        delay(50);
    }
    Serialprintln(F(" "));
    
    for(i=0 ; i<5 ; i++)
        _gRCSignal_M[i] = (_gRCSignal_H[i] + _gRCSignal_L[i]) / 2;
    
    for(i=0 ; i<5 ; i++)
    {
        Serialprint(F(" *              Ch"));
        Serialprint(i);
        Serialprint(F(" Range: "));
        Serialprint(_gRCSignal_L[i]);
        Serialprint(F(" ~ "));
        Serialprint(_gRCSignal_M[i]);
        Serialprint(F(" ~ "));
        Serialprintln(_gRCSignal_H[i]);
    }
    
    Serialprintln(F(" *            => Done!!   "));
    
    _RC_Wait_CenterPos();

    Serialprintln(F(" "));
}


// Get Transmitter Signal From Digital Pin 2, 3, 4, 5, and 6 by HW Interrupt
ISR(PCINT2_vect)
{
    const unsigned long     nCurrTime = micros();
    
    // Check Status of Digital Pin 2
    if(PIND & B00000100)
    {
        if(0 == (_gRCRisingFlag & B00000001))
        {
            _gRCRisingFlag |= B00000001;
            _gRCChRisingTime[0] = nCurrTime;
        }
    }
    else if(_gRCRisingFlag & B00000001)
    {
        _gRCRisingFlag &= B11111110;
        _gRCSignalVal[0] = nCurrTime - _gRCChRisingTime[0];
    }

    // Check Status of Digital Pin 3
    if(PIND & B00001000)
    {
        if(0 == (_gRCRisingFlag & B00000010))
        {
            _gRCRisingFlag |= B00000010;
            _gRCChRisingTime[1] = nCurrTime;
        }
    }
    else if(_gRCRisingFlag & B00000010)
    {
        _gRCRisingFlag &= B11111101;
        _gRCSignalVal[1] = nCurrTime - _gRCChRisingTime[1];
    }

    // Check Status of Digital Pin 4
    if(PIND & B00010000)
    {
        if(0 == (_gRCRisingFlag & B00000100))
        {
            _gRCRisingFlag |= B00000100;
            _gRCChRisingTime[2] = nCurrTime;
        }
    }
    else if(_gRCRisingFlag & B00000100)
    {
        _gRCRisingFlag &= B11111011;
        _gRCSignalVal[2] = nCurrTime - _gRCChRisingTime[2];
    }

    // Check Status of Digital Pin 5
    if(PIND & B00100000)
    {
        if(0 == (_gRCRisingFlag & B00001000))
        {
            _gRCRisingFlag |= B00001000;
            _gRCChRisingTime[3] = nCurrTime;
        }
    }
    else if(_gRCRisingFlag & B00001000)
    {
        _gRCRisingFlag &= B11110111;
        _gRCSignalVal[3] = nCurrTime - _gRCChRisingTime[3];
    }
    
    // Check Status of Digital Pin 6
    if(PIND & B01000000)
    {
        if(0 == (_gRCRisingFlag & B00010000))
        {
            _gRCRisingFlag |= B00010000;
            _gRCChRisingTime[4] = nCurrTime;
        }
    }
    else if(_gRCRisingFlag & B00010000)
    {
        _gRCRisingFlag &= B11101111;
        _gRCSignalVal[4] = nCurrTime - _gRCChRisingTime[4];
    }
}


void _RC_DispStatus(int nCase)
{
    #if PRINT_SERIAL
        if(0 == nCase)
        {
            Serialprint(F(" *      "));
            Serialprint(_gDroneInitStep++);        
            Serialprintln(F(". Start Receiver Module Initialization   "));
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
                _gLCDHndl.print(".Init RadioCtrl");
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

#endif /* RC_Controller_h */

















