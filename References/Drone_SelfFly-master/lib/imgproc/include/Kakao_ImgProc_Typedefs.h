//
//  Kakao_ImgProc_Typedefs.h
//
//  Created by Maverick Park on 2014. 8. 10.
//  Copyright (c) 2015년 Kakao. All rights reserved.
//

#ifndef __KAKAO_IMGPROC_TYPEDEF_H__
#define __KAKAO_IMGPROC_TYPEDEF_H__

#include "Kakao_ImgProc_Platform.h"



#if ENABLE_OS(OS_WIN32) //////////////////////////// WIN32
    typedef signed char             INT8;
    typedef unsigned char           UINT8;
    typedef signed short            INT16;
    typedef unsigned short          UINT16;
    typedef signed int              INT32;
    typedef unsigned int            UINT32;
    typedef signed __int64          INT64;
    typedef unsigned __int64        UINT64;
    typedef float                   F32;
    typedef double                  D64;
    typedef void*                   HANDLE;
    typedef int                     IMGPROC_BOOL;
#elif ENABLE_OS(OS_WIN64) //////////////////////////// WIN64
    typedef signed char             INT8;
    typedef unsigned char           UINT8;
    typedef signed short            INT16;
    typedef unsigned short          UINT16;
    typedef signed int              INT32;
    typedef unsigned int            UINT32;
    typedef signed __int64          INT64;
    typedef unsigned __int64        UINT64;
    typedef float                   F32;
    typedef double                  D64;
    typedef void*                   HANDLE;
    typedef int                     IMGPROC_BOOL;
#elif ENABLE_OS(OS_OSX) //////////////////////////// MAC
    typedef signed char             INT8;
    typedef unsigned char           UINT8;
    typedef signed short            INT16;
    typedef unsigned short          UINT16;
    typedef signed int              INT32;
    typedef unsigned int            UINT32;
    typedef signed long long        INT64;
    typedef unsigned long long      UINT64;
    typedef float                   F32;
    typedef double                  D64;
    typedef void*                   HANDLE;
    typedef int                     IMGPROC_BOOL;
#elif ENABLE_OS(OS_IOS) //////////////////////////// iOS
    typedef signed char             INT8;
    typedef unsigned char           UINT8;
    typedef signed short            INT16;
    typedef unsigned short          UINT16;
    typedef signed int              INT32;
    typedef unsigned int            UINT32;
    typedef signed long long        INT64;
    typedef unsigned long long      UINT64;
    typedef float                   F32;
    typedef double                  D64;
    typedef void*                   HANDLE;
    typedef int                     IMGPROC_BOOL;
#elif ENABLE_OS(OS_ANDROID) //////////////////////////// Android
    typedef signed char             INT8;
    typedef unsigned char           UINT8;
    typedef signed short            INT16;
    typedef unsigned short          UINT16;
    typedef signed int              INT32;
    typedef unsigned int            UINT32;
    typedef signed long long        INT64;
    typedef unsigned long long      UINT64;
    typedef float                   F32;
    typedef double                  D64;
    typedef void*                   HANDLE;
    typedef int                     IMGPROC_BOOL;
#else
    #error   "Please Define the Default Type"
#endif


#ifndef FOURCC
    #ifdef __cplusplus
        #define FOURCC(a, b, c, d) ( \
                                                                                (static_cast<UINT32>(a)) | (static_cast<UINT32>(b) << 8) | \
                                                                                (static_cast<UINT32>(c) << 16) | (static_cast<UINT32>(d) << 24))
    #else
        #define FOURCC(a, b, c, d) ( \
                                                                                ((UINT32)(a)) | ((UINT32)(b) << 8) | /* NOLINT */ \
                                                                                ((UINT32)(c) << 16) | ((UINT32)(d) << 24))  /* NOLINT */
    #endif
#endif


typedef enum _ImgFormat
{
    IMG_FORMAT_GRAY                 = FOURCC('G', 'R', 'A', 'Y'),
    IMG_FORMAT_RGB24                = FOURCC('R', 'G', 'B', '3'),
    IMG_FORMAT_BGR24                = FOURCC('B', 'G', 'R', '3'),
    IMG_FORMAT_RGBA32               = FOURCC('R', 'G', 'B', 'A'),
    IMG_FORMAT_ARGB32               = FOURCC('A', 'R', 'G', 'B'),
    IMG_FORMAT_BGRA32               = FOURCC('B', 'G', 'R', 'A'),
    IMG_FORMAT_ABGR32               = FOURCC('A', 'B', 'G', 'R'),
    IMG_FORMAT_I420                 = FOURCC('I', '4', '2', '0'),
    IMG_FORMAT_I422                 = FOURCC('I', '4', '2', '2'),
    IMG_FORMAT_HSV                  = FOURCC('H', 'S', 'V', '0'),
    IMG_FORMAT_HSL                  = FOURCC('H', 'S', 'L', '0'),
    IMG_FORMAT_NV12                 = FOURCC('N', 'V', '1', '2'),
    IMG_FORMAT_NV21                 = FOURCC('N', 'V', '2', '1'),
    IMG_FORMAT_UYVY                 = FOURCC('U', 'Y', 'V', 'Y'),
    IMG_FORMAT_MAX                  = 15,

    // Defined for Using External Color Convertor
    IMG_FORMAT_IYUV                 = FOURCC('I', 'Y', 'U', 'V'),   // Alias for I420.
    IMG_FORMAT_YU12                 = FOURCC('Y', 'U', '1', '2'),   // Alias for I420.
    IMG_FORMAT_YU16                 = FOURCC('Y', 'U', '1', '6'),   // Alias for I422.
    IMG_FORMAT_YU24                 = FOURCC('Y', 'U', '2', '4'),   // Alias for I444.
    IMG_FORMAT_YV12                 = FOURCC('Y', 'V', '1', '2'),   // Alias for I420.
    IMG_FORMAT_YV16                 = FOURCC('Y', 'V', '1', '6'),   // Alias for I422.
    IMG_FORMAT_YV24                 = FOURCC('Y', 'V', '2', '4'),   // Alias for I444.
    IMG_FORMAT_YUYV                 = FOURCC('Y', 'U', 'Y', 'V'),   // Alias for YUY2.
    IMG_FORMAT_YVYU                 = FOURCC('Y', 'V', 'Y', 'U'),   // Alias for YUY2.
    IMG_FORMAT_YUVS                 = FOURCC('y', 'u', 'v', 's'),   // Alias for YUY2 on Mac.
    IMG_FORMAT_HDYC                 = FOURCC('H', 'D', 'Y', 'C'),   // Alias for UYVY.
    IMG_FORMAT_2VUY                 = FOURCC('2', 'v', 'u', 'y'),   // Alias for UYVY on Mac.
    IMG_FORMAT_JPEG                 = FOURCC('J', 'P', 'E', 'G'),   // Alias for MJPG.
    IMG_FORMAT_DMB1                 = FOURCC('d', 'm', 'b', '1'),   // Alias for MJPG on Mac.
    IMG_FORMAT_BA81                 = FOURCC('B', 'A', '8', '1'),   // Alias for BGGR.
    IMG_FORMAT_RGB3                 = FOURCC('R', 'G', 'B', '3'),   // Alias for RAW.
    IMG_FORMAT_BGR3                 = FOURCC('B', 'G', 'R', '3'),   // Alias for 24BG.
    IMG_FORMAT_CM32                 = FOURCC( 0,   0,   0,  32),    // Alias for BGRA kCMPixelFormat_32ARGB
    IMG_FORMAT_CM24                 = FOURCC( 0,   0,   0,  24),    // Alias for RAW kCMPixelFormat_24RGB
    IMG_FORMAT_L555                 = FOURCC('L', '5', '5', '5'),   // Alias for RGBO.
    IMG_FORMAT_L565                 = FOURCC('L', '5', '6', '5'),   // Alias for RGBP.
    IMG_FORMAT_5551                 = FOURCC('5', '5', '5', '1'),   // Alias for RGBO.
    
    IMG_FORMAT_INVALID              = FOURCC('I', 'N', 'V', 'A'),   // Invalid Image Format
}ImgFormat;


typedef enum _ImgDepth
{
    IMG_DEPTH_U8                    = 0,
    IMG_DEPTH_U32,
    IMG_DEPTH_F32,
    IMG_DEPTH_D64,
    
    IMG_DEPTH_INVALID               = -1
}ImgDepth;


typedef enum _ImgPlaneIdx
{
    IMG_PLANE_0                     = 0,
    IMG_PLANE_1,
    IMG_PLANE_2,
    IMG_PLANE_3,
    
    IMG_PLANE_GRAY                  = IMG_PLANE_0,
    
    IMG_PLANE_RGB_R                 = IMG_PLANE_0,
    IMG_PLANE_RGB_G,
    IMG_PLANE_RGB_B,
    IMG_PLANE_RGB_A,
    
    IMG_PLANE_BGR_B                 = IMG_PLANE_0,
    IMG_PLANE_BGR_G,
    IMG_PLANE_BGR_R,
    IMG_PLANE_BGR_A,
    
    IMG_PLANE_YUV_Y                 = IMG_PLANE_0,
    IMG_PLANE_YUV_U,
    IMG_PLANE_YUV_V,
    IMG_PLANE_YUV_UV                = IMG_PLANE_YUV_U,
    IMG_PLANE_YUV_VU                = IMG_PLANE_YUV_U,

    IMG_PLANE_HSV_H                 = IMG_PLANE_0,
    IMG_PLANE_HSV_S,
    IMG_PLANE_HSV_V,

    IMG_PLANE_HSL_H                 = IMG_PLANE_0,
    IMG_PLANE_HSL_S,
    IMG_PLANE_HSL_L,
}ImgPlaneIdx;


typedef enum _BinaryMapType
{
    IMG_BINARYMAP_MIN               = 0,
    IMG_BINARYMAP_EDGE              = IMG_BINARYMAP_MIN,
    IMG_BINARYMAP_SKIN,
    IMG_BINARYMAP_BACKGND,
    IMG_BINARYMAP_HISTO_BP,
    IMG_BINARYMAP_MAX,
}BinaryMapType;


typedef enum _ImgSizeType
{
    IMG_SIZE_MIN                    = 0,
    IMG_SIZE_QCIF,
    IMG_SIZE_QVGA,
    IMG_SIZE_CIF,
    IMG_SIZE_VGA,
    IMG_SIZE_480P,
    IMG_SIZE_540P,
    IMG_SIZE_720P,
    IMG_SIZE_1080P,
    IMG_SIZE_MAX,
}ImgSizeType;


typedef enum _ImgHistCompMode
{
    IMG_HISTCOMP_CORREL             = 0,
    IMG_HISTCOMP_INTERSECTION,
    IMG_HISTCOMP_CHISQR,
    IMG_HISTCOMP_BHATTACHARYYA,
}ImgHistCompMode;


typedef enum _ImgBPMode
{
    IMG_BP_NORMAL                   = 0,
    IMG_BP_RATIO,
}ImgBPMode;


typedef enum _BlockType
{
    BLOCKTYPE_INVALID               = 0,
    BLOCKTYPE_4x4,
    BLOCKTYPE_8x4,
    BLOCKTYPE_4x8,
    BLOCKTYPE_8x8,
    BLOCKTYPE_16x8,
    BLOCKTYPE_8x16,
    BLOCKTYPE_16x16,
    BLOCKTYPE_MAX,
}BlockType;


typedef enum _Kakao_Status
{
    KAKAO_STAT_FAIL                 = -1,
    KAKAO_STAT_OK                   = 1,

    KAKAO_STAT_INVALID_PARAM        = -1000,
    KAKAO_STAT_NULL_POINTER         = -1001,
    KAKAO_STAT_ALLOC_FAIL           = -1002,
    
    KAKAO_STAT_READY                = KAKAO_STAT_OK,
    KAKAO_STAT_NOT_READY            = -1003,    
}Kakao_Status;

#endif
