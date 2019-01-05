
#ifndef __KAKAO_IMGPROC_PRE_PROCESSOR_ROUTINE_H__
#define __KAKAO_IMGPROC_PRE_PROCESSOR_ROUTINE_H__


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <float.h>
#include <limits.h>
#include <time.h>
#include <ctype.h>

#include "Kakao_ImgProc_Platform.h"
#include "Kakao_ImgProc_typedefs.h"
#include "Kakao_ImgProc_Misc.h"
#include "Kakao_ImgProc_Define.h"
#include "Kakao_ImgProc_Profile.h"



#if ENABLE_ISET(CPU_ARMX)
    #if ENABLE_OS(OS_IOS)
        //
    #elif ENABLE_OS(OS_ANDROID)
        //
    #else
        //
    #endif
#elif ENABLE_ISET(CPU_INTEL_IA32) | ENABLE_ISET(CPU_INTEL_IA64)
    #if ENABLE_OS(OS_WIN32) | ENABLE_OS(OS_WIN64)
        //
    #elif ENABLE_OS(OS_OSX)
        #include <sys/types.h>
        #include <sys/sysctl.h>
    #else
        //
    #endif
#else
    //
#endif

#endif