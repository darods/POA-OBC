//
//  Kakao_ImgProc_Platform.h
//
//  Created by Maverick Park on 2014. 8. 10.
//  Copyright (c) 2015년 Kakao. All rights reserved.
//

#ifndef __KAKAO_IMGPROC_PLATFORM_H__
#define __KAKAO_IMGPROC_PLATFORM_H__


//--------------------------------------------------------------
// 0. default 
//--------------------------------------------------------------
#define CPU_NONE            (0x00000000)    // unknown CPU venter
#define SUB_NONE            (0x00000000)    // unknown cpu sub group
#define OS_NONE             (0x00000000)    // unknown OS/Compiler
#define ISET_NONE           (0x00000000)    // only c
// when  default setting is applied,
 


//--------------------------------------------------------------
// 1. cpu group (vender)
//--------------------------------------------------------------
#define CPU_INTEL_IA32      (0x01000000)
#define CPU_INTEL_IA64      (0x02000000)
#define CPU_AMD             (0x03000000)
#define CPU_ARMX            (0x10000000)
// define here to add the other CPUs


//--------------------------------------------------------------
// 2. cpu sub group
//--------------------------------------------------------------
#define INTEL_P4            (0x00010000)
#define INTEL_P4DUO         (0x00020000)
#define INTEL_P4QUAD        (0x00030000)
#define ARM_ARMX            (0x00100000)
#define ARM_ARM11           (0x00200000)
#define ARM_CORTEX          (0x00300000)


//--------------------------------------------------------------
// 3. OS (or Compiler)
//--------------------------------------------------------------
#define OS_WIN32            (0x00000100)
#define OS_WIN64            (0x00000200)
#define OS_OSX              (0x00000300)
#define OS_ANDROID          (0x00000400)
#define OS_IOS              (0x00000500)


//--------------------------------------------------------------
// 4. ISET (Instruction Set) ID
//--------------------------------------------------------------
#define ISET_MMX            ((0x00000001) | CPU_INTEL_IA32)
#define ISET_SSE            ((0x00000002) | CPU_INTEL_IA32)
#define ISET_SSE2           ((0x00000003) | CPU_INTEL_IA32)
#define ISET_SSE3           ((0x00000004) | CPU_INTEL_IA32)
#define ISET_SSE4           ((0x00000005) | CPU_INTEL_IA32)
#define ISET_ARM            ((0x00000010) | CPU_ARMX)
#define ISET_WMMX           ((0x00000020) | CPU_ARMX)
#define ISET_NEON           ((0x00000030) | CPU_ARMX)

    
//--------------------------------------------------------------
// MACRO Functions
//--------------------------------------------------------------
#define GET_CPU(ID)         ((ID) & 0xFF000000)
#define GET_SUB(ID)         ((ID) & 0x00FF0000)
#define GET_OS(ID)          ((ID) & 0x0000FF00)
#define GET_ISET(ID)        ((ID) & 0x000000FF)
#define GET_ISET_EX(ID)     ((ID) & 0x00FFFFFF)

#define ENABLE_ISET(ID)     ((GET_CPU(ID) == GET_CPU(PLATFORM_ID)) && (GET_ISET(ID) <= GET_ISET(ISET_ID)))
#define ENABLE_CPU(ID)      (GET_CPU(ID) == GET_CPU(PLATFORM_ID))
#define ENABLE_OS(ID)       (GET_OS(ID) == GET_OS(PLATFORM_ID))
#define ENABLE_SUB(ID)      (GET_SUB(ID) == GET_SUB(PLATFORM_ID))



//--------------------------------------------------------------
// PLATFORM_ID
//--------------------------------------------------------------
#define MAKE_PLATFORM_ID(CPU, SUB, OS, ISET) \
                            (((CPU) & 0xFF000000) | ((SUB) & 0x00FF0000) | \
                              ((OS) & 0x0000FF00) | ((ISET) & 0x000000FF))


#if defined(__SSE4__)
    #define ISET_ID             (ISET_SSE4)
#elif defined(__SSE3__)
    #define ISET_ID             (ISET_SSE3)
#elif defined(__SSE2__)
    #define ISET_ID             (ISET_SSE2)
#elif defined(__SSE__)
    #define ISET_ID             (ISET_SSE)
#elif defined(__MMX__)
    #define ISET_ID             (ISET_MMX)
#elif defined(__WMMX__)
    #define ISET_ID             (ISET_WMMX)
#elif defined(__ARM__)
    #if defined(__ARM_NEON__)
        #define ISET_ID         (ISET_NEON)
    #else
        #define ISET_ID         (ISET_NONE)
    #endif
#else
    #define ISET_ID             (ISET_NONE)
#endif



#if defined(__IA32__)
    #if defined(__OSX__)
        #define PLATFORM_ID         MAKE_PLATFORM_ID(CPU_INTEL_IA32, SUB_NONE, OS_OSX, ISET_ID)
    #elif defined(__WIN__)
        #define PLATFORM_ID         MAKE_PLATFORM_ID(CPU_INTEL_IA32, SUB_NONE, OS_WIN32, ISET_ID)
    #else
        #define PLATFORM_ID         MAKE_PLATFORM_ID(CPU_INTEL_IA32, SUB_NONE, OS_NONE, ISET_ID)
    #endif
#elif defined(__IA64__)
    #if defined(__OSX__)
        #define PLATFORM_ID         MAKE_PLATFORM_ID(CPU_INTEL_IA64, SUB_NONE, OS_OSX, ISET_ID)
    #elif defined(__WIN__)
        #define PLATFORM_ID         MAKE_PLATFORM_ID(CPU_INTEL_IA64, SUB_NONE, OS_WIN64, ISET_ID)
    #else
        #define PLATFORM_ID         MAKE_PLATFORM_ID(CPU_INTEL_IA64, SUB_NONE, OS_NONE, ISET_ID)
    #endif
#elif defined(__ARM__)
    #if defined(__WINCE__)
        #define PLATFORM_ID         MAKE_PLATFORM_ID(CPU_ARMX, ARM_ARMX, OS_WINCE, ISET_ID)
    #elif defined(__IOS__)
        #define PLATFORM_ID         MAKE_PLATFORM_ID(CPU_ARMX, ARM_CORTEX, OS_IOS, ISET_ID)
    #elif defined(__ANDROID__)
        #define PLATFORM_ID         MAKE_PLATFORM_ID(CPU_ARMX, ARM_CORTEX, OS_ANDROID, ISET_ID)
    #else
        #define PLATFORM_ID         MAKE_PLATFORM_ID(CPU_ARMX, ARM_ARMX, OS_NONE, ISET_ID)
    #endif
#else
    #define PLATFORM_ID             MAKE_PLATFORM_ID(CPU_NONE, SUB_NONE, OS_NONE, ISET_ID)
#endif



#if ENABLE_ISET(CPU_ARMX | ISET_NEON)
    #define MAKE_ACCEL_FUNC(FUNC_NAME)      FUNC_NAME##_NEON
#elif ENABLE_ISET(ISET_SSE4)
    #define MAKE_ACCEL_FUNC(FUNC_NAME)      FUNC_NAME##_SSE4
#elif ENABLE_ISET(ISET_SSE3)
    #define MAKE_ACCEL_FUNC(FUNC_NAME)      FUNC_NAME##_SSE3
#elif ENABLE_ISET(ISET_SSE2)
    #define MAKE_ACCEL_FUNC(FUNC_NAME)      FUNC_NAME##_SSE2
#elif ENABLE_ISET(ISET_SSE)
    #define MAKE_ACCEL_FUNC(FUNC_NAME)      FUNC_NAME##_SSE
#elif ENABLE_ISET(ISET_MMX)
    #define MAKE_ACCEL_FUNC(FUNC_NAME)      FUNC_NAME##_MMX
#elif ENABLE_ISET(ISET_NONE)
    #define MAKE_ACCEL_FUNC(FUNC_NAME)      FUNC_NAME##_C
#else
    #define MAKE_ACCEL_FUNC(FUNC_NAME)      FUNC_NAME##_C
#endif


#endif



