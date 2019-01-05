


/*----------------------------------------------------------------------------------------
 File Inclusions
 ----------------------------------------------------------------------------------------*/
#include "Kakao_ImgProc_Modules.h"


/*----------------------------------------------------------------------------------------
 Constant Definitions
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Macro Definitions
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Type Definitions
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Static Function
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Static Variable
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Global Variable
 ----------------------------------------------------------------------------------------*/
#if MODULE_PROFILE
    Profiler     p_ColorCvt;
    Profiler     p_Edge_Sobel;
    Profiler     p_Edge_Canny;
    Profiler     p_Edge_Laplacian;
    Profiler     p_ExtractBG;
    Profiler     p_FilterImg;
    Profiler     p_ImgEnhance;
    Profiler     p_ImgFeature;
    Profiler     p_IQA;
    Profiler     p_BlobLabel;
    Profiler     p_ObjDetect;
    Profiler     p_SBDetect;
    Profiler     p_Scaler_Sub;
    Profiler     p_Scaler_Bil;
    Profiler     p_Scaler_Bic;
    Profiler     p_Scaler_Ext;
    Profiler     p_SkinDetect;
    Profiler     p_VisualAttention;
    Profiler     p_Vid_Encoder;
    Profiler     p_Vid_Decoder;
    Profiler     p_Module0;
    Profiler     p_Module1;
    Profiler     p_Module2;
    Profiler     p_Module3;


    #if ENABLE_ISET(CPU_ARMX)
        #if ENABLE_OS(OS_IOS)
            mach_timebase_info_data_t       time_info;
        #elif ENABLE_OS(OS_ANDROID)
        #else
        #endif
    #elif ENABLE_ISET(CPU_INTEL_IA32) | ENABLE_ISET(CPU_INTEL_IA64)
        #if ENABLE_OS(OS_WIN32) | ENABLE_OS(OS_WIN64)
            LARGE_INTEGER                   time_info;          // higher-resolution timer
        #elif ENABLE_OS(OS_OSX)
            mach_timebase_info_data_t       time_info;
        #else
        #endif
    #endif
#endif


/*----------------------------------------------------------------------------------------
 Function Implementation
 ----------------------------------------------------------------------------------------*/
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Profiler_Create()
{
#if MODULE_PROFILE
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    #if ENABLE_ISET(CPU_ARMX | ISET_NEON)
        #if ENABLE_OS(OS_IOS)
            mach_timebase_info(&time_info);
        #elif ENABLE_OS(OS_ANDROID)
        #else
        #endif
    #elif ENABLE_ISET(CPU_INTEL_IA32) | ENABLE_ISET(CPU_INTEL_IA64)
        #if ENABLE_OS(OS_WIN32)
            QueryPerformanceFrequency(&time_info);	//[counter/sec]
        #elif ENABLE_OS(OS_OSX)
            mach_timebase_info(&time_info);
        #else
        #endif
    #else
    #endif

    InitProfiler(ColorCvt);
    InitProfiler(Edge_Sobel);
    InitProfiler(Edge_Canny);
    InitProfiler(Edge_Laplacian);
    InitProfiler(ExtractBG);
    InitProfiler(FilterImg);
    InitProfiler(ImgEnhance);
    InitProfiler(ImgFeature);
    InitProfiler(IQA);
    InitProfiler(BlobLabel);
    InitProfiler(ObjDetect);
    InitProfiler(SBDetect);
    InitProfiler(Scaler_Sub);
    InitProfiler(Scaler_Bil);
    InitProfiler(Scaler_Bic);
    InitProfiler(Scaler_Ext);
    InitProfiler(SkinDetect);
    InitProfiler(VisualAttention);
    InitProfiler(Vid_Encoder);
    InitProfiler(Vid_Decoder);
    InitProfiler(Module0);
    InitProfiler(Module1);
    InitProfiler(Module2);
    InitProfiler(Module3);
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
#else
    return KAKAO_STAT_OK;
#endif
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Profiler_Destroy()
{
#if MODULE_PROFILE
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
#else
    return KAKAO_STAT_OK;
#endif
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Profiler_Report()
{
#if MODULE_PROFILE
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    ReportProfile(ColorCvt);
    ReportProfile(Edge_Sobel);
    ReportProfile(Edge_Canny);
    ReportProfile(Edge_Laplacian);
    ReportProfile(ExtractBG);
    ReportProfile(FilterImg);
    ReportProfile(ImgEnhance);
    ReportProfile(ImgFeature);
    ReportProfile(IQA);
    ReportProfile(BlobLabel);
    ReportProfile(ObjDetect);
    ReportProfile(SBDetect);
    ReportProfile(Scaler_Sub);
    ReportProfile(Scaler_Bil);
    ReportProfile(Scaler_Bic);
    ReportProfile(Scaler_Ext);
    ReportProfile(SkinDetect);
    ReportProfile(VisualAttention);
    ReportProfile(Vid_Encoder);
    ReportProfile(Vid_Decoder);
    ReportProfile(Module0);
    ReportProfile(Module1);
    ReportProfile(Module2);
    ReportProfile(Module3);
    
    printf("\n");
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
#else
    return KAKAO_STAT_OK;
#endif
}
