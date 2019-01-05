//
//  Kakao_ImgProc_Profile.h
//
//  Created by Maverick Park on 2014. 8. 10.
//  Copyright (c) 2015년 Kakao. All rights reserved.
//

#ifndef     __KAKAO_IMGPROC_PROFILER_H__
#define     __KAKAO_IMGPROC_PROFILER_H__

#include "Kakao_ImgProc_Core.h"

#if MODULE_PROFILE
    typedef struct _Profiler{
        D64                 tSelfTime;
        D64                 tSubTime;
        D64                 tCurrTime;
        UINT32              nCalls;
        D64                 tMinTime;
        D64                 tMaxTime;
    }Profiler;

    #if ENABLE_ISET(CPU_ARMX)
        #if ENABLE_OS(OS_IOS)
            #include <mach/mach.h>
            #include <mach/mach_time.h>

            extern mach_timebase_info_data_t time_info;

            #define     ___START_PROFILE(ModuleName)\
            { \
                uint64_t ___hStart##ModuleName; \
                uint64_t ___hEnd##ModuleName; \
                D64 ___hLocalTime; \
                p_##ModuleName.nCalls++;  \
                ___hStart##ModuleName = mach_absolute_time();
            
            #define     ___STOP_PROFILE(ModuleName) \
                ___hEnd##ModuleName = mach_absolute_time();\
                ___hLocalTime = (D64)(___hEnd##ModuleName - ___hStart##ModuleName) * time_info.numer / time_info.denom; \
                p_##ModuleName.tCurrTime = ___hLocalTime; \
                p_##ModuleName.tSelfTime += ___hLocalTime; \
                if((p_##ModuleName.nCalls > 10) && (p_##ModuleName.tMaxTime <= ___hLocalTime)) p_##ModuleName.tMaxTime = ___hLocalTime; \
                if((p_##ModuleName.nCalls > 10) && (p_##ModuleName.tMinTime >= ___hLocalTime)) p_##ModuleName.tMinTime = ___hLocalTime; \
            }

            #define     BASE_CLOCK_UNIT_MS              (1000000)
        #elif ENABLE_OS(OS_ANDROID)
            #include "stdio.h"
            #include "time.h"
            #include <android/log.h>

            #define     ___START_PROFILE(ModuleName) \
            { \
                clock_t ___hStart##ModuleName; \
                clock_t ___hEnd##ModuleName; \
                D64 ___hLocalTime; \
                p_##ModuleName.nCalls++;  \
                ___hStart##ModuleName = clock();
                
            #define     ___STOP_PROFILE(ModuleName) \
                ___hEnd##ModuleName = clock(); \
                ___hLocalTime = (D64)(___hEnd##ModuleName - ___hStart##ModuleName); \
                p_##ModuleName.tCurrTime = ___hLocalTime; \
                p_##ModuleName.tSelfTime += ___hLocalTime; \
                if((p_##ModuleName.nCalls > 10) && (p_##ModuleName.tMaxTime <= ___hLocalTime)) p_##ModuleName.tMaxTime = ___hLocalTime; \
                if((p_##ModuleName.nCalls > 10) && (p_##ModuleName.tMinTime >= ___hLocalTime)) p_##ModuleName.tMinTime = ___hLocalTime; \
            }

            #define     BASE_CLOCK_UNIT_MS              (CLOCKS_PER_SEC / 1000)
        #else
            #define     ___START_PROFILE(ModuleName)
            #define     ___STOP_PROFILE(ModuleName)
            #define     BASE_CLOCK_UNIT_MS                  (1)
        #endif
    #elif ENABLE_ISET(CPU_INTEL_IA32) | ENABLE_ISET(CPU_INTEL_IA64)
        #if ENABLE_OS(OS_WIN32) | ENABLE_OS(OS_WIN64)
            #include <windows.h>
            
            extern LARGE_INTEGER                   time_info;    // higher-resolution timer

            #define     ___START_PROFILE(ModuleName) \
            { \
                LARGE_INTEGER ___hStart##ModuleName; \
                LARGE_INTEGER ___hEnd##ModuleName; \
                D64 ___hLocalTime; \
                p_##ModuleName.nCalls++;  \
                QueryPerformanceCounter(&___hStart##ModuleName);
                
            #define     ___STOP_PROFILE(ModuleName) \
                QueryPerformanceCounter(&___hEnd##ModuleName); \
                ___hLocalTime = (D64)(___hEnd##ModuleName.QuadPart - ___hStart##ModuleName.QuadPart)/time_info.QuadPart*1000; \
                p_##ModuleName.tCurrTime = ___hLocalTime; \
                p_##ModuleName.tSelfTime += ___hLocalTime; \
                if((p_##ModuleName.nCalls > 10) && (p_##ModuleName.tMaxTime <= ___hLocalTime)) p_##ModuleName.tMaxTime = ___hLocalTime; \
                if((p_##ModuleName.nCalls > 10) && (p_##ModuleName.tMinTime >= ___hLocalTime)) p_##ModuleName.tMinTime = ___hLocalTime; \
            }

            #define     BASE_CLOCK_UNIT_MS              (1)
        #elif ENABLE_OS(OS_OSX)
            #include <mach/mach.h>
            #include <mach/mach_time.h>

            extern mach_timebase_info_data_t        time_info;

            #define     ___START_PROFILE(ModuleName)\
            { \
                uint64_t ___hStart##ModuleName; \
                uint64_t ___hEnd##ModuleName; \
                D64 ___hLocalTime; \
                p_##ModuleName.nCalls++;  \
                ___hStart##ModuleName = mach_absolute_time();
                
            #define     ___STOP_PROFILE(ModuleName) \
                ___hEnd##ModuleName = mach_absolute_time();\
                ___hLocalTime = (D64)(___hEnd##ModuleName - ___hStart##ModuleName) * time_info.numer / time_info.denom; \
                p_##ModuleName.tCurrTime = ___hLocalTime; \
                p_##ModuleName.tSelfTime += ___hLocalTime; \
                if((p_##ModuleName.nCalls > 10) && (p_##ModuleName.tMaxTime <= ___hLocalTime)) p_##ModuleName.tMaxTime = ___hLocalTime; \
                if((p_##ModuleName.nCalls > 10) && (p_##ModuleName.tMinTime >= ___hLocalTime)) p_##ModuleName.tMinTime = ___hLocalTime; \
            }

            #define     BASE_CLOCK_UNIT_MS              (1000000)
        #else
            #define     ___START_PROFILE(ModuleName)
            #define     ___STOP_PROFILE(ModuleName)
            #define     BASE_CLOCK_UNIT_MS              (1)
        #endif
    #else
        #define     ___START_PROFILE(ModuleName)
        #define     ___STOP_PROFILE(ModuleName)
        #define     BASE_CLOCK_UNIT_MS                  (1)
    #endif

    #define    InitProfiler(ModuleName)\
    {\
        p_##ModuleName.tSelfTime = 0;\
        p_##ModuleName.tSubTime = 0;\
        p_##ModuleName.tCurrTime = 0;\
        p_##ModuleName.nCalls = 0;\
        p_##ModuleName.tMinTime = 9999999999.0;\
        p_##ModuleName.tMaxTime = 0.0;\
    }

    #if ENABLE_ISET(CPU_ARMX)
        #if ENABLE_OS(OS_IOS)
            #define     ReportProfile(ModuleName)\
            {\
                if(0 != p_##ModuleName.nCalls)\
                    printf(#ModuleName"\t\t%.04f (M:%.04f\tm:%.04f)\t\tC:%.04f\t\t%.04f\t\t%07d\n",\
                           p_##ModuleName.tSelfTime/p_##ModuleName.nCalls / BASE_CLOCK_UNIT_MS,\
                           p_##ModuleName.tMaxTime / BASE_CLOCK_UNIT_MS, p_##ModuleName.tMinTime / BASE_CLOCK_UNIT_MS, \
                           p_##ModuleName.tCurrTime / BASE_CLOCK_UNIT_MS,\
                           p_##ModuleName.tSelfTime / BASE_CLOCK_UNIT_MS, p_##ModuleName.nCalls);\
            }
        #elif ENABLE_OS(OS_ANDROID)
            #define     ReportProfile(ModuleName)\
            {\
                if(0 != p_##ModuleName.nCalls)\
                    __android_log_print(ANDROID_LOG_DEBUG, "video", #ModuleName"\t\t%.04f (M:%.04f\tm:%.04f)\t\tC:%.04f\t\t%.04f\t\t%07d\n",\
                           p_##ModuleName.tSelfTime/p_##ModuleName.nCalls / BASE_CLOCK_UNIT_MS,\
                           p_##ModuleName.tMaxTime / BASE_CLOCK_UNIT_MS, p_##ModuleName.tMinTime / BASE_CLOCK_UNIT_MS, \
                           p_##ModuleName.tCurrTime / BASE_CLOCK_UNIT_MS,\
                           p_##ModuleName.tSelfTime / BASE_CLOCK_UNIT_MS, p_##ModuleName.nCalls);\
            }
        #else
            #define     ReportProfile(ModuleName)\
            {\
                if(0 != p_##ModuleName.nCalls)\
                printf(#ModuleName"\t\t%.04f (M:%.04f\tm:%.04f)\t\tC:%.04f\t\t%.04f\t\t%07d\n",\
                       p_##ModuleName.tSelfTime/p_##ModuleName.nCalls / BASE_CLOCK_UNIT_MS,\
                       p_##ModuleName.tMaxTime / BASE_CLOCK_UNIT_MS, p_##ModuleName.tMinTime / BASE_CLOCK_UNIT_MS, \
                       p_##ModuleName.tCurrTime / BASE_CLOCK_UNIT_MS,\
                       p_##ModuleName.tSelfTime / BASE_CLOCK_UNIT_MS, p_##ModuleName.nCalls);\
            }
        #endif
    #elif ENABLE_ISET(CPU_INTEL_IA32) | ENABLE_ISET(CPU_INTEL_IA64)
        #if ENABLE_OS(OS_WIN32) | ENABLE_OS(OS_WIN64)
            #define     ReportProfile(ModuleName)\
            {\
                if(0 != p_##ModuleName.nCalls)\
                    printf(#ModuleName"\t\t%.04f (M:%.04f\tm:%.04f)\t\tC:%.04f\t\t%.04f\t\t%07d\n",\
                           p_##ModuleName.tSelfTime/p_##ModuleName.nCalls / BASE_CLOCK_UNIT_MS,\
                           p_##ModuleName.tMaxTime / BASE_CLOCK_UNIT_MS, p_##ModuleName.tMinTime / BASE_CLOCK_UNIT_MS, \
                           p_##ModuleName.tCurrTime / BASE_CLOCK_UNIT_MS,\
                           p_##ModuleName.tSelfTime / BASE_CLOCK_UNIT_MS, p_##ModuleName.nCalls);\
            }
        #elif ENABLE_OS(OS_OSX)
            #define     ReportProfile(ModuleName)\
            {\
                if(0 != p_##ModuleName.nCalls)\
                    printf(#ModuleName"\t\t%.04f (M:%.04f\tm:%.04f)\t\tC:%.04f\t\t%.04f\t\t%07d\n",\
                           p_##ModuleName.tSelfTime/p_##ModuleName.nCalls / BASE_CLOCK_UNIT_MS,\
                           p_##ModuleName.tMaxTime / BASE_CLOCK_UNIT_MS, p_##ModuleName.tMinTime / BASE_CLOCK_UNIT_MS, \
                           p_##ModuleName.tCurrTime / BASE_CLOCK_UNIT_MS,\
                           p_##ModuleName.tSelfTime / BASE_CLOCK_UNIT_MS, p_##ModuleName.nCalls);\
            }
        #else
            #define     ReportProfile(ModuleName)\
            {\
                if(0 != p_##ModuleName.nCalls)\
                    printf(#ModuleName"\t\t%.04f (M:%.04f\tm:%.04f)\t\tC:%.04f\t\t%.04f\t\t%07d\n",\
                           p_##ModuleName.tSelfTime/p_##ModuleName.nCalls / BASE_CLOCK_UNIT_MS,\
                           p_##ModuleName.tMaxTime / BASE_CLOCK_UNIT_MS, p_##ModuleName.tMinTime / BASE_CLOCK_UNIT_MS, \
                           p_##ModuleName.tCurrTime / BASE_CLOCK_UNIT_MS,\
                           p_##ModuleName.tSelfTime / BASE_CLOCK_UNIT_MS, p_##ModuleName.nCalls);\
            }
        #endif
    #else
        #define     ReportProfile(ModuleName)\
        {\
            if(0 != p_##ModuleName.nCalls)\
                printf(#ModuleName"\t\t%.04f (M:%.04f\tm:%.04f)\t\tC:%.04f\t\t%.04f\t\t%07d\n",\
                       p_##ModuleName.tSelfTime/p_##ModuleName.nCalls / BASE_CLOCK_UNIT_MS,\
                       p_##ModuleName.tMaxTime / BASE_CLOCK_UNIT_MS, p_##ModuleName.tMinTime / BASE_CLOCK_UNIT_MS, \
                       p_##ModuleName.tCurrTime / BASE_CLOCK_UNIT_MS,\
                       p_##ModuleName.tSelfTime / BASE_CLOCK_UNIT_MS, p_##ModuleName.nCalls);\
        }
    #endif

    extern Profiler     p_ColorCvt;
    extern Profiler     p_Edge_Sobel;
    extern Profiler     p_Edge_Canny;
    extern Profiler     p_Edge_Laplacian;
    extern Profiler     p_ExtractBG;
    extern Profiler     p_FilterImg;
    extern Profiler     p_ImgEnhance;
    extern Profiler     p_ImgFeature;
    extern Profiler     p_IQA;
    extern Profiler     p_BlobLabel;
    extern Profiler     p_ObjDetect;
    extern Profiler     p_SBDetect;
    extern Profiler     p_Scaler_Sub;
    extern Profiler     p_Scaler_Bil;
    extern Profiler     p_Scaler_Bic;
    extern Profiler     p_Scaler_Ext;
    extern Profiler     p_SkinDetect;
    extern Profiler     p_VisualAttention;
    extern Profiler     p_Vid_Encoder;
    extern Profiler     p_Vid_Decoder;
    extern Profiler     p_Module0;
    extern Profiler     p_Module1;
    extern Profiler     p_Module2;
    extern Profiler     p_Module3;


#else
    #define     InitProfiler(ModuleName)
    #define     ___START_PROFILE(ModuleName)
    #define     ___STOP_PROFILE(ModuleName)
    #define     ReportProfile(ModuleName)
#endif



KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Profiler_Create();
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Profiler_Destroy();
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Profiler_Report();


#endif

