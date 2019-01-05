//
//  Kakao_ImgProc_Define.h
//
//  Created by Maverick Park on 2014. 8. 7.
//  Copyright (c) 2015년 Kakao Corp. All rights reserved.
//

#ifndef __KAKAO_IMGPROC_PRE_DEFINE_H__
#define __KAKAO_IMGPROC_PRE_DEFINE_H__

#include        "Kakao_ImgProc_Misc.h"


#ifdef __cplusplus
    #ifdef _USRDLL
        #define KAKAO_EXPORT_API extern "C" __declspec(dllexport)
    #else
        #define KAKAO_EXPORT_API extern "C"
    #endif
#else
    #ifdef _USRDLL
        #define KAKAO_EXPORT_API extern __declspec(dllexport)
    #else
        #define KAKAO_EXPORT_API extern //__declspec(dllimport)
    #endif
#endif


#define KAKAO_IMGPROC_ON                (1)
#define KAKAO_IMGPROC_OFF               (0)

#define USE_OPENCV_INTERFACE            KAKAO_IMGPROC_OFF
#define DO_VISUALATTEND                 KAKAO_IMGPROC_OFF
#define USE_IPP_FUNC                    KAKAO_IMGPROC_OFF
#define USE_SIMD_INTRINSIC              KAKAO_IMGPROC_ON
#define MODULE_PROFILE                  KAKAO_IMGPROC_OFF
#define NORMAL_IMG_ENHANCE              KAKAO_IMGPROC_OFF
#if ENABLE_ISET(CPU_INTEL_IA32) | ENABLE_ISET(CPU_INTEL_IA64)
    #define USE_MULTI_THREAD            KAKAO_IMGPROC_OFF
#else
    #define USE_MULTI_THREAD            KAKAO_IMGPROC_OFF
#endif


#define IN
#define OUT

#define IMGPROC_TRUE                    (1)
#define IMGPROC_FALSE                   (0)

#define MACRO_BLOCK_SIZE                (16)
#define PADDING_SIZE                    (MACRO_BLOCK_SIZE)

#define PIXEL_MIN                       (0)
#define PIXEL_MAX                       (255)

#define FLOATFACTOR                     (12)
#define FLOATPRECISION                  (1 << FLOATFACTOR)
#define FLOATMASK                       (FLOATPRECISION - 1)

#define BIN                             (16)

#define PI                              ((F32)(3.1415926535f))
#define LnE                             ((F32)(2.718f))

#ifndef DBL_MAX
    //#define DBL_MAX                     (1.7976931348623157e+308)
#endif

#ifndef DBL_MIN
    //#define DBL_MIN                     (2.2250738585072014e-308)
#endif

#define WIDTH_QCIF                      (176)
#define HEIGHT_QCIF                     (144)
#define SIZE_QCIF                       (WIDTH_QCIF * HEIGHT_QCIF)
#define WIDTH_QCIF_PADDED               (192)
#define HEIGHT_QCIF_PADDED              (144)
#define WIDTH_QVGA                      (320)
#define HEIGHT_QVGA                     (240)
#define SIZE_QVGA                       (WIDTH_QVGA * HEIGHT_QVGA)
#define WIDTH_CIF                       (352)
#define HEIGHT_CIF                      (288)
#define SIZE_CIF                        (WIDTH_CIF * HEIGHT_CIF)
#define WIDTH_VGA                       (640)
#define HEIGHT_VGA                      (480)
#define SIZE_VGA                        (WIDTH_VGA * HEIGHT_VGA)
#define WIDTH_480P                      (720)
#define HEIGHT_480P                     (480)
#define SIZE_480P                       (WIDTH_480P * HEIGHT_480P)
#define WIDTH_540P                      (960)
#define HEIGHT_540P                     (540)
#define SIZE_540P                       (WIDTH_540P * HEIGHT_540P)
#define WIDTH_720P                      (1280)
#define HEIGHT_720P                     (720)
#define SIZE_720P                       (WIDTH_720P * HEIGHT_720P)
#define WIDTH_1080P                     (1920)
#define HEIGHT_1080P                    (1080)
#define SIZE_1080P                       (WIDTH_1080P * HEIGHT_1080P)

#define INDICATE_VALUE                  (1)

#if !defined(WIN32) && !defined(__WIN__)
    #ifndef HAVE_UNISTD_H
        #define HAVE_UNISTD_H           (1)
    #endif
#else
    #ifndef HAVE_UNISTD_H
        #define HAVE_UNISTD_H               (0)
    #endif
#endif

#endif
