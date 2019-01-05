//
//  CommHeader.h
//  SelfFly
//
//  Created by Maverick on 2016. 1. 21..
//

#ifndef __COMMON_HEADER__
#define __COMMON_HEADER__

#define EEPROM_SIZE                         (36)

// Define Axis
#define X_AXIS                              (0)
#define Y_AXIS                              (1)
#define Z_AXIS                              (2)

// Define Max Number of Each Field
#define MAX_CH_ESC                          (4)

// Arduino Pin configuration
#define PIN_LED_BLUE                        (13)
#define PIN_LED_GREEN                       (12)
#define PIN_ESC_CH3                         (11)
#define PIN_ESC_CH2                         (10)
#define PIN_ESC_CH1                         (9)
#define PIN_ESC_CH0                         (8)
#define PIN_LED_RED                         (7)
#define PIN_RC_CH4                          (6)
#define PIN_RC_CH3                          (5)
#define PIN_RC_CH2                          (4)
#define PIN_RC_CH1                          (3)
#define PIN_RC_CH0                          (2)
#define PIN_RESERVED_D01                    (1)
#define PIN_RESERVED_D00                    (0)
#define PIN_RESERVED_A07                    (A7)
#define PIN_RESERVED_A06                    (A6)
#define PIN_GY86_SCL                        (A5)
#define PIN_GY86_SDA                        (A4)
#define PIN_CHECK_POWER_STAT                (A3)
#define PIN_RESERVED_A02                    (A2)
#define PIN_SONAR_TRIG                      (A1)
#define PIN_SONAR_ECHO                      (A0)

// ESC configuration
#define ESC_MIN                             (1000)
#define ESC_MAX                             (2000)
#define ESC_TAKEOFF_OFFSET                  (1100)
#define ESC_ACTUAL_MIN                      (1150)
#define ESC_ACTUAL_MAX                      (1800)

// RC configuration
#define RC_CH0_HIGH                         (1884)
#define RC_CH0_LOW                          (1055)
#define RC_CH1_HIGH                         (1884)
#define RC_CH1_LOW                          (1055)
#define RC_CH2_HIGH                         (1884)
#define RC_CH2_LOW                          (1055)
#define RC_CH3_HIGH                         (1884)
#define RC_CH3_LOW                          (1055)
#define RC_CH4_HIGH                         (1884)
#define RC_CH4_LOW                          (1055)
#define INVERSE_RC_ROLL                     (0)
#define INVERSE_RC_PITCH                    (1)
#define INVERSE_RC_YAW                      (0)

#define IMU_ADDRESS                         (0x68)                          // MPU6050 Address
#define GYRO_FS_PRECISIOM                   (MPU6050_GYRO_FS_250)           // MPU6050_GYRO_FS_250 / MPU6050_GYRO_FS_500 / MPU6050_GYRO_FS_1000 / MPU6050_GYRO_FS_2000
#define GYRO_FS                             (65.5)                          // (2^15 - 1) / (500 * (1 << GYRO_FS_PRECISIOM))
#define ACCEL_FS_PRECISIOM                  (MPU6050_ACCEL_FS_8)            // MPU6050_ACCEL_FS_2 / MPU6050_ACCEL_FS_4  MPU6050_ACCEL_FS_8  MPU6050_ACCEL_FS_16
#define ACCEL_FS                            (16384.0 / (1 << ACCEL_FS_PRECISIOM))
#define MPU6050_DLP_PRECISION               (MPU6050_DLPF_BW_42)

// Flight parameters
#define PITCH_ANG_MIN                       (-30)
#define PITCH_ANG_MAX                       (30)
#define ROLL_ANG_MIN                        (-30)
#define ROLL_ANG_MAX                        (30)
#define YAW_RATE_MIN                        (-30)
#define YAW_RATE_MAX                        (30)

// Offset values
#define PITCH_ANG_OFFSET                    (-4)
#define ROLL_ANG_OFFSET                     (1.6)

#define SAMPLEFREQ                          (133.0)                         // sample frequency in Hz
#define BETADEF                             (1.1)

// AccelGyro Offset Value
#define MPU6050_GYRO_OFFSET_X               (65)
#define MPU6050_GYRO_OFFSET_Y               (-42)
#define MPU6050_GYRO_OFFSET_Z               (-3)
#define MPU6050_ACCEL_OFFSET_X              (-73)
#define MPU6050_ACCEL_OFFSET_Y              (-737)
#define MPU6050_ACCEL_OFFSET_Z              (0)

// Sonar sensor
#define SONAR_MAX_WAIT                      (500)                           // Unit: microsecond
#define SONAR_GETDATA_PERIOD                (200000)                        // Unit: microsecond

#define ROUNDING_BASE                       (10)
#define SAMPLING_TIME                       (0.01)                          // Unit: Seconds

#define RAD_TO_DEG_SCALE                    (57.2958)                       // = 180 / PI
#define DEG_TO_RAD_SCALE                    (0.0175)                        // = PI / 180
#define SINGLE_RADIAN                       (3.141592)                      // = PI
#define DOUBLE_RADIAN                       (6.283184)                      // = 2 * PI
#define BARO_SEA_LEVEL_BASE                 (1013.25)                       // Base Sea Level

//#define ACCELGYRO_FS                        (0.000076335)                   // 1 / (200Hz / GYRO_FS=65.5)
//#define APPROX_SIN_SCALE                    (0.00000133229)                 // ACCELGYRO_FS * (3.142(PI) / 180degr)
#define ACCELGYRO_FS                        (0.0000611)                     // 1 / (250Hz / GYRO_FS=65.5)
#define APPROX_SIN_SCALE                    (0.000001066)                   // ACCELGYRO_FS * (3.142(PI) / 180degr)

#define DRONE_STOP_TIME_TH                  (3000)                          // Unit: num of loop() count, About 30 Sec.
#define RPY_OFFSET_DELAY                    (4000000)                       // Unit: microsecond

// LED Control
#define LED_BLINK_PERIOD                    (1000000)                       // Unit: microsecond

// Hovering
#define HOVERING_ALTITUDE                   (150)                           // Unit: Centimeter

#define USE_AUTO_LEVEL                      (1)
#define USE_SETUP_MODE                      (0)

#if USE_SETUP_MODE
    #define USE_PRINT                       (1)
#else
    #define USE_PRINT                       (1)
#endif
    
#if USE_PRINT
    #define PRINT_SERIAL                    (0)
    #define USE_LCD_DISPLAY                 (0)
    #define USE_PROFILE                     (0)
    #define USE_EXT_SR_READ                 (0)
    #define SERIAL_BAUDRATE                 (115200)
#else
    #define PRINT_SERIAL                    (0)
    #define USE_LCD_DISPLAY                 (!PRINT_SERIAL)
    #define USE_PROFILE                     (0)
#endif

#if (USE_PRINT && PRINT_SERIAL)
    #define Serialprint(...)                Serial.print(__VA_ARGS__)
    #define Serialprintln(...)              Serial.println(__VA_ARGS__)
#else
    #define Serialprint(...)
    #define Serialprintln(...)
#endif

// EEPROM Data Address
typedef enum _EEPROM_DataMap
{
    EEPROM_DATA_RESERVED,
    EEPROM_DATA_RESERVED00 = EEPROM_DATA_RESERVED,
    EEPROM_DATA_RESERVED01,
    EEPROM_DATA_RESERVED02,
    EEPROM_DATA_RESERVED03,
    EEPROM_DATA_RESERVED04,
    EEPROM_DATA_RESERVED05,
    EEPROM_DATA_RESERVED06,
    EEPROM_DATA_RESERVED07,
    EEPROM_DATA_RESERVED08,
    EEPROM_DATA_RESERVED09,
    EEPROM_DATA_RESERVED10,
    EEPROM_DATA_RESERVED11,
    EEPROM_DATA_RESERVED12,
    EEPROM_DATA_RESERVED13,
    EEPROM_DATA_RESERVED14,
    EEPROM_DATA_RESERVED15,
    
    EEPROM_DATA_SIGN,
    EEPROM_DATA_SIGN00 = EEPROM_DATA_SIGN,
    EEPROM_DATA_SIGN01,
    EEPROM_DATA_SIGN02,
    EEPROM_DATA_SIGN03,
    EEPROM_DATA_SIGN04,
    EEPROM_DATA_SIGN05,
    EEPROM_DATA_SIGN06,
    EEPROM_DATA_SIGN07,
    
    // For Type of Accel & Gyro
    EEPROM_DATA_MPU_AXIS,
    EEPROM_DATA_MPU_AXIS0_TYPE = EEPROM_DATA_MPU_AXIS,
    EEPROM_DATA_MPU_AXIS1_TYPE,
    EEPROM_DATA_MPU_AXIS2_TYPE,
    
    EEPROM_DATA_MPU_CALIMEAN,
    EEPROM_DATA_MPU_CALIMEAN_G_AXIS0_0 = EEPROM_DATA_MPU_CALIMEAN,
    EEPROM_DATA_MPU_CALIMEAN_G_AXIS0_1,
    EEPROM_DATA_MPU_CALIMEAN_G_AXIS0_2,
    EEPROM_DATA_MPU_CALIMEAN_G_AXIS0_3,
    EEPROM_DATA_MPU_CALIMEAN_G_AXIS1_0,
    EEPROM_DATA_MPU_CALIMEAN_G_AXIS1_1,
    EEPROM_DATA_MPU_CALIMEAN_G_AXIS1_2,
    EEPROM_DATA_MPU_CALIMEAN_G_AXIS1_3,
    EEPROM_DATA_MPU_CALIMEAN_G_AXIS2_0,
    EEPROM_DATA_MPU_CALIMEAN_G_AXIS2_1,
    EEPROM_DATA_MPU_CALIMEAN_G_AXIS2_2,
    EEPROM_DATA_MPU_CALIMEAN_G_AXIS2_3,
    EEPROM_DATA_MPU_CALIMEAN_A_AXIS0_0,
    EEPROM_DATA_MPU_CALIMEAN_A_AXIS0_1,
    EEPROM_DATA_MPU_CALIMEAN_A_AXIS0_2,
    EEPROM_DATA_MPU_CALIMEAN_A_AXIS0_3,
    EEPROM_DATA_MPU_CALIMEAN_A_AXIS1_0,
    EEPROM_DATA_MPU_CALIMEAN_A_AXIS1_1,
    EEPROM_DATA_MPU_CALIMEAN_A_AXIS1_2,
    EEPROM_DATA_MPU_CALIMEAN_A_AXIS1_3,
    EEPROM_DATA_MPU_CALIMEAN_A_AXIS2_0,
    EEPROM_DATA_MPU_CALIMEAN_A_AXIS2_1,
    EEPROM_DATA_MPU_CALIMEAN_A_AXIS2_2,
    EEPROM_DATA_MPU_CALIMEAN_A_AXIS2_3,
    
    // For Type of Transmitter Stick & Indicatinf Whether Transmitter Value is Reverse or Not
    EEPROM_DATA_RC_TYPE,
    EEPROM_DATA_RC_CH0_TYPE = EEPROM_DATA_RC_TYPE,
    EEPROM_DATA_RC_CH1_TYPE,
    EEPROM_DATA_RC_CH2_TYPE,
    EEPROM_DATA_RC_CH3_TYPE,
    EEPROM_DATA_RC_CH4_TYPE,

    // For Transmitter Range
    EEPROM_DATA_RC_RANGE,
    EEPROM_DATA_RC_CH0_LOW_H = EEPROM_DATA_RC_RANGE,
    EEPROM_DATA_RC_CH0_LOW_L,
    EEPROM_DATA_RC_CH0_MID_H,
    EEPROM_DATA_RC_CH0_MID_L,
    EEPROM_DATA_RC_CH0_HIG_H,
    EEPROM_DATA_RC_CH0_HIG_L,
    EEPROM_DATA_RC_CH1_LOW_H,
    EEPROM_DATA_RC_CH1_LOW_L,
    EEPROM_DATA_RC_CH1_MID_H,
    EEPROM_DATA_RC_CH1_MID_L,
    EEPROM_DATA_RC_CH1_HIG_H,
    EEPROM_DATA_RC_CH1_HIG_L,
    EEPROM_DATA_RC_CH2_LOW_H,
    EEPROM_DATA_RC_CH2_LOW_L,
    EEPROM_DATA_RC_CH2_MID_H,
    EEPROM_DATA_RC_CH2_MID_L,
    EEPROM_DATA_RC_CH2_HIG_H,
    EEPROM_DATA_RC_CH2_HIG_L,
    EEPROM_DATA_RC_CH3_LOW_H,
    EEPROM_DATA_RC_CH3_LOW_L,
    EEPROM_DATA_RC_CH3_MID_H,
    EEPROM_DATA_RC_CH3_MID_L,
    EEPROM_DATA_RC_CH3_HIG_H,
    EEPROM_DATA_RC_CH3_HIG_L,
    EEPROM_DATA_RC_CH4_LOW_H,
    EEPROM_DATA_RC_CH4_LOW_L,
    EEPROM_DATA_RC_CH4_MID_H,
    EEPROM_DATA_RC_CH4_MID_L,
    EEPROM_DATA_RC_CH4_HIG_H,
    EEPROM_DATA_RC_CH4_HIG_L,
    
    EEPROM_DATA_MAX,
}EEPROM_DataMap;


void _Wait(const unsigned long nBaseTime, unsigned long nMicroTime)
{
    while((micros() - nBaseTime) < nMicroTime);
}


float _Clip3Float(const float nValue, const float nMIN, const float nMAX)
{
    float               nClipVal = nValue;
    
    if(nValue < nMIN)
        nClipVal = nMIN;
    else if(nValue > nMAX)
        nClipVal = nMAX;
    
    return nClipVal;
}


int _Clip3Int(const int nValue, const int nMIN, const int nMAX)
{
    int                 nClipVal = nValue;
    
    if(nValue < nMIN)
        nClipVal = nMIN;
    else if(nValue > nMAX)
        nClipVal = nMAX;
    
    return nClipVal;
}


/**
 * Fast inverse square root implementation
 * @see http://en.wikipedia.org/wiki/Fast_inverse_square_root
 */
float _InvSqrt(float nNumber)
{
    long                i = 0;
    float               x = 0.0f, y = 0.0f;
    const float         f = 1.5F;
    
    x = nNumber * 0.5F;
    y = nNumber;
    i = * ( long * ) &y;
    i = 0x5f375a86 - ( i >> 1 );
    y = * ( float * ) &i;
    y = y * ( f - ( x * y * y ) );
    
    return y;
}


#endif /* CommHeader_h */

















