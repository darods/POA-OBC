
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
#define ACCEL_FS                            (16384.0f / (1 << ACCEL_FS_PRECISIOM))

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

typedef struct _SelfFly_T
{
    // For Accelerator & Gyroscope Sensor
    MPU6050             nAccelGyroHndl;                         // MPU6050 Gyroscope Interface
    AccelGyroParam_T    nAccelGyroParam;
    int                 nCalibMean_AX, nCalibMean_AY, nCalibMean_AZ;
    int                 nCalibMean_GX, nCalibMean_GY, nCalibMean_GZ;
    
    unsigned long       _gCurrSensorCapTime;
    unsigned long       _gPrevSensorCapTime;
    float               _gDiffTime;
    float               nSampleFreq;                            // half the sample period expressed in seconds
}SelfFly_T;


/*----------------------------------------------------------------------------------------
 Static Function
 ----------------------------------------------------------------------------------------*/
void _AccelGyro_Initialize();
void _AccelGyro_GetGyroData();
void _AccelGyro_CalculateAngle();
void _AccelGyro_GetMeanSensor();
void _AccelGyro_Calibration();
void _print_Gyro_Signals();


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
    AccelGyroParam_T        *pAccelGyroParam = &(nAccelGyroParam);
    
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
    
    Serial.println("****************************************************");
    Serial.println("****************************************************");
    Serial.println("");    Serial.println("");    Serial.println("");    Serial.println("");
}


void loop()
{
    AccelGyroParam_T        *pAccelGyroParam = &(nAccelGyroParam);

    _gPrevSensorCapTime = _gCurrSensorCapTime;
    _gCurrSensorCapTime = micros();
    
    _gDiffTime = (_gCurrSensorCapTime - _gPrevSensorCapTime) / 1000000.0;
    nSampleFreq = 1000000.0 / ((_gCurrSensorCapTime - _gPrevSensorCapTime));
    
    _AccelGyro_GetGyroData();
    
    delay(50);
    
    _print_Gyro_Signals();
}


void _AccelGyro_Initialize()
{
    uint8_t             *pOffset;
    int                 i = 0;

    Serialprintln(F(" *      5. Start MPU6050 Module Initialization   "));

    //    nAccelGyroHndl = MPU6050();
    //
    //    Serialprintln(F(" Initializing MPU..."));
    //    nAccelGyroHndl.initialize();
    //
    //    delay(100);
    //
    //    // Verify Vonnection
    //    Serialprint(F("    Testing device connections..."));
    //    Serialprintln(nAccelGyroHndl.testConnection() ? F("  MPU6050 connection successful") : F("  MPU6050 connection failed"));
    //
    //    nAccelGyroHndl.setI2CMasterModeEnabled(false);
    //    nAccelGyroHndl.setI2CBypassEnabled(true);
    //    nAccelGyroHndl.setSleepEnabled(false);
    //
    //    // Calibrate GyroAccel
    //    //nAccelGyroHndl.doCalibration();
    //    nAccelGyroHndl.setXGyroOffset(MPU6050_GYRO_OFFSET_X);
    //    nAccelGyroHndl.setYGyroOffset(MPU6050_GYRO_OFFSET_Y);
    //    nAccelGyroHndl.setZGyroOffset(MPU6050_GYRO_OFFSET_Z);
    //    nAccelGyroHndl.setXAccelOffset(MPU6050_ACCEL_OFFSET_X);
    //    nAccelGyroHndl.setYAccelOffset(MPU6050_ACCEL_OFFSET_Y);
    //    //nAccelGyroHndl.setZAccelOffset(MPU6050_ACCEL_OFFSET_Z);
    //
    //    // supply your own gyro offsets here, scaled for min sensitivity
    //    nAccelGyroHndl.setRate(1);                                            // Sample Rate (500Hz = 1Hz Gyro SR / 1+1)
    //    nAccelGyroHndl.setDLPFMode(MPU6050_DLP_PRECISION);
    //    nAccelGyroHndl.setFullScaleGyroRange(GYRO_FS_PRECISIOM);
    //    nAccelGyroHndl.setFullScaleAccelRange(ACCEL_FS_PRECISIOM);

    Wire.beginTransmission(0x68);
    Wire.write(0x6B);
    Wire.write(0x00);
    Wire.endTransmission();

    Wire.beginTransmission(0x68);
    Wire.write(0x1B);
    Wire.write(0x08);
    Wire.endTransmission();

    Wire.beginTransmission(0x68);
    Wire.write(0x1B);
    Wire.endTransmission();
    Wire.requestFrom(0x68, 1);
    while(Wire.available() < 1);
    if(Wire.read() != 0x08)
    {
        digitalWrite(12,HIGH);
        while(1)
            delay(10);
    }

    Wire.beginTransmission(0x68);
    Wire.write(0x1A);
    Wire.write(0x03);
    Wire.endTransmission();

    delay(300);

    // Calibration
    _AccelGyro_Calibration();

    #if 0
    // Set Gyro Offset
    for(i=0 ; i<=Z_AXIS ; i++)
    {
        pOffset = &(_gCalibMeanGyro[i]);
        Wire.beginTransmission(0x68);
        Wire.write(0x13 + (2 * i));
        Wire.write((pOffset[0]);
        Wire.write((pOffset[1]);
        Wire.endTransmission();
    }

    // Set Accel Offset
    for(i=0 ; i<=Z_AXIS ; i++)
    {
        pOffset = &(_gCalibMeanAccel[i]);
        Wire.beginTransmission(0x68);
        Wire.write(0x06 + (2 * i));
        Wire.write((pOffset[0]);
        Wire.write((pOffset[1]);
        Wire.endTransmission();
    }
    #endif

    _gEstRollPY[0] = _gEstRollPY[1] = _gEstRollPY[2] = 0.0;

    delay(300);

    Serialprintln(F(" *            => Done!!   "));

    return;
}


void _AccelGyro_GetGyroData()
{
    Wire.beginTransmission(0x68);
    Wire.write(0x43);                                               // starting with register 0x43 (GYRO_XOUT_H)
    Wire.endTransmission();
    Wire.requestFrom(0x68, 6);                                      // request a total of 14 registers

    _gRawGyro[X_AXIS] = (Wire.read()<<8 | Wire.read());              // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
    _gRawGyro[Y_AXIS] = (Wire.read()<<8 | Wire.read());              // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
    _gRawGyro[Z_AXIS] = (Wire.read()<<8 | Wire.read());              // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)

    _gRawGyro[X_AXIS] -= _gCalibMeanGyro[X_AXIS];
    if(1 == _gMPUAxisRvrsFlag[X_AXIS]) _gRawGyro[X_AXIS] *= -1;
    _gRawGyro[Y_AXIS] -= _gCalibMeanGyro[Y_AXIS];
    if(1 == _gMPUAxisRvrsFlag[Y_AXIS]) _gRawGyro[Y_AXIS] *= -1;
    _gRawGyro[Z_AXIS] -= _gCalibMeanGyro[Z_AXIS];
    if(1 == _gMPUAxisRvrsFlag[Z_AXIS]) _gRawGyro[Z_AXIS] *= -1;
}


void _AccelGyro_GetAccelData()
{
    Wire.beginTransmission(0x68);
    Wire.write(0x3B);                                               // starting with register 0x3B (ACCEL_XOUT_H)
    Wire.endTransmission();
    Wire.requestFrom(0x68, 6);                                      // request a total of 6 registers

    _gRawAccel[X_AXIS] = (Wire.read()<<8 | Wire.read());              // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
    _gRawAccel[Y_AXIS] = (Wire.read()<<8 | Wire.read());              // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
    _gRawAccel[Z_AXIS] = (Wire.read()<<8 | Wire.read());              // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)

    _gRawAccel[X_AXIS] -= _gCalibMeanAccel[X_AXIS];
    if(1 == _gMPUAxisRvrsFlag[X_AXIS]) _gRawAccel[X_AXIS] *= -1;
    _gRawAccel[Y_AXIS] -= _gCalibMeanAccel[Y_AXIS];
    if(1 == _gMPUAxisRvrsFlag[Y_AXIS]) _gRawAccel[Y_AXIS] *= -1;
    _gRawAccel[Z_AXIS] -= _gCalibMeanAccel[Z_AXIS];
    if(1 == _gMPUAxisRvrsFlag[Z_AXIS]) _gRawAccel[Z_AXIS] *= -1;
}


void _AccelGyro_Calibration()
{
    int                     i = 0;

    Serialprint(F(" *          Calibrating "));

    for(i=0 ; i<=Z_AXIS ; i++)
        _gCalibMeanGyro[i] = 0.0;

    for(i=0 ; i<2000 ; i++)
    {
        Wire.beginTransmission(0x68);
        Wire.write(0x43);                                               // starting with register 0x43 (GYRO_XOUT_H)
        Wire.endTransmission();
        Wire.requestFrom(0x68, 6);                                      // request a total of 6 registers

        _gCalibMeanGyro[X_AXIS] += (Wire.read()<<8 | Wire.read());      // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
        _gCalibMeanGyro[Y_AXIS] += (Wire.read()<<8 | Wire.read());      // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
        _gCalibMeanGyro[Z_AXIS] += (Wire.read()<<8 | Wire.read());      // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)

        Wire.beginTransmission(0x68);
        Wire.write(0x3B);                                               // starting with register 0x3B (ACCEL_XOUT_H)
        Wire.endTransmission();
        Wire.requestFrom(0x68, 6);                                      // request a total of 6 registers

        _gCalibMeanAccel[X_AXIS] += (Wire.read()<<8 | Wire.read());     // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
        _gCalibMeanAccel[Y_AXIS] += (Wire.read()<<8 | Wire.read());     // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
        _gCalibMeanAccel[Z_AXIS] += (Wire.read()<<8 | Wire.read());     // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)

        if(0 == (i % 20))
            Serialprint(F("."));
    }

    for(i=0 ; i<=Z_AXIS ; i++)
    {
        _gCalibMeanGyro[i] /= 2000;
        _gCalibMeanAccel[i] /= 2000;
    }

    Serialprintln(F("."));

    Serialprintln(F(" *          => Done!!   "));
}


void _AccelGyro_CalculateAngle()
{
    AccelGyroParam_T        *pAccelGyroParam = &(nAccelGyroParam);
    const float             *pRawGyro = &(pAccelGyroParam->_gRawGyro[X_AXIS]);
    const float             *pRawAccel = &(pAccelGyroParam->_gRawAccel[X_AXIS]);
    const float             *pBaseGyro = &(pAccelGyroParam->nBaseGyro[X_AXIS]);
    float                   *pFineAngle = &(pAccelGyroParam->nFineAngle[X_AXIS]);
    static float            nGyroAngle[3] = {0, };
    float                   nAccelAngle[3] = {0, };
    float                   nGyroDiffAngle[3] = {0, };
    const float             nGyroWeight = _gDiffTime / GYRO_FS;
    
    // Convert Gyro Values to Degrees/Sec
    nGyroDiffAngle[X_AXIS] = pRawGyro[X_AXIS] * nGyroWeight;
    nGyroDiffAngle[Y_AXIS] = pRawGyro[Y_AXIS] * nGyroWeight;
    nGyroDiffAngle[Z_AXIS] = pRawGyro[Z_AXIS] * nGyroWeight;
    
    // Calculate
    nAccelAngle[X_AXIS] = atan(pRawAccel[Y_AXIS] / sqrt(pow(pRawAccel[X_AXIS], 2) + pow(pRawAccel[Z_AXIS], 2))) * RAD_TO_DEG_SCALE;
    nAccelAngle[Y_AXIS] = atan((-1) * pRawAccel[X_AXIS] / sqrt(pow(pRawAccel[Y_AXIS], 2) + pow(pRawAccel[Z_AXIS], 2))) * RAD_TO_DEG_SCALE;
    nAccelAngle[Z_AXIS] = 0;
    
    // Compute the (filtered) gyro angles
    nGyroAngle[X_AXIS] = nGyroDiffAngle[X_AXIS] + pFineAngle[X_AXIS];
    nGyroAngle[Y_AXIS] = nGyroDiffAngle[Y_AXIS] + pFineAngle[Y_AXIS];
    nGyroAngle[Z_AXIS] = nGyroDiffAngle[Z_AXIS] + pFineAngle[Z_AXIS];
    
    // Apply the complementary filter to figure out the change in angle - choice of alpha is
    // estimated now.  Alpha depends on the sampling rate...
    {
        const float     nOffset0 = 0.80;
        const float     nOffset1 = 1.0 - nOffset0;
        
        pFineAngle[X_AXIS] = nOffset0 * nGyroAngle[X_AXIS] + nOffset1 * nAccelAngle[X_AXIS];
        pFineAngle[Y_AXIS] = nOffset0 * nGyroAngle[Y_AXIS] + nOffset1 * nAccelAngle[Y_AXIS];
        pFineAngle[Z_AXIS] = nGyroAngle[Z_AXIS];  //Accelerometer doesn't give z-angle
    }
}


void _AccelGyro_GetMeanSensor()
{
    long                i=0;
    long                nAccelBufX=0, nAccelBufY=0, nAccelBufZ=0;
    long                nGyroBufX=0, nGyroBufY=0, nGyroBufZ=0;
    const int           nLoopCnt = 1000;
    
    while (i<(nLoopCnt + 101))
    {
        // read raw accel/gyro measurements from device
        // Read Gyro and Accelerate Data
        Wire.beginTransmission(MPU6050_ADDRESS_AD0_LOW);
        Wire.write(MPU6050_RA_ACCEL_XOUT_H);  // starting with register 0x3B (ACCEL_XOUT_H)
        Wire.endTransmission(false);
        Wire.requestFrom(MPU6050_ADDRESS_AD0_LOW, 14, true);  // request a total of 14 registers
        
        if (i>100 && i<=(nLoopCnt + 100))
        {
            //First 100 measures are discarded
            nAccelBufX += (Wire.read()<<8 | Wire.read());
            nAccelBufY += (Wire.read()<<8 | Wire.read());
            nAccelBufZ += (Wire.read()<<8 | Wire.read());
            Wire.read(); Wire.read();
            nGyroBufX += (Wire.read()<<8 | Wire.read());
            nGyroBufY += (Wire.read()<<8 | Wire.read());
            nGyroBufZ += (Wire.read()<<8 | Wire.read());
        }
        
        if (i==(nLoopCnt + 100))
        {
            nCalibMean_AX = nAccelBufX / nLoopCnt;
            nCalibMean_AY = nAccelBufY / nLoopCnt;
            nCalibMean_AZ = nAccelBufZ / nLoopCnt;
            nCalibMean_GX = nGyroBufX / nLoopCnt;
            nCalibMean_GY = nGyroBufY / nLoopCnt;
            nCalibMean_GZ = nGyroBufZ / nLoopCnt;
        }
        
        i++;
        delay(2); //Needed so we don't get repeated measures
    }
}


// Reference
// http://wired.chillibasket.com/2015/01/calibrating-mpu6050/
void _AccelGyro_Calibration()
{
    int             nOffset_AX = 0, nOffset_AY = 0, nOffset_AZ;
    int             nOffset_GX = 0, nOffset_GY = 0, nOffset_GZ = 0;
    const int       nAccelDeadZone = 8;     //Acelerometer error allowed, make it lower to get more precision, but sketch may not converge  (default:8)
    const int       nGyroDeadZone = 1;     //Giro error allowed, make it lower to get more precision, but sketch may not converge  (default:1)
    
    _AccelGyro_GetMeanSensor();
    
    nOffset_AX = -nCalibMean_AX / 8;
    nOffset_AY = -nCalibMean_AY / 8;
    nOffset_AZ = (16384 - nCalibMean_AZ) / 8;
    nOffset_GX = -nCalibMean_GX / 4;
    nOffset_GY = -nCalibMean_GY / 4;
    nOffset_GZ = -nCalibMean_GZ / 4;
    
    while (1)
    {
        int         ready = 0;
        
        nAccelGyroHndl.setXAccelOffset(nOffset_AX);
        nAccelGyroHndl.setYAccelOffset(nOffset_AY);
        nAccelGyroHndl.setZAccelOffset(nOffset_AZ);
        nAccelGyroHndl.setXGyroOffset(nOffset_GX);
        nAccelGyroHndl.setYGyroOffset(nOffset_GY);
        nAccelGyroHndl.setZGyroOffset(nOffset_GZ);
        
        _AccelGyro_GetMeanSensor();
        Serial.print("...");
        
        if (abs(nCalibMean_AX) <= nAccelDeadZone) ready++;
        else nOffset_AX -= (nCalibMean_AX / nAccelDeadZone);
        
        if (abs(nCalibMean_AY)<=nAccelDeadZone) ready++;
        else nOffset_AY -= (nCalibMean_AY / nAccelDeadZone);
        
        if (abs(16384-nCalibMean_AZ)<=nAccelDeadZone) ready++;
        else nOffset_AZ += ((16384 - nCalibMean_AZ) / nAccelDeadZone);
        
        if (abs(nCalibMean_GX)<=nGyroDeadZone) ready++;
        else nOffset_GX -= (nCalibMean_GX / (nGyroDeadZone + 1));
        
        if (abs(nCalibMean_GY)<=nGyroDeadZone) ready++;
        else nOffset_GY -= (nCalibMean_GY / (nGyroDeadZone + 1));
        
        if (abs(nCalibMean_GZ)<=nGyroDeadZone) ready++;
        else nOffset_GZ -= (nCalibMean_GZ / (nGyroDeadZone + 1));
        
        Serial.print("   Gx: ");
        Serial.print(nOffset_GX);
        Serial.print("   Gy: ");
        Serial.print(nOffset_GY);
        Serial.print("   Gz: ");
        Serial.print(nOffset_GZ);
        
        Serial.print("   Ax: ");
        Serial.print(nOffset_AX);
        Serial.print("   Ay: ");
        Serial.print(nOffset_AY);
        Serial.print("   Az: ");
        Serial.println(nOffset_AZ);
        
        if(6 == ready)
            break;
    }
}


void _print_Gyro_Signals()
{
    Serial.print("            ");
    Serial.print(nAccelGyroParam._gRawGyro[0]);
    Serial.print("            ");
    Serial.print(nAccelGyroParam._gRawGyro[1]);
    Serial.print("            ");
    Serial.print(nAccelGyroParam._gRawGyro[2]);
    
    Serial.print("            ");
    Serial.print(nAccelGyroParam._gRawAccel[0]);
    Serial.print("          ");
    Serial.print(nAccelGyroParam._gRawAccel[1]);
    Serial.print("           ");
    Serial.println(nAccelGyroParam._gRawAccel[2]);
}

