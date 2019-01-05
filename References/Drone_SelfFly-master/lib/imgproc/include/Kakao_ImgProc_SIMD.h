//
//  Kakao_ImgProc_SIMD.h
//
//  Created by Maverick Park on 2014. 10. 6.
//  Copyright (c) 2015년 Kakao Corp. All rights reserved.
//

#ifndef __KAKAO_IMGPROC_SIMD_H__
#define __KAKAO_IMGPROC_SIMD_H__

#if USE_SIMD_INTRINSIC
    #if ENABLE_ISET(CPU_ARMX | ISET_NEON)
        #include <arm_neon.h>
        #include "Kakao_ImgProc_NEON.h"
        #include "Kakao_ImgProc_C_Opt.h"
    #elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_SSE4) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_SSE4)\
        | ENABLE_ISET(CPU_INTEL_IA32 | ISET_SSE3) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_SSE3)\
        | ENABLE_ISET(CPU_INTEL_IA32 | ISET_SSE2) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_SSE2)\
        | ENABLE_ISET(CPU_INTEL_IA32 | ISET_SSE) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_SSE)\
        | ENABLE_ISET(CPU_INTEL_IA32 | ISET_MMX) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_MMX)\
        | ENABLE_ISET(CPU_INTEL_IA32 | ISET_NONE) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_NONE)
        #include <mmintrin.h>           // for MMX
        #include <xmmintrin.h>          // for SSE
        #include <emmintrin.h>          // for SSE2
        //#include <pmmintrin.h>        // for SSE3
        //#include <tmmintrin.h>        // for SSSE3
        //#include <smmintrin.h>        // for SSE4.1
        //#include <nmmintrin.h>        // for SSE4.2
        //#include <immintrin.h>        // for AVX & AVX2
        //#include <ia32intrin.h>
        #if USE_IPP_FUNC
            #include "ipp.h"
            #include "ippi.h"
        #endif
        #include "Kakao_ImgProc_SSE.h"
        #include "Kakao_ImgProc_SSE2.h"
        #include "Kakao_ImgProc_SSE3.h"
        #include "Kakao_ImgProc_SSE4.h"
        #include "Kakao_ImgProc_C_Opt.h"
    #else
        #include "Kakao_ImgProc_C_Opt.h"
    #endif
#endif




#ifdef __cplusplus
extern "C" {
#endif


#define OneRow_Copy                         MAKE_ACCEL_FUNC(OneRow_Copy)
#define OneRow_InterleaveUV                 MAKE_ACCEL_FUNC(OneRow_InterleaveUV)
#define OneRow_Subsample_Even               MAKE_ACCEL_FUNC(OneRow_Subsample_Even)
#define OneRow_Subsample_Odd                MAKE_ACCEL_FUNC(OneRow_Subsample_Odd)
#define OneRow_Subsample_1_4                MAKE_ACCEL_FUNC(OneRow_Subsample_1_4)
#define OneRow_ScaleBilinear                MAKE_ACCEL_FUNC(OneRow_ScaleBilinear)
#define OneCol_ScaleBilinear                MAKE_ACCEL_FUNC(OneCol_ScaleBilinear)

#define OneRow_MeanBlur                     MAKE_ACCEL_FUNC(OneRow_MeanBlur)
#define OneRow_MeanBlur_Hor                 MAKE_ACCEL_FUNC(OneRow_MeanBlur_Hor)
#define OneRow_MeanBlur_Ver                 MAKE_ACCEL_FUNC(OneRow_MeanBlur_Ver)
#define OneRow_Inverse                      MAKE_ACCEL_FUNC(OneRow_Inverse)
#define OneRow_Mosaic4                      MAKE_ACCEL_FUNC(OneRow_Mosaic4)
#define OneRow_Mosaic8                      MAKE_ACCEL_FUNC(OneRow_Mosaic8)
#define OneRow_Mosaic16                     MAKE_ACCEL_FUNC(OneRow_Mosaic16)
#define OneRow_Clip                         MAKE_ACCEL_FUNC(OneRow_Clip)
#define OneRow_DiffImg                      MAKE_ACCEL_FUNC(OneRow_DiffImg)
#define OneRow_Multiply                     MAKE_ACCEL_FUNC(OneRow_Multiply)
#define OneRow_PropMix                      MAKE_ACCEL_FUNC(OneRow_PropMix)
#define OneRow_ContrastShift                MAKE_ACCEL_FUNC(OneRow_ContrastShift)
#define OneRow_BlurNoiseReduction           MAKE_ACCEL_FUNC(OneRow_BlurNoiseReduction)

#define Mean_MxN                            MAKE_ACCEL_FUNC(Mean_MxN)
#define Variance_MxN                        MAKE_ACCEL_FUNC(Variance_MxN)
#define DiffVariance_MxN                    MAKE_ACCEL_FUNC(DiffVariance_MxN)
    
#ifdef __cplusplus
}
#endif


#endif
