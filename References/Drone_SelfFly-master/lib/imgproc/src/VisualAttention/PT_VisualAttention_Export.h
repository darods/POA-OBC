#ifndef     _PT_VISUALATTENTION__
#define     _PT_VISUALATTENTION__


#ifdef __cplusplus
extern "C" {
#endif

#ifdef _PT_API_EXPORT_
    #define API_EXPORT          __declspec(dllexport)
#else
    #define API_EXPORT
#endif

#define     IN
#define     OUT

typedef void *HANDLE;


#define     PT_OK           1
#define     PT_FAIL        0



typedef struct _PT_VisualAttention_Struct
{
    unsigned char           *pIn;
    unsigned char           *pI_FeatureMap;
    unsigned char           *pC_FeatureMap;
    unsigned char           *pO_FeatureMap;
    unsigned char           *pS_FeatureMap;
    int                            nWidth;
    int                            nHeight;
}PT_VisualAttention_Struct;




API_EXPORT void PT_Init_VisualAttention(HANDLE *pHandle, PT_VisualAttention_Struct *pVisialAttension);
API_EXPORT unsigned char * PT_VisualAttention(HANDLE pHandle, PT_VisualAttention_Struct *pVisialAttension);
API_EXPORT void PT_Destroy_VisualAttention(HANDLE pHandle, PT_VisualAttention_Struct *pVisialAttension);

#ifdef __cplusplus
}
#endif


#endif
