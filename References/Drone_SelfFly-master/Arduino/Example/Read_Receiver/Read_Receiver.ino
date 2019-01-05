
#include <EEPROM.h>
#include "CommHeader.h"

#define Serialprint(...)                Serial.print(__VA_ARGS__)
#define Serialprintln(...)              Serial.println(__VA_ARGS__)

typedef enum _RC_CH_Type
{
    CH_TYPE_ROLL                = 0,
    CH_TYPE_PITCH,
    CH_TYPE_THROTTLE,
    CH_TYPE_YAW,
    CH_TYPE_TAKE_LAND,
    CH_TYPE_MAX,
}RC_CH_Type;

byte                _gEEPROMData[EEPROM_DATA_MAX] = {0, };
unsigned long       _gCurrTime = 0;
int                 _gRCSignal_L[CH_TYPE_MAX] = {0, };
int                 _gRCSignal_M[CH_TYPE_MAX] = {0, };
int                 _gRCSignal_H[CH_TYPE_MAX] = {0, };
byte                _gRCRvrsFlag[CH_TYPE_MAX] = {0, };
byte                _gRCRisingFlag = 0;
unsigned long       _gRCChRisingTime[CH_TYPE_MAX] = {0, };
int                 _gRCSignalVal[CH_TYPE_MAX] = {0, };
int                 _gCompensatedRCVal[CH_TYPE_MAX] = {0, };
int                 _gRCStickChMapTable[CH_TYPE_MAX] = {0, };
int                 nCheckProcessDone = 0;

void _RC_Initialize();
void _RC_EstimateRCRange();
void _RC_Compensate(byte nRCCh);
void _RC_CheckStickType(int nStickType);
void _RC_Wait_Signal();
void _print_CaturedRC_Signals();
void _print_CompensatedRC_Signals();


void setup() 
{
    Serial.begin(115200);
    Serial.flush();

    Serialprintln(F("")); Serialprintln(F("")); Serialprintln(F("")); Serialprintln(F(""));
    Serialprintln(F("********************************************************************"));
    Serialprintln(F(" *               Initialize Transmitter & Receiver                  "));
    Serialprintln(F("********************************************************************"));

    _RC_Initialize();

    Serialprintln(F("********************************************************************"));
    Serialprintln(F("********************************************************************"));
    Serialprintln(F("")); Serialprintln(F("")); Serialprintln(F("")); Serialprintln(F(""));    
}


void loop()
{
    int                     i = 0;

    if(0 == nCheckProcessDone)
    {
        int                 nError = 0;

        Serialprintln(F("")); Serialprintln(F(""));
        Serialprintln(F("********************************************************************"));
        Serialprintln(F(" *                 Mapping Each Stick to RC Channel                 "));
        Serialprintln(F("********************************************************************"));
        _RC_CheckStickType(CH_TYPE_ROLL);
        _RC_CheckStickType(CH_TYPE_PITCH);
        _RC_CheckStickType(CH_TYPE_THROTTLE);
        _RC_CheckStickType(CH_TYPE_YAW);
        _RC_CheckStickType(CH_TYPE_TAKE_LAND);


        Serialprintln(F("")); Serialprintln(F(""));
        Serialprintln(F("********************************************************************"));
        Serialprintln(F(" *                 Get Min & Max Range of Transmitter               "));
        Serialprintln(F("********************************************************************"));
        Serialprintln(F(" * "));
        Serialprintln(F(" *       Keep Moving Sticks End to End for 10 Sec When Ready  "));
        _RC_EstimateRCRange();

        // Write RC Data to EEPROM
        //_Write_RCData_To_EEPROM();

        // Read RC Data from EEPROM
        //_Read_RCData_From_EEPROM();

        // Verify EEPROM Data
        {
            int                 nStickType = 0;
            int                 nEEPRomAddress = 0;

            nEEPRomAddress = EEPROM_DATA_RC_CH0_LOW_H;
            for(i=0 ; i<CH_TYPE_MAX ; i++, nEEPRomAddress+=6)
            {
                nStickType = _gRCStickChMapTable[i];
                if((_gEEPROMData[nEEPRomAddress] != (_gRCSignal_L[nStickType] >> 8)) ||
                    (_gEEPROMData[nEEPRomAddress+1] != (_gRCSignal_L[nStickType] & 0b11111111)) ||
                    (_gEEPROMData[nEEPRomAddress+2] != (_gRCSignal_M[nStickType] >> 8)) ||
                    (_gEEPROMData[nEEPRomAddress+3] != (_gRCSignal_M[nStickType] & 0b11111111)) ||
                    (_gEEPROMData[nEEPRomAddress+4] != (_gRCSignal_H[nStickType] >> 8)) ||
                    (_gEEPROMData[nEEPRomAddress+5] != (_gRCSignal_H[nStickType] & 0b11111111)))
                    nError = 1;
            }

            nEEPRomAddress = EEPROM_DATA_RC_CH0_TYPE;
            for(i=0 ; i<CH_TYPE_MAX ; i++, nEEPRomAddress++)
                if(_gEEPROMData[nEEPRomAddress] != _gRCStickChMapTable[i])
                    nError = 1;

            nEEPRomAddress = EEPROM_DATA_RC_CH0_REVERSE;
            for(i=0 ; i<CH_TYPE_MAX ; i++, nEEPRomAddress++)
                if(_gEEPROMData[nEEPRomAddress] != _gRCRvrsFlag[i])
                    nError = 1;
        }

        if(0 == nError)
        {
            Serialprintln(F("********************************************************************"));
            Serialprintln(F(" *                     Writing Data is Succeed!!!                   "));
            Serialprintln(F(" *                  Next Step is Calibrating Gyro                   "));
            Serialprintln(F("********************************************************************"));

            nCheckProcessDone = 1;
        }
        else
        {
            Serialprintln(F("********************************************************************"));
            Serialprintln(F(" *              There is Something Wrong!!! Try Again!!!            "));
            Serialprintln(F("********************************************************************"));

            delay(3000);

            nCheckProcessDone = 1;
        }
    }
    else
    {
        // Get Receiver Input
        // Then Mapping Actual Reciever Value to 1000 ~ 2000
        for(i=0 ; i<CH_TYPE_MAX ; i++)
            _RC_Compensate(i);

        //_print_CaturedRC_Signals();
     
        _print_CompensatedRC_Signals();

        Serialprintln(F(""));
    }
}


void _RC_Initialize()
{
    Serialprintln(F(" *      Start Receiver Module Initialize   "));

    PCICR |= (1 << PCIE2);                // Set PCIE2 to Enable Scan ISR
    PCMSK2 |= (1 << PCINT18);             // Set Digital Input 2 as RC Input (Roll)
    PCMSK2 |= (1 << PCINT19);             // Set Digital Input 3 as RC Input (Pitch)
    PCMSK2 |= (1 << PCINT20);             // Set Digital Input 4 as RC Input (Throttle)
    PCMSK2 |= (1 << PCINT21);             // Set Digital Input 5 as RC Input (Yaw)
    PCMSK2 |= (1 << PCINT22);             // Set Digital Input 6 as RC Input (Landing & TakeOff)

    Serialprintln(F(" *        Pease Move All Sticks to Center"));
    _RC_Wait_Signal();
    
    delay(300);
    
    Serialprintln(F(" *            => Done!!   "));
}


void _RC_EstimateRCRange()
{
    int                     i = 0;
    int                     nStickType = 0;
    int                     nOffset = 100;
    int                     nFlag = 0;
    
    for(i=0 ; i<CH_TYPE_MAX ; i++)
    {
        _gRCSignal_L[i] = 9999;
        _gRCSignal_M[i] = 1500;
        _gRCSignal_H[i] = 0;
    }

    Serialprint(F(" *           "));
    while(nFlag < 200)
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

        if(0 == (nFlag % 20))
            Serialprint(F("."));

        nFlag++;
        
        delay(50);
    }

    Serialprintln(F("."));

    nStickType = _gRCStickChMapTable[0];
    _gRCSignal_M[nStickType] = (_gRCSignal_L[nStickType] + _gRCSignal_H[nStickType]) / 2;
    Serialprint(F(" *         Roll Ch Range: "));Serialprint(_gRCSignal_L[nStickType]);Serialprint(F(" ~ "));
    Serialprint(_gRCSignal_M[nStickType]);Serialprint(F(" ~ "));Serialprintln(_gRCSignal_H[nStickType]);

    nStickType = _gRCStickChMapTable[1];
    _gRCSignal_M[nStickType] = (_gRCSignal_L[nStickType] + _gRCSignal_H[nStickType]) / 2;
    Serialprint(F(" *        Pitch Ch Range: "));Serialprint(_gRCSignal_L[nStickType]);Serialprint(F(" ~ "));
    Serialprint(_gRCSignal_M[nStickType]);Serialprint(F(" ~ "));Serialprintln(_gRCSignal_H[nStickType]);

    nStickType = _gRCStickChMapTable[2];
    _gRCSignal_M[nStickType] = (_gRCSignal_L[nStickType] + _gRCSignal_H[nStickType]) / 2;
    Serialprint(F(" *     Throttle Ch Range: "));Serialprint(_gRCSignal_L[nStickType]);Serialprint(F(" ~ "));
    Serialprint(_gRCSignal_M[nStickType]);Serialprint(F(" ~ "));Serialprintln(_gRCSignal_H[nStickType]);

    nStickType = _gRCStickChMapTable[3];
    _gRCSignal_M[nStickType] = (_gRCSignal_L[nStickType] + _gRCSignal_H[nStickType]) / 2;
    Serialprint(F(" *          Yaw Ch Range: "));Serialprint(_gRCSignal_L[nStickType]);Serialprint(F(" ~ "));
    Serialprint(_gRCSignal_M[nStickType]);Serialprint(F(" ~ "));Serialprintln(_gRCSignal_H[nStickType]);

    nStickType = _gRCStickChMapTable[4];
    _gRCSignal_M[nStickType] = (_gRCSignal_L[nStickType] + _gRCSignal_H[nStickType]) / 2;
    Serialprint(F(" *  Take & Land Ch Range: "));Serialprint(_gRCSignal_L[nStickType]);Serialprint(F(" ~ "));
    Serialprint(_gRCSignal_M[nStickType]);Serialprint(F(" ~ "));Serialprintln(_gRCSignal_H[nStickType]);

    Serialprintln(F(" *            => Done!!   "));
    Serialprintln(F(" "));
}


//This part converts the actual receiver signals to a standardized 1000 – 1500 – 2000 microsecond value.
//The stored data in the EEPROM is used.
void _RC_Compensate(byte nRCCh)
{
    byte                    nReverse = _gRCRvrsFlag[nRCCh];
    int                     nLow = _gRCSignal_L[nRCCh];
    int                     nCenter = _gRCSignal_M[nRCCh];
    int                     nHigh = _gRCSignal_H[nRCCh];
    int                     nActualRC = _gRCSignalVal[nRCCh];
    int                     nDiff = 0;
    int                     nCompensatedRC = 0;
  
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


void _RC_CheckStickType(int nStickType)
{
    int                 nMin = 1100;
    int                 nMax = 1800;
    int                 nRCChNum = -1;
    int                 nReverseFlag = 0;

    Serialprintln(F(" * "));

    if(CH_TYPE_ROLL == nStickType)
        Serialprintln(F(" *      Please Move Stick to Left Wing Up, then Return to Center  "));
    else if(CH_TYPE_PITCH == nStickType)
        Serialprintln(F(" *      Please Move Stick to Nose Up, then Return to Center  "));
    else if(CH_TYPE_THROTTLE == nStickType)
        Serialprintln(F(" *      Please Move Stick to Full Throttle, then Return to Center  "));
    else if(CH_TYPE_YAW == nStickType)
        Serialprintln(F(" *      Please Move Stick to Nose Right, then Return to Center  "));
    else if(CH_TYPE_ROLL == nStickType)
        Serialprintln(F(" *      Please Take & Land Stick to Up and Down  "));

    while(-1 >= nRCChNum)
    {
        if((_gRCSignalVal[0] > nMax) || (_gRCSignalVal[0] < nMin))
        {
            nRCChNum = 0;
            if(_gRCSignalVal[0] < nMin) 
                nReverseFlag = 1;
        }
      
        if((_gRCSignalVal[1] > nMax) || (_gRCSignalVal[1] < nMin))
        {
            nRCChNum = 1;
            if(_gRCSignalVal[1] < nMin) 
                nReverseFlag = 1;
        }
      
        if((_gRCSignalVal[2] > nMax) || (_gRCSignalVal[2] < nMin))
        {
            nRCChNum = 2;
            if(_gRCSignalVal[2] < nMin) 
                nReverseFlag = 1;
        }
      
        if((_gRCSignalVal[3] > nMax) || (_gRCSignalVal[3] < nMin))
        {
            nRCChNum = 3;
            if(_gRCSignalVal[3] < nMin) 
                nReverseFlag = 1;
        }
      
        if((-1 == nRCChNum) && (_gRCSignalVal[4] < nMin))
            nRCChNum = -2;
        if(_gRCSignalVal[4] > nMax)
            nRCChNum = 4;

        delay(50);
    }
        
    _gRCStickChMapTable[nStickType] = nRCChNum;
    _gRCRvrsFlag[nStickType] = nReverseFlag;

    if(CH_TYPE_ROLL == nStickType)
        Serialprint(F(" *        => Roll Stick is Mapped to Ch"));
    else if(CH_TYPE_PITCH == nStickType)
        Serialprint(F(" *        => Pitch Stick is Mapped to Ch"));
    else if(CH_TYPE_THROTTLE == nStickType)
        Serialprint(F(" *        => Throttle Stick is Mapped to Ch"));
    else if(CH_TYPE_YAW == nStickType)
        Serialprint(F(" *        => Yw Stick is Mapped to Ch"));
    else if(CH_TYPE_ROLL == nStickType)
        Serialprint(F(" *        => Take & Land Stick is Mapped to Ch"));

    Serialprint(_gRCStickChMapTable[nStickType]);
    Serialprintln(_gRCRvrsFlag[nStickType] ? F("  [Reversed]") : F(" "));

    _RC_Wait_Signal();
}


void _RC_Wait_Signal()
{
    byte                nFlag = 0;
    int                 nMin = 1100;
    int                 nMax = 1800;

    while(nFlag < 15)
    {
        if((_gRCSignalVal[0] < nMax) && (_gRCSignalVal[0] > nMin))
            nFlag |= B00000001;
        
        if((_gRCSignalVal[1] < nMax) && (_gRCSignalVal[1] > nMin))
            nFlag |= B00000010;
        
        if((_gRCSignalVal[2] < nMax) && (_gRCSignalVal[2] > nMin))
            nFlag |= B00000100;
        
        if((_gRCSignalVal[3] < nMax) && (_gRCSignalVal[3] > nMin))
            nFlag |= B00001000;

        delay(500);
    }
}


void _Read_RCData_From_EEPROM()
{
    int                 i = 0;
    int                 nStickType = 0;
    int                 nEEPRomAddress = 0;

    Serialprintln(F("")); Serialprintln(F(""));
    Serialprintln(F("********************************************************************"));
    Serialprintln(F(" *       Reading Drone Setting from EEPROM          "));
    Serialprintln(F("********************************************************************"));

    // Read Range of Transmitter
    for(i=EEPROM_DATA_RC_CH0_TYPE ; i<=EEPROM_DATA_RC_CH4_TYPE ; i++)
        _gEEPROMData[i] = EEPROM.read(i);

    for(i=EEPROM_DATA_RC_CH0_REVERSE ; i<=EEPROM_DATA_RC_CH4_REVERSE ; i++)
        _gEEPROMData[i] = EEPROM.read(i);

    for(i=EEPROM_DATA_RC_CH0_LOW_H ; i<=EEPROM_DATA_RC_CH4_HIG_L ; i++)
        _gEEPROMData[i] = EEPROM.read(i);

    delay(300);

    Serialprintln(F(" *            => Done!!   "));
}


void _Write_RCData_To_EEPROM()
{
    int                 i = 0;
    int                 nStickType = 0;
    int                 nEEPRomAddress = 0;

    Serialprintln(F("")); Serialprintln(F(""));
    Serialprintln(F("********************************************************************"));
    Serialprintln(F(" *         Writing Drone Setting to EEPROM          "));
    Serialprintln(F("********************************************************************"));

    // Write Range of Transmitter
    Serialprintln(F(" *            => Write Transmitter Range   "));

    nEEPRomAddress = EEPROM_DATA_RC_CH0_LOW_H;
    for(i=0 ; i<CH_TYPE_MAX ; i++, nEEPRomAddress+=6)
    {
        nStickType = _gRCStickChMapTable[i];
        EEPROM.write(nEEPRomAddress,   _gRCSignal_L[nStickType] >> 8);
        EEPROM.write(nEEPRomAddress+1, _gRCSignal_L[nStickType] & 0b11111111);
        EEPROM.write(nEEPRomAddress+2, _gRCSignal_M[nStickType] >> 8);
        EEPROM.write(nEEPRomAddress+3, _gRCSignal_M[nStickType] & 0b11111111);
        EEPROM.write(nEEPRomAddress+4, _gRCSignal_H[nStickType] >> 8);
        EEPROM.write(nEEPRomAddress+5, _gRCSignal_H[nStickType] & 0b11111111);
    }

    nEEPRomAddress = EEPROM_DATA_RC_CH0_TYPE;
    for(i=0 ; i<CH_TYPE_MAX ; i++, nEEPRomAddress++)
        EEPROM.write(nEEPRomAddress, _gRCStickChMapTable[i]);

    nEEPRomAddress = EEPROM_DATA_RC_CH0_REVERSE;
    for(i=0 ; i<CH_TYPE_MAX ; i++, nEEPRomAddress++)
        EEPROM.write(nEEPRomAddress, _gRCRvrsFlag[i]);

    delay(300);

    Serialprintln(F(" *            => Done!!   "));
}


void _print_CaturedRC_Signals()
{
    Serialprint(F("   //   RC_Roll:"));
    Serialprint(_gRCSignalVal[0]);
    Serialprint(F("   RC_Pitch:"));
    Serialprint(_gRCSignalVal[1]);
    Serialprint(F("   RC_Throttle:"));
    Serialprint(_gRCSignalVal[2]);
    Serialprint(F("   RC_Yaw:"));
    Serialprint(_gRCSignalVal[3]);
    Serialprint(F("   RC_Gear:"));
    Serialprintln(_gRCSignalVal[4]);
}


void _print_CompensatedRC_Signals()
{
    Serialprint(F("   //   RC_Roll:"));
    Serialprint(_gCompensatedRCVal[0]);
    Serialprint(F("   RC_Pitch:"));
    Serialprint(_gCompensatedRCVal[1]);
    Serialprint(F("   RC_Throttle:"));
    Serialprint(_gCompensatedRCVal[2]);
    Serialprint(F("   RC_Yaw:"));
    Serialprint(_gCompensatedRCVal[3]);
    Serialprint(F("   RC_Gear:"));
    Serialprintln(_gCompensatedRCVal[4]);
}


// Get Transmitter Signal From Digital Pin 2, 3, 4, 5, and 6 by HW Interrupt
ISR(PCINT2_vect)
{
    _gCurrTime = micros();

    // Check Status of Digital Pin 2
    if(PIND & B00000100)
    {
        if(0 == (_gRCRisingFlag & B00000001))
        {
            _gRCRisingFlag |= B00000001;
            _gRCChRisingTime[0] = _gCurrTime;
        }
    }
    else if(_gRCRisingFlag & B00000001)
    {
        _gRCRisingFlag &= B11111110;
        _gRCSignalVal[0] = _gCurrTime - _gRCChRisingTime[0];
    }

    // Check Status of Digital Pin 3
    if(PIND & B00001000)
    {
        if(0 == (_gRCRisingFlag & B00000010))
        {
            _gRCRisingFlag |= B00000010;
            _gRCChRisingTime[1] = _gCurrTime;
        }
    }
    else if(_gRCRisingFlag & B00000010)
    {
        _gRCRisingFlag &= B11111101;
        _gRCSignalVal[1] = _gCurrTime - _gRCChRisingTime[1];
    }

    // Check Status of Digital Pin 4
    if(PIND & B00010000)
    {
        if(0 == (_gRCRisingFlag & B00000100))
        {
            _gRCRisingFlag |= B00000100;
            _gRCChRisingTime[2] = _gCurrTime;
        }
    }
    else if(_gRCRisingFlag & B00000100)
    {
        _gRCRisingFlag &= B11111011;
        _gRCSignalVal[2] = _gCurrTime - _gRCChRisingTime[2];
    }

    // Check Status of Digital Pin 5
    if(PIND & B00100000)
    {
        if(0 == (_gRCRisingFlag & B00001000))
        {
            _gRCRisingFlag |= B00001000;
            _gRCChRisingTime[3] = _gCurrTime;
        }
    }
    else if(_gRCRisingFlag & B00001000)
    {
        _gRCRisingFlag &= B11110111;
        _gRCSignalVal[3] = _gCurrTime - _gRCChRisingTime[3];
    }

    // Check Status of Digital Pin 6
    if(PIND & B01000000)
    {
        if(0 == (_gRCRisingFlag & B00010000))
        {
            _gRCRisingFlag |= B00010000;
            _gRCChRisingTime[4] = _gCurrTime;
        }
    }
    else if(_gRCRisingFlag & B00010000)
    {
        _gRCRisingFlag &= B11101111;
        _gRCSignalVal[4] = _gCurrTime - _gRCChRisingTime[4];
    }
}

