


/*----------------------------------------------------------------------------------------
 File Inclusions
 ----------------------------------------------------------------------------------------*/
#include "Kakao_ImgProc_Modules.h"
#include "Kakao_ImgProc_SIMD.h"


/*----------------------------------------------------------------------------------------
 Constant Definitions
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Macro Definitions
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Type Definitions
 ----------------------------------------------------------------------------------------*/
typedef Kakao_Status (* fpColorTool_Cvt)(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);


/*----------------------------------------------------------------------------------------
 Static Function
 ----------------------------------------------------------------------------------------*/
static Kakao_Status _ColorTool_CvtRGB24toYUV420(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
static Kakao_Status _ColorTool_CvtRGBA32toYUV420(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
static Kakao_Status _ColorTool_CvtBGR24toYUV420(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
static Kakao_Status _ColorTool_CvtBGRA32toYUV420(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
static Kakao_Status _ColorTool_CvtYUV420toRGB24(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
static Kakao_Status _ColorTool_CvtYUV420toRGBA32(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
static Kakao_Status _ColorTool_CvtYUV420toBGR24(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
static Kakao_Status _ColorTool_CvtYUV420toBGRA32(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
static Kakao_Status _ColorTool_CvtRGB24toHSV(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
static Kakao_Status _ColorTool_CvtRGBA32toHSV(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
static Kakao_Status _ColorTool_CvtBGR24toHSV(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
static Kakao_Status _ColorTool_CvtBGRA32toHSV(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
static Kakao_Status _ColorTool_CvtHSVtoRGB24(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
static Kakao_Status _ColorTool_CvtHSVtoBGR24(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
static Kakao_Status _ColorTool_CvtHSVtoRGBA32(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
static Kakao_Status _ColorTool_CvtHSVtoBGRA32(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
static Kakao_Status _ColorTool_CopyImg(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);

#if ENABLE_ISET(CPU_ARMX | ISET_NEON)
    static Kakao_Status _ColorTool_CvtRGB24toYUV420_OneRowYUV_NEON(IN const UINT8 * __restrict pSrc,
                                                                  OUT UINT8 * __restrict pDstY, OUT UINT8 * __restrict pDstU,
                                                                  OUT UINT8 * __restrict pDstV,IN INT32 nWidth);
    static Kakao_Status _ColorTool_CvtRGB24toYUV420_OneRowY_NEON(IN const UINT8 * __restrict pSrc, IN UINT8 * __restrict pDst, IN INT32 nWidth);
    static Kakao_Status _ColorTool_CvtNV12toYUV420_NEON(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
    static Kakao_Status _ColorTool_CvtNV21toYUV420_NEON(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_SSE4) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_SSE4)
    static Kakao_Status _ColorTool_CvtRGB24toYUV420_OneRowYUV_SSE4(IN const UINT8 * __restrict pSrc,
                                                                   OUT UINT8 * __restrict pDstY, OUT UINT8 * __restrict pDstU,
                                                                   OUT UINT8 * __restrict pDstV,IN INT32 nWidth);
    static Kakao_Status _ColorTool_CvtRGB24toYUV420_OneRowY_SSE4(IN const UINT8 * __restrict pSrc, IN UINT8 * __restrict pDst, IN INT32 nWidth);
    static Kakao_Status _ColorTool_CvtNV12toYUV420_SSE4(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
    static Kakao_Status _ColorTool_CvtNV21toYUV420_SSE4(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_SSE3) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_SSE3)
    static Kakao_Status _ColorTool_CvtRGB24toYUV420_OneRowYUV_SSE3(IN const UINT8 * __restrict pSrc,
                                                                   OUT UINT8 * __restrict pDstY, OUT UINT8 * __restrict pDstU,
                                                                   OUT UINT8 * __restrict pDstV,IN INT32 nWidth);
    static Kakao_Status _ColorTool_CvtRGB24toYUV420_OneRowY_SSE3(IN const UINT8 * __restrict pSrc, IN UINT8 * __restrict pDst, IN INT32 nWidth);
    static Kakao_Status _ColorTool_CvtNV12toYUV420_SSE3(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
    static Kakao_Status _ColorTool_CvtNV21toYUV420_SSE3(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_SSE2) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_SSE2)
    static Kakao_Status _ColorTool_CvtRGB24toYUV420_OneRowYUV_SSE2(IN const UINT8 * __restrict pSrc,
                                                                   OUT UINT8 * __restrict pDstY, OUT UINT8 * __restrict pDstU,
                                                                   OUT UINT8 * __restrict pDstV,IN INT32 nWidth);
    static Kakao_Status _ColorTool_CvtRGB24toYUV420_OneRowY_SSE2(IN const UINT8 * __restrict pSrc, IN UINT8 * __restrict pDst, IN INT32 nWidth);
    static Kakao_Status _ColorTool_CvtNV12toYUV420_SSE2(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
    static Kakao_Status _ColorTool_CvtNV21toYUV420_SSE2(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_SSE) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_SSE)
    static Kakao_Status _ColorTool_CvtRGB24toYUV420_OneRowYUV_SSE(IN const UINT8 * __restrict pSrc,
                                                                   OUT UINT8 * __restrict pDstY, OUT UINT8 * __restrict pDstU,
                                                                   OUT UINT8 * __restrict pDstV,IN INT32 nWidth);
    static Kakao_Status _ColorTool_CvtRGB24toYUV420_OneRowY_SSE(IN const UINT8 * __restrict pSrc, IN UINT8 * __restrict pDst, IN INT32 nWidth);
    static Kakao_Status _ColorTool_CvtNV12toYUV420_SSE(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
    static Kakao_Status _ColorTool_CvtNV21toYUV420_SSE(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_NONE) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_NONE)
    static Kakao_Status _ColorTool_CvtRGB24toYUV420_OneRowYUV_C(IN const UINT8 * __restrict pSrc,
                                                                  OUT UINT8 * __restrict pDstY, OUT UINT8 * __restrict pDstU,
                                                                  OUT UINT8 * __restrict pDstV,IN INT32 nWidth);
    static Kakao_Status _ColorTool_CvtRGB24toYUV420_OneRowY_C(IN const UINT8 * __restrict pSrc, IN UINT8 * __restrict pDst, IN INT32 nWidth);
    static Kakao_Status _ColorTool_CvtNV12toYUV420_C(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
    static Kakao_Status _ColorTool_CvtNV21toYUV420_C(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
#else
    static Kakao_Status _ColorTool_CvtRGB24toYUV420_OneRowYUV_C(IN const UINT8 * __restrict pSrc,
                                                                OUT UINT8 * __restrict pDstY, OUT UINT8 * __restrict pDstU,
                                                                OUT UINT8 * __restrict pDstV,IN INT32 nWidth);
    static Kakao_Status _ColorTool_CvtRGB24toYUV420_OneRowY_C(IN const UINT8 * __restrict pSrc, IN UINT8 * __restrict pDst, IN INT32 nWidth);
    static Kakao_Status _ColorTool_CvtNV12toYUV420_C(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
    static Kakao_Status _ColorTool_CvtNV21toYUV420_C(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
#endif



#define _ColorTool_CvtRGB24toYUV420_OneRowYUV       MAKE_ACCEL_FUNC(_ColorTool_CvtRGB24toYUV420_OneRowYUV)
#define _ColorTool_CvtRGB24toYUV420_OneRowY         MAKE_ACCEL_FUNC(_ColorTool_CvtRGB24toYUV420_OneRowY)
#define _ColorTool_CvtNV12toYUV420                  MAKE_ACCEL_FUNC(_ColorTool_CvtNV12toYUV420)
#define _ColorTool_CvtNV21toYUV420                  MAKE_ACCEL_FUNC(_ColorTool_CvtNV21toYUV420)


/*----------------------------------------------------------------------------------------
 Static Variable
 ----------------------------------------------------------------------------------------*/
static fpColorTool_Cvt nColorTool_CvtArr[IMG_FORMAT_MAX][IMG_FORMAT_MAX] =
{
   /* IN Gray   */ {_ColorTool_CopyImg, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
   /* IN RGB24  */ {NULL, _ColorTool_CopyImg, NULL, NULL, NULL, _ColorTool_CvtRGB24toYUV420, NULL, _ColorTool_CvtRGB24toHSV, NULL, NULL, NULL, NULL},
   /* IN BGR24  */ {NULL, NULL, _ColorTool_CopyImg, NULL, NULL, _ColorTool_CvtBGR24toYUV420, NULL, _ColorTool_CvtBGR24toHSV, NULL, NULL, NULL, NULL},
   /* IN RGBA32 */ {NULL, NULL, NULL, _ColorTool_CopyImg, NULL, _ColorTool_CvtRGBA32toYUV420, NULL, _ColorTool_CvtRGBA32toHSV, NULL, NULL, NULL, NULL},
   /* IN BGRA32 */ {NULL, NULL, NULL, NULL, _ColorTool_CopyImg, _ColorTool_CvtBGRA32toYUV420, NULL, _ColorTool_CvtBGRA32toHSV, NULL, NULL, NULL, NULL},
   /* IN I420   */ {NULL, _ColorTool_CvtYUV420toRGB24, _ColorTool_CvtYUV420toBGR24, _ColorTool_CvtYUV420toRGBA32, _ColorTool_CvtYUV420toBGRA32, _ColorTool_CopyImg, NULL, NULL, NULL, NULL, NULL, NULL},
   /* IN I422   */ {NULL, NULL, NULL, NULL, NULL, NULL, _ColorTool_CopyImg, NULL, NULL, NULL, NULL, NULL},
   /* IN HSV    */ {NULL, _ColorTool_CvtHSVtoRGB24, _ColorTool_CvtHSVtoBGR24, _ColorTool_CvtHSVtoRGBA32, _ColorTool_CvtHSVtoBGRA32, NULL, NULL, _ColorTool_CopyImg, NULL, NULL, NULL, NULL},
   /* IN HSL    */ {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, _ColorTool_CopyImg, NULL, NULL, NULL},
   /* IN NV12   */ {NULL, NULL, NULL, NULL, NULL, _ColorTool_CvtNV12toYUV420, NULL, NULL, NULL, _ColorTool_CopyImg, NULL, NULL},
   /* IN NV21   */ {NULL, NULL, NULL, NULL, NULL, _ColorTool_CvtNV21toYUV420, NULL, NULL, NULL, NULL, _ColorTool_CopyImg, NULL},
   /* IN UYVY   */ {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, _ColorTool_CopyImg},
};


/*----------------------------------------------------------------------------------------
 Global Variable
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Function Implementation
 ----------------------------------------------------------------------------------------*/
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_ColorTool_Create(IN OUT ColorTool_T **ppColorTool, IN const INT32 nWidth, IN const INT32 nHeight)
{
    ColorTool_T             *pTmpColorTool = NULL;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   nAlignedWidth = GET_ALIGNED_LENGTH(nWidth, MACRO_BLOCK_SIZE);
    
    SAFEALLOC(pTmpColorTool, 1, 32, ColorTool_T);

    Kakao_ImgProc_Util_UpdateSize(&pTmpColorTool->nBaseSize, nWidth, nHeight, NULL, IMG_FORMAT_MAX, 4);
    
    *ppColorTool = pTmpColorTool;
    
    nRet = KAKAO_STAT_OK;
    return nRet;

Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_ColorTool_Destroy(IN OUT ColorTool_T **ppColorTool)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    SAFEFREE((*ppColorTool));
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_ColorTool_UpdateImgSize(IN OUT ColorTool_T *pColorTool, IN const INT32 nWidth, IN const INT32 nHeight)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pColorTool, KAKAO_STAT_NULL_POINTER);
    
    Kakao_ImgProc_Util_UpdateSize(&(pColorTool->nBaseSize), nWidth, nHeight, NULL, IMG_FORMAT_MAX, 4);
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_ColorTool_Cvt(IN ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   nSrcImgFormatIdx = 0;
    INT32                   nDstImgFormatIdx = 0;
    
    CHECK_POINTER_VALIDATION(pColorTool, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER)

    ___START_PROFILE(ColorCvt)

    Kakao_ImgProc_Util_Get_ImgFormatIdx(pSrcImg->GetImgFormat(pSrcImg), &nSrcImgFormatIdx);
    Kakao_ImgProc_Util_Get_ImgFormatIdx(pDstImg->GetImgFormat(pDstImg), &nDstImgFormatIdx);
    
    if(NULL != nColorTool_CvtArr[nSrcImgFormatIdx][nDstImgFormatIdx])
    {
        nRet = nColorTool_CvtArr[nSrcImgFormatIdx][nDstImgFormatIdx](pColorTool, pSrcImg, pDstImg);
    }
    else
        nRet = KAKAO_STAT_FAIL;

    ___STOP_PROFILE(ColorCvt)

    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_ColorTool_ExtCvtToI420(IN ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg,
                                                                   IN const INT32 nCropX, IN const INT32 nCropY,
                                                                   IN const INT32 nCropWidth, IN const INT32 nCropHeight,
                                                                   IN const INT32 nSrcWidth, IN const INT32 nSrcHeight,
                                                                   IN const RotateDegree nDegree, IN const ImgFormat nFourCC)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   nRetVal = -1;
    
    CHECK_POINTER_VALIDATION(pColorTool, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER)
    
    ___START_PROFILE(ColorCvt)
    
    nRetVal = ConvertToI420((const UINT8 *)(pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0)),
                            (nSrcWidth * nSrcHeight),
                            (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0), pDstImg->GetImgStride(pDstImg, IMG_PLANE_0),
                            (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_1), pDstImg->GetImgStride(pDstImg, IMG_PLANE_1),
                            (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_2), pDstImg->GetImgStride(pDstImg, IMG_PLANE_2),
                            nCropX, nCropY,
                            nSrcWidth, nSrcHeight,
                            nCropWidth, nCropHeight,
                            (90 * nDegree),
                            (UINT32)nFourCC);
    
    ___STOP_PROFILE(ColorCvt)
    
    if(-1 == nRetVal)
        return nRet;

    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _ColorTool_CvtRGB24toYUV420(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    INT32                   i = 0, j = 0;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nSrcStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    INT32                   nDstStrideY = pDstImg->GetImgStride(pDstImg, IMG_PLANE_0);
    INT32                   nDstStrideUV = pDstImg->GetImgStride(pDstImg, IMG_PLANE_1);
    INT32                   nSrcPixPos = 0;
    UINT8                   *pSrcR = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
    UINT8                   *pSrcG = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_1);
    UINT8                   *pSrcB = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_2);
    UINT8                   *pDstY = NULL;
    UINT8                   *pDstU = NULL;
    UINT8                   *pDstV = NULL;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    for(j=0 ; j<nHeight ; j+=2)
    {
        nSrcPixPos = (j * nSrcStride);
        pDstY = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0) + (j * nDstStrideY);
        pDstU = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_1) + ((j / 2) * nDstStrideUV);
        pDstV = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_2) + ((j / 2) * nDstStrideUV);
        for(i=0 ; i<nWidth ; i+=2)
        {            
            nSrcPixPos += 3;
            *(pDstY++) = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)( (0.257 * (D64)pSrcR[nSrcPixPos]) + (0.504 * (D64)pSrcG[nSrcPixPos]) + (0.098 * (D64)pSrcB[nSrcPixPos]) + 16));
            *(pDstU++) = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)(-(0.148 * (D64)pSrcR[nSrcPixPos]) - (0.291 * (D64)pSrcG[nSrcPixPos]) + (0.439 * (D64)pSrcB[nSrcPixPos]) + 128));
            *(pDstV++) = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)( (0.439 * (D64)pSrcR[nSrcPixPos]) - (0.368 * (D64)pSrcG[nSrcPixPos]) - (0.071 * (D64)pSrcB[nSrcPixPos]) + 128));

            nSrcPixPos += 3;
            *(pDstY++) = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)( (0.257 * (D64)pSrcR[nSrcPixPos]) + (0.504 * (D64)pSrcG[nSrcPixPos]) + (0.098 * (D64)pSrcB[nSrcPixPos]) + 16));
        }
        
        nSrcPixPos = ((j + 1) * nSrcStride);
        pDstY = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0) + ((j + 1) * nDstStrideY);
        for(i=0 ; i<nWidth ; i+=2)
        {
            nSrcPixPos += 3;
            *(pDstY++) = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)( (0.257 * (D64)pSrcR[nSrcPixPos]) + (0.504 * (D64)pSrcG[nSrcPixPos]) + (0.098 * (D64)pSrcB[nSrcPixPos]) + 16));
            nSrcPixPos += 3;
            *(pDstY++) = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)( (0.257 * (D64)pSrcR[nSrcPixPos]) + (0.504 * (D64)pSrcG[nSrcPixPos]) + (0.098 * (D64)pSrcB[nSrcPixPos]) + 16));
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _ColorTool_CvtRGBA32toYUV420(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    INT32                   i = 0, j = 0;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nSrcStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    INT32                   nDstStrideY = pDstImg->GetImgStride(pDstImg, IMG_PLANE_0);
    INT32                   nDstStrideUV = pDstImg->GetImgStride(pDstImg, IMG_PLANE_1);
    INT32                   nSrcPixPos = 0;
    UINT8                   *pSrcR = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
    UINT8                   *pSrcG = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_1);
    UINT8                   *pSrcB = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_2);
    UINT8                   *pDstY = NULL;
    UINT8                   *pDstU = NULL;
    UINT8                   *pDstV = NULL;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    for(j=0 ; j<nHeight ; j+=2)
    {
        nSrcPixPos = (j * nSrcStride);
        pDstY = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0) + (j * nDstStrideY);
        pDstU = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_1) + ((j / 2) * nDstStrideUV);
        pDstV = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_2) + ((j / 2) * nDstStrideUV);
        for(i=0 ; i<nWidth ; i+=2)
        {
            nSrcPixPos += 4;
            *(pDstY++) = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)( (0.257 * (D64)pSrcR[nSrcPixPos]) + (0.504 * (D64)pSrcG[nSrcPixPos]) + (0.098 * (D64)pSrcB[nSrcPixPos]) + 16));
            *(pDstU++) = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)(-(0.148 * (D64)pSrcR[nSrcPixPos]) - (0.291 * (D64)pSrcG[nSrcPixPos]) + (0.439 * (D64)pSrcB[nSrcPixPos]) + 128));
            *(pDstV++) = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)( (0.439 * (D64)pSrcR[nSrcPixPos]) - (0.368 * (D64)pSrcG[nSrcPixPos]) - (0.071 * (D64)pSrcB[nSrcPixPos]) + 128));
            
            nSrcPixPos += 4;
            *(pDstY++) = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)( (0.257 * (D64)pSrcR[nSrcPixPos]) + (0.504 * (D64)pSrcG[nSrcPixPos]) + (0.098 * (D64)pSrcB[nSrcPixPos]) + 16));
        }
        
        nSrcPixPos = ((j + 1) * nSrcStride);
        pDstY = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0) + ((j + 1) * nDstStrideY);
        for(i=0 ; i<nWidth ; i+=2)
        {
            nSrcPixPos += 4;
            *(pDstY++) = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)( (0.257 * (D64)pSrcR[nSrcPixPos]) + (0.504 * (D64)pSrcG[nSrcPixPos]) + (0.098 * (D64)pSrcB[nSrcPixPos]) + 16));
            nSrcPixPos += 4;
            *(pDstY++) = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)( (0.257 * (D64)pSrcR[nSrcPixPos]) + (0.504 * (D64)pSrcG[nSrcPixPos]) + (0.098 * (D64)pSrcB[nSrcPixPos]) + 16));
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _ColorTool_CvtBGR24toYUV420(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    INT32                   i = 0, j = 0;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nSrcStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    INT32                   nDstStrideY = pDstImg->GetImgStride(pDstImg, IMG_PLANE_0);
    INT32                   nDstStrideUV = pDstImg->GetImgStride(pDstImg, IMG_PLANE_1);
    INT32                   nSrcPixPos = 0;
    UINT8                   *pSrcB = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
    UINT8                   *pSrcG = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_1);
    UINT8                   *pSrcR = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_2);
    UINT8                   *pDstY = NULL;
    UINT8                   *pDstU = NULL;
    UINT8                   *pDstV = NULL;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    for(j=0 ; j<nHeight ; j+=2)
    {
        nSrcPixPos = (j * nSrcStride);
        pDstY = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0) + (j * nDstStrideY);
        pDstU = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_1) + ((j / 2) * nDstStrideUV);
        pDstV = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_2) + ((j / 2) * nDstStrideUV);
        for(i=0 ; i<nWidth ; i+=2)
        {
            nSrcPixPos += 3;
            *(pDstY++) = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)( (0.257 * (D64)pSrcR[nSrcPixPos]) + (0.504 * (D64)pSrcG[nSrcPixPos]) + (0.098 * (D64)pSrcB[nSrcPixPos]) + 16));
            *(pDstU++) = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)(-(0.148 * (D64)pSrcR[nSrcPixPos]) - (0.291 * (D64)pSrcG[nSrcPixPos]) + (0.439 * (D64)pSrcB[nSrcPixPos]) + 128));
            *(pDstV++) = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)( (0.439 * (D64)pSrcR[nSrcPixPos]) - (0.368 * (D64)pSrcG[nSrcPixPos]) - (0.071 * (D64)pSrcB[nSrcPixPos]) + 128));
            
            nSrcPixPos += 3;
            *(pDstY++) = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)( (0.257 * (D64)pSrcR[nSrcPixPos]) + (0.504 * (D64)pSrcG[nSrcPixPos]) + (0.098 * (D64)pSrcB[nSrcPixPos]) + 16));
        }
        
        nSrcPixPos = ((j + 1) * nSrcStride);
        pDstY = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0) + ((j + 1) * nDstStrideY);
        for(i=0 ; i<nWidth ; i+=2)
        {
            nSrcPixPos += 3;
            *(pDstY++) = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)( (0.257 * (D64)pSrcR[nSrcPixPos]) + (0.504 * (D64)pSrcG[nSrcPixPos]) + (0.098 * (D64)pSrcB[nSrcPixPos]) + 16));
            nSrcPixPos += 3;
            *(pDstY++) = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)( (0.257 * (D64)pSrcR[nSrcPixPos]) + (0.504 * (D64)pSrcG[nSrcPixPos]) + (0.098 * (D64)pSrcB[nSrcPixPos]) + 16));
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _ColorTool_CvtBGRA32toYUV420(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    INT32                   i = 0, j = 0;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nSrcStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    INT32                   nDstStrideY = pDstImg->GetImgStride(pDstImg, IMG_PLANE_0);
    INT32                   nDstStrideUV = pDstImg->GetImgStride(pDstImg, IMG_PLANE_1);
    INT32                   nSrcPixPos = 0;
    UINT8                   *pSrcB = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
    UINT8                   *pSrcG = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_1);
    UINT8                   *pSrcR = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_2);
    UINT8                   *pDstY = NULL;
    UINT8                   *pDstU = NULL;
    UINT8                   *pDstV = NULL;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    for(j=0 ; j<nHeight ; j+=2)
    {
        nSrcPixPos = (j * nSrcStride);
        pDstY = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0) + (j * nDstStrideY);
        pDstU = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_1) + ((j / 2) * nDstStrideUV);
        pDstV = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_2) + ((j / 2) * nDstStrideUV);
        for(i=0 ; i<nWidth ; i+=2)
        {
            nSrcPixPos += 4;
            *(pDstY++) = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)( (0.257 * (D64)pSrcR[nSrcPixPos]) + (0.504 * (D64)pSrcG[nSrcPixPos]) + (0.098 * (D64)pSrcB[nSrcPixPos]) + 16));
            *(pDstU++) = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)(-(0.148 * (D64)pSrcR[nSrcPixPos]) - (0.291 * (D64)pSrcG[nSrcPixPos]) + (0.439 * (D64)pSrcB[nSrcPixPos]) + 128));
            *(pDstV++) = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)( (0.439 * (D64)pSrcR[nSrcPixPos]) - (0.368 * (D64)pSrcG[nSrcPixPos]) - (0.071 * (D64)pSrcB[nSrcPixPos]) + 128));
            
            nSrcPixPos += 4;
            *(pDstY++) = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)( (0.257 * (D64)pSrcR[nSrcPixPos]) + (0.504 * (D64)pSrcG[nSrcPixPos]) + (0.098 * (D64)pSrcB[nSrcPixPos]) + 16));
        }
        
        nSrcPixPos = ((j + 1) * nSrcStride);
        pDstY = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0) + ((j + 1) * nDstStrideY);
        for(i=0 ; i<nWidth ; i+=2)
        {
            nSrcPixPos += 4;
            *(pDstY++) = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)( (0.257 * (D64)pSrcR[nSrcPixPos]) + (0.504 * (D64)pSrcG[nSrcPixPos]) + (0.098 * (D64)pSrcB[nSrcPixPos]) + 16));
            nSrcPixPos += 4;
            *(pDstY++) = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)( (0.257 * (D64)pSrcR[nSrcPixPos]) + (0.504 * (D64)pSrcG[nSrcPixPos]) + (0.098 * (D64)pSrcB[nSrcPixPos]) + 16));
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _ColorTool_CvtYUV420toRGB24(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    INT32                   i = 0, j = 0;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nSrcStrideY = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    INT32                   nSrcStrideUV = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_1);
    INT32                   nDstStride = pDstImg->GetImgStride(pDstImg, IMG_PLANE_0);
    INT32                   nDstPixPos = 0;
    UINT8                   *pSrcY = NULL;
    UINT8                   *pSrcU = NULL;
    UINT8                   *pSrcV = NULL;
    UINT8                   *pSrcNextY = 0;
    UINT8                   *pDstR = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0);
    UINT8                   *pDstG = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_1);
    UINT8                   *pDstB = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_2);
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    for(j=0 ; j<nHeight ; j+=2)
    {
        nDstPixPos = (j * nDstStride);
        pSrcY = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0) + (j * nSrcStrideY);
        pSrcU = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_1) + ((j / 2) * nSrcStrideUV);
        pSrcV = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_2) + ((j / 2) * nSrcStrideUV);
        pSrcNextY = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0) + ((j + 1) * nSrcStrideY);
        for(i=0 ; i<nWidth ; i+=2)
        {
            pDstR[nDstPixPos] = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)(1.164*(D64)(*(pSrcY) - 16) + 1.596*(D64)(*(pSrcV) - 128)));               // R
            pDstG[nDstPixPos] = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)(1.164*(D64)(*(pSrcY) - 16) - 0.391*(D64)(*(pSrcU) - 128) - 0.813*(D64)(*(pSrcV) - 128)));
            pDstB[nDstPixPos] = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)(1.164*(D64)(*(pSrcY) - 16) + 2.018*(D64)(*(pSrcU) - 128)));              // B
            pDstR[nDstPixPos + nDstStride] = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)(1.164*(D64)(*(pSrcNextY) - 16) + 1.596*(D64)(*(pSrcV) - 128)));               // R
            pDstG[nDstPixPos + nDstStride] = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)(1.164*(D64)(*(pSrcNextY) - 16) - 0.391*(D64)(*(pSrcU) - 128) - 0.813*(D64)(*(pSrcV) - 128)));
            pDstB[nDstPixPos + nDstStride] = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)(1.164*(D64)(*(pSrcNextY) - 16) + 2.018*(D64)(*(pSrcU) - 128)));              // B
            nDstPixPos += 3;
            pSrcY++;
            pSrcNextY++;
            
            pDstR[nDstPixPos] = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)(1.164*(D64)(*(pSrcY) - 16) + 1.596*(D64)(*(pSrcV) - 128)));               // R
            pDstG[nDstPixPos] = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)(1.164*(D64)(*(pSrcY) - 16) - 0.391*(D64)(*(pSrcU) - 128) - 0.813*(D64)(*(pSrcV) - 128)));
            pDstB[nDstPixPos] = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)(1.164*(D64)(*(pSrcY) - 16) + 2.018*(D64)(*(pSrcU) - 128)));              // B
            pDstR[nDstPixPos + nDstStride] = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)(1.164*(D64)(*(pSrcNextY) - 16) + 1.596*(D64)(*(pSrcV) - 128)));               // R
            pDstG[nDstPixPos + nDstStride] = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)(1.164*(D64)(*(pSrcNextY) - 16) - 0.391*(D64)(*(pSrcU) - 128) - 0.813*(D64)(*(pSrcV) - 128)));
            pDstB[nDstPixPos + nDstStride] = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)(1.164*(D64)(*(pSrcNextY) - 16) + 2.018*(D64)(*(pSrcU) - 128)));              // B
            nDstPixPos += 3;
            pSrcY++;
            pSrcU++;
            pSrcV++;
            pSrcNextY++;
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _ColorTool_CvtYUV420toRGBA32(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    INT32                   i = 0, j = 0;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nSrcStrideY = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    INT32                   nSrcStrideUV = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_1);
    INT32                   nDstStride = pDstImg->GetImgStride(pDstImg, IMG_PLANE_0);
    INT32                   nDstPixPos = 0;
    UINT8                   *pSrcY = NULL;
    UINT8                   *pSrcU = NULL;
    UINT8                   *pSrcV = NULL;
    UINT8                   *pSrcNextY = 0;
    UINT8                   *pDstR = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0);
    UINT8                   *pDstG = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_1);
    UINT8                   *pDstB = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_2);
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    for(j=0 ; j<nHeight ; j+=2)
    {
        nDstPixPos = (j * nDstStride);
        pSrcY = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0) + (j * nSrcStrideY);
        pSrcU = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_1) + ((j / 2) * nSrcStrideUV);
        pSrcV = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_2) + ((j / 2) * nSrcStrideUV);
        pSrcNextY = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0) + ((j + 1) * nSrcStrideY);
        for(i=0 ; i<nWidth ; i+=2)
        {
            pDstR[nDstPixPos] = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)(1.164*(D64)(*(pSrcY) - 16) + 1.596*(D64)(*(pSrcV) - 128)));               // R
            pDstG[nDstPixPos] = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)(1.164*(D64)(*(pSrcY) - 16) - 0.391*(D64)(*(pSrcU) - 128) - 0.813*(D64)(*(pSrcV) - 128)));
            pDstB[nDstPixPos] = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)(1.164*(D64)(*(pSrcY) - 16) + 2.018*(D64)(*(pSrcU) - 128)));              // B
            pDstR[nDstPixPos + nDstStride] = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)(1.164*(D64)(*(pSrcNextY) - 16) + 1.596*(D64)(*(pSrcV) - 128)));               // R
            pDstG[nDstPixPos + nDstStride] = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)(1.164*(D64)(*(pSrcNextY) - 16) - 0.391*(D64)(*(pSrcU) - 128) - 0.813*(D64)(*(pSrcV) - 128)));
            pDstB[nDstPixPos + nDstStride] = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)(1.164*(D64)(*(pSrcNextY) - 16) + 2.018*(D64)(*(pSrcU) - 128)));              // B
            nDstPixPos += 4;
            pSrcY++;
            pSrcNextY++;
            
            pDstR[nDstPixPos] = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)(1.164*(D64)(*(pSrcY) - 16) + 1.596*(D64)(*(pSrcV) - 128)));               // R
            pDstG[nDstPixPos] = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)(1.164*(D64)(*(pSrcY) - 16) - 0.391*(D64)(*(pSrcU) - 128) - 0.813*(D64)(*(pSrcV) - 128)));
            pDstB[nDstPixPos] = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)(1.164*(D64)(*(pSrcY) - 16) + 2.018*(D64)(*(pSrcU) - 128)));              // B
            pDstR[nDstPixPos + nDstStride] = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)(1.164*(D64)(*(pSrcNextY) - 16) + 1.596*(D64)(*(pSrcV) - 128)));               // R
            pDstG[nDstPixPos + nDstStride] = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)(1.164*(D64)(*(pSrcNextY) - 16) - 0.391*(D64)(*(pSrcU) - 128) - 0.813*(D64)(*(pSrcV) - 128)));
            pDstB[nDstPixPos + nDstStride] = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)(1.164*(D64)(*(pSrcNextY) - 16) + 2.018*(D64)(*(pSrcU) - 128)));              // B
            nDstPixPos += 4;
            pSrcY++;
            pSrcU++;
            pSrcV++;
            pSrcNextY++;
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _ColorTool_CvtYUV420toBGR24(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    INT32                   i = 0, j = 0;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nSrcStrideY = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    INT32                   nSrcStrideUV = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_1);
    INT32                   nDstStride = pDstImg->GetImgStride(pDstImg, IMG_PLANE_0);
    INT32                   nDstPixPos = 0;
    UINT8                   *pSrcY = NULL;
    UINT8                   *pSrcU = NULL;
    UINT8                   *pSrcV = NULL;
    UINT8                   *pSrcNextY = 0;
    UINT8                   *pDstB = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0);
    UINT8                   *pDstG = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_1);
    UINT8                   *pDstR = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_2);
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    for(j=0 ; j<nHeight ; j+=2)
    {
        nDstPixPos = (j * nDstStride);
        pSrcY = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0) + (j * nSrcStrideY);
        pSrcU = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_1) + ((j / 2) * nSrcStrideUV);
        pSrcV = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_2) + ((j / 2) * nSrcStrideUV);
        pSrcNextY = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0) + ((j + 1) * nSrcStrideY);
        for(i=0 ; i<nWidth ; i+=2)
        {
            pDstR[nDstPixPos] = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)(1.164*(D64)(*(pSrcY) - 16) + 1.596*(D64)(*(pSrcV) - 128)));               // R
            pDstG[nDstPixPos] = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)(1.164*(D64)(*(pSrcY) - 16) - 0.391*(D64)(*(pSrcU) - 128) - 0.813*(D64)(*(pSrcV) - 128)));
            pDstB[nDstPixPos] = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)(1.164*(D64)(*(pSrcY) - 16) + 2.018*(D64)(*(pSrcU) - 128)));              // B
            pDstR[nDstPixPos + nDstStride] = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)(1.164*(D64)(*(pSrcNextY) - 16) + 1.596*(D64)(*(pSrcV) - 128)));               // R
            pDstG[nDstPixPos + nDstStride] = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)(1.164*(D64)(*(pSrcNextY) - 16) - 0.391*(D64)(*(pSrcU) - 128) - 0.813*(D64)(*(pSrcV) - 128)));
            pDstB[nDstPixPos + nDstStride] = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)(1.164*(D64)(*(pSrcNextY) - 16) + 2.018*(D64)(*(pSrcU) - 128)));              // B
            nDstPixPos += 3;
            pSrcY++;
            pSrcNextY++;
            
            pDstR[nDstPixPos] = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)(1.164*(D64)(*(pSrcY) - 16) + 1.596*(D64)(*(pSrcV) - 128)));               // R
            pDstG[nDstPixPos] = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)(1.164*(D64)(*(pSrcY) - 16) - 0.391*(D64)(*(pSrcU) - 128) - 0.813*(D64)(*(pSrcV) - 128)));
            pDstB[nDstPixPos] = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)(1.164*(D64)(*(pSrcY) - 16) + 2.018*(D64)(*(pSrcU) - 128)));              // B
            pDstR[nDstPixPos + nDstStride] = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)(1.164*(D64)(*(pSrcNextY) - 16) + 1.596*(D64)(*(pSrcV) - 128)));               // R
            pDstG[nDstPixPos + nDstStride] = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)(1.164*(D64)(*(pSrcNextY) - 16) - 0.391*(D64)(*(pSrcU) - 128) - 0.813*(D64)(*(pSrcV) - 128)));
            pDstB[nDstPixPos + nDstStride] = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)(1.164*(D64)(*(pSrcNextY) - 16) + 2.018*(D64)(*(pSrcU) - 128)));              // B
            nDstPixPos += 3;
            pSrcY++;
            pSrcU++;
            pSrcV++;
            pSrcNextY++;
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _ColorTool_CvtYUV420toBGRA32(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    INT32                   i = 0, j = 0;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nSrcStrideY = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    INT32                   nSrcStrideUV = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_1);
    INT32                   nDstStride = pDstImg->GetImgStride(pDstImg, IMG_PLANE_0);
    INT32                   nDstPixPos = 0;
    UINT8                   *pSrcY = NULL;
    UINT8                   *pSrcU = NULL;
    UINT8                   *pSrcV = NULL;
    UINT8                   *pSrcNextY = 0;
    UINT8                   *pDstB = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0);
    UINT8                   *pDstG = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_1);
    UINT8                   *pDstR = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_2);
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    for(j=0 ; j<nHeight ; j+=2)
    {
        nDstPixPos = (j * nDstStride);
        pSrcY = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0) + (j * nSrcStrideY);
        pSrcU = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_1) + ((j / 2) * nSrcStrideUV);
        pSrcV = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_2) + ((j / 2) * nSrcStrideUV);
        pSrcNextY = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0) + ((j + 1) * nSrcStrideY);
        for(i=0 ; i<nWidth ; i+=2)
        {
            pDstR[nDstPixPos] = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)(1.164*(D64)(*(pSrcY) - 16) + 1.596*(D64)(*(pSrcV) - 128)));               // R
            pDstG[nDstPixPos] = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)(1.164*(D64)(*(pSrcY) - 16) - 0.391*(D64)(*(pSrcU) - 128) - 0.813*(D64)(*(pSrcV) - 128)));
            pDstB[nDstPixPos] = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)(1.164*(D64)(*(pSrcY) - 16) + 2.018*(D64)(*(pSrcU) - 128)));              // B
            pDstR[nDstPixPos + nDstStride] = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)(1.164*(D64)(*(pSrcNextY) - 16) + 1.596*(D64)(*(pSrcV) - 128)));               // R
            pDstG[nDstPixPos + nDstStride] = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)(1.164*(D64)(*(pSrcNextY) - 16) - 0.391*(D64)(*(pSrcU) - 128) - 0.813*(D64)(*(pSrcV) - 128)));
            pDstB[nDstPixPos + nDstStride] = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)(1.164*(D64)(*(pSrcNextY) - 16) + 2.018*(D64)(*(pSrcU) - 128)));              // B
            nDstPixPos += 4;
            pSrcY++;
            pSrcNextY++;
            
            pDstR[nDstPixPos] = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)(1.164*(D64)(*(pSrcY) - 16) + 1.596*(D64)(*(pSrcV) - 128)));               // R
            pDstG[nDstPixPos] = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)(1.164*(D64)(*(pSrcY) - 16) - 0.391*(D64)(*(pSrcU) - 128) - 0.813*(D64)(*(pSrcV) - 128)));
            pDstB[nDstPixPos] = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)(1.164*(D64)(*(pSrcY) - 16) + 2.018*(D64)(*(pSrcU) - 128)));              // B
            pDstR[nDstPixPos + nDstStride] = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)(1.164*(D64)(*(pSrcNextY) - 16) + 1.596*(D64)(*(pSrcV) - 128)));               // R
            pDstG[nDstPixPos + nDstStride] = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)(1.164*(D64)(*(pSrcNextY) - 16) - 0.391*(D64)(*(pSrcU) - 128) - 0.813*(D64)(*(pSrcV) - 128)));
            pDstB[nDstPixPos + nDstStride] = CLIP3(PIXEL_MIN, PIXEL_MAX, (INT32)(1.164*(D64)(*(pSrcNextY) - 16) + 2.018*(D64)(*(pSrcU) - 128)));              // B
            nDstPixPos += 4;
            pSrcY++;
            pSrcU++;
            pSrcV++;
            pSrcNextY++;
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}


// RGB  <-->  HSV
// Reference web site
// http://www.cs.rit.edu/~ncs/color/t_convert.html
static Kakao_Status _ColorTool_CvtRGB24toHSV(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    INT32                   i = 0, j = 0;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nSrcStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    INT32                   nDstStride = pDstImg->GetImgStride(pDstImg, IMG_PLANE_0);
    INT32                   nPixPosRGB = 0, nPixPosHSV = 0;
    UINT8                   *pSrcR = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
    UINT8                   *pSrcG = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_1);
    UINT8                   *pSrcB = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_2);
    D64                     *pDstH = (D64 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0);
    D64                     *pDstS = (D64 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_1);
    D64                     *pDstV = (D64 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_2);
    D64                     nR, nG, nB;
    D64                     nMin, nMax, nDelta;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    for(j=0 ; j<nHeight ; j++)
    {
        nPixPosRGB = (j * nSrcStride);
        nPixPosHSV = (j * nDstStride);

        for(i=0 ; i<nWidth ; i++)
        {
            nR = (D64)pSrcR[nPixPosRGB] / 256;
            nG = (D64)pSrcG[nPixPosRGB] / 256;
            nB = (D64)pSrcB[nPixPosRGB] / 256;

            nMin = GET_MIN(GET_MIN(nR, nG), nB);
            nMax = GET_MAX(GET_MAX(nR, nG), nB);

            pDstV[nPixPosHSV] = nMax;
            nDelta = nMax - nMin;
            
            if(nMax != 0)
            {
                pDstS[nPixPosHSV] = nDelta / nMax;

                if(nR == nMax)
                    pDstH[nPixPosHSV] = (nG - nB) / nDelta;         // between yellow & magenta
                else if(nG == nMax)
                    pDstH[nPixPosHSV] = 2 + (nB - nR) / nDelta;     // between cyan & yellow
                else
                    pDstH[nPixPosHSV] = 4 + (nR - nG) / nDelta;     // between magenta & cyan

                pDstH[nPixPosHSV] *= 60;                            // degrees
                    
                if(pDstH[nPixPosHSV] < 0)
                    pDstH[nPixPosHSV] += 360;

                //CLIP3(0, 360, pDstH[nPixPosHSV]);
            }
            else 
            {
                // r = g = b = 0
                // s = 0, v is undefined
                pDstS[nPixPosHSV] = 0;
                pDstH[nPixPosHSV] = 0;
            }

            nPixPosRGB += 3;
            nPixPosHSV++;
        }
    }

    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _ColorTool_CvtRGBA32toHSV(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    INT32                   i = 0, j = 0;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nSrcStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    INT32                   nDstStride = pDstImg->GetImgStride(pDstImg, IMG_PLANE_0);
    INT32                   nPixPosRGB = 0, nPixPosHSV = 0;
    UINT8                   *pSrcR = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
    UINT8                   *pSrcG = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_1);
    UINT8                   *pSrcB = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_2);
    D64                     *pDstH = (D64 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0);
    D64                     *pDstS = (D64 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_1);
    D64                     *pDstV = (D64 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_2);
    D64                     nR, nG, nB;
    D64                     nMin, nMax, nDelta;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    for(j=0 ; j<nHeight ; j++)
    {
        nPixPosRGB = (j * nSrcStride);
        nPixPosHSV = (j * nDstStride);
        
        for(i=0 ; i<nWidth ; i++)
        {
            nR = (D64)pSrcR[nPixPosRGB] / 256;
            nG = (D64)pSrcG[nPixPosRGB] / 256;
            nB = (D64)pSrcB[nPixPosRGB] / 256;
            
            nMin = GET_MIN(GET_MIN(nR, nG), nB);
            nMax = GET_MAX(GET_MAX(nR, nG), nB);
            
            pDstV[nPixPosHSV] = nMax;
            nDelta = nMax - nMin;
            
            if(nMax != 0)
            {
                pDstS[nPixPosHSV] = nDelta / nMax;
                
                if(nR == nMax)
                    pDstH[nPixPosHSV] = (nG - nB) / nDelta;         // between yellow & magenta
                else if(nG == nMax)
                    pDstH[nPixPosHSV] = 2 + (nB - nR) / nDelta;     // between cyan & yellow
                else
                    pDstH[nPixPosHSV] = 4 + (nR - nG) / nDelta;     // between magenta & cyan
                
                pDstH[nPixPosHSV] *= 60;                            // degrees
                
                if(pDstH[nPixPosHSV] < 0)
                    pDstH[nPixPosHSV] += 360;
                
                //CLIP3(0, 360, pDstH[nPixPosHSV]);
            }
            else
            {
                // r = g = b = 0
                // s = 0, v is undefined
                pDstS[nPixPosHSV] = 0;
                pDstH[nPixPosHSV] = 0;
            }
            
            nPixPosRGB += 4;
            nPixPosHSV++;
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _ColorTool_CvtBGR24toHSV(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    INT32                   i = 0, j = 0;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nSrcStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    INT32                   nDstStride = pDstImg->GetImgStride(pDstImg, IMG_PLANE_0);
    INT32                   nPixPosRGB = 0, nPixPosHSV = 0;
    UINT8                   *pSrcB = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
    UINT8                   *pSrcG = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_1);
    UINT8                   *pSrcR = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_2);
    D64                     *pDstH = (D64 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0);
    D64                     *pDstS = (D64 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_1);
    D64                     *pDstV = (D64 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_2);
    D64                     nR, nG, nB;
    D64                     nMin, nMax, nDelta;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    for(j=0 ; j<nHeight ; j++)
    {
        nPixPosRGB = (j * nSrcStride);
        nPixPosHSV = (j * nDstStride);
        
        for(i=0 ; i<nWidth ; i++)
        {
            nR = (D64)pSrcR[nPixPosRGB] / 256;
            nG = (D64)pSrcG[nPixPosRGB] / 256;
            nB = (D64)pSrcB[nPixPosRGB] / 256;
            
            nMin = GET_MIN(GET_MIN(nR, nG), nB);
            nMax = GET_MAX(GET_MAX(nR, nG), nB);
            
            pDstV[nPixPosHSV] = nMax;
            nDelta = nMax - nMin;
            
            if(nMax != 0)
            {
                pDstS[nPixPosHSV] = nDelta / nMax;
                
                if(nR == nMax)
                    pDstH[nPixPosHSV] = (nG - nB) / nDelta;         // between yellow & magenta
                else if(nG == nMax)
                    pDstH[nPixPosHSV] = 2 + (nB - nR) / nDelta;     // between cyan & yellow
                else
                    pDstH[nPixPosHSV] = 4 + (nR - nG) / nDelta;     // between magenta & cyan
                
                pDstH[nPixPosHSV] *= 60;                            // degrees
                
                if(pDstH[nPixPosHSV] < 0)
                    pDstH[nPixPosHSV] += 360;
                
                //CLIP3(0, 360, pDstH[nPixPosHSV]);
            }
            else
            {
                // r = g = b = 0
                // s = 0, v is undefined
                pDstS[nPixPosHSV] = 0;
                pDstH[nPixPosHSV] = 0;
            }
            
            nPixPosRGB += 3;
            nPixPosHSV++;
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _ColorTool_CvtBGRA32toHSV(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    INT32                   i = 0, j = 0;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nSrcStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    INT32                   nDstStride = pDstImg->GetImgStride(pDstImg, IMG_PLANE_0);
    INT32                   nPixPosRGB = 0, nPixPosHSV = 0;
    UINT8                   *pSrcB = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
    UINT8                   *pSrcG = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_1);
    UINT8                   *pSrcR = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_2);
    D64                     *pDstH = (D64 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0);
    D64                     *pDstS = (D64 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_1);
    D64                     *pDstV = (D64 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_2);
    D64                     nR, nG, nB;
    D64                     nMin, nMax, nDelta;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    for(j=0 ; j<nHeight ; j++)
    {
        nPixPosRGB = (j * nSrcStride);
        nPixPosHSV = (j * nDstStride);
        
        for(i=0 ; i<nWidth ; i++)
        {
            nR = (D64)pSrcR[nPixPosRGB] / 256;
            nG = (D64)pSrcG[nPixPosRGB] / 256;
            nB = (D64)pSrcB[nPixPosRGB] / 256;
            
            nMin = GET_MIN(GET_MIN(nR, nG), nB);
            nMax = GET_MAX(GET_MAX(nR, nG), nB);
            
            pDstV[nPixPosHSV] = nMax;
            nDelta = nMax - nMin;
            
            if(nMax != 0)
            {
                pDstS[nPixPosHSV] = nDelta / nMax;
                
                if(nR == nMax)
                    pDstH[nPixPosHSV] = (nG - nB) / nDelta;         // between yellow & magenta
                else if(nG == nMax)
                    pDstH[nPixPosHSV] = 2 + (nB - nR) / nDelta;     // between cyan & yellow
                else
                    pDstH[nPixPosHSV] = 4 + (nR - nG) / nDelta;     // between magenta & cyan
                
                pDstH[nPixPosHSV] *= 60;                            // degrees
                
                if(pDstH[nPixPosHSV] < 0)
                    pDstH[nPixPosHSV] += 360;
                
                //CLIP3(0, 360, pDstH[nPixPosHSV]);
            }
            else
            {
                // r = g = b = 0
                // s = 0, v is undefined
                pDstS[nPixPosHSV] = 0;
                pDstH[nPixPosHSV] = 0;
            }
            
            nPixPosRGB += 4;
            nPixPosHSV++;
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _ColorTool_CvtHSVtoRGB24(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    INT32                   i = 0, j = 0;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nSrcStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    INT32                   nDstStride = pDstImg->GetImgStride(pDstImg, IMG_PLANE_0);
    INT32                   nPixPosRGB = 0, nPixPosHSV = 0;
    D64                     *pSrcH = (D64 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0);
    D64                     *pSrcS = (D64 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_1);
    D64                     *pSrcV = (D64 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_2);
    UINT8                   *pDstR = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
    UINT8                   *pDstG = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_1);
    UINT8                   *pDstB = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_2);
    D64                     nFactor, p, q, t, nSectorH;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    for(j=0 ; j<nHeight ; j++)
    {
        nPixPosRGB = (j * nSrcStride);
        nPixPosHSV = (j * nDstStride);

        for(i=0 ; i<nWidth ; i++)
        {
            if((*pSrcS) != 0)
            {
                nSectorH = pSrcH[nPixPosHSV] / 60;                              // sector 0 to 5
                nFactor = nSectorH - (INT32)(floor(nSectorH));                  // factorial part of h
                
                p = (D64)(pSrcV[nPixPosHSV]) * (1 - (pSrcS[nPixPosHSV]));
                q = (D64)(pSrcV[nPixPosHSV]) * (1 - (pSrcS[nPixPosHSV]) * nFactor);
                t = (D64)(pSrcV[nPixPosHSV]) * (1 - (pSrcS[nPixPosHSV]) * (1 - nFactor));

                switch((INT32)(floor(nSectorH)))
                {
                    case 0:
                        pDstR[nPixPosRGB] = (UINT8)(pSrcV[nPixPosHSV]);
                        pDstG[nPixPosRGB] = (UINT8)t;
                        pDstB[nPixPosRGB] = (UINT8)p;
                        break;
                    case 1:
                        pDstR[nPixPosRGB] = (UINT8)q;
                        pDstG[nPixPosRGB] = (UINT8)(pSrcV[nPixPosHSV]);
                        pDstB[nPixPosRGB] = (UINT8)p;
                        break;
                    case 2:
                        pDstR[nPixPosRGB] = (UINT8)p;
                        pDstG[nPixPosRGB] = (UINT8)(pSrcV[nPixPosHSV]);
                        pDstB[nPixPosRGB] = (UINT8)t;
                        break;
                    case 3:
                        pDstR[nPixPosRGB] = (UINT8)p;
                        pDstG[nPixPosRGB] = (UINT8)q;
                        pDstB[nPixPosRGB] = (UINT8)(pSrcV[nPixPosHSV]);
                        break;
                    case 4:
                        pDstR[nPixPosRGB] = (UINT8)t;
                        pDstG[nPixPosRGB] = (UINT8)p;
                        pDstB[nPixPosRGB] = (UINT8)(pSrcV[nPixPosHSV]);
                        break;
                    default:
                        pDstR[nPixPosRGB] = (UINT8)(pSrcV[nPixPosHSV]);
                        pDstG[nPixPosRGB] = (UINT8)p;
                        pDstB[nPixPosRGB] = (UINT8)q;
                        break;
                }
            }
            else
            {
                pDstR[nPixPosRGB] = pDstG[nPixPosRGB] = pDstB[nPixPosRGB] = (UINT8)(pSrcV[nPixPosHSV]);
            }
            
            nPixPosRGB += 3;
            nPixPosHSV++;
        }
    }  

    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _ColorTool_CvtHSVtoBGR24(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    INT32                   i = 0, j = 0;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nSrcStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    INT32                   nDstStride = pDstImg->GetImgStride(pDstImg, IMG_PLANE_0);
    INT32                   nPixPosBGR = 0, nPixPosHSV = 0;
    D64                     *pSrcH = (D64 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0);
    D64                     *pSrcS = (D64 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_1);
    D64                     *pSrcV = (D64 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_2);
    UINT8                   *pDstB = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
    UINT8                   *pDstG = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_1);
    UINT8                   *pDstR = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_2);
    D64                     nFactor, p, q, t, nSectorH;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    for(j=0 ; j<nHeight ; j++)
    {
        nPixPosBGR = (j * nSrcStride);
        nPixPosHSV = (j * nDstStride);
        
        for(i=0 ; i<nWidth ; i++)
        {
            if((*pSrcS) != 0)
            {
                nSectorH = pSrcH[nPixPosHSV] / 60;                              // sector 0 to 5
                nFactor = nSectorH - (INT32)(floor(nSectorH));                  // factorial part of h
                
                p = (D64)(pSrcV[nPixPosHSV]) * (1 - (pSrcS[nPixPosHSV]));
                q = (D64)(pSrcV[nPixPosHSV]) * (1 - (pSrcS[nPixPosHSV]) * nFactor);
                t = (D64)(pSrcV[nPixPosHSV]) * (1 - (pSrcS[nPixPosHSV]) * (1 - nFactor));
                
                switch((INT32)(floor(nSectorH)))
                {
                    case 0:
                        pDstR[nPixPosBGR] = (UINT8)(pSrcV[nPixPosHSV]);
                        pDstG[nPixPosBGR] = (UINT8)t;
                        pDstB[nPixPosBGR] = (UINT8)p;
                        break;
                    case 1:
                        pDstR[nPixPosBGR] = (UINT8)q;
                        pDstG[nPixPosBGR] = (UINT8)(pSrcV[nPixPosHSV]);
                        pDstB[nPixPosBGR] = (UINT8)p;
                        break;
                    case 2:
                        pDstR[nPixPosBGR] = (UINT8)p;
                        pDstG[nPixPosBGR] = (UINT8)(pSrcV[nPixPosHSV]);
                        pDstB[nPixPosBGR] = (UINT8)t;
                        break;
                    case 3:
                        pDstR[nPixPosBGR] = (UINT8)p;
                        pDstG[nPixPosBGR] = (UINT8)q;
                        pDstB[nPixPosBGR] = (UINT8)(pSrcV[nPixPosHSV]);
                        break;
                    case 4:
                        pDstR[nPixPosBGR] = (UINT8)t;
                        pDstG[nPixPosBGR] = (UINT8)p;
                        pDstB[nPixPosBGR] = (UINT8)(pSrcV[nPixPosHSV]);
                        break;
                    default:
                        pDstR[nPixPosBGR] = (UINT8)(pSrcV[nPixPosHSV]);
                        pDstG[nPixPosBGR] = (UINT8)p;
                        pDstB[nPixPosBGR] = (UINT8)q;
                        break;
                }
            }
            else
            {
                pDstR[nPixPosBGR] = pDstG[nPixPosBGR] = pDstB[nPixPosBGR] = (UINT8)(pSrcV[nPixPosHSV]);
            }
            
            nPixPosBGR += 3;
            nPixPosHSV++;
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _ColorTool_CvtHSVtoRGBA32(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    INT32                   i = 0, j = 0;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nSrcStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    INT32                   nDstStride = pDstImg->GetImgStride(pDstImg, IMG_PLANE_0);
    INT32                   nPixPosRGB = 0, nPixPosHSV = 0;
    D64                     *pSrcH = (D64 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0);
    D64                     *pSrcS = (D64 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_1);
    D64                     *pSrcV = (D64 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_2);
    UINT8                   *pDstR = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
    UINT8                   *pDstG = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_1);
    UINT8                   *pDstB = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_2);
    D64                     nFactor, p, q, t, nSectorH;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    for(j=0 ; j<nHeight ; j++)
    {
        nPixPosRGB = (j * nSrcStride);
        nPixPosHSV = (j * nDstStride);
        
        for(i=0 ; i<nWidth ; i++)
        {
            if((*pSrcS) != 0)
            {
                nSectorH = pSrcH[nPixPosHSV] / 60;                              // sector 0 to 5
                nFactor = nSectorH - (INT32)(floor(nSectorH));                  // factorial part of h
                
                p = (D64)(pSrcV[nPixPosHSV]) * (1 - (pSrcS[nPixPosHSV]));
                q = (D64)(pSrcV[nPixPosHSV]) * (1 - (pSrcS[nPixPosHSV]) * nFactor);
                t = (D64)(pSrcV[nPixPosHSV]) * (1 - (pSrcS[nPixPosHSV]) * (1 - nFactor));
                
                switch((INT32)(floor(nSectorH)))
                {
                    case 0:
                        pDstR[nPixPosRGB] = (UINT8)(pSrcV[nPixPosHSV]);
                        pDstG[nPixPosRGB] = (UINT8)t;
                        pDstB[nPixPosRGB] = (UINT8)p;
                        break;
                    case 1:
                        pDstR[nPixPosRGB] = (UINT8)q;
                        pDstG[nPixPosRGB] = (UINT8)(pSrcV[nPixPosHSV]);
                        pDstB[nPixPosRGB] = (UINT8)p;
                        break;
                    case 2:
                        pDstR[nPixPosRGB] = (UINT8)p;
                        pDstG[nPixPosRGB] = (UINT8)(pSrcV[nPixPosHSV]);
                        pDstB[nPixPosRGB] = (UINT8)t;
                        break;
                    case 3:
                        pDstR[nPixPosRGB] = (UINT8)p;
                        pDstG[nPixPosRGB] = (UINT8)q;
                        pDstB[nPixPosRGB] = (UINT8)(pSrcV[nPixPosHSV]);
                        break;
                    case 4:
                        pDstR[nPixPosRGB] = (UINT8)t;
                        pDstG[nPixPosRGB] = (UINT8)p;
                        pDstB[nPixPosRGB] = (UINT8)(pSrcV[nPixPosHSV]);
                        break;
                    default:
                        pDstR[nPixPosRGB] = (UINT8)(pSrcV[nPixPosHSV]);
                        pDstG[nPixPosRGB] = (UINT8)p;
                        pDstB[nPixPosRGB] = (UINT8)q;
                        break;
                }
            }
            else
            {
                pDstR[nPixPosRGB] = pDstG[nPixPosRGB] = pDstB[nPixPosRGB] = (UINT8)(pSrcV[nPixPosHSV]);
            }
            
            nPixPosRGB += 3;
            nPixPosHSV++;
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _ColorTool_CvtHSVtoBGRA32(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    INT32                   i = 0, j = 0;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nSrcStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    INT32                   nDstStride = pDstImg->GetImgStride(pDstImg, IMG_PLANE_0);
    INT32                   nPixPosBGR = 0, nPixPosHSV = 0;
    D64                     *pSrcH = (D64 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0);
    D64                     *pSrcS = (D64 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_1);
    D64                     *pSrcV = (D64 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_2);
    UINT8                   *pDstB = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
    UINT8                   *pDstG = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_1);
    UINT8                   *pDstR = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_2);
    D64                     nFactor, p, q, t, nSectorH;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    for(j=0 ; j<nHeight ; j++)
    {
        nPixPosBGR = (j * nSrcStride);
        nPixPosHSV = (j * nDstStride);
        
        for(i=0 ; i<nWidth ; i++)
        {
            if((*pSrcS) != 0)
            {
                nSectorH = pSrcH[nPixPosHSV] / 60;                              // sector 0 to 5
                nFactor = nSectorH - (INT32)(floor(nSectorH));                  // factorial part of h
                
                p = (D64)(pSrcV[nPixPosHSV]) * (1 - (pSrcS[nPixPosHSV]));
                q = (D64)(pSrcV[nPixPosHSV]) * (1 - (pSrcS[nPixPosHSV]) * nFactor);
                t = (D64)(pSrcV[nPixPosHSV]) * (1 - (pSrcS[nPixPosHSV]) * (1 - nFactor));
                
                switch((INT32)(floor(nSectorH)))
                {
                    case 0:
                        pDstR[nPixPosBGR] = (UINT8)(pSrcV[nPixPosHSV]);
                        pDstG[nPixPosBGR] = (UINT8)t;
                        pDstB[nPixPosBGR] = (UINT8)p;
                        break;
                    case 1:
                        pDstR[nPixPosBGR] = (UINT8)q;
                        pDstG[nPixPosBGR] = (UINT8)(pSrcV[nPixPosHSV]);
                        pDstB[nPixPosBGR] = (UINT8)p;
                        break;
                    case 2:
                        pDstR[nPixPosBGR] = (UINT8)p;
                        pDstG[nPixPosBGR] = (UINT8)(pSrcV[nPixPosHSV]);
                        pDstB[nPixPosBGR] = (UINT8)t;
                        break;
                    case 3:
                        pDstR[nPixPosBGR] = (UINT8)p;
                        pDstG[nPixPosBGR] = (UINT8)q;
                        pDstB[nPixPosBGR] = (UINT8)(pSrcV[nPixPosHSV]);
                        break;
                    case 4:
                        pDstR[nPixPosBGR] = (UINT8)t;
                        pDstG[nPixPosBGR] = (UINT8)p;
                        pDstB[nPixPosBGR] = (UINT8)(pSrcV[nPixPosHSV]);
                        break;
                    default:
                        pDstR[nPixPosBGR] = (UINT8)(pSrcV[nPixPosHSV]);
                        pDstG[nPixPosBGR] = (UINT8)p;
                        pDstB[nPixPosBGR] = (UINT8)q;
                        break;
                }
            }
            else
            {
                pDstR[nPixPosBGR] = pDstG[nPixPosBGR] = pDstB[nPixPosBGR] = (UINT8)(pSrcV[nPixPosHSV]);
            }
            
            nPixPosBGR += 4;
            nPixPosHSV++;
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}


#if ENABLE_ISET(CPU_ARMX | ISET_NEON)
static Kakao_Status _ColorTool_CvtRGB24toYUV420_OneRowYUV_NEON(IN const UINT8 * __restrict pSrc,
                                                              OUT UINT8 * __restrict pDstY, OUT UINT8 * __restrict pDstU,
                                                              OUT UINT8 * __restrict pDstV,IN INT32 nWidth)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   i = 0;
    uint8x8_t               nFac77 = vdup_n_u8(77);
    uint8x8_t               nFac151 = vdup_n_u8(151);
    uint8x8_t               nFac28 = vdup_n_u8(28);
    uint8x8_t               nFac44 = vdup_n_u8(44);
    uint8x8_t               nFac87 = vdup_n_u8(87);
    uint8x8_t               nFac131 = vdup_n_u8(131);
    uint8x8_t               nFac110 = vdup_n_u8(110);
    uint8x8_t               nFac21 = vdup_n_u8(21);
    uint8x8_t               nFac1 = vdup_n_u8(1);
    uint8x8_t               nFac128 = vdup_n_u8(128);
    
    nWidth /= 16;
    
    for(i=0 ; i<nWidth ; i++)
    {
        uint16x8_t          nTmpVal;
        uint8x8x3_t         nRGBPrev = vld3_u8(pSrc);
        uint8x8x3_t         nRGBNext = vld3_u8(pSrc+24);
        uint8x8x2_t         nYVal;
        uint8x8x2_t         nSubSampledR, nSubSampledG, nSubSampledB;
        uint8x8_t           nUVal, nVVal;
        
        nTmpVal = vmull_u8(nRGBPrev.val[0],      nFac77);
        nTmpVal = vmlal_u8(nTmpVal, nRGBPrev.val[1], nFac151);
        nTmpVal = vmlal_u8(nTmpVal, nRGBPrev.val[2], nFac28);
        nYVal.val[0] = vshrn_n_u16(nTmpVal, 8);

        nTmpVal = vmull_u8(nRGBNext.val[0],      nFac77);
        nTmpVal = vmlal_u8(nTmpVal, nRGBNext.val[1], nFac151);
        nTmpVal = vmlal_u8(nTmpVal, nRGBNext.val[2], nFac28);
        nYVal.val[1] = vshrn_n_u16(nTmpVal, 8);
        vst2_u8(pDstY, nYVal);

        nSubSampledR = vuzp_u8(nRGBPrev.val[0], nRGBNext.val[0]);
        nSubSampledG = vuzp_u8(nRGBPrev.val[1], nRGBNext.val[1]);
        nSubSampledB = vuzp_u8(nRGBPrev.val[2], nRGBNext.val[2]);
        
        nTmpVal = vmull_u8(nSubSampledR.val[0],      nFac131);
        nTmpVal = vmlsl_u8(nTmpVal, nSubSampledG.val[0], nFac44);
        nTmpVal = vmlsl_u8(nTmpVal, nSubSampledB.val[0], nFac87);
        nUVal = vshrn_n_u16(nTmpVal, 8);
        nUVal = vmla_u8(nFac128, nUVal, nFac1);
        vst1_u8(pDstU, nUVal);
        
        nTmpVal = vmull_u8(nSubSampledR.val[0],      nFac131);
        nTmpVal = vmlsl_u8(nTmpVal, nSubSampledG.val[0], nFac110);
        nTmpVal = vmlsl_u8(nTmpVal, nSubSampledB.val[0], nFac21);
        nVVal = vshrn_n_u16(nTmpVal, 8);
        nVVal = vmla_u8(nFac128, nVVal, nFac1);
        vst1_u8(pDstV, nVVal);
        
        pSrc += 48;  // 8 *3
        pDstY += 16;
        pDstU += 8;
        pDstV += 8;
    }

    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _ColorTool_CvtRGB24toYUV420_OneRowY_NEON(IN const UINT8 * __restrict pSrc, IN UINT8 * __restrict pDst, IN INT32 nWidth)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   i = 0;
    uint8x8_t               nFac77 = vdup_n_u8(77);
    uint8x8_t               nFac151 = vdup_n_u8(151);
    uint8x8_t               nFac28 = vdup_n_u8(28);
    
    nWidth /= 16;
    
    for(i=0 ; i<nWidth ; i++)
    {
        uint16x8_t          nTmpVal;
        uint8x8x3_t         nRGBPrev = vld3_u8(pSrc);
        uint8x8x3_t         nRGBNext = vld3_u8(pSrc+24);
        uint8x8x2_t         nYVal;
        
        nTmpVal = vmull_u8(nRGBPrev.val[0],      nFac77);
        nTmpVal = vmlal_u8(nTmpVal, nRGBPrev.val[1], nFac151);
        nTmpVal = vmlal_u8(nTmpVal, nRGBPrev.val[2], nFac28);
        nYVal.val[0] = vshrn_n_u16(nTmpVal, 8);
        
        nTmpVal = vmull_u8(nRGBNext.val[0],      nFac77);
        nTmpVal = vmlal_u8(nTmpVal, nRGBNext.val[1], nFac151);
        nTmpVal = vmlal_u8(nTmpVal, nRGBNext.val[2], nFac28);
        nYVal.val[1] = vshrn_n_u16(nTmpVal, 8);
        vst2_u8(pDst, nYVal);
        
        pSrc += 48;  // 8 *3
        pDst += 16;
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _ColorTool_CvtNV12toYUV420_NEON(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    INT32                   j = 0;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nSrcStrideY = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    INT32                   nDstStrideY = pDstImg->GetImgStride(pDstImg, IMG_PLANE_0);
    INT32                   nDstStrideUV = pDstImg->GetImgStride(pDstImg, IMG_PLANE_1);
    UINT8                   *pSrcY = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
    UINT8                   *pSrcUV = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_1);
    UINT8                   *pDstY = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0);
    UINT8                   *pDstU = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_1);
    UINT8                   *pDstV = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_2);
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    // Copy Y-Plane
    for(j=0 ; j<nHeight ; j++)
    {
        OneRow_Copy(pDstY, (const UINT8 *)pSrcY, nWidth);
        pSrcY += nSrcStrideY;
        pDstY += nDstStrideY;
    }

    // Copy UV-Plane
    nWidth = (nWidth >> 1);
    nHeight = (nHeight >> 1);
    for(j=0 ; j<nHeight ; j++)
    {
        OneRow_InterleaveUV((const UINT8 *)pSrcUV, pDstU, pDstV, nWidth);
        pSrcUV += nSrcStrideY;
        pDstU += nDstStrideUV;
        pDstV += nDstStrideUV;
    }

    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _ColorTool_CvtNV21toYUV420_NEON(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    INT32                   j = 0;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nSrcStrideY = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    INT32                   nDstStrideY = pDstImg->GetImgStride(pDstImg, IMG_PLANE_0);
    INT32                   nDstStrideUV = pDstImg->GetImgStride(pDstImg, IMG_PLANE_1);
    UINT8                   *pSrcY = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
    UINT8                   *pSrcVU = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_1);
    UINT8                   *pDstY = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0);
    UINT8                   *pDstU = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_1);
    UINT8                   *pDstV = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_2);
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    // Copy Y-Plane
    for(j=0 ; j<nHeight ; j++)
    {
        OneRow_Copy(pDstY, (const UINT8 *)pSrcY, nWidth);
        pSrcY += nSrcStrideY;
        pDstY += nDstStrideY;
    }

    // Copy VU-Plane
    nWidth = (nWidth >> 1);
    nHeight = (nHeight >> 1);
    for(j=0 ; j<nHeight ; j++)
    {
        OneRow_InterleaveUV((const UINT8 *)pSrcVU, pDstV, pDstU, nWidth);
        pSrcVU += nSrcStrideY;
        pDstU += nDstStrideUV;
        pDstV += nDstStrideUV;
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_SSE4) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_SSE4)
static Kakao_Status _ColorTool_CvtRGB24toYUV420_OneRowYUV_SSE4(IN const UINT8 * __restrict pSrc,
                                                              OUT UINT8 * __restrict pDstY, OUT UINT8 * __restrict pDstU,
                                                              OUT UINT8 * __restrict pDstV,IN INT32 nWidth)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   i = 0;
    
    for(i=0 ; i<nWidth ; i++)
    {
        INT32              nR = *pSrc++;
        INT32              nG = *pSrc++;
        INT32              nB = *pSrc++;
        
        *pDstY++ = ((77 * nR) + (151 * nG) + (28 * nB)) >> 8;

        if(i & 0x1)
        {
            *pDstU++ = ((((-44) * nR) + ((-87) * nG) + (131 * nB)) >> 8) + 128;
            *pDstV++ = (((131 * nR) + ((-110) * nG) + ((-21) * nB)) >> 8) + 128;
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _ColorTool_CvtRGB24toYUV420_OneRowY_SSE4(IN const UINT8 * __restrict pSrc, IN UINT8 * __restrict pDstY, IN INT32 nWidth)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   i = 0;
    
    for(i=0 ; i<nWidth ; i++)
    {
        UINT32              nR = *pSrc++;
        UINT32              nG = *pSrc++;
        UINT32              nB = *pSrc++;
        
        *pDstY++ = ((nR*77) + (nG*151) + (nB*28)) >> 8;
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _ColorTool_CvtNV12toYUV420_SSE4(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    INT32                   i = 0, j = 0;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nSrcStrideY = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    INT32                   nDstStrideY = pDstImg->GetImgStride(pDstImg, IMG_PLANE_0);
    UINT8                   *pSrcY = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
    UINT8                   *pSrcUV = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_1);
    UINT8                   *pDstY = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0);
    UINT8                   *pDstU = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_1);
    UINT8                   *pDstV = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_2);
    INT32                   nSrcOffset = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_1) - (nWidth >> 1);
    INT32                   nDstOffset = pDstImg->GetImgStride(pDstImg, IMG_PLANE_1) - (nWidth >> 1);
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    // Copy Y-Plane
    for(j=0 ; j<nHeight ; j++)
    {
        MEMCPY(pDstY, pSrcY, nWidth);
        pSrcY += nSrcStrideY;
        pDstY += nDstStrideY;
    }
    
    // Copy UV-Plane
    nWidth = (nWidth >> 1);
    nHeight = (nHeight >> 1);
    for(j=0 ; j<nHeight ; j++)
    {
        for(i=0 ; i<nWidth ; i++)
        {
            *pDstU++ = *pSrcUV++;
            *pDstV++ = *pSrcUV++;
        }
        pSrcUV += nSrcOffset;
        pDstU += nDstOffset;
        pDstV += nDstOffset;
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _ColorTool_CvtNV21toYUV420_SSE4(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    INT32                   i = 0, j = 0;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nSrcStrideY = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    INT32                   nDstStrideY = pDstImg->GetImgStride(pDstImg, IMG_PLANE_0);
    UINT8                   *pSrcY = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
    UINT8                   *pSrcVU = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_1);
    UINT8                   *pDstY = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0);
    UINT8                   *pDstU = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_1);
    UINT8                   *pDstV = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_2);
    INT32                   nSrcOffset = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_1) - (nWidth >> 1);
    INT32                   nDstOffset = pDstImg->GetImgStride(pDstImg, IMG_PLANE_1) - (nWidth >> 1);
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    // Copy Y-Plane
    for(j=0 ; j<nHeight ; j++)
    {
        MEMCPY(pDstY, pSrcY, nWidth);
        pSrcY += nSrcStrideY;
        pDstY += nDstStrideY;
    }
    
    // Copy UV-Plane
    nWidth = (nWidth >> 1);
    nHeight = (nHeight >> 1);
    for(j=0 ; j<nHeight ; j++)
    {
        for(i=0 ; i<nWidth ; i++)
        {
            *pDstU++ = *pSrcVU++;
            *pDstV++ = *pSrcVU++;
        }
        pSrcVU += nSrcOffset;
        pDstU += nDstOffset;
        pDstV += nDstOffset;
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_SSE3) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_SSE3)
static Kakao_Status _ColorTool_CvtRGB24toYUV420_OneRowYUV_SSE3(IN const UINT8 * __restrict pSrc,
                                                               OUT UINT8 * __restrict pDstY, OUT UINT8 * __restrict pDstU,
                                                               OUT UINT8 * __restrict pDstV,IN INT32 nWidth)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   i = 0;
    
    for(i=0 ; i<nWidth ; i++)
    {
        INT32              nR = *pSrc++;
        INT32              nG = *pSrc++;
        INT32              nB = *pSrc++;
        
        *pDstY++ = ((77 * nR) + (151 * nG) + (28 * nB)) >> 8;
        
        if(i & 0x1)
        {
            *pDstU++ = ((((-44) * nR) + ((-87) * nG) + (131 * nB)) >> 8) + 128;
            *pDstV++ = (((131 * nR) + ((-110) * nG) + ((-21) * nB)) >> 8) + 128;
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _ColorTool_CvtRGB24toYUV420_OneRowY_SSE3(IN const UINT8 * __restrict pSrc, IN UINT8 * __restrict pDstY, IN INT32 nWidth)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   i = 0;
    
    for(i=0 ; i<nWidth ; i++)
    {
        UINT32              nR = *pSrc++;
        UINT32              nG = *pSrc++;
        UINT32              nB = *pSrc++;
        
        *pDstY++ = ((nR*77) + (nG*151) + (nB*28)) >> 8;
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _ColorTool_CvtNV12toYUV420_SSE3(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    INT32                   i = 0, j = 0;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nSrcStrideY = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    INT32                   nDstStrideY = pDstImg->GetImgStride(pDstImg, IMG_PLANE_0);
    UINT8                   *pSrcY = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
    UINT8                   *pSrcUV = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_1);
    UINT8                   *pDstY = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0);
    UINT8                   *pDstU = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_1);
    UINT8                   *pDstV = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_2);
    INT32                   nSrcOffset = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_1) - (nWidth >> 1);
    INT32                   nDstOffset = pDstImg->GetImgStride(pDstImg, IMG_PLANE_1) - (nWidth >> 1);
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    // Copy Y-Plane
    for(j=0 ; j<nHeight ; j++)
    {
        MEMCPY(pDstY, pSrcY, nWidth);
        pSrcY += nSrcStrideY;
        pDstY += nDstStrideY;
    }
    
    // Copy UV-Plane
    nWidth = (nWidth >> 1);
    nHeight = (nHeight >> 1);
    for(j=0 ; j<nHeight ; j++)
    {
        for(i=0 ; i<nWidth ; i++)
        {
            *pDstU++ = *pSrcUV++;
            *pDstV++ = *pSrcUV++;
        }
        pSrcUV += nSrcOffset;
        pDstU += nDstOffset;
        pDstV += nDstOffset;
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _ColorTool_CvtNV21toYUV420_SSE3(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    INT32                   i = 0, j = 0;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nSrcStrideY = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    INT32                   nDstStrideY = pDstImg->GetImgStride(pDstImg, IMG_PLANE_0);
    UINT8                   *pSrcY = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
    UINT8                   *pSrcVU = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_1);
    UINT8                   *pDstY = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0);
    UINT8                   *pDstU = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_1);
    UINT8                   *pDstV = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_2);
    INT32                   nSrcOffset = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_1) - (nWidth >> 1);
    INT32                   nDstOffset = pDstImg->GetImgStride(pDstImg, IMG_PLANE_1) - (nWidth >> 1);
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    // Copy Y-Plane
    for(j=0 ; j<nHeight ; j++)
    {
        MEMCPY(pDstY, pSrcY, nWidth);
        pSrcY += nSrcStrideY;
        pDstY += nDstStrideY;
    }
    
    // Copy UV-Plane
    nWidth = (nWidth >> 1);
    nHeight = (nHeight >> 1);
    for(j=0 ; j<nHeight ; j++)
    {
        for(i=0 ; i<nWidth ; i++)
        {
            *pDstU++ = *pSrcVU++;
            *pDstV++ = *pSrcVU++;
        }
        pSrcVU += nSrcOffset;
        pDstU += nDstOffset;
        pDstV += nDstOffset;
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_SSE2) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_SSE2)
static Kakao_Status _ColorTool_CvtRGB24toYUV420_OneRowYUV_SSE2(IN const UINT8 * __restrict pSrc,
                                                               OUT UINT8 * __restrict pDstY, OUT UINT8 * __restrict pDstU,
                                                               OUT UINT8 * __restrict pDstV,IN INT32 nWidth)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   i = 0;
    
    for(i=0 ; i<nWidth ; i++)
    {
        INT32              nR = *pSrc++;
        INT32              nG = *pSrc++;
        INT32              nB = *pSrc++;
        
        *pDstY++ = ((77 * nR) + (151 * nG) + (28 * nB)) >> 8;
        
        if(i & 0x1)
        {
            *pDstU++ = ((((-44) * nR) + ((-87) * nG) + (131 * nB)) >> 8) + 128;
            *pDstV++ = (((131 * nR) + ((-110) * nG) + ((-21) * nB)) >> 8) + 128;
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _ColorTool_CvtRGB24toYUV420_OneRowY_SSE2(IN const UINT8 * __restrict pSrc, IN UINT8 * __restrict pDstY, IN INT32 nWidth)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   i = 0;
    
    for(i=0 ; i<nWidth ; i++)
    {
        UINT32              nR = *pSrc++;
        UINT32              nG = *pSrc++;
        UINT32              nB = *pSrc++;
        
        *pDstY++ = ((nR*77) + (nG*151) + (nB*28)) >> 8;
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _ColorTool_CvtNV12toYUV420_SSE2(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    INT32                   i = 0, j = 0;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nSrcStrideY = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    INT32                   nDstStrideY = pDstImg->GetImgStride(pDstImg, IMG_PLANE_0);
    UINT8                   *pSrcY = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
    UINT8                   *pSrcUV = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_1);
    UINT8                   *pDstY = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0);
    UINT8                   *pDstU = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_1);
    UINT8                   *pDstV = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_2);
    INT32                   nSrcOffset = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_1) - (nWidth >> 1);
    INT32                   nDstOffset = pDstImg->GetImgStride(pDstImg, IMG_PLANE_1) - (nWidth >> 1);
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    // Copy Y-Plane
    for(j=0 ; j<nHeight ; j++)
    {
        MEMCPY(pDstY, pSrcY, nWidth);
        pSrcY += nSrcStrideY;
        pDstY += nDstStrideY;
    }
    
    // Copy UV-Plane
    nWidth = (nWidth >> 1);
    nHeight = (nHeight >> 1);
    for(j=0 ; j<nHeight ; j++)
    {
        for(i=0 ; i<nWidth ; i++)
        {
            *pDstU++ = *pSrcUV++;
            *pDstV++ = *pSrcUV++;
        }
        pSrcUV += nSrcOffset;
        pDstU += nDstOffset;
        pDstV += nDstOffset;
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _ColorTool_CvtNV21toYUV420_SSE2(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    INT32                   i = 0, j = 0;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nSrcStrideY = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    INT32                   nDstStrideY = pDstImg->GetImgStride(pDstImg, IMG_PLANE_0);
    UINT8                   *pSrcY = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
    UINT8                   *pSrcVU = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_1);
    UINT8                   *pDstY = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0);
    UINT8                   *pDstU = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_1);
    UINT8                   *pDstV = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_2);
    INT32                   nSrcOffset = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_1) - (nWidth >> 1);
    INT32                   nDstOffset = pDstImg->GetImgStride(pDstImg, IMG_PLANE_1) - (nWidth >> 1);
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    // Copy Y-Plane
    for(j=0 ; j<nHeight ; j++)
    {
        MEMCPY(pDstY, pSrcY, nWidth);
        pSrcY += nSrcStrideY;
        pDstY += nDstStrideY;
    }
    
    // Copy UV-Plane
    nWidth = (nWidth >> 1);
    nHeight = (nHeight >> 1);
    for(j=0 ; j<nHeight ; j++)
    {
        for(i=0 ; i<nWidth ; i++)
        {
            *pDstU++ = *pSrcVU++;
            *pDstV++ = *pSrcVU++;
        }
        pSrcVU += nSrcOffset;
        pDstU += nDstOffset;
        pDstV += nDstOffset;
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_SSE) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_SSE)
static Kakao_Status _ColorTool_CvtRGB24toYUV420_OneRowYUV_SSE(IN const UINT8 * __restrict pSrc,
                                                               OUT UINT8 * __restrict pDstY, OUT UINT8 * __restrict pDstU,
                                                               OUT UINT8 * __restrict pDstV,IN INT32 nWidth)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   i = 0;
    
    for(i=0 ; i<nWidth ; i++)
    {
        INT32              nR = *pSrc++;
        INT32              nG = *pSrc++;
        INT32              nB = *pSrc++;
        
        *pDstY++ = ((77 * nR) + (151 * nG) + (28 * nB)) >> 8;
        
        if(i & 0x1)
        {
            *pDstU++ = ((((-44) * nR) + ((-87) * nG) + (131 * nB)) >> 8) + 128;
            *pDstV++ = (((131 * nR) + ((-110) * nG) + ((-21) * nB)) >> 8) + 128;
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _ColorTool_CvtRGB24toYUV420_OneRowY_SSE(IN const UINT8 * __restrict pSrc, IN UINT8 * __restrict pDstY, IN INT32 nWidth)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   i = 0;
    
    for(i=0 ; i<nWidth ; i++)
    {
        UINT32              nR = *pSrc++;
        UINT32              nG = *pSrc++;
        UINT32              nB = *pSrc++;
        
        *pDstY++ = ((nR*77) + (nG*151) + (nB*28)) >> 8;
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _ColorTool_CvtNV12toYUV420_SSE(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    INT32                   i = 0, j = 0;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nSrcStrideY = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    INT32                   nDstStrideY = pDstImg->GetImgStride(pDstImg, IMG_PLANE_0);
    UINT8                   *pSrcY = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
    UINT8                   *pSrcUV = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_1);
    UINT8                   *pDstY = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0);
    UINT8                   *pDstU = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_1);
    UINT8                   *pDstV = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_2);
    INT32                   nSrcOffset = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_1) - (nWidth >> 1);
    INT32                   nDstOffset = pDstImg->GetImgStride(pDstImg, IMG_PLANE_1) - (nWidth >> 1);
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    // Copy Y-Plane
    for(j=0 ; j<nHeight ; j++)
    {
        MEMCPY(pDstY, pSrcY, nWidth);
        pSrcY += nSrcStrideY;
        pDstY += nDstStrideY;
    }
    
    // Copy UV-Plane
    nWidth = (nWidth >> 1);
    nHeight = (nHeight >> 1);
    for(j=0 ; j<nHeight ; j++)
    {
        for(i=0 ; i<nWidth ; i++)
        {
            *pDstU++ = *pSrcUV++;
            *pDstV++ = *pSrcUV++;
        }
        pSrcUV += nSrcOffset;
        pDstU += nDstOffset;
        pDstV += nDstOffset;
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _ColorTool_CvtNV21toYUV420_SSE(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    INT32                   i = 0, j = 0;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nSrcStrideY = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    INT32                   nDstStrideY = pDstImg->GetImgStride(pDstImg, IMG_PLANE_0);
    UINT8                   *pSrcY = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
    UINT8                   *pSrcVU = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_1);
    UINT8                   *pDstY = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0);
    UINT8                   *pDstU = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_1);
    UINT8                   *pDstV = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_2);
    INT32                   nSrcOffset = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_1) - (nWidth >> 1);
    INT32                   nDstOffset = pDstImg->GetImgStride(pDstImg, IMG_PLANE_1) - (nWidth >> 1);
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    // Copy Y-Plane
    for(j=0 ; j<nHeight ; j++)
    {
        MEMCPY(pDstY, pSrcY, nWidth);
        pSrcY += nSrcStrideY;
        pDstY += nDstStrideY;
    }
    
    // Copy UV-Plane
    nWidth = (nWidth >> 1);
    nHeight = (nHeight >> 1);
    for(j=0 ; j<nHeight ; j++)
    {
        for(i=0 ; i<nWidth ; i++)
        {
            *pDstU++ = *pSrcVU++;
            *pDstV++ = *pSrcVU++;
        }
        pSrcVU += nSrcOffset;
        pDstU += nDstOffset;
        pDstV += nDstOffset;
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_NONE) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_NONE)
static Kakao_Status _ColorTool_CvtRGB24toYUV420_OneRowYUV_C(IN const UINT8 * __restrict pSrc,
                                                              OUT UINT8 * __restrict pDstY, OUT UINT8 * __restrict pDstU,
                                                              OUT UINT8 * __restrict pDstV,IN INT32 nWidth)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   i = 0;
    
    for(i=0 ; i<nWidth ; i++)
    {
        UINT32             nR = *pSrc++;
        UINT32             nG = *pSrc++;
        UINT32             nB = *pSrc++;
        
        *pDstY++ = ((77 * nR) + (151 * nG) + (28 * nB)) >> 8;
        
        if(i & 0x1)
        {
            *pDstU++ = ((((-44) * nR) + ((-87) * nG) + (131 * nB)) >> 8) + 128;
            *pDstV++ = (((131 * nR) + ((-110) * nG) + ((-21) * nB)) >> 8) + 128;
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _ColorTool_CvtRGB24toYUV420_OneRowY_C(IN const UINT8 * __restrict pSrc, IN UINT8 * __restrict pDstY, IN INT32 nWidth)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   i = 0;
    
    for(i=0 ; i<nWidth ; i++)
    {
        UINT32              nR = *pSrc++;
        UINT32              nG = *pSrc++;
        UINT32              nB = *pSrc++;
        
        *pDstY++ = ((nR*77) + (nG*151) + (nB*28)) >> 8;
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _ColorTool_CvtNV12toYUV420_C(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    INT32                   i = 0, j = 0;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nSrcStrideY = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    INT32                   nDstStrideY = pDstImg->GetImgStride(pDstImg, IMG_PLANE_0);
    UINT8                   *pSrcY = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
    UINT8                   *pSrcUV = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_1);
    UINT8                   *pDstY = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0);
    UINT8                   *pDstU = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_1);
    UINT8                   *pDstV = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_2);
    INT32                   nSrcOffset = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_1) - (nWidth >> 1);
    INT32                   nDstOffset = pDstImg->GetImgStride(pDstImg, IMG_PLANE_1) - (nWidth >> 1);
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    // Copy Y-Plane
    for(j=0 ; j<nHeight ; j++)
    {
        MEMCPY(pDstY, pSrcY, nWidth);
        pSrcY += nSrcStrideY;
        pDstY += nDstStrideY;
    }
    
    // Copy UV-Plane
    nWidth = (nWidth >> 1);
    nHeight = (nHeight >> 1);
    for(j=0 ; j<nHeight ; j++)
    {
        for(i=0 ; i<nWidth ; i++)
        {
            *pDstU++ = *pSrcUV++;
            *pDstV++ = *pSrcUV++;
        }
        pSrcUV += nSrcOffset;
        pDstU += nDstOffset;
        pDstV += nDstOffset;
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _ColorTool_CvtNV21toYUV420_C(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    INT32                   i = 0, j = 0;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nSrcStrideY = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    INT32                   nDstStrideY = pDstImg->GetImgStride(pDstImg, IMG_PLANE_0);
    UINT8                   *pSrcY = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
    UINT8                   *pSrcVU = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_1);
    UINT8                   *pDstY = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0);
    UINT8                   *pDstU = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_1);
    UINT8                   *pDstV = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_2);
    INT32                   nSrcOffset = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_1) - (nWidth >> 1);
    INT32                   nDstOffset = pDstImg->GetImgStride(pDstImg, IMG_PLANE_1) - (nWidth >> 1);
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    // Copy Y-Plane
    for(j=0 ; j<nHeight ; j++)
    {
        MEMCPY(pDstY, pSrcY, nWidth);
        pSrcY += nSrcStrideY;
        pDstY += nDstStrideY;
    }
    
    // Copy UV-Plane
    nWidth = (nWidth >> 1);
    nHeight = (nHeight >> 1);
    for(j=0 ; j<nHeight ; j++)
    {
        for(i=0 ; i<nWidth ; i++)
        {
            *pDstU++ = *pSrcVU++;
            *pDstV++ = *pSrcVU++;
        }
        pSrcVU += nSrcOffset;
        pDstU += nDstOffset;
        pDstV += nDstOffset;
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}
#else
static Kakao_Status _ColorTool_CvtRGB24toYUV420_OneRowYUV_C(IN const UINT8 * __restrict pSrc,
                                                            OUT UINT8 * __restrict pDstY, OUT UINT8 * __restrict pDstU,
                                                            OUT UINT8 * __restrict pDstV,IN INT32 nWidth)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   i = 0;
    
    for(i=0 ; i<nWidth ; i++)
    {
        UINT32             nR = *pSrc++;
        UINT32             nG = *pSrc++;
        UINT32             nB = *pSrc++;
        
        *pDstY++ = ((77 * nR) + (151 * nG) + (28 * nB)) >> 8;
        
        if(i & 0x1)
        {
            *pDstU++ = ((((-44) * nR) + ((-87) * nG) + (131 * nB)) >> 8) + 128;
            *pDstV++ = (((131 * nR) + ((-110) * nG) + ((-21) * nB)) >> 8) + 128;
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _ColorTool_CvtRGB24toYUV420_OneRowY_C(IN const UINT8 * __restrict pSrc, IN UINT8 * __restrict pDstY, IN INT32 nWidth)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   i = 0;
    
    for(i=0 ; i<nWidth ; i++)
    {
        UINT32              nR = *pSrc++;
        UINT32              nG = *pSrc++;
        UINT32              nB = *pSrc++;
        
        *pDstY++ = ((nR*77) + (nG*151) + (nB*28)) >> 8;
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _ColorTool_CvtNV12toYUV420_C(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    INT32                   i = 0, j = 0;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nSrcStrideY = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    INT32                   nDstStrideY = pDstImg->GetImgStride(pDstImg, IMG_PLANE_0);
    UINT8                   *pSrcY = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
    UINT8                   *pSrcUV = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_1);
    UINT8                   *pDstY = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0);
    UINT8                   *pDstU = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_1);
    UINT8                   *pDstV = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_2);
    INT32                   nSrcOffset = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_1) - (nWidth >> 1);
    INT32                   nDstOffset = pDstImg->GetImgStride(pDstImg, IMG_PLANE_1) - (nWidth >> 1);
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    // Copy Y-Plane
    for(j=0 ; j<nHeight ; j++)
    {
        MEMCPY(pDstY, pSrcY, nWidth);
        pSrcY += nSrcStrideY;
        pDstY += nDstStrideY;
    }
    
    // Copy UV-Plane
    nWidth = (nWidth >> 1);
    nHeight = (nHeight >> 1);
    for(j=0 ; j<nHeight ; j++)
    {
        for(i=0 ; i<nWidth ; i++)
        {
            *pDstU++ = *pSrcUV++;
            *pDstV++ = *pSrcUV++;
        }
        pSrcUV += nSrcOffset;
        pDstU += nDstOffset;
        pDstV += nDstOffset;
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _ColorTool_CvtNV21toYUV420_C(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    INT32                   i = 0, j = 0;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nSrcStrideY = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    INT32                   nDstStrideY = pDstImg->GetImgStride(pDstImg, IMG_PLANE_0);
    UINT8                   *pSrcY = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
    UINT8                   *pSrcVU = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_1);
    UINT8                   *pDstY = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0);
    UINT8                   *pDstU = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_1);
    UINT8                   *pDstV = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_2);
    INT32                   nSrcOffset = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_1) - (nWidth >> 1);
    INT32                   nDstOffset = pDstImg->GetImgStride(pDstImg, IMG_PLANE_1) - (nWidth >> 1);
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    // Copy Y-Plane
    for(j=0 ; j<nHeight ; j++)
    {
        MEMCPY(pDstY, pSrcY, nWidth);
        pSrcY += nSrcStrideY;
        pDstY += nDstStrideY;
    }
    
    // Copy UV-Plane
    nWidth = (nWidth >> 1);
    nHeight = (nHeight >> 1);
    for(j=0 ; j<nHeight ; j++)
    {
        for(i=0 ; i<nWidth ; i++)
        {
            *pDstU++ = *pSrcVU++;
            *pDstV++ = *pSrcVU++;
        }
        pSrcVU += nSrcOffset;
        pDstU += nDstOffset;
        pDstV += nDstOffset;
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}
#endif


static Kakao_Status _ColorTool_CopyImg(IN const ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    if(KAKAO_STAT_OK != (nRet = Kakao_ImgProc_Util_CopyImg(pDstImg, pSrcImg)))
        goto Error;
        
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}







