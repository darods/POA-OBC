
/*----------------------------------------------------------------------------------------
 File Inclusions
 ----------------------------------------------------------------------------------------*/
#include "CommHeader.h"
#include <EEPROM.h>
#include <I2Cdev.h>
#include <Wire.h>
#include <HMC5883L.h>
#include <MS561101BA.h>
#include <math.h>
#include <Timer.h>
#if USE_LCD_DISPLAY
    #include <LiquidCrystal_I2C.h>
#endif

/*----------------------------------------------------------------------------------------
 Constant Definitions
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Macro Definitions
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Type Definitions
 ----------------------------------------------------------------------------------------*/
typedef enum _RC_CH_Type
{
    CH_TYPE_ROLL                = 0,
    CH_TYPE_PITCH,
    CH_TYPE_THROTTLE,
    CH_TYPE_YAW,
    CH_TYPE_TAKE_LAND,
    CH_TYPE_MAX,
}RC_CH_Type;

typedef enum _DroneStatus
{
    DRONESTATUS_STOP            = 0,
    DRONESTATUS_READY,
    DRONESTATUS_START,
}DroneStatus;

typedef struct _AxisErrRate_T
{
    float           nP_ErrRate;
    float           nI_ErrRate;
    float           nD_ErrRate;
    float           nPrevErrRate;
    float           nBalance;
}AxisErrRate_T;

byte                _gEEPROMData[EEPROM_DATA_MAX] = {0, };

// For Accelerator & Gyroscope Sensor
float               _gRawGyro[3] = {0.0, 0.0, 0.0};
float               _gRawAccel[3] = {0.0, 0.0, 0.0};
float               _gTemperature = 0.0;
float               _gAccTotalVector = 0.0;

float               _gGyro_Roll = 0.0, _gGyro_Pitch = 0.0, _gGyro_Yaw = 0.0;
float               _gAccel_X = 0.0, _gAccel_Y = 0.0, _gAccel_Z = 0.0;

float               _gAngleRollAcc = 0.0, _gAnglePitchAcc = 0.0 ,_gAngleYawAcc = 0.0;
float               _gAngleRoll = 0.0, _gAnglePitch = 0.0 ,_gAngleYaw = 0.0;
float               _gAngleRollOut = 0.0, _gAnglePitchOut = 0.0, _gAngleYawOut = 0.0;
float               _gRollLevelAdjust = 0.0, _gPitchLevelAdjust = 0.0, _gYawLevelAdjust = 0.0;
float               _gCalibMeanGyro[3] = {0.0, 0.0, 0.0};
float               _gCalibMeanAccel[3] = {0.0, 0.0, 0.0};
float               _gCalibMeanTemp = 0.0;
byte                _gGyroAccelAxis[3] = {0, 0, 0};
int                 _gbAngleSet = false;

// For Magnetometer Sensor
HMC5883L            _gMagHndl;                              // HMC5883 Magnetic Interface
float               _gRawMag[3] = {0.0, 0.0, 0.0};
float               _gMagHeadingRad = 0.0;
float               _gMagHeadingDeg = 0.0;
float               _gSmoothHeadingDegrees = 0.0;
float               _gPrevHeadingDegrees = 0.0;
float               _gDeclinationAngle = 0.0;

// For Barometer Sensor
MS561101BA          _gBaroHndl;                             // MS5611 Barometer Interface
float               _gRawTemp = 0.0;                        // Raw Temperature Data
float               _gRawPressure = 0.0;                    // Raw Pressure Data
float               _gRawAbsoluteAltitude = 0.0;            // Estimated Absolute Altitude
float               _gAvgPressure = 0.0;                    // Average Pressure Data
float               _gAvgTemp = 0.0;                        // Average Temperature Data
float               _gAvgAbsoluteAltitude = 0.0;            // Average Absolute Altitude Data
float               _gRelativeAltitude = 0.0;               // Relative Absolute Altitude Data
float               _gPrevAvgAbsoluteAltitude = 0.0;        // Average Absolute Altitude Data
float               _gRefAbsoluteAltitude = 0.0;            // Reference Absolute Altitude Data
float               _gVerticalSpeed = 0.0;                  // Estimated Vertical Speed

// For Sonar Sensor
float               _gRawDist = 0.0;                        // Indicate Distance Calculated From Sensor
float               _gDistFromGnd = 0.0;                    // Indicate istance from Ground

// For PID Control
AxisErrRate_T       _gRPY_PID[3] = {0, };
float               _gPIDGainTable[3][4] = {{1.3, 0.04, 18.00, 400.0},     // P, I, D for Roll
                                            {1.3, 0.04, 18.00, 400.0},     // P, I, D for Pitch
                                            {4.00, 0.02, 00.00, 400.0}};   // P, I, D for Yaw

// For Motor Control
int                 _gRCSignal_L[CH_TYPE_MAX] = {0, };
int                 _gRCSignal_M[CH_TYPE_MAX] = {0, };
int                 _gRCSignal_H[CH_TYPE_MAX] = {0, };
byte                _gRCRvrsFlag[CH_TYPE_MAX] = {0, };
int                 _gESCOutput[MAX_CH_ESC] = {0, };
int                 _gMPUAxisRvrsFlag[3] = {0, };
byte                _gRCRisingFlag = 0;
unsigned long       _gRCChRisingTime[CH_TYPE_MAX] = {0, };
int                 _gRCSignalVal[CH_TYPE_MAX] = {0, };
byte                _gRCChMap[CH_TYPE_MAX] = {0, };
int                 _gCompensatedRCVal[CH_TYPE_MAX] = {0, };

// For Estimated Status of Drone
float               _gEstRoll = 0.0, _gEstPitch = 0.0, _gEstYaw = 0.0;
float               _gEstimatedRPY[3] = {0.0, };            // 0:Roll,   1:Pitch,   2:Yaw

// For Control Interval
unsigned long       _gLoopTimer = 0;
unsigned long       _gPrevSensorCapTime = 0;
unsigned long       _gCurrSensorCapTime = 0;
unsigned long       _gCurrTime = 0;
unsigned long       _gLoopStartTime = 0;
unsigned long       _gLoopEndTime = 0;

#if USE_PROFILE
    unsigned long       _gProfileStartTime = 0;
    unsigned long       _gProfileEndTime = 0;
#endif
float               _gDiffTime = 0.0;

// For Status of Drone
DroneStatus         _gDroneStatus = DRONESTATUS_STOP;

// For Battery Status
float                _gCurrBatteryVolt = 0;

// For LED Control
int                 _gLED_Status = 0;
unsigned long       _gPrevBlinkTime = 0;

// For LCD Control
#if USE_LCD_DISPLAY
    LiquidCrystal_I2C   _gLCDHndl(0x3F,16,2);
#endif

String              _gInputFromHost = "";
boolean             _gInputFromHostComplete = false;

int                 _gDroneInitStep = 1;

Timer               LEDTimer;
//Timer               SonarTimer;

byte                bIsMPUInitialized = 0;
byte                bIsMagnitudeInitialized = 0;
byte                bIsBarometerInitialized = 0;
byte                bIsSonarInitialized = 0;
byte                bIsThrottleInitialized = 0;


/*----------------------------------------------------------------------------------------
 Static Function
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Static Variable
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Global Variable
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Function Implementation
 ----------------------------------------------------------------------------------------*/
#include "LCD_Control.h"
#include "LED_Control.h"
#include "ESC_Control.h"
#include "RC_Control.h"
#include "MPU6050_Control.h"
#include "HMC5883L_Control.h"
//#include "SR04_Control.h"
#include "Misc.h"
#include "AHRS_Control.h"
#include "PID_Control.h"
#include "ExtComm_Control.h"
#include "Debugger.h"


#if !USE_SETUP_MODE
void setup()
{
    int                 i;

    Serialprintln(F(" . ")); Serialprintln(F(" . ")); Serialprintln(F(" . ")); Serialprintln(F(" . "));
    Serialprintln(F("********************************************************************"));
    Serialprintln(F("********************************************************************"));

    // Set I2C Enable
    Wire.begin();

    // Set the I2C clock speed to 400kHz.
    TWBR = 12;
    
    #if USE_PRINT
    Serial.begin(SERIAL_BAUDRATE);
    Serial.flush();
    #endif

    // Initialize LCD
    _LCD_Initialize();

    // Read EEPROM Data
    _EEPROM_Read(EEPROM_DATA_SIGN, 0);
    _EEPROM_Read(EEPROM_DATA_RC_TYPE, 0);
    _EEPROM_Read(EEPROM_DATA_MPU_AXIS, 0);
    _EEPROM_Read(EEPROM_DATA_MPU_CALIMEAN, 0);
    _EEPROM_Read(EEPROM_DATA_RC_RANGE, 0);

    // Initialize LED
    _LED_Initialize();

    // Initialize Gyro_Accel
    _AccelGyro_Initialize();

    // Initialize ESCs
    _ESC_Initialize();

    // Initialize RemoteController
    _RC_Initialize();

    // Initialize Magnetic
    //_Mag_Initialize();

    // Initialize Barometer
    //_Barometer_Initialize();

    // Initialize Sonar Sensor
    //_Sonar_Initialize();

    // Get Initial Accel & Gyro Value
    _AccelGyro_GetGyroAccelData();

    _gInputFromHost.reserve(200);

    _gDroneStatus = DRONESTATUS_STOP;
    _gCurrBatteryVolt = (analogRead(PIN_CHECK_POWER_STAT) + 65) * 1.2317;
    _gLED_Status = 0;
    
    Serialprintln(F("********************************************************************"));
    Serialprintln(F("********************************************************************"));
    Serialprintln(F("   ")); Serialprintln(F("   ")); Serialprintln(F("   ")); Serialprintln(F("   "));

    #if USE_LCD_DISPLAY
        delay(500);
        _gLCDHndl.clear();
    #endif

    // Initialize Loop Timer
    _gLoopTimer = micros();
}


void loop()
{
    static int       nLoopCnt = 0;
    
    _gLoopStartTime = micros();
    
    LEDTimer.update();
    
    _AccelGyro_GetGyroAccelData();
    
    // Get Receiver Input
    // Then Mapping Actual Reciever Value to 1000 ~ 2000
    for(int i=0 ; i<=CH_TYPE_TAKE_LAND ; i++)
        _RC_Compensate(i);

    // Check Drone Status
    _Check_Drone_Status();

    // Calculate Roll, Pitch, and Yaw by Quaternion
    _Get_RollPitchYaw();

    // Check Battery Voltage Status
    _Check_BatteryVolt();

    // PID Computation
    _CalculatePID();

    // Throttle Calculation
    _CalculateThrottleVal();

    // Set Throttle to ESCs
    _ESC_Update();
    
    nLoopCnt++;
    
    #if PRINT_SERIAL || USE_EXT_SR_READ
        _print_Data();
    #endif

    #if USE_LCD_DISPLAY
        _LCD_DispInfo();
    #endif

    _gLoopEndTime = micros();

    Serial.println(_gLoopEndTime - _gLoopStartTime);
}
#else
void setup()
{
    int                 i;

    Serialprintln(F(" . ")); Serialprintln(F(" . ")); Serialprintln(F(" . ")); Serialprintln(F(" . "));
    Serialprintln(F("********************************************************************"));
    Serialprintln(F("********************************************************************"));

    // Set I2C Enable
    Wire.begin();

    #if (USE_PRINT && PRINT_SERIAL)
    Serial.begin(SERIAL_BAUDRATE);
    Serial.flush();
    #endif

    _EEPROM_Read(EEPROM_DATA_MPU_AXIS, 0);

    // Initialize LCD
    _LCD_Initialize();

    // Initialize RemoteController
    _RC_Initialize();

    // Initialize Gyro_Accel
    _AccelGyro_Initialize();

    // Initialize Magnetic
    //_Mag_Initialize();

    // Initialize Barometer
    //_Barometer_Initialize();

    // Initialize Sonar Sensor
    //_Sonar_Initialize();

    Serialprintln(F("********************************************************************"));
    Serialprintln(F("********************************************************************"));
    Serialprintln(F("   ")); Serialprintln(F("   ")); Serialprintln(F("   ")); Serialprintln(F("   "));

    _LCD_Clear();

    _gLoopTimer = micros();
}


void loop()
{
    static int bAllProcessDone = 0;
    static unsigned long cnt = 0;
    
    if(!bAllProcessDone)
    {
        Serialprintln(F("********************************************************************"));
        Serialprintln(F("********************************************************************"));
        Serialprintln(F("   Step 0. Please Set a RemoteController Stick to Center"));
        Serialprintln(F("********************************************************************"));
        Serialprintln(F("********************************************************************"));
        _RC_Wait_CenterPos();
        Serialprintln(F(""));

        // Get Asix of Each RC Channel
        Serialprintln(F("********************************************************************"));
        Serialprintln(F("********************************************************************"));
        Serialprintln(F("   Step 1. Get Axis Type of RC Channel"));
        Serialprintln(F("********************************************************************"));
        Serialprintln(F("********************************************************************"));
        _RC_CheckAxis(0);
        _RC_CheckAxis(1);
        _RC_CheckAxis(2);
        _RC_CheckAxis(3);
        _RC_CheckAxis(4);
        Serialprintln(F(""));

        // Get Range of RC Signal
        Serialprintln(F("********************************************************************"));
        Serialprintln(F("********************************************************************"));
        Serialprintln(F("   Step 2. Get Range of Each RC Channel"));
        Serialprintln(F("********************************************************************"));
        Serialprintln(F("********************************************************************"));
        _RC_GetRCRange();
        Serialprintln(F(""));

        // Calibration Gyro
        Serialprintln(F("********************************************************************"));
        Serialprintln(F("********************************************************************"));
        Serialprintln(F("   Step 3. Gyro & Accel Calibration"));
        Serialprintln(F("********************************************************************"));
        Serialprintln(F("********************************************************************"));
        Serialprintln(F(""));
        _AccelGyro_Calibration();
        Serialprintln(F(""));

        // Get Axis Type of Gyro & Accel
        Serialprintln(F("********************************************************************"));
        Serialprintln(F("********************************************************************"));
        Serialprintln(F("   Step 4. Get Axis Type of Gyro & Accel"));
        Serialprintln(F("********************************************************************"));
        Serialprintln(F("********************************************************************"));
        Serialprintln(F(""));
        _AccelGyro_CheckAxis(0);
        _AccelGyro_CheckAxis(1);
        _AccelGyro_CheckAxis(2);

        Serialprintln(F("   ")); Serialprintln(F("   ")); Serialprintln(F("   ")); Serialprintln(F("   "));
        Serialprintln(F("********************************************************************"));
        Serialprintln(F(" *         Writing Drone Setting to EEPROM          "));
        Serialprintln(F("********************************************************************"));
        _EEPROM_Write(EEPROM_DATA_SIGN);
        _EEPROM_Write(EEPROM_DATA_MPU_AXIS);
        _EEPROM_Write(EEPROM_DATA_MPU_CALIMEAN);
        _EEPROM_Write(EEPROM_DATA_RC_TYPE);
        _EEPROM_Write(EEPROM_DATA_RC_RANGE);

        Serialprintln(F("   ")); Serialprintln(F("   ")); Serialprintln(F("   ")); Serialprintln(F("   "));
        Serialprintln(F("********************************************************************"));
        Serialprintln(F(" *         Reading Drone Setting to EEPROM          "));
        Serialprintln(F("********************************************************************"));
        {
            int         nValidationChk = 0;
            if((0 != _EEPROM_Read(EEPROM_DATA_SIGN, 1)) || 
               (0 != _EEPROM_Read(EEPROM_DATA_MPU_AXIS, 1)) ||
               (0 != _EEPROM_Read(EEPROM_DATA_MPU_CALIMEAN, 1)) ||
               (0 != _EEPROM_Read(EEPROM_DATA_RC_TYPE, 1)) ||
               (0 != _EEPROM_Read(EEPROM_DATA_RC_RANGE, 1)))
            {
                Serialprintln(F(" * "));
                Serialprintln(F(" *    !!!  There are Invalid Data in EEPROM."));
                Serialprintln(F(" *    !!!  Clear EEPROM"));
                Serialprintln(F(" * "));
                Serialprintln(F(" *    !!!  Please Setup Again"));
                _EEPROM_Clear();
            }
            else
                Serialprintln(F(" *    Drone setting Done!!!! Let's go Fly~"));
        }
    }

    _Wait(4000);

    bAllProcessDone = 1;
}
#endif


void serialEvent() 
{
    while(Serial.available()) 
    {
        char inChar = (char)Serial.read();
        
        _gInputFromHost += inChar;
        
        if(inChar == '\n') 
            _gInputFromHostComplete = true;
    }
}


















