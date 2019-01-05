//
//  LCD_Controller.h
//  SelfFly
//
//  Created by Maverick on 2016. 8. 17..
//

#ifndef __LCD_CONTROL__
#define __LCD_CONTROL__

#if USE_LCD_DISPLAY
void _LCD_Initialize()
{
    Serialprintln(F(" *      2. Start LCD Module Initialization   "));
    
    _gLCDHndl.begin();                                                 //Initialize the LCD
    _gLCDHndl.backlight();                                                     //Activate backlight
    _gLCDHndl.clear();                                                         //Clear the LCD
    
    _gLCDHndl.setCursor(0,0);                                                  //Set the LCD cursor to position to position 0,0
    _gLCDHndl.print(" Maverick Drone");                                        //Print text to screen
    _gLCDHndl.setCursor(0,1);                                                  //Set the LCD cursor to position to position 0,1
    _gLCDHndl.print("      V1.0");                                             //Print text to screen
    
    delay(1500);                                                               //Delay 1.5 second to display the text
    _gLCDHndl.clear();                                                         //Clear the LCD
    delay(100);
    
    Serialprintln(F(" *            => Done!!   "));
}


void _LCD_Clear()
{
    delay(100);                                                               //Delay 1.5 second to display the text
    _gLCDHndl.clear();                                                         //Clear the LCD  
    delay(100);                                                               //Delay 1.5 second to display the text
}


void _LCD_DispDissolveClear(const int nLoopCnt)
{
         if( 0 == nLoopCnt)_gLCDHndl.setCursor(0, 0);
    else if( 4 <= nLoopCnt)_gLCDHndl.print(" ");
    else if( 5 == nLoopCnt)_gLCDHndl.setCursor(0, 1);
    else if( 9 <= nLoopCnt)_gLCDHndl.print(" ");
    
    else if(10 == nLoopCnt)_gLCDHndl.setCursor(4, 0);
    else if(14 <= nLoopCnt)_gLCDHndl.print(" ");
    else if(15 == nLoopCnt)_gLCDHndl.setCursor(4, 1);
    else if(19 <= nLoopCnt)_gLCDHndl.print(" ");
    
    else if(20 == nLoopCnt)_gLCDHndl.setCursor(8, 0);
    else if(24 <= nLoopCnt)_gLCDHndl.print(" ");
    else if(25 == nLoopCnt)_gLCDHndl.setCursor(8, 1);
    else if(29 <= nLoopCnt)_gLCDHndl.print(" ");
    
    else if(30 == nLoopCnt)_gLCDHndl.setCursor(12, 0);
    else if(34 <= nLoopCnt)_gLCDHndl.print(" ");
    else if(35 == nLoopCnt)_gLCDHndl.setCursor(12, 1);
    else if(39 <= nLoopCnt)_gLCDHndl.print(" ");
}

                                  
void _LCD_DispRPY(const int nLoopCnt)
{
    static int nVal;
    
         if( 0 == nLoopCnt){nVal = _gEstimatedRPY[0] * 10.0; _gLCDHndl.setCursor(0, 0);}
    else if( 1 == nLoopCnt)_gLCDHndl.print("R");
    else if( 2 == nLoopCnt){if(nVal < 0) _gLCDHndl.print("-"); else _gLCDHndl.print("+");}
    else if( 3 == nLoopCnt)_gLCDHndl.print(abs(nVal) / 1000);
    else if( 4 == nLoopCnt)_gLCDHndl.print((abs(nVal) / 100) % 10);
    else if( 5 == nLoopCnt)_gLCDHndl.print((abs(nVal) / 10) % 10);
    else if( 6 == nLoopCnt)_gLCDHndl.print(".");
    else if( 7 == nLoopCnt)_gLCDHndl.print(abs(nVal) % 10);
    else if( 8 == nLoopCnt)_gLCDHndl.print(" ");

    else if( 9 == nLoopCnt){nVal = _gEstimatedRPY[1] * 10.0; _gLCDHndl.setCursor(8, 0);}
    else if(10 == nLoopCnt)_gLCDHndl.print("P");
    else if(11 == nLoopCnt){if(nVal < 0) _gLCDHndl.print("-"); else _gLCDHndl.print("+");}
    else if(12 == nLoopCnt)_gLCDHndl.print(abs(nVal) / 1000);
    else if(13 == nLoopCnt)_gLCDHndl.print((abs(nVal) / 100) % 10);
    else if(14 == nLoopCnt)_gLCDHndl.print((abs(nVal) / 10) % 10);
    else if(15 == nLoopCnt)_gLCDHndl.print(".");
    else if(16 == nLoopCnt)_gLCDHndl.print(abs(nVal) % 10);
    else if(17 == nLoopCnt)_gLCDHndl.print(" ");
    
    else if(18 == nLoopCnt){nVal = _gEstimatedRPY[2] * 10.0; _gLCDHndl.setCursor(0, 1);}
    else if(19 == nLoopCnt)_gLCDHndl.print("Y");
    else if(20 == nLoopCnt){if(nVal < 0) _gLCDHndl.print("-"); else _gLCDHndl.print("+");}
    else if(21 == nLoopCnt)_gLCDHndl.print(abs(nVal) / 1000);
    else if(22 == nLoopCnt)_gLCDHndl.print((abs(nVal) / 100) % 10);
    else if(23 == nLoopCnt)_gLCDHndl.print((abs(nVal) / 10) % 10);
    else if(24 == nLoopCnt)_gLCDHndl.print(".");
    else if(25 == nLoopCnt)_gLCDHndl.print(abs(nVal) % 10);
    else if(26 == nLoopCnt)_gLCDHndl.print(" ");
    
    else if(27 == nLoopCnt)_gLCDHndl.setCursor(8, 1);
    else if(28 == nLoopCnt)_gLCDHndl.print(" ");
    else if(29 == nLoopCnt)_gLCDHndl.print(" ");
    else if(30 == nLoopCnt)_gLCDHndl.print("R");
    else if(31 == nLoopCnt)_gLCDHndl.print("P");
    else if(32 == nLoopCnt)_gLCDHndl.print("Y");
    else if(33 == nLoopCnt)_gLCDHndl.print(" ");
    else if(34 == nLoopCnt)_gLCDHndl.print(" ");
    else if(35 == nLoopCnt)_gLCDHndl.print(" ");
}


void _LCD_DispMag(const int nLoopCnt)
{
    static int nVal;

         if( 0 == nLoopCnt){nVal = (int)(_gRawMag[X_AXIS]); _gLCDHndl.setCursor(0, 0);}
    else if( 1 == nLoopCnt)_gLCDHndl.print("X:");
    else if( 2 == nLoopCnt)_gLCDHndl.print(abs(nVal) / 1000);
    else if( 3 == nLoopCnt)_gLCDHndl.print((abs(nVal) / 100) % 10);
    else if( 4 == nLoopCnt)_gLCDHndl.print((abs(nVal) / 10) % 10);
    else if( 5 == nLoopCnt)_gLCDHndl.print(abs(nVal) % 10);
    else if( 6 == nLoopCnt)_gLCDHndl.print(" ");
    else if( 7 == nLoopCnt)_gLCDHndl.print(" ");
    else if( 8 == nLoopCnt)_gLCDHndl.print(" ");
    
    else if( 9 == nLoopCnt){nVal = (int)(_gRawMag[Y_AXIS]); _gLCDHndl.setCursor(8, 0);}
    else if(10 == nLoopCnt)_gLCDHndl.print("Y:");
    else if(11 == nLoopCnt)_gLCDHndl.print(abs(nVal) / 1000);
    else if(12 == nLoopCnt)_gLCDHndl.print((abs(nVal) / 100) % 10);
    else if(13 == nLoopCnt)_gLCDHndl.print((abs(nVal) / 10) % 10);
    else if(14 == nLoopCnt)_gLCDHndl.print(abs(nVal) % 10);
    else if(15 == nLoopCnt)_gLCDHndl.print(" ");
    else if(16 == nLoopCnt)_gLCDHndl.print(" ");
    else if(17 == nLoopCnt)_gLCDHndl.print(" ");
    
    else if(18 == nLoopCnt){nVal = (int)(_gRawMag[Z_AXIS]); _gLCDHndl.setCursor(0, 1);}
    else if(19 == nLoopCnt)_gLCDHndl.print("Z:");
    else if(20 == nLoopCnt)_gLCDHndl.print(abs(nVal) / 1000);
    else if(21 == nLoopCnt)_gLCDHndl.print((abs(nVal) / 100) % 10);
    else if(22 == nLoopCnt)_gLCDHndl.print((abs(nVal) / 10) % 10);
    else if(23 == nLoopCnt)_gLCDHndl.print(abs(nVal) % 10);
    else if(24 == nLoopCnt)_gLCDHndl.print(" ");
    else if(25 == nLoopCnt)_gLCDHndl.print(" ");
    else if(26 == nLoopCnt)_gLCDHndl.print(" ");
    
    else if(27 == nLoopCnt)_gLCDHndl.setCursor(8, 1);
    else if(28 == nLoopCnt)_gLCDHndl.print(" ");
    else if(29 == nLoopCnt)_gLCDHndl.print(" ");
    else if(30 == nLoopCnt)_gLCDHndl.print("M");
    else if(31 == nLoopCnt)_gLCDHndl.print("a");
    else if(32 == nLoopCnt)_gLCDHndl.print("g");
    else if(33 == nLoopCnt)_gLCDHndl.print(" ");
    else if(34 == nLoopCnt)_gLCDHndl.print(" ");
    else if(35 == nLoopCnt)_gLCDHndl.print(" ");
}


void _LCD_DispThrottle(const int nLoopCnt)
{
    static int nVal;
    
         if( 0 == nLoopCnt){nVal = _gESCOutput[0]; _gLCDHndl.setCursor(0, 0);}
    else if( 1 == nLoopCnt)_gLCDHndl.print("T");
    else if( 2 == nLoopCnt)_gLCDHndl.print("0:");
    else if( 3 == nLoopCnt)_gLCDHndl.print(abs(nVal) / 1000);
    else if( 4 == nLoopCnt)_gLCDHndl.print((abs(nVal) / 100) % 10);
    else if( 5 == nLoopCnt)_gLCDHndl.print((abs(nVal) / 10) % 10);
    else if( 6 == nLoopCnt)_gLCDHndl.print(abs(nVal) % 10);
    else if( 7 == nLoopCnt)_gLCDHndl.print(" ");
    else if( 8 == nLoopCnt)_gLCDHndl.print(" ");
    
    else if( 9 == nLoopCnt){nVal = _gESCOutput[1]; _gLCDHndl.setCursor(8, 0);}
    else if(10 == nLoopCnt)_gLCDHndl.print(" ");
    else if(11 == nLoopCnt)_gLCDHndl.print("T");
    else if(12 == nLoopCnt)_gLCDHndl.print("1:");
    else if(13 == nLoopCnt)_gLCDHndl.print(abs(nVal) / 1000);
    else if(14 == nLoopCnt)_gLCDHndl.print((abs(nVal) / 100) % 10);
    else if(15 == nLoopCnt)_gLCDHndl.print((abs(nVal) / 10) % 10);
    else if(16 == nLoopCnt)_gLCDHndl.print(abs(nVal) % 10);
    else if(17 == nLoopCnt)_gLCDHndl.print(" ");
    
    else if(18 == nLoopCnt){nVal = _gESCOutput[2]; _gLCDHndl.setCursor(0, 1);}
    else if(19 == nLoopCnt)_gLCDHndl.print("T");
    else if(20 == nLoopCnt)_gLCDHndl.print("2:");
    else if(21 == nLoopCnt)_gLCDHndl.print(abs(nVal) / 1000);
    else if(22 == nLoopCnt)_gLCDHndl.print((abs(nVal) / 100) % 10);
    else if(23 == nLoopCnt)_gLCDHndl.print((abs(nVal) / 10) % 10);
    else if(24 == nLoopCnt)_gLCDHndl.print(abs(nVal) % 10);
    else if(25 == nLoopCnt)_gLCDHndl.print(" ");
    else if(26 == nLoopCnt)_gLCDHndl.print(" ");
    
    else if(27 == nLoopCnt){nVal = _gESCOutput[3]; _gLCDHndl.setCursor(8, 1);}
    else if(28 == nLoopCnt)_gLCDHndl.print(" ");
    else if(29 == nLoopCnt)_gLCDHndl.print("T");
    else if(30 == nLoopCnt)_gLCDHndl.print("3:");
    else if(31 == nLoopCnt)_gLCDHndl.print(abs(nVal) / 1000);
    else if(32 == nLoopCnt)_gLCDHndl.print((abs(nVal) / 100) % 10);
    else if(33 == nLoopCnt)_gLCDHndl.print((abs(nVal) / 10) % 10);
    else if(34 == nLoopCnt)_gLCDHndl.print(abs(nVal) % 10);
    else if(35 == nLoopCnt)_gLCDHndl.print(" ");
}

void _LCD_DispSonar(const int nLoopCnt)
{
    static int nVal;
    
         if( 0 == nLoopCnt){nVal = _gDistFromGnd; _gLCDHndl.setCursor(0, 0);}
    else if( 1 == nLoopCnt)_gLCDHndl.print(" ");
    else if( 2 == nLoopCnt)_gLCDHndl.print(" ");
    else if( 3 == nLoopCnt)_gLCDHndl.print(" ");
    else if( 4 == nLoopCnt)_gLCDHndl.print(" ");
    else if( 5 == nLoopCnt)_gLCDHndl.print("A");
    else if( 6 == nLoopCnt)_gLCDHndl.print("l");
    else if( 7 == nLoopCnt)_gLCDHndl.print("t");
    else if( 8 == nLoopCnt)_gLCDHndl.print("i");
    else if( 9 == nLoopCnt)_gLCDHndl.print("t");
    else if(10 == nLoopCnt)_gLCDHndl.print("u");
    else if(11 == nLoopCnt)_gLCDHndl.print("d");
    else if(12 == nLoopCnt)_gLCDHndl.print("e");
    else if(13 == nLoopCnt)_gLCDHndl.print(" ");
    else if(14 == nLoopCnt)_gLCDHndl.print(" ");
    else if(15 == nLoopCnt)_gLCDHndl.print(" ");
    else if(16 == nLoopCnt)_gLCDHndl.print(" ");
    
    else if(17 == nLoopCnt)_gLCDHndl.setCursor(0, 1);
    else if(18 == nLoopCnt)_gLCDHndl.print(" ");
    else if(19 == nLoopCnt)_gLCDHndl.print(" ");
    else if(20 == nLoopCnt)_gLCDHndl.print(" ");
    else if(21 == nLoopCnt)_gLCDHndl.print(" ");
    else if(22 == nLoopCnt)_gLCDHndl.print(" ");
    else if(23 == nLoopCnt)_gLCDHndl.print(" ");
    else if(24 == nLoopCnt)_gLCDHndl.print(abs(nVal) / 1000);
    else if(25 == nLoopCnt)_gLCDHndl.print((abs(nVal) / 100) % 10);
    else if(26 == nLoopCnt)_gLCDHndl.print((abs(nVal) / 10) % 10);
    else if(27 == nLoopCnt)_gLCDHndl.print(abs(nVal) % 10);
    else if(28 == nLoopCnt)_gLCDHndl.print("c");
    else if(29 == nLoopCnt)_gLCDHndl.print("m");
    else if(30 == nLoopCnt)_gLCDHndl.print(" ");
    else if(31 == nLoopCnt)_gLCDHndl.print(" ");
    else if(32 == nLoopCnt)_gLCDHndl.print(" ");
    else if(33 == nLoopCnt)_gLCDHndl.print(" ");
}

void _LCD_DispMainLoopTime(const int nLoopCnt)
{
    static int nVal;
    
         if( 0 == nLoopCnt){nVal = (_gLoopEndTime - _gLoopStartTime); _gLCDHndl.setCursor(0, 0);}
    else if( 1 == nLoopCnt)_gLCDHndl.print(" ");
    else if( 2 == nLoopCnt)_gLCDHndl.print(" ");
    else if( 3 == nLoopCnt)_gLCDHndl.print(" ");
    else if( 4 == nLoopCnt)_gLCDHndl.print("L");
    else if( 5 == nLoopCnt)_gLCDHndl.print("o");
    else if( 6 == nLoopCnt)_gLCDHndl.print("o");
    else if( 7 == nLoopCnt)_gLCDHndl.print("p");
    else if( 8 == nLoopCnt)_gLCDHndl.print(" ");
    else if( 9 == nLoopCnt)_gLCDHndl.print("T");
    else if(10 == nLoopCnt)_gLCDHndl.print("i");
    else if(11 == nLoopCnt)_gLCDHndl.print("m");
    else if(12 == nLoopCnt)_gLCDHndl.print("e");
    else if(13 == nLoopCnt)_gLCDHndl.print(" ");
    else if(14 == nLoopCnt)_gLCDHndl.print(" ");
    else if(15 == nLoopCnt)_gLCDHndl.print(" ");
    else if(16 == nLoopCnt)_gLCDHndl.print(" ");
    else if(17 == nLoopCnt)_gLCDHndl.setCursor(0, 1);
    else if(18 == nLoopCnt)_gLCDHndl.print(" ");
    else if(19 == nLoopCnt)_gLCDHndl.print(" ");
    else if(20 == nLoopCnt)_gLCDHndl.print(" ");
    else if(21 == nLoopCnt)_gLCDHndl.print(" ");
    else if(22 == nLoopCnt)_gLCDHndl.print(abs(nVal) / 1000);
    else if(23 == nLoopCnt)_gLCDHndl.print((abs(nVal) / 100) % 10);
    else if(24 == nLoopCnt)_gLCDHndl.print((abs(nVal) / 10) % 10);
    else if(25 == nLoopCnt)_gLCDHndl.print(abs(nVal) % 10);
    else if(26 == nLoopCnt)_gLCDHndl.print("u");
    else if(27 == nLoopCnt)_gLCDHndl.print("s");
    else if(28 == nLoopCnt)_gLCDHndl.print(" ");
    else if(29 == nLoopCnt)_gLCDHndl.print(" ");
    else if(30 == nLoopCnt)_gLCDHndl.print(" ");
    else if(31 == nLoopCnt)_gLCDHndl.print(" ");
    else if(32 == nLoopCnt)_gLCDHndl.print(" ");
    else if(33 == nLoopCnt)_gLCDHndl.print(" ");
}
#else
void _LCD_Initialize()
{
}

void _LCD_Clear()
{
}

void _LCD_DispDissolveClear(const int nLoopCnt)
{
}
                                 
void _LCD_DispRPY(const int nLoopCnt)
{
}

void _LCD_DispMag(const int nLoopCnt)
{
}

void _LCD_DispThrottle(const int nLoopCnt)
{
}

void _LCD_DispSonar(const int nLoopCnt)
{
}

void _LCD_DispMainLoopTime(const int nLoopCnt)
{ 
}
#endif
        

void _LCD_DispInfo()
{
    static int          nDispCnt = 0;
    static int          nModule = 0;
    
    switch(nModule)
    {
        case 0:
            if((1 == bIsMPUInitialized) && (nDispCnt < 1080))
               _LCD_DispRPY(nDispCnt % 36);
            else{
                nDispCnt = 0;
                nModule++;
            }
            break;
            
        case 1:
            if((1 == bIsThrottleInitialized) && (nDispCnt < 1080))
                _LCD_DispThrottle(nDispCnt % 36);
            else{
                nDispCnt = 0;
                nModule++;
            }
            break;
        
        case 2:
            if((1 == bIsMagnitudeInitialized) && (nDispCnt < 1080))
                _LCD_DispMag(nDispCnt % 36);
            else{
                nDispCnt = 0;
                nModule++;
            }
            break;
            
        case 3:
            if((1 == bIsSonarInitialized) && (nDispCnt < 1020))
                _LCD_DispSonar(nDispCnt % 34);
            else{
                nDispCnt = 0;
                nModule++;
            }
            break;
            
        case 4:
            if(nDispCnt < 1020)
                _LCD_DispMainLoopTime(nDispCnt % 34);
            else{
                nDispCnt = 0;
                nModule++;
            }
            break;
            
        default:
            nModule = 0;
            nDispCnt = 0;
            break;
    }
    
    nDispCnt++;
}
#endif /* LCD_Controller_h */



















