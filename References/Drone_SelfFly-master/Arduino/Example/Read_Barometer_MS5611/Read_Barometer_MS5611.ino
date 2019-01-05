
/*----------------------------------------------------------------------------------------
 File Inclusions
 ----------------------------------------------------------------------------------------*/
#include <Servo.h>
#include <I2Cdev.h>
#include <PinChangeInt.h>
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
#include <Wire.h>
#endif
#include <MPU6050_6Axis_MotionApps20.h>
#include <MS561101BA.h>

/*----------------------------------------------------------------------------------------
 Constant Definitions
 ----------------------------------------------------------------------------------------*/
// Define Axis
#define X_AXIS                              (0)
#define Y_AXIS                              (1)
#define Z_AXIS                              (2)

#define GYRO_FS_PRECISIOM                   (MPU6050_GYRO_FS_250)
#define GYRO_FS                             (131.0f)                        // (2^15 - 1) / (250 * (1 << GYRO_FS_PRECISIOM))
#define ACCEL_FS_PRECISIOM                  (MPU6050_ACCEL_FS_2)            //  MPU6050_ACCEL_FS_4  MPU6050_ACCEL_FS_8  MPU6050_ACCEL_FS_16
#define ACCEL_FS                     (16384.0f / (1 << ACCEL_FS_PRECISIOM))

#define ROUNDING_BASE                       (50)
#define SAMPLING_TIME                       (0.01)                          // Unit: Seconds

#define RAD_TO_DEG_SCALE                    (57.2958f)                      // = 180 / PI
#define DEG_TO_RAD_SCALE                    (0.0175f)                       // = PI / 180
#define SINGLE_RADIAN                       (3.141592)                      // = PI
#define DOUBLE_RADIAN                       (6.283184)                      // = 2 * PI
#define BARO_SEA_LEVEL_BASE                 (1013.25)                       // Base Sea Level


/*----------------------------------------------------------------------------------------
 Macro Definitions
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Type Definitions
 ----------------------------------------------------------------------------------------*/
typedef struct _AccelGyroParam_T
{
    float               _gRawGyro[3];
    float               _gRawAccel[3];
    float               _gRawTemp;
    float               nBaseGyro[3];
    float               nBaseAccel[3];
    float               nFineAngle[3];                          // Filtered Angles
}AccelGyroParam_T;

typedef struct _BaroParam_T
{
    float               _gRawTemp;                               // Raw Temperature Data
    float               _gRawPressure;                           // Raw Pressure Data
    float               _gRawAbsoluteAltitude;                   // Estimated Absolute Altitude
    
    float               _gAvgPressure;                           // Average Pressure Data
    float               _gAvgTemp;                               // Average Temperature Data
    float               _gAvgAbsoluteAltitude;                   // Average Absolute Altitude Data
    float               _gRelativeAltitude;                      // Relative Absolute Altitude Data
    float               _gPrevAvgAbsoluteAltitude;               // Average Absolute Altitude Data
    float               _gRefAbsoluteAltitude;                   // Reference Absolute Altitude Data
    float               _gVerticalSpeed;                         // Estimated Vertical Speed
}BaroParam_T;

typedef struct _SelfFly_T
{
    // For Accelerator & Gyroscope Sensor
    MPU6050             nAccelGyroHndl;                         // MPU6050 Gyroscope Interface
    AccelGyroParam_T    nAccelGyroParam;
    int                 nCalibMean_AX, nCalibMean_AY, nCalibMean_AZ;
    int                 nCalibMean_GX, nCalibMean_GY, nCalibMean_GZ;
    
    // For Barometer Sensor
    MS561101BA          _gBaroHndl;                              // MS5611 Barometer Interface
    BaroParam_T         nBaroParam;
        
    unsigned long       _gCurrSensorCapTime;
    unsigned long       _gPrevSensorCapTime;
    float               _gDiffTime;
    float               nSampleFreq;                            // half the sample period expressed in seconds
}SelfFly_T;


/*----------------------------------------------------------------------------------------
 Static Function
 ----------------------------------------------------------------------------------------*/
void _AccelGyro_Initialize();
void _Barometer_Initialize();
void _Barometer_GetData();
void _Barometer_CalculateData();
void _print_BarometerData();


/*----------------------------------------------------------------------------------------
 Static Variable
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Global Variable
 ----------------------------------------------------------------------------------------*/
SelfFly_T               *pSelfFlyHndl = NULL;                            // SelfFly Main Handle

/*----------------------------------------------------------------------------------------
 Function Implementation
 ----------------------------------------------------------------------------------------*/
void setup()
{
    int32_t                 i = 0;
    uint8_t                 nDevStatus;                         // return status after each device operation (0 = success, !0 = error)
    
    Serial.println("");    Serial.println("");    Serial.println("");    Serial.println("");
    Serial.println("****************************************************");
    Serial.println("****************************************************");
    
    pSelfFlyHndl = (SelfFly_T *) malloc(sizeof(SelfFly_T));
    
    memset(pSelfFlyHndl, 0, sizeof(SelfFly_T));
    
    // join I2C bus (I2Cdev library doesn't do this automatically)
    #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    Wire.begin();
    TWBR = 24; // 400kHz I2C clock (200kHz if CPU is 8MHz)
    #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
    Fastwire::setup(400, true);
    #endif
    
    Serial.begin(115200);
    Serial.flush();
    
    while(!Serial); // wait for Leonardo enumeration, others continue immediately
    
    // Initialize Gyro_Accel
    _AccelGyro_Initialize();
    
    // Initialize Barometer
    _Barometer_Initialize();
    
    Serial.println("****************************************************");
    Serial.println("****************************************************");
    Serial.println("");    Serial.println("");    Serial.println("");    Serial.println("");
}


void loop()
{
    _gPrevSensorCapTime = _gCurrSensorCapTime;
    _gCurrSensorCapTime = micros();
    
    _gDiffTime = (_gCurrSensorCapTime - _gPrevSensorCapTime) / 1000000.0;
    nSampleFreq = 1000000.0 / ((_gCurrSensorCapTime - _gPrevSensorCapTime));
    
    _Barometer_GetData();
    
    // Calculate Altitude
    _Barometer_CalculateData();
    
    _print_BarometerData();
    
    delay(50);
}


void _AccelGyro_Initialize()
{
    nAccelGyroHndl = MPU6050();
    
    Serial.println(F(" Initializing MPU..."));
    nAccelGyroHndl.initialize();
    
    // Verify Vonnection
    Serial.print(F("    Testing device connections..."));
    Serial.println(nAccelGyroHndl.testConnection() ? F("  MPU6050 connection successful") : F("  MPU6050 connection failed"));
    
    nAccelGyroHndl.setI2CMasterModeEnabled(false);
    nAccelGyroHndl.setI2CBypassEnabled(true);
    nAccelGyroHndl.setSleepEnabled(false);
    
    // supply your own gyro offsets here, scaled for min sensitivity
    nAccelGyroHndl.setRate(1);                                            // Sample Rate (500Hz = 1Hz Gyro SR / 1+1)
    nAccelGyroHndl.setDLPFMode(MPU6050_DLPF_BW_20);                       // Low Pass filter 20hz
    nAccelGyroHndl.setFullScaleGyroRange(GYRO_FS_PRECISIOM);              // 250? / s (MPU6050_GYRO_FS_250)
    nAccelGyroHndl.setFullScaleAccelRange(ACCEL_FS_PRECISIOM);            // +-2g (MPU6050_ACCEL_FS_2)
    
    Serial.println(F(" MPU Initialized!!!"));
    
    return;
}


void _Barometer_Initialize()
{
    int                     i = 0;
    BaroParam_T             *pBaroParam = &(nBaroParam);
    
    Serial.print(F(" Initializing Barometer Sensor (MS5611)..."));
    
    _gBaroHndl.init(MS561101BA_ADDR_CSB_LOW);
    
    for(i=0 ; i<50 ; i++)
    {
        _Barometer_GetData();
        delay(20);
    }
    
    // Get Average Pressure & Temperature
    pBaroParam->_gAvgTemp = _gBaroHndl.getAvgTemp();
    pBaroParam->_gAvgPressure = _gBaroHndl.getAvgPressure();
    
    // Get Reference Altitude
    pBaroParam->_gRefAbsoluteAltitude = _gBaroHndl.getAltitude(pBaroParam->_gAvgPressure, pBaroParam->_gAvgTemp);
    
    Serial.println(F(" Done"));
}


void _Barometer_GetData()
{
    BaroParam_T             *pBaroParam = &(nBaroParam);
    
    pBaroParam->_gRawTemp = _gBaroHndl.getTemperature(MS561101BA_OSR_512);
    pBaroParam->_gRawPressure = _gBaroHndl.getPressure(MS561101BA_OSR_512);
    
    // Push to Array to Get Average Pressure & Temperature
    _gBaroHndl.pushTemp(pBaroParam->_gRawTemp);
    _gBaroHndl.pushPressure(pBaroParam->_gRawPressure);
}


void _Barometer_CalculateData()
{
    BaroParam_T             *pBaroParam = &(nBaroParam);
    
    // Get Average Pressure & Temperature
    pBaroParam->_gAvgTemp = _gBaroHndl.getAvgTemp();
    pBaroParam->_gAvgPressure = _gBaroHndl.getAvgPressure();
    
    // Get Altitude
    pBaroParam->_gRawAbsoluteAltitude = _gBaroHndl.getAltitude(pBaroParam->_gAvgPressure, pBaroParam->_gAvgTemp);
    
    // Push to Array to Get Average Altitude
    _gBaroHndl.pushAltitude(pBaroParam->_gRawAbsoluteAltitude);
    
    // Get Average Pressure & Temperature
    pBaroParam->_gAvgAbsoluteAltitude = _gBaroHndl.getAvgAltitude();
    
    // Get Vertical Speed
    pBaroParam->_gVerticalSpeed = abs(pBaroParam->_gAvgAbsoluteAltitude - pBaroParam->_gPrevAvgAbsoluteAltitude) / (_gDiffTime);
    pBaroParam->_gRelativeAltitude = pBaroParam->_gAvgAbsoluteAltitude - pBaroParam->_gRefAbsoluteAltitude;
    
    pBaroParam->_gPrevAvgAbsoluteAltitude = pBaroParam->_gAvgAbsoluteAltitude;
}


void _print_BarometerData()
{
    BaroParam_T             *pBaroParam = &(nBaroParam);
    
    Serial.print("           "); Serial.print(pBaroParam->_gAvgTemp);                  // Barometer AvgTemp
    Serial.print("           "); Serial.print(pBaroParam->_gAvgPressure);              // AvgPress
    Serial.print("           "); Serial.print(pBaroParam->_gAvgAbsoluteAltitude);      // AvgAlt
    Serial.print("           "); Serial.print(pBaroParam->_gRelativeAltitude);         // RelativeAlt
    Serial.print("           "); Serial.print(pBaroParam->_gVerticalSpeed);            // VerticalSpeed
    Serial.println("   ");
}

