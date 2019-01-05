


/*----------------------------------------------------------------------------------------
 File Inclusions
 ----------------------------------------------------------------------------------------*/
#include "Kakao_ImgProc_Modules.h"
#include "Kakao_ImgProc_SIMD.h"

#if HAVE_UNISTD_H && !defined(__OS2__)
    #include <unistd.h>
#elif defined(_WIN32)
    #include <windows.h>
    typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
#elif defined(__OS2__)
    #define INCL_DOS
    #define INCL_DOSSPINLOCK
    #include <os2.h>
#endif


/*----------------------------------------------------------------------------------------
 Constant Definitions
 ----------------------------------------------------------------------------------------*/
#define     ADVANCED_EROSION_TH         (4)
#define     ADVANCED_DILATION_TH        (2)


/*----------------------------------------------------------------------------------------
 Macro Definitions
 ----------------------------------------------------------------------------------------*/
#define SET_ROI_REGION(ROI, nSX, nSY, nEX, nEY)         \
                                            {\
                                                ROI.nStartX = nSX;\
                                                ROI.nStartY = nSY;\
                                                ROI.nEndX = nEX;\
                                                ROI.nEndY = nEY;\
                                            }


/*----------------------------------------------------------------------------------------
 Type Definitions
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Static Function
 ----------------------------------------------------------------------------------------*/
#if ENABLE_ISET(CPU_ARMX | ISET_NEON)
static INT32 _Util_Cal_SAD16x16_NEON(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, IN const INT32 nStride);
static INT32 _Util_Cal_Accum16x16_NEON(UINT8 *pSrc, INT32 nStride);
static INT32 _Util_Cal_Mean16x16_NEON(IN UINT8 *pSrc, IN const INT32 nStride);
static INT32 _Util_Cal_Variance16x16_NEON(IN UINT8 *pSrc, IN const INT32 nMean, IN const INT32 nStride);
static Kakao_Status _Util_FramewiseToMBwise_NEON(IN OUT Img_T *pSrcImg,
                                                            IN const BinaryMapType nBinaryMap, IN const UINT32 nThreshold);
static Kakao_Status _Util_ScaleBinaryMap_NEON(IN OUT Img_T *pSrcImg, IN const BinaryMapType nBinaryMap,
                                                            IN const UINT32 nScaleFactor);
static Kakao_Status _Util_MergeImgs_WithBinaryMap_OneRow_NEON(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, OUT UINT8 *pDst,
                                                            IN const UINT8 *pMaskMap, IN const INT32 nWidth, IN const INT32 nStride);
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_SSE4) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_SSE4)
static INT32 _Util_Cal_SAD16x16_SSE4(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, IN const INT32 nStride);
static INT32 _Util_Cal_Accum16x16_SSE4(UINT8 *pSrc, INT32 nStride);
static INT32 _Util_Cal_Mean16x16_SSE4(IN UINT8 *pSrc, IN const INT32 nStride);
static INT32 _Util_Cal_Variance16x16_SSE4(IN UINT8* pSrc, IN const INT32 nMean,
                                                            IN const INT32 nStride);
static Kakao_Status _Util_FramewiseToMBwise_SSE4(IN OUT Img_T *pSrcImg, IN const BinaryMapType nBinaryMap,
                                                            IN const UINT32 nThreshold);
static Kakao_Status _Util_ScaleBinaryMap_SSE4(IN OUT Img_T *pSrcImg, IN const BinaryMapType nBinaryMap,
                                                            IN const UINT32 nScaleFactor);
static Kakao_Status _Util_MergeImgs_WithBinaryMap_OneRow_SSE4(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, OUT UINT8 *pDst,
                                                            IN const UINT8 *pMaskMap, IN const INT32 nWidth, IN const INT32 nStride);
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_SSE3) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_SSE3)
static INT32 _Util_Cal_SAD16x16_SSE3(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, IN const INT32 nStride);
static INT32 _Util_Cal_Accum16x16_SSE3(UINT8 *pSrc, INT32 nStride);
static INT32 _Util_Cal_Mean16x16_SSE3(IN UINT8 *pSrc, IN const INT32 nStride);
static INT32 _Util_Cal_Variance16x16_SSE3(IN UINT8* pSrc, IN const INT32 nMean,
                                                            IN const INT32 nStride);
static Kakao_Status _Util_FramewiseToMBwise_SSE3(IN OUT Img_T *pSrcImg, IN const BinaryMapType nBinaryMap,
                                                            IN const UINT32 nThreshold);
static Kakao_Status _Util_ScaleBinaryMap_SSE3(IN OUT Img_T *pSrcImg, IN const BinaryMapType nBinaryMap,
                                                            IN const UINT32 nScaleFactor);
static Kakao_Status _Util_MergeImgs_WithBinaryMap_OneRow_SSE3(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, OUT UINT8 *pDst,
                                                            IN const UINT8 *pMaskMap, IN const INT32 nWidth, IN const INT32 nStride);
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_SSE2) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_SSE2)
static INT32 _Util_Cal_SAD16x16_SSE2(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, IN const INT32 nStride);
static INT32 _Util_Cal_Accum16x16_SSE2(UINT8 *pSrc, INT32 nStride);
static INT32 _Util_Cal_Mean16x16_SSE2(IN UINT8 *pSrc, IN const INT32 nStride);
static INT32 _Util_Cal_Variance16x16_SSE2(IN UINT8* pSrc, IN const INT32 nMean,
                                          IN const INT32 nStride);
static Kakao_Status _Util_FramewiseToMBwise_SSE2(IN OUT Img_T *pSrcImg, IN const BinaryMapType nBinaryMap,
                                                 IN const UINT32 nThreshold);
static Kakao_Status _Util_ScaleBinaryMap_SSE2(IN OUT Img_T *pSrcImg, IN const BinaryMapType nBinaryMap,
                                              IN const UINT32 nScaleFactor);
static Kakao_Status _Util_MergeImgs_WithBinaryMap_OneRow_SSE2(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, OUT UINT8 *pDst,
                                                              IN const UINT8 *pMaskMap, IN const INT32 nWidth, IN const INT32 nStride);
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_SSE) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_SSE)
static INT32 _Util_Cal_SAD16x16_SSE(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, IN const INT32 nStride);
static INT32 _Util_Cal_Accum16x16_SSE(UINT8 *pSrc, INT32 nStride);
static INT32 _Util_Cal_Mean16x16_SSE(IN UINT8 *pSrc, IN const INT32 nStride);
static INT32 _Util_Cal_Variance16x16_SSE(IN UINT8* pSrc, IN const INT32 nMean,
                                          IN const INT32 nStride);
static Kakao_Status _Util_FramewiseToMBwise_SSE(IN OUT Img_T *pSrcImg, IN const BinaryMapType nBinaryMap,
                                                 IN const UINT32 nThreshold);
static Kakao_Status _Util_ScaleBinaryMap_SSE(IN OUT Img_T *pSrcImg, IN const BinaryMapType nBinaryMap,
                                              IN const UINT32 nScaleFactor);
static Kakao_Status _Util_MergeImgs_WithBinaryMap_OneRow_SSE(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, OUT UINT8 *pDst,
                                                              IN const UINT8 *pMaskMap, IN const INT32 nWidth, IN const INT32 nStride);
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_NONE) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_NONE)
static INT32 _Util_Cal_SAD16x16_C(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, IN const INT32 nStride);
static INT32 _Util_Cal_Accum16x16_C(UINT8 *pSrc, INT32 nStride);
static INT32 _Util_Cal_Mean16x16_C(IN UINT8 *pSrc, IN const INT32 nStride);
static INT32 _Util_Cal_Variance16x16_C(IN UINT8* pSrc, IN const INT32 nMean,
                                                            IN const INT32 nStride);
static Kakao_Status _Util_FramewiseToMBwise_C(IN OUT Img_T *pSrcImg, IN const BinaryMapType nBinaryMap,
                                                            IN const UINT32 nThreshold);
static Kakao_Status _Util_ScaleBinaryMap_C(IN OUT Img_T *pSrcImg, IN const BinaryMapType nBinaryMap,
                                                            IN const UINT32 nScaleFactor);
static Kakao_Status _Util_MergeImgs_WithBinaryMap_OneRow_C(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, OUT UINT8 *pDst,
                                                            IN const UINT8 *pMaskMap, IN const INT32 nWidth, IN const INT32 nStride);
#else
static INT32 _Util_Cal_SAD16x16_C(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, IN const INT32 nStride);
static INT32 _Util_Cal_Accum16x16_C(UINT8 *pSrc, INT32 nStride);
static INT32 _Util_Cal_Mean16x16_C(IN UINT8 *pSrc, IN const INT32 nStride);
static INT32 _Util_Cal_Variance16x16_C(IN UINT8* pSrc, IN const INT32 nMean,
                                                            IN const INT32 nStride);
static Kakao_Status _Util_FramewiseToMBwise_C(IN OUT Img_T *pSrcImg, IN const BinaryMapType nBinaryMap,
                                                            IN const UINT32 nThreshold);
static Kakao_Status _Util_ScaleBinaryMap_C(IN OUT Img_T *pSrcImg, IN const BinaryMapType nBinaryMap,
                                                            IN const UINT32 nScaleFactor);
static Kakao_Status _Util_MergeImgs_WithBinaryMap_OneRow_C(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, OUT UINT8 *pDst,
                                                            IN const UINT8 *pMaskMap, IN const INT32 nWidth, IN const INT32 nStride);
#endif

__INLINE INT32 _Util_GetImgWidth(IN const Img_T *pSrcImg, IN const INT32 nPlaneIdx);
__INLINE INT32 _Util_SetImgWidth(IN Img_T *pSrcImg, IN const INT32 nPlaneIdx, IN const INT32 nWidth);
__INLINE INT32 _Util_GetImgHeight(IN const Img_T *pSrcImg, IN const INT32 nPlaneIdx);
__INLINE INT32 _Util_SetImgHeight(IN Img_T *pSrcImg, IN const INT32 nPlaneIdx, IN const INT32 nHeight);
__INLINE INT32 _Util_GetImgStride(IN const Img_T *pSrcImg, IN const INT32 nPlaneIdx);
__INLINE INT32 _Util_SetImgStride(IN Img_T *pSrcImg, IN const INT32 nPlaneIdx, IN const INT32 nStride);
__INLINE KISize _Util_GetImgSize(IN const Img_T *pSrcImg);
__INLINE ImgFormat _Util_GetImgFormat(IN const Img_T *pSrcImg);
__INLINE INT32 _Util_SetImgFormat(IN const Img_T *pSrcImg, IN const ImgFormat nImgFormat);
__INLINE ImgDepth _Util_GetImgDepth(IN const Img_T *pSrcImg);
__INLINE INT32 _Util_SetImgDepth(IN const Img_T *pSrcImg, IN const ImgDepth nImgDepth);
__INLINE INT32 _Util_GetImgNumofPlanes(IN const Img_T *pSrcImg);
__INLINE INT32 _Util_SetImgNumofPlanes(IN const Img_T *pSrcImg, IN const INT32 nNumofPlanes);
__INLINE ROI _Util_GetImgROI(IN const Img_T *pSrcImg);
__INLINE INT32 _Util_SetImgROI(IN Img_T *pSrcImg, IN const INT32 nStartX, IN const INT32 nEndX, IN const INT32 nStartY, IN const INT32 nEndY);
__INLINE UINT8* _Util_GetImgMBWiseMap(IN const Img_T *pSrcImg);
__INLINE INT32* _Util_GetImgMBWiseSADMap(IN const Img_T *pSrcImg);
__INLINE UINT8* _Util_GetBinaryMap(IN Img_T *pSrcImg, IN const BinaryMapType nBinaryMap);
__INLINE F32* _Util_GetImgHistMap(IN const Img_T *pSrcImg, IN const INT32 nPlaneIdx, IN const INT32 nDimension);
__INLINE void* _Util_GetImgPlanes(IN const Img_T *pSrcImg, IN const INT32 nPlaneIdx);
__INLINE INT32 _Util_SetImgPlanes(IN Img_T *pSrcImg, IN const INT32 nPlaneIdx, IN HANDLE pPtr);
__INLINE INT32 _Util_GetImgBins(IN const Img_T *pSrcImg);
__INLINE INT32 _Util_SetImgBins(IN Img_T *pSrcImg, IN const INT32 nBins);
__INLINE IMGPROC_BOOL _Util_GetImgROIStatus(IN const Img_T *pSrcImg);
__INLINE INT32 _Util_SetImgROIStatus(IN Img_T *pSrcImg, IN const IMGPROC_BOOL nROIStatus);


static Kakao_Status _Util_AllocBinaryMap(IN OUT Img_T *pSrcImg, IN const BinaryMapType nBinaryMap);
static Kakao_Status _Util_Padding_Gray(IN Img_T *pSrcImg);
static Kakao_Status _Util_Copy_YUV(OUT Img_T *pDstImg, IN const Img_T *pSrcImg);
static Kakao_Status _Util_Padding_YUV(IN Img_T *pSrcImg);
static Kakao_Status _Util_Padding_Plane(UINT8 *pSrc, INT32 nWidth, INT32 nHeight, INT32 nStride);
__INLINE void _ImgProc_Util_Flip(IN UINT8 *pSrc, OUT UINT8 *pDst, OUT UINT8 *pTmp, IN const INT32 nWidth);
static Kakao_Status _Util_CopyPlane(UINT8 *pDst, const UINT8 *pSrc, IN const INT32 nWidth, IN const INT32 nHeight,
                                                            IN const INT32 nSrcStride, IN const INT32 nDstStride,
                                                            IN const UINT32 nPixelSize);
static Kakao_Status _Util_MergeImgs_WithBinaryMap(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, OUT UINT8 *pDst,
                                                            IN const UINT8 *pMaskMap, IN const INT32 nWidth,
                                                            IN const INT32 nHeight, IN const INT32 nStride);
static Kakao_Status _Util_UpdateImgSize(IN OUT Img_T *pSrcImg, IN const INT32 nWidth, IN const INT32 nHeight,
                                                            IN const UINT32 *pStride, IN const ImgFormat nImgFormat);
static Kakao_Status _Util_UpdateSize(IN OUT KISize *pSize, IN const INT32 nWidth, IN const INT32 nHeight,
                                                            IN const UINT32 *pStride, IN const INT32 nNumofPlanes, IN const ImgFormat nImgFormat);
static INT32 _Util_Get_CpuCores(void);
static UINT64 _Util_Get_CpuFrequency(void);


#define _Util_Cal_SAD16x16                          MAKE_ACCEL_FUNC(_Util_Cal_SAD16x16)
#define _Util_Cal_Accum16x16                        MAKE_ACCEL_FUNC(_Util_Cal_Accum16x16)
#define _Util_Cal_Mean16x16                         MAKE_ACCEL_FUNC(_Util_Cal_Mean16x16)
#define _Util_Cal_Variance16x16                     MAKE_ACCEL_FUNC(_Util_Cal_Variance16x16)
#define _Util_FramewiseToMBwise                     MAKE_ACCEL_FUNC(_Util_FramewiseToMBwise)
#define _Util_ScaleBinaryMap                        MAKE_ACCEL_FUNC(_Util_ScaleBinaryMap)
#define _Util_MergeImgs_WithBinaryMap_OneRow        MAKE_ACCEL_FUNC(_Util_MergeImgs_WithBinaryMap_OneRow)


/*----------------------------------------------------------------------------------------
 Static Variable
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Global Variable
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Function Implementation
 ----------------------------------------------------------------------------------------*/
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_CreateImg(IN OUT Img_T **ppTargetImg, IN const INT32 nWidth, IN const INT32 nHeight, IN const ImgFormat nImgFormat, IN const ImgDepth nImgDepth)
{
    BaseImg_T               *pTmpBaseImg = NULL;
    Img_T                   *pTmpImg = NULL;
    UINT32                  nImgSize = 0, nImgSizeUV = 0;
    UINT32                  nPixelSize = GET_PIXEL_SIZE(nImgDepth);
    INT32                   nAlignedWidth = GET_ALIGNED_LENGTH(nWidth, MACRO_BLOCK_SIZE);
    INT32                   nAlignedHeight = GET_ALIGNED_LENGTH(nHeight, MACRO_BLOCK_SIZE);
    INT32                   nStride = 0;
    INT32                   i = 0;
    INT32                   nNumofPlanes = -1;
    KISize                  *pImgSize = NULL;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    SAFEALLOC(pTmpBaseImg, 1, 32, BaseImg_T);
    
    pImgSize = &(pTmpBaseImg->nImgSize);
    pTmpImg = &(pTmpBaseImg->nImgT);
    
    Kakao_ImgProc_Util_SetImgDefaultParam(pTmpImg);
    
    pTmpBaseImg->nImgSize.nBaseWidth        = nWidth;
    pTmpBaseImg->nImgSize.nBaseHeight       = nHeight;
    pTmpBaseImg->nImgSize.nAllocatedWidth   = nWidth;
    pTmpBaseImg->nImgSize.nAllocatedHeight  = nHeight;
 
    switch(nImgFormat)
    {
        case IMG_FORMAT_GRAY:
            nNumofPlanes = 1;
            nStride = MAKE_STRIDE(nAlignedWidth);
            nImgSize = nPixelSize * (nStride * MAKE_STRIDE(nAlignedHeight));
            
            SAFEALLOC(pTmpBaseImg->pImgData, nImgSize, 32, UINT8);
            pTmpBaseImg->pRealImgData = (HANDLE)((UINT8 *)pTmpBaseImg->pImgData + (nPixelSize * PADDING_SIZE * (nStride + 1)));
            
            for(i=0 ; i<nNumofPlanes ; i++)
            {
                pTmpImg->SetImgWidth(pTmpImg, i, nWidth);
                pTmpImg->SetImgHeight(pTmpImg, i, nHeight);
                pTmpImg->SetImgStride(pTmpImg, i, nStride);
                pTmpBaseImg->pOrgImgPointer[i] = pTmpBaseImg->pPlanes[i] = (HANDLE)((UINT8 *)pTmpBaseImg->pRealImgData + (i * nPixelSize));
            }
            break;
        case IMG_FORMAT_RGB24:
        case IMG_FORMAT_BGR24:
        case IMG_FORMAT_RGBA32:
        case IMG_FORMAT_BGRA32:
            nNumofPlanes = ((IMG_FORMAT_RGB24 == nImgFormat) || (IMG_FORMAT_BGR24 == nImgFormat)) ? 3 : 4;
            nStride = ((((nNumofPlanes * nAlignedWidth) + 2 * (nNumofPlanes * PADDING_SIZE)) + (PADDING_SIZE - 1)) & ~(PADDING_SIZE - 1));
            nImgSize = nPixelSize * (nStride * MAKE_STRIDE(nAlignedHeight));
            
            SAFEALLOC(pTmpBaseImg->pImgData, nImgSize, 32, UINT8);
            pTmpBaseImg->pRealImgData = (HANDLE)((UINT8 *)pTmpBaseImg->pImgData + (nPixelSize * PADDING_SIZE * (nStride + nNumofPlanes)));
            
            for(i=0 ; i<nNumofPlanes ; i++)
            {
                pTmpImg->SetImgWidth(pTmpImg, i, nWidth);
                pTmpImg->SetImgHeight(pTmpImg, i, nHeight);
                pTmpImg->SetImgStride(pTmpImg, i, nStride);
                pTmpBaseImg->pOrgImgPointer[i] = pTmpBaseImg->pPlanes[i] = (HANDLE)((UINT8 *)pTmpBaseImg->pRealImgData + (i * nPixelSize));
            }
            break;
        case IMG_FORMAT_HSV:
        case IMG_FORMAT_HSL:
            nNumofPlanes = 3;
            nStride = MAKE_STRIDE(nAlignedWidth);
            nImgSize = nPixelSize * (nStride * MAKE_STRIDE(nAlignedHeight));
            
            SAFEALLOC(pTmpBaseImg->pImgData, nNumofPlanes * nImgSize, 32, UINT8);
            pTmpBaseImg->pRealImgData = (HANDLE)((UINT8 *)pTmpBaseImg->pImgData + (nPixelSize * PADDING_SIZE * (nStride + 1)));
            
            for(i=0 ; i<nNumofPlanes ; i++)
            {
                pTmpImg->SetImgWidth(pTmpImg, i, nWidth);
                pTmpImg->SetImgHeight(pTmpImg, i, nHeight);
                pTmpImg->SetImgStride(pTmpImg, i, nStride);
                pTmpBaseImg->pOrgImgPointer[i] = pTmpBaseImg->pPlanes[i] = (HANDLE)((UINT8 *)pTmpBaseImg->pRealImgData + (i * nImgSize));
            }
            break;
        case IMG_FORMAT_I420:
            nNumofPlanes = 3;
            nStride = MAKE_STRIDE(nAlignedWidth);
            
            for(i=0 ; i<nNumofPlanes ; i++)
            {
                pTmpImg->SetImgWidth(pTmpImg, i, nWidth >> !(!(i)));
                pTmpImg->SetImgHeight(pTmpImg, i, nHeight >> !(!(i)));
                pTmpImg->SetImgStride(pTmpImg, i, MAKE_STRIDE((nAlignedWidth >> !(!(i)))));
            }

            nImgSize = nPixelSize * (nStride * MAKE_STRIDE(nAlignedHeight));
            nImgSizeUV = nPixelSize * (pImgSize->nStride[IMG_PLANE_1] * MAKE_STRIDE(nAlignedHeight/2));
            
            SAFEALLOC(pTmpBaseImg->pImgData, nImgSize + (2 * nImgSizeUV), 32, UINT8);
            pTmpBaseImg->pRealImgData = (HANDLE)((UINT8 *)pTmpBaseImg->pImgData + (nPixelSize * PADDING_SIZE * (nStride + 1)));

            pTmpBaseImg->pOrgImgPointer[IMG_PLANE_0] = pTmpBaseImg->pPlanes[IMG_PLANE_0] = pTmpBaseImg->pRealImgData;
            pTmpBaseImg->pOrgImgPointer[IMG_PLANE_1] = pTmpBaseImg->pPlanes[IMG_PLANE_1] = (HANDLE)((UINT8 *)pTmpBaseImg->pImgData + nImgSize + (nPixelSize * PADDING_SIZE * (pImgSize->nStride[IMG_PLANE_1] + 1)));
            pTmpBaseImg->pOrgImgPointer[IMG_PLANE_2] = pTmpBaseImg->pPlanes[IMG_PLANE_2] = (HANDLE)((UINT8 *)pTmpBaseImg->pPlanes[IMG_PLANE_1] + nImgSizeUV);
            
            MEMSET((HANDLE)((UINT8 *)pTmpBaseImg->pImgData + nImgSize), 128, nImgSizeUV);
            MEMSET((HANDLE)((UINT8 *)pTmpBaseImg->pImgData + nImgSize + nImgSizeUV), 128, nImgSizeUV);
            break;
        default:
            break;
    }
    
    pTmpImg->SetImgFormat(pTmpImg, nImgFormat);
    pTmpImg->SetImgDepth(pTmpImg, nImgDepth);
    pTmpImg->SetNumofImgPlanes(pTmpImg, nNumofPlanes);
    pTmpImg->SetImgROIStatus(pTmpImg, IMGPROC_FALSE);
    pTmpImg->SetImgBins(pTmpImg, -1);
    pTmpImg->SetImgROI(pTmpImg, 0, nWidth, 0, nHeight);
    
    *ppTargetImg = pTmpImg;
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_ReleaseImg(IN OUT Img_T **ppTargetImg)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    BaseImg_T               *pTmpBaseImg = (BaseImg_T *)(*ppTargetImg);
    INT32                   i = 0;

    CHECK_POINTER_VALIDATION((*ppTargetImg), KAKAO_STAT_OK)
    
    for(i=IMG_BINARYMAP_MIN ; i<IMG_BINARYMAP_MAX ; i++)
        SAFEFREE(pTmpBaseImg->pBinaryMap[i]);
    
    SAFEFREE(pTmpBaseImg->pMBwiseMap);
    SAFEFREE(pTmpBaseImg->pMBwiseSADMap);
    SAFEFREE(pTmpBaseImg->pImgData);
    SAFEFREE(pTmpBaseImg->pOneRowBuf);
    for(i=0 ; i<4 ; i++)
        SAFEFREE(pTmpBaseImg->p1DHist[i]);
    SAFEFREE(pTmpBaseImg->p2DHist);
    SAFEFREE(pTmpBaseImg->pTmp1DHist);
    SAFEFREE(pTmpBaseImg->pTmp2DHist);
    SAFEFREE(pTmpBaseImg->p1DHistBPTable);
    SAFEFREE(pTmpBaseImg->p2DHistBPTable);
    
    SAFEFREE((pTmpBaseImg));
    
    *ppTargetImg = NULL;
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_CreateDummyImg(IN OUT Img_T **ppTargetImg, IN const INT32 nWidth, IN const INT32 nHeight, IN const ImgFormat nImgFormat, IN const ImgDepth nImgDepth)
{
    BaseImg_T               *pTmpBaseImg = NULL;
    Img_T                   *pTmpImg = NULL;
    UINT32                  nImgSize = 0, nImgSizeUV = 0;
    UINT32                  nPixelSize = GET_PIXEL_SIZE(nImgDepth);
    INT32                   nAlignedWidth = GET_ALIGNED_LENGTH(nWidth, MACRO_BLOCK_SIZE);
    INT32                   nAlignedHeight = GET_ALIGNED_LENGTH(nHeight, MACRO_BLOCK_SIZE);
    INT32                   nStride = 0;
    INT32                   i = 0;
    INT32                   nNumofPlanes = -1;
    KISize                  *pImgSize = NULL;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    SAFEALLOC(pTmpBaseImg, 1, 32, BaseImg_T);
    
    pImgSize = &(pTmpBaseImg->nImgSize);
    pTmpImg = &(pTmpBaseImg->nImgT);
    
    Kakao_ImgProc_Util_SetImgDefaultParam(pTmpImg);
    
    pTmpBaseImg->nImgSize.nBaseWidth        = nWidth;
    pTmpBaseImg->nImgSize.nBaseHeight       = nHeight;
    pTmpBaseImg->nImgSize.nAllocatedWidth   = nWidth;
    pTmpBaseImg->nImgSize.nAllocatedHeight  = nHeight;
    
    switch(nImgFormat)
    {
        case IMG_FORMAT_GRAY:
            nNumofPlanes = 1;
            nStride = MAKE_STRIDE(nAlignedWidth);
            
            for(i=0 ; i<nNumofPlanes ; i++)
            {
                pTmpImg->SetImgWidth(pTmpImg, i, nWidth);
                pTmpImg->SetImgHeight(pTmpImg, i, nHeight);
                pTmpImg->SetImgStride(pTmpImg, i, nStride);
                pTmpImg->SetImgPlanes(pTmpImg, i, NULL);
            }
            break;
        case IMG_FORMAT_RGB24:
        case IMG_FORMAT_BGR24:
        case IMG_FORMAT_RGBA32:
        case IMG_FORMAT_BGRA32:
            nNumofPlanes = ((IMG_FORMAT_RGB24 == nImgFormat) || (IMG_FORMAT_BGR24 == nImgFormat)) ? 3 : 4;
            nStride = ((((nNumofPlanes * nAlignedWidth) + 2 * (nNumofPlanes * PADDING_SIZE)) + (PADDING_SIZE - 1)) & ~(PADDING_SIZE - 1));
            
            for(i=0 ; i<nNumofPlanes ; i++)
            {
                pTmpImg->SetImgWidth(pTmpImg, i, nWidth);
                pTmpImg->SetImgHeight(pTmpImg, i, nHeight);
                pTmpImg->SetImgStride(pTmpImg, i, nStride);
                pTmpImg->SetImgPlanes(pTmpImg, i, NULL);
            }
            break;
        case IMG_FORMAT_HSV:
        case IMG_FORMAT_HSL:
            nNumofPlanes = 3;
            nStride = MAKE_STRIDE(nAlignedWidth);
            
            for(i=0 ; i<nNumofPlanes ; i++)
            {
                pTmpImg->SetImgWidth(pTmpImg, i, nWidth);
                pTmpImg->SetImgHeight(pTmpImg, i, nHeight);
                pTmpImg->SetImgStride(pTmpImg, i, nStride);
                pTmpImg->SetImgPlanes(pTmpImg, i, NULL);
            }
            break;
        case IMG_FORMAT_I420:
            nNumofPlanes = 3;
            nStride = MAKE_STRIDE(nAlignedWidth);
            
            for(i=0 ; i<nNumofPlanes ; i++)
            {
                pTmpImg->SetImgWidth(pTmpImg, i, nWidth >> !(!(i)));
                pTmpImg->SetImgHeight(pTmpImg, i, nHeight >> !(!(i)));
                pTmpImg->SetImgStride(pTmpImg, i, MAKE_STRIDE((nAlignedWidth >> !(!(i)))));
                pTmpImg->SetImgPlanes(pTmpImg, i, NULL);
            }
            break;
        default:
            break;
    }
    
    pTmpImg->SetImgFormat(pTmpImg, nImgFormat);
    pTmpImg->SetImgDepth(pTmpImg, nImgDepth);
    pTmpImg->SetNumofImgPlanes(pTmpImg, nNumofPlanes);
    pTmpImg->SetImgROIStatus(pTmpImg, IMGPROC_FALSE);
    pTmpImg->SetImgBins(pTmpImg, -1);
    pTmpImg->SetImgROI(pTmpImg, 0, nWidth, 0, nHeight);
    
    *ppTargetImg = pTmpImg;
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_CreateBinaryMap(IN Img_T *pSrcImg, IN const BinaryMapType nBinaryMap)
{
    UINT8                   *pDstPtr = NULL;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER)
    CHECK_PARAM_VALIDATION((nBinaryMap < IMG_BINARYMAP_MAX), KAKAO_STAT_INVALID_PARAM)

    pDstPtr = pSrcImg->GetImgBinaryMap(pSrcImg, nBinaryMap);
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_CreateMBwiseMap(IN OUT Img_T *pSrcImg)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    BaseImg_T               *pBaseImg = ((BaseImg_T *)pSrcImg);
    
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER)
    
    SAFEALLOC(pBaseImg->pMBwiseMap, ((MAKE_STRIDE(pBaseImg->nImgSize.nAllocatedWidth) * pBaseImg->nImgSize.nAllocatedHeight) >> 8), 32, UINT8);
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_CreateMBwiseSADMap(IN OUT Img_T *pSrcImg)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    BaseImg_T               *pBaseImg = ((BaseImg_T *)pSrcImg);
    
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER)
    
    SAFEALLOC(pBaseImg->pMBwiseSADMap, ((MAKE_STRIDE(pBaseImg->nImgSize.nAllocatedWidth) * pBaseImg->nImgSize.nAllocatedHeight) >> 8), 32, INT32);
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_CopyImg(OUT Img_T *pDstImg, IN const Img_T *pSrcImg)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    BaseImg_T               *pSrcBaseImg = ((BaseImg_T *)pSrcImg);
    BaseImg_T               *pDstBaseImg = ((BaseImg_T *)pDstImg);
    INT32                   i = 0;

    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION((pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0)), KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION((pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0)), KAKAO_STAT_NULL_POINTER)

    //pDstImg->nImgSize = pSrcImg->nImgSize;
    {
        const INT32         nWidth = pDstImg->GetImgWidth(pDstImg, IMG_PLANE_0);
        const INT32         nHeight = pDstImg->GetImgHeight(pDstImg, IMG_PLANE_0);
        const INT32         nStride = pDstImg->GetImgStride(pDstImg, IMG_PLANE_0);
        const INT32         nColorDepth = GET_PIXEL_SIZE(pSrcImg->GetImgDepth(pSrcImg));
        const ROI           nSrcROI = pSrcImg->GetImgROI(pSrcImg);
        
        pDstImg->SetImgFormat(pDstImg, pSrcImg->GetImgFormat(pSrcImg));
        pDstImg->SetImgDepth(pDstImg, pSrcImg->GetImgDepth(pSrcImg));
        pDstImg->SetImgROI(pDstImg, nSrcROI.nStartX, nSrcROI.nEndX, nSrcROI.nStartY, nSrcROI.nEndY);
        
        switch(pDstImg->GetImgFormat(pDstImg))
        {
            case IMG_FORMAT_GRAY:
            case IMG_FORMAT_RGB24:
            case IMG_FORMAT_BGR24:
            case IMG_FORMAT_RGBA32:
            case IMG_FORMAT_BGRA32:
                MEMCPY(((BaseImg_T *)pDstImg)->pImgData, ((BaseImg_T *)pSrcImg)->pImgData, nColorDepth * (nStride * MAKE_STRIDE(nHeight)));
                break;
            case IMG_FORMAT_HSV:
            case IMG_FORMAT_HSL:
                MEMCPY(((BaseImg_T *)pDstImg)->pImgData, ((BaseImg_T *)pSrcImg)->pImgData, nColorDepth * (3 * (nStride * MAKE_STRIDE(nHeight))));
                break;
            case IMG_FORMAT_I420:
                _Util_Copy_YUV(pDstImg, pSrcImg);
                break;
            default:
                break;
        }
        
        {
            const UINT8     *pSrcMBwiseMap = pSrcBaseImg->pMBwiseMap;
            
            if(NULL != pSrcMBwiseMap)
            {
                UINT8       *pDstMBwiseMap = pDstImg->GetImgMBWiseMap(pDstImg);
                
                if(NULL != pDstMBwiseMap)
                    OneRow_Copy(pDstMBwiseMap, pSrcMBwiseMap, ((nStride * nHeight) >> 8));
            }
        }
        
        {
            const INT32     *pSrcMBwiseSADMap = pSrcBaseImg->pMBwiseSADMap;
            
            if(NULL != pSrcMBwiseSADMap)
            {
                INT32       *pDstMBwiseSADMap = pDstImg->GetImgMBWiseSADMap(pDstImg);
                
                if(NULL != pDstMBwiseSADMap)
                    OneRow_Copy((UINT8 *)pDstMBwiseSADMap, (UINT8 *)pSrcMBwiseSADMap, ((nStride * nHeight) >> 8) * sizeof(INT32));
            }
        }
        
        for(i=IMG_BINARYMAP_MIN ; i<IMG_BINARYMAP_MAX ; i++)
        {
            const UINT8     *pSrcBinaryMap = pSrcBaseImg->pBinaryMap[i];
            
            if(NULL != pSrcBinaryMap)
            {
                UINT8       *pDstBinaryMap = NULL;
                
                pDstImg->GetImgBinaryMap(pDstImg, i);
                
                pDstBinaryMap = pDstBaseImg->pBinaryMap[i];
                
                if(NULL != pDstBinaryMap)
                    OneRow_Copy(pDstBinaryMap, pSrcBinaryMap, (nStride * nHeight));
            }
        }
    }

    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_CopyPlane(OUT UINT8 *pDst, IN const UINT8 *pSrc,
                                                           IN const INT32 nWidth, IN const INT32 nHeight,
                                                           IN const INT32 nDstStride, IN const INT32 nSrcStride)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    UINT32                  nPixelSize = 1;
    
    nRet = _Util_CopyPlane(pDst, pSrc, nWidth, nHeight, nSrcStride, nDstStride, nPixelSize);
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_CopyOneRow(OUT UINT8 *pDst, IN const UINT8 *pSrc, IN const INT32 nWidth)
{
    OneRow_Copy(pDst, (UINT8 *)pSrc, nWidth);

    return KAKAO_STAT_OK;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_Padding_Frame(IN OUT Img_T *pSrcImg)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER)
    
    switch(pSrcImg->GetImgFormat(pSrcImg))
    {
        case IMG_FORMAT_GRAY:
            _Util_Padding_Gray(pSrcImg);
            break;
        case IMG_FORMAT_RGB24:
            break;
        case IMG_FORMAT_BGR24:
            break;
        case IMG_FORMAT_I420:
            _Util_Padding_YUV(pSrcImg);
            break;
        default:
            return KAKAO_STAT_FAIL;
            break;
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_Padding_ImgPlane(IN OUT Img_T *pSrcImg)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER)

    {
        INT32                   j = 0;
        const INT32             nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
        const INT32             nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
        const INT32             nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
        INT32                   nPixPos = 0;
        UINT8                   *pSrc0 = NULL;
        
        switch(pSrcImg->GetImgFormat(pSrcImg))
        {
            case IMG_FORMAT_RGB24:
                printf("Do Not Support Padding RGB24");
                return KAKAO_STAT_FAIL;
                break;
            case IMG_FORMAT_BGR24:
                printf("Do Not Support Padding BGR24");
                return KAKAO_STAT_FAIL;
                break;
            case IMG_FORMAT_I420:
                pSrc0 = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
                break;
            default:
                return KAKAO_STAT_FAIL;
                break;
        }
        
        /* ----------- Y  -----------*/
        // Left-Side & Right-Side
        for(j=0 ; j<nHeight ; j++)
        {
            nPixPos = j * nStride;
            
            MEMSET(&(pSrc0[nPixPos - PADDING_SIZE]), pSrc0[nPixPos], PADDING_SIZE);
            MEMSET(&(pSrc0[nPixPos + nWidth]), pSrc0[nPixPos + nWidth - 1], PADDING_SIZE);
        }
        
        // Top-Side
        pSrc0 -= PADDING_SIZE;
        for(j=0 ; j<PADDING_SIZE ; j++)
            OneRow_Copy(&(pSrc0[-((j + 1) * nStride)]), &(pSrc0[0]), nStride);
        
        // Bottom-Side
        pSrc0 += (nStride * (nHeight - 1));
        for(j=0 ; j<PADDING_SIZE ; j++)
            OneRow_Copy(&(pSrc0[((j + 1) * nStride)]), &(pSrc0[0]), nStride);
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_SubtractAccImg(OUT Img_T *pSubtratcImg, IN const Img_T *pSrcImg0, IN const Img_T *pSrcImg1)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pSubtratcImg, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pSrcImg0, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pSrcImg1, KAKAO_STAT_NULL_POINTER)
    
    {
        INT32                   i = 0, j = 0;
        INT32                   nLocalPos = 0;
        const INT32             nWidth = pSrcImg0->GetImgWidth(pSrcImg0, IMG_PLANE_0);
        const INT32             nHeight = pSrcImg0->GetImgHeight(pSrcImg0, IMG_PLANE_0);
        const INT32             nStride = pSrcImg0->GetImgStride(pSrcImg0, IMG_PLANE_0);
        const UINT8             *pOrg = (UINT8 *)pSrcImg0->GetImgPlanes(pSrcImg0, IMG_PLANE_0);
        const UINT8             *pSrc = (UINT8 *)pSrcImg1->GetImgPlanes(pSrcImg1, IMG_PLANE_0);
        UINT8                   *pDst = pSubtratcImg->GetImgPlanes(pSubtratcImg, IMG_PLANE_0);
        static UINT32           nLoopCnt = 0;
        
        for(j=0 ; j<nHeight ; j++)
        {
            nLocalPos = j * nStride;
            for(i=0 ; i<nWidth ; i++)
            {
                //pDst[nLocalPos] += ABSM(pSrc[nLocalPos] - pOrg[nLocalPos]);
                pDst[nLocalPos] = (UINT32)((D64)((pDst[nLocalPos] * (nLoopCnt - 1)) + ABSM(pSrc[nLocalPos] - pOrg[nLocalPos])) / (D64)nLoopCnt);
                nLocalPos++;
            }
        }
        
        nLoopCnt++;
    }

    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_SubtractAccbyThImg(OUT Img_T *pSubtratcImg, IN const Img_T *pSrcImg0, IN const Img_T *pSrcImg1, IN const UINT32 nTh)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pSubtratcImg, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pSrcImg0, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pSrcImg1, KAKAO_STAT_NULL_POINTER)
    
    {
        INT32                   i = 0, j = 0;
        INT32                   nLocalPos = 0;
        const INT32             nWidth = pSrcImg0->GetImgWidth(pSrcImg0, IMG_PLANE_0);
        const INT32             nHeight = pSrcImg0->GetImgHeight(pSrcImg0, IMG_PLANE_0);
        const INT32             nStride = pSrcImg0->GetImgStride(pSrcImg0, IMG_PLANE_0);
        const UINT8             *pOrg = (UINT8 *)pSrcImg0->GetImgPlanes(pSrcImg0, IMG_PLANE_0);
        const UINT8             *pSrc = (UINT8 *)pSrcImg1->GetImgPlanes(pSrcImg1, IMG_PLANE_0);
        UINT8                   *pDst = pSubtratcImg->GetImgPlanes(pSubtratcImg, IMG_PLANE_0);
        
        for(j=0 ; j<nHeight ; j++)
        {
            nLocalPos = j * nStride;
            for(i=0 ; i<nWidth ; i++)
            {
                if(255 != pDst[nLocalPos])
                {
                    UINT32          nLocalVal = ABSM(pSrc[nLocalPos] - pOrg[nLocalPos]);
                    
                    if(nLocalVal > nTh)
                        pDst[nLocalPos] = 255;
                }
                nLocalPos++;
            }
        }
    }
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_GetBinaryMap(IN Img_T *pSrcImg, IN const BinaryMapType nBinaryMap, OUT UINT8 **ppBinaryMap)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER)
    CHECK_PARAM_VALIDATION((nBinaryMap < IMG_BINARYMAP_MAX), KAKAO_STAT_INVALID_PARAM)
    
    if(NULL == (*ppBinaryMap = pSrcImg->GetImgBinaryMap(pSrcImg, nBinaryMap)))
        return nRet;
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_SetROI(IN OUT Img_T *pSrcImg, IN const ROI nROI)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER)
    
    pSrcImg->SetImgROI(pSrcImg, nROI.nStartX, nROI.nEndX, nROI.nStartY, nROI.nEndY);
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_UpdateImgSize(IN OUT Img_T *pSrcImg, IN const INT32 nWidth, IN const INT32 nHeight, IN const UINT32 *pStride, IN const ImgFormat nImgFormat)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    if(KAKAO_STAT_OK != (nRet = _Util_UpdateImgSize(pSrcImg, nWidth, nHeight, pStride, nImgFormat)))
        goto Error;
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_UpdateSize(IN OUT KISize *pSize, IN const INT32 nWidth, IN const INT32 nHeight, IN const UINT32 *pStride, IN const ImgFormat nImgFormat, IN const INT32 nNumofPlanes)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    if(KAKAO_STAT_OK != (nRet = _Util_UpdateSize(pSize, nWidth, nHeight, pStride, nImgFormat, nNumofPlanes)))
        goto Error;
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_SetImgDefaultParam(IN OUT Img_T *pSrcImg)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER)
 
    pSrcImg->GetImgWidth                = _Util_GetImgWidth;
    pSrcImg->SetImgWidth                = _Util_SetImgWidth;
    pSrcImg->GetImgHeight               = _Util_GetImgHeight;
    pSrcImg->SetImgHeight               = _Util_SetImgHeight;
    pSrcImg->GetImgStride               = _Util_GetImgStride;
    pSrcImg->SetImgStride               = _Util_SetImgStride;
    pSrcImg->GetImgSize                 = _Util_GetImgSize;
    pSrcImg->GetImgFormat               = _Util_GetImgFormat;
    pSrcImg->SetImgFormat               = _Util_SetImgFormat;
    pSrcImg->GetImgDepth                = _Util_GetImgDepth;
    pSrcImg->SetImgDepth                = _Util_SetImgDepth;
    pSrcImg->GetNumofImgPlanes          = _Util_GetImgNumofPlanes;
    pSrcImg->SetNumofImgPlanes          = _Util_SetImgNumofPlanes;
    
    pSrcImg->GetImgROI                  = _Util_GetImgROI;
    pSrcImg->SetImgROI                  = _Util_SetImgROI;
    
    pSrcImg->GetImgMBWiseMap            = _Util_GetImgMBWiseMap;
    pSrcImg->GetImgMBWiseSADMap         = _Util_GetImgMBWiseSADMap;
    pSrcImg->GetImgBinaryMap            = _Util_GetBinaryMap;
    pSrcImg->GetImgHistMap              = _Util_GetImgHistMap;
    pSrcImg->GetImgPlanes               = _Util_GetImgPlanes;
    pSrcImg->SetImgPlanes               = _Util_SetImgPlanes;

    pSrcImg->GetImgBins                 = _Util_GetImgBins;
    pSrcImg->SetImgBins                 = _Util_SetImgBins;

    pSrcImg->GetImgROIStatus            = _Util_GetImgROIStatus;
    pSrcImg->SetImgROIStatus            = _Util_SetImgROIStatus;

    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_Flip(IN OUT Img_T *pSrcImg)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   i = 0, j = 0;
    
    if(NULL == ((BaseImg_T *)pSrcImg)->pOneRowBuf)
        SAFEALLOC(((BaseImg_T *)pSrcImg)->pOneRowBuf, GET_PIXEL_SIZE(pSrcImg->GetImgDepth(pSrcImg)) * pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0), 32, UINT8);
    
    for(i=0 ; i<pSrcImg->GetNumofImgPlanes(pSrcImg) ; i++)
    {
        const INT32         nWidth = pSrcImg->GetImgWidth(pSrcImg, i);
        const INT32         nHeight = pSrcImg->GetImgHeight(pSrcImg, i);
        const INT32         nStride = pSrcImg->GetImgStride(pSrcImg, i);
        UINT8               *pSrc = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, i);
        UINT8               *pDst = pSrc + ((nHeight - 1) * nStride);
        UINT8               *pTmp = ((BaseImg_T *)pSrcImg)->pOneRowBuf;
        
        for(j=0 ; j<nHeight/2 ; j++)
        {
            _ImgProc_Util_Flip(pSrc, pDst, pTmp, nWidth);
            pSrc += nStride;
            pDst -= nStride;
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_SetVal(IN OUT Img_T *pSrcImg, IN const UINT8 nVal)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    BaseImg_T               *pBaseImg = ((BaseImg_T *)pSrcImg);
    const UINT32            nPixelSize = GET_PIXEL_SIZE(pSrcImg->GetImgDepth(pSrcImg));
    const INT32             nAlignedWidth = GET_ALIGNED_LENGTH(pBaseImg->nImgSize.nAllocatedWidth, MACRO_BLOCK_SIZE);
    const INT32             nAlignedHeight = GET_ALIGNED_LENGTH(pBaseImg->nImgSize.nAllocatedHeight, MACRO_BLOCK_SIZE);
    UINT32                  nImgSize = 0, nImgSizeUV = 0;
    INT32                   nStride = 0;
    
    switch(pSrcImg->GetImgFormat(pSrcImg))
    {
        case IMG_FORMAT_GRAY:
            nStride = MAKE_STRIDE(nAlignedWidth);
            
            MEMSET(pBaseImg->pImgData, nVal, nPixelSize * (nStride * MAKE_STRIDE(nAlignedHeight)));
            break;
        case IMG_FORMAT_RGB24:
        case IMG_FORMAT_BGR24:
            nStride = ((((3 * nAlignedWidth) + 2 * (3 * PADDING_SIZE)) + (PADDING_SIZE - 1)) & ~(PADDING_SIZE - 1));
            
            MEMSET(pBaseImg->pImgData, nVal, nPixelSize * (nStride * MAKE_STRIDE(nAlignedHeight)));
            break;
        case IMG_FORMAT_RGBA32:
        case IMG_FORMAT_BGRA32:
            nStride = ((((4 * nAlignedWidth) + 2 * (4 * PADDING_SIZE)) + (PADDING_SIZE - 1)) & ~(PADDING_SIZE - 1));
        
            MEMSET(pBaseImg->pImgData, nVal, nPixelSize * (nStride * MAKE_STRIDE(nAlignedHeight)));
        case IMG_FORMAT_HSV:
            nStride = MAKE_STRIDE(nAlignedWidth);
            
            MEMSET(pBaseImg->pImgData, nVal, nPixelSize * 3 * (nStride * MAKE_STRIDE(nAlignedHeight)));
            break;
        case IMG_FORMAT_HSL:
            nStride = MAKE_STRIDE(nAlignedWidth);
            
            MEMSET(pBaseImg->pImgData, nVal, nPixelSize * 3 * (nStride * MAKE_STRIDE(nAlignedHeight)));
            break;
        case IMG_FORMAT_I420:
            nStride = MAKE_STRIDE(nAlignedWidth);
            
            nImgSize = nPixelSize * (nStride * MAKE_STRIDE(nAlignedHeight));
            nImgSizeUV = nPixelSize * (pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_1) * MAKE_STRIDE(nAlignedHeight/2));
            
            MEMSET(pBaseImg->pImgData, nVal, nImgSize);
            MEMSET((HANDLE)((UINT8 *)pBaseImg->pImgData + nImgSize), nVal, (2 * nImgSizeUV));
            break;
        default:
            break;
    }
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_SetVal_Plane(IN OUT Img_T *pSrcImg, IN const UINT8 nVal, IN const UINT32 nPlaneIdx)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    BaseImg_T               *pBaseImg = ((BaseImg_T *)pSrcImg);
    const UINT32            nPixelSize = GET_PIXEL_SIZE(pSrcImg->GetImgDepth(pSrcImg));
    const INT32             nAlignedWidth = GET_ALIGNED_LENGTH(pBaseImg->nImgSize.nAllocatedWidth, MACRO_BLOCK_SIZE);
    const INT32             nAlignedHeight = GET_ALIGNED_LENGTH(pBaseImg->nImgSize.nAllocatedHeight, MACRO_BLOCK_SIZE);
    UINT32                  nImgSize = 0, nImgSizeUV = 0;
    INT32                   nStride = 0;
    
    switch(pSrcImg->GetImgFormat(pSrcImg))
    {
        case IMG_FORMAT_GRAY:
            nStride = MAKE_STRIDE(nAlignedWidth);
            
            MEMSET(pBaseImg->pImgData, nVal, nPixelSize * (nStride * MAKE_STRIDE(nAlignedHeight)));
            break;
        case IMG_FORMAT_RGB24:
        case IMG_FORMAT_BGR24:
            nStride = ((((3 * nAlignedWidth) + 2 * (3 * PADDING_SIZE)) + (PADDING_SIZE - 1)) & ~(PADDING_SIZE - 1));
            
            MEMSET(pBaseImg->pImgData, nVal, nPixelSize * (nStride * MAKE_STRIDE(nAlignedHeight)));
            break;
        case IMG_FORMAT_RGBA32:
        case IMG_FORMAT_BGRA32:
            nStride = ((((4 * nAlignedWidth) + 2 * (4 * PADDING_SIZE)) + (PADDING_SIZE - 1)) & ~(PADDING_SIZE - 1));
            
            MEMSET(pBaseImg->pImgData, nVal, nPixelSize * (nStride * MAKE_STRIDE(nAlignedHeight)));
        case IMG_FORMAT_HSV:
        case IMG_FORMAT_HSL:
            nStride = MAKE_STRIDE(nAlignedWidth);
            nImgSize = nPixelSize * (nStride * MAKE_STRIDE(nAlignedHeight));

            MEMSET((HANDLE)((UINT8 *)pBaseImg->pImgData + (nPlaneIdx * nImgSize)), nVal, nImgSize);
            break;
        case IMG_FORMAT_I420:
            nStride = MAKE_STRIDE(nAlignedWidth);
            
            nImgSize = nPixelSize * (nStride * MAKE_STRIDE(nAlignedHeight));
            nImgSizeUV = nPixelSize * (pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_1) * MAKE_STRIDE(nAlignedHeight/2));
            
            if(IMG_PLANE_0 == nPlaneIdx)
                MEMSET(pBaseImg->pImgData, nVal, nImgSize);
            else if(IMG_PLANE_1 == nPlaneIdx)
                MEMSET((HANDLE)((UINT8 *)pBaseImg->pImgData + nImgSize), nVal, nImgSizeUV);
            else if(IMG_PLANE_2 == nPlaneIdx)
                MEMSET((HANDLE)((UINT8 *)pBaseImg->pImgData + nImgSize + nImgSizeUV), nVal, nImgSizeUV);
            break;
        default:
            break;
    }
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_CleanImg(IN OUT Img_T *pSrcImg)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    BaseImg_T               *pBaseImg = ((BaseImg_T *)pSrcImg);
    const UINT32            nPixelSize = GET_PIXEL_SIZE(pSrcImg->GetImgDepth(pSrcImg));
    const INT32             nAlignedWidth = GET_ALIGNED_LENGTH(pBaseImg->nImgSize.nAllocatedWidth, MACRO_BLOCK_SIZE);
    const INT32             nAlignedHeight = GET_ALIGNED_LENGTH(pBaseImg->nImgSize.nAllocatedHeight, MACRO_BLOCK_SIZE);
    UINT32                  nImgSize = 0, nImgSizeUV = 0;
    INT32                   nStride = 0;
    
    switch(pSrcImg->GetImgFormat(pSrcImg))
    {
        case IMG_FORMAT_GRAY:
            nStride = MAKE_STRIDE(nAlignedWidth);
            
            MEMSET(pBaseImg->pImgData, 0, nPixelSize * (nStride * MAKE_STRIDE(nAlignedHeight)));
            break;
        case IMG_FORMAT_RGB24:
        case IMG_FORMAT_BGR24:
            nStride = ((((3 * nAlignedWidth) + 2 * (3 * PADDING_SIZE)) + (PADDING_SIZE - 1)) & ~(PADDING_SIZE - 1));
            
            MEMSET(pBaseImg->pImgData, 0, nPixelSize * (nStride * MAKE_STRIDE(nAlignedHeight)));
            break;
        case IMG_FORMAT_RGBA32:
        case IMG_FORMAT_BGRA32:
            nStride = ((((4 * nAlignedWidth) + 2 * (4 * PADDING_SIZE)) + (PADDING_SIZE - 1)) & ~(PADDING_SIZE - 1));
            
            MEMSET(pBaseImg->pImgData, 0, nPixelSize * (nStride * MAKE_STRIDE(nAlignedHeight)));
        case IMG_FORMAT_HSV:
            nStride = MAKE_STRIDE(nAlignedWidth);
            
            MEMSET(pBaseImg->pImgData, 0, nPixelSize * 3 * (nStride * MAKE_STRIDE(nAlignedHeight)));
            break;
        case IMG_FORMAT_HSL:
            nStride = MAKE_STRIDE(nAlignedWidth);
            
            MEMSET(pBaseImg->pImgData, 0, nPixelSize * 3 * (nStride * MAKE_STRIDE(nAlignedHeight)));
            break;
        case IMG_FORMAT_I420:
            nStride = MAKE_STRIDE(nAlignedWidth);
            
            nImgSize = nPixelSize * (nStride * MAKE_STRIDE(nAlignedHeight));
            nImgSizeUV = nPixelSize * (pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_1) * MAKE_STRIDE(nAlignedHeight/2));
            
            MEMSET(pBaseImg->pImgData, 0, nImgSize);
            MEMSET((HANDLE)((UINT8 *)pBaseImg->pImgData + nImgSize), 128, (2 * nImgSizeUV));
            break;
        default:
            break;
    }
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_MakeIntegralImg(IN const Img_T *pSrcImg, OUT Img_T *pIntegralImg, OUT Img_T *pSqIntegralImg)
{
    Kakao_Status                nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pIntegralImg, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pSqIntegralImg, KAKAO_STAT_NULL_POINTER)
    
    {
        const INT32             nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
        const INT32             nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
        const INT32             nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
        UINT8                   *pSrc = NULL;
        UINT32                  *pIntegralSrc = NULL;
        UINT32                  *pSqIntegralSrc = NULL;
        UINT32                  nCurrVal = 0;
        UINT32                  nRowSum = 0;
        UINT32                  nRowSqSum = 0;
        UINT32                  nLocalPos = 0;
        INT32                   i = 0, j = 0;
        
        // Integral Image & Square Integral Image
        pSrc = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
        pIntegralSrc = (UINT32 *)(pIntegralImg->GetImgPlanes(pIntegralImg, IMG_PLANE_0));
        pSqIntegralSrc = (UINT32 *)(pSqIntegralImg->GetImgPlanes(pSqIntegralImg, IMG_PLANE_0));
        for(i=0 ; i<nWidth ; i++)
        {
            nCurrVal = pSrc[i];
            nRowSum += nCurrVal;
            nRowSqSum += nCurrVal * nCurrVal;
            pIntegralSrc[i] = nRowSum;
            pSqIntegralSrc[i] = nRowSqSum;
        }
        
        for(j=1 ; j<nHeight ; j++)
        {
            UINT32              *pPrevLineIntegralImg = NULL;
            UINT32              *pPrevLineSqIntegralImg = NULL;
            
            nRowSum = 0;
            nRowSqSum = 0;
            nLocalPos = j * nStride;
            
            pPrevLineIntegralImg = &(pIntegralSrc[nLocalPos - nStride]);
            pPrevLineSqIntegralImg = &(pSqIntegralSrc[nLocalPos - nStride]);
            
            for(i=0 ; i<nWidth ; i++)
            {
                nCurrVal = pSrc[nLocalPos];
                nRowSum += nCurrVal;
                nRowSqSum += nCurrVal * nCurrVal;
                pIntegralSrc[nLocalPos] = nRowSum + pPrevLineIntegralImg[i];
                pSqIntegralSrc[nLocalPos] = nRowSqSum + pPrevLineSqIntegralImg[i];
                nLocalPos++;
            }
        }
    }

    nRet = KAKAO_STAT_OK;

    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_MergeImgs_WithBinaryMap(IN const Img_T *pSrcImg0, IN OUT Img_T *pSrcImg1, IN OUT Img_T *pDstImg, UINT8 *pMaskMap)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pSrcImg0, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pSrcImg1, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pMaskMap, KAKAO_STAT_NULL_POINTER)
    
    {
        const INT32             nWidth = pSrcImg0->GetImgWidth(pSrcImg0, IMG_PLANE_0);
        const INT32             nHeight = pSrcImg0->GetImgHeight(pSrcImg0, IMG_PLANE_0);
        INT32                   nStride = 0;
        UINT8                   *pSrc0 = NULL, *pSrc1 = NULL;
        UINT8                   *pDst = NULL;
        
        pSrc0 = (UINT8 *)pSrcImg0->GetImgPlanes(pSrcImg0, IMG_PLANE_0);
        pSrc1 = (UINT8 *)pSrcImg1->GetImgPlanes(pSrcImg1, IMG_PLANE_0);
        pDst = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0);
        nStride = pSrcImg0->GetImgStride(pSrcImg0, IMG_PLANE_0);
        _Util_MergeImgs_WithBinaryMap(pSrc0, pSrc1, pDst, pMaskMap, nWidth, nHeight, nStride);
    }

    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API INT32 Kakao_ImgProc_Util_Cal_SAD16x16(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, IN const INT32 nStride)
{
    INT32                   nRet = (INT32)KAKAO_STAT_FAIL;

    CHECK_POINTER_VALIDATION(pSrc0, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrc1, KAKAO_STAT_NULL_POINTER);

    nRet = _Util_Cal_SAD16x16(pSrc0, pSrc1, nStride);
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API INT32 Kakao_ImgProc_Util_Cal_MBWiseSAD16x16_Plane(IN const Img_T *pSrcImg0, IN const Img_T *pSrcImg1, OUT Img_T *pDstImg, IN const UINT32 nPlaneIdx)
{
    INT32                   nRet = (INT32)KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pSrcImg0, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg1, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER);
    
    Kakao_ImgProc_Util_CreateMBwiseSADMap(pDstImg);
    
    {
        const UINT8         *pSrc0 = (UINT8 *)pSrcImg0->GetImgPlanes(pSrcImg0, nPlaneIdx);
        const UINT8         *pSrc1 = (UINT8 *)pSrcImg1->GetImgPlanes(pSrcImg1, nPlaneIdx);
        const INT32         nWidth = pSrcImg0->GetImgWidth(pSrcImg0, nPlaneIdx);
        const INT32         nHeight = pSrcImg0->GetImgHeight(pSrcImg0, nPlaneIdx);
        const INT32         nStride = pSrcImg0->GetImgStride(pSrcImg0, nPlaneIdx);
        INT32               *pMBwiseSADMap = pDstImg->GetImgMBWiseSADMap(pDstImg);
        INT32               i = 0, j = 0, k = 0;
        
        for(j=0 ; j<nHeight ; j+=MACRO_BLOCK_SIZE)
        {
            for(i=0 ; i<nWidth ; i+=MACRO_BLOCK_SIZE)
            {
                const INT32 nOffset = j * nStride + i;
                const UINT8 *pLocalSrc0 = pSrc0 + nOffset;
                const UINT8 *pLocalSrc1 = pSrc1 + nOffset;
            
                pMBwiseSADMap[k++] = _Util_Cal_SAD16x16(pLocalSrc0, pLocalSrc1, nStride);
            }
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API INT32 Kakao_ImgProc_Util_Cal_Accum16x16(UINT8 *pSrc, INT32 nStride)
{
    INT32                   nRet = (INT32)KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pSrc, KAKAO_STAT_NULL_POINTER)
    
    nRet = _Util_Cal_Accum16x16(pSrc, nStride);
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API INT32 Kakao_ImgProc_Util_Cal_Mean16x16(UINT8 *pSrc, INT32 nStride)
{
    return Kakao_ImgProc_Util_Cal_Accum16x16(pSrc, nStride) / 256;
}


KAKAO_EXPORT_API INT32 Kakao_ImgProc_Util_Cal_Variance16x16(IN UINT8 *pSrc, IN const INT32 nMean, IN const INT32 nStride)
{
    INT32                   nRet = (INT32)KAKAO_STAT_FAIL;

    CHECK_POINTER_VALIDATION(pSrc, KAKAO_STAT_NULL_POINTER)

    nRet = _Util_Cal_Variance16x16(pSrc, nMean, nStride);
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_FramewiseToMBwise(IN OUT Img_T *pSrcImg, IN const BinaryMapType nBinaryMap, IN const UINT32 nThreshold)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER)
    
    nRet = _Util_FramewiseToMBwise(pSrcImg, nBinaryMap, nThreshold);
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_ScaleBinaryMap(IN OUT Img_T *pSrcImg, IN const BinaryMapType nBinaryMap, IN const UINT32 nScaleFactor)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER);
    
    nRet = _Util_ScaleBinaryMap(pSrcImg, nBinaryMap, nScaleFactor);
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_Get_CPUCores(OUT INT32 *pNumofCPUCore)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pNumofCPUCore, KAKAO_STAT_NULL_POINTER)
    
    *pNumofCPUCore = _Util_Get_CpuCores();
    
    nRet = KAKAO_STAT_OK;

    return nRet;

Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_Get_CPUFrequency(OUT INT32 *pCPUFrequency)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pCPUFrequency, KAKAO_STAT_NULL_POINTER)

    // Return CPU Frequency as Unit of MHz
    *pCPUFrequency = (INT32)(_Util_Get_CpuFrequency());
    
    nRet = KAKAO_STAT_OK;

    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_Get_ImgFormatIdx(IN const ImgFormat nImgFormat, OUT INT32 *pImgFormatIdx)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    CHECK_POINTER_VALIDATION(pImgFormatIdx, KAKAO_STAT_NULL_POINTER)

    switch(nImgFormat)
    {
        case IMG_FORMAT_GRAY:
            *pImgFormatIdx = 0;
            break;
        case IMG_FORMAT_RGB24:
            *pImgFormatIdx = 1;
            break;
        case IMG_FORMAT_BGR24:
            *pImgFormatIdx = 2;
            break;
        case IMG_FORMAT_RGBA32:
            *pImgFormatIdx = 3;
            break;
        case IMG_FORMAT_BGRA32:
            *pImgFormatIdx = 4;
            break;
        case IMG_FORMAT_I420:
            *pImgFormatIdx = 5;
            break;
        case IMG_FORMAT_I422:
            *pImgFormatIdx = 6;
            break;
        case IMG_FORMAT_HSV:
            *pImgFormatIdx = 7;
            break;
        case IMG_FORMAT_HSL:
            *pImgFormatIdx = 8;
            break;
        case IMG_FORMAT_NV12:
            *pImgFormatIdx = 9;
            break;
        case IMG_FORMAT_NV21:
            *pImgFormatIdx = 10;
            break;
        case IMG_FORMAT_UYVY:
            *pImgFormatIdx = 11;
            break;
        default:
            *pImgFormatIdx = 0;
            break;
    }

    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


#if ENABLE_ISET(CPU_ARMX | ISET_NEON)
static INT32 _Util_Cal_SAD16x16_NEON(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, IN const INT32 nStride)
{
    uint16x8_t              nAccumLane;
    uint8x16_t              nCurrSrcLane, nPrevSrcLane, nAbsLane;
    uint64x2_t              q3;
    uint32x2_t              d5;
    INT32                   n = 0;

    nAccumLane = vdupq_n_u16(0);
    
    for(n=3 ; n>=0 ; n--)
    {
        UINT32          nLine_2 = nStride;
        UINT32          nLine_3 = 2 * nStride;
        UINT32          nLine_4 = 3 * nStride;
        UINT32          nLine_5 = 4 * nStride;
        
        nCurrSrcLane = vld1q_u8(pSrc1);
        nPrevSrcLane = vld1q_u8(pSrc0);
        nAbsLane = vabdq_u8(nCurrSrcLane, nPrevSrcLane);
        nAccumLane = vpadalq_u8(nAccumLane, nAbsLane);
        
        nCurrSrcLane = vld1q_u8(pSrc1 + nLine_2);
        nPrevSrcLane = vld1q_u8(pSrc0 + nLine_2);
        nAbsLane = vabdq_u8(nCurrSrcLane, nPrevSrcLane);
        nAccumLane = vpadalq_u8(nAccumLane, nAbsLane);
        
        nCurrSrcLane = vld1q_u8(pSrc1 + nLine_3);
        nPrevSrcLane = vld1q_u8(pSrc0 + nLine_3);
        nAbsLane = vabdq_u8(nCurrSrcLane, nPrevSrcLane);
        nAccumLane = vpadalq_u8(nAccumLane, nAbsLane);
        
        nCurrSrcLane = vld1q_u8(pSrc1 + nLine_4);
        nPrevSrcLane = vld1q_u8(pSrc0 + nLine_4);
        nAbsLane = vabdq_u8(nCurrSrcLane, nPrevSrcLane);
        nAccumLane = vpadalq_u8(nAccumLane, nAbsLane);
        
        pSrc0 += nLine_5;
        pSrc1 += nLine_5;
    }
    
    q3 = vpaddlq_u32(vpaddlq_u16(nAccumLane));
    d5 = vadd_u32(vreinterpret_u32_u64(vget_low_u64(q3)), vreinterpret_u32_u64(vget_high_u64(q3)));
    
    return vget_lane_u32(d5, 0);
}


static INT32 _Util_Cal_Accum16x16_NEON(UINT8 *pSrc, INT32 nStride)
{
    uint16x8_t              nAccumLane;
    uint8x16_t              nCurrLane;
    uint64x2_t              q3;
    uint32x2_t              d5;
    INT32                   n = 0;
    
    nAccumLane = vdupq_n_u16(0);
    
    for(n=3 ; n>=0 ; n--)
    {
        UINT32          nLine_2 = nStride;
        UINT32          nLine_3 = 2 * nStride;
        UINT32          nLine_4 = 3 * nStride;
        UINT32          nLine_5 = 4 * nStride;
        
        nCurrLane = vld1q_u8(pSrc);
        nAccumLane = vpadalq_u8(nAccumLane, nCurrLane);
        
        nCurrLane = vld1q_u8(pSrc + nLine_2);
        nAccumLane = vpadalq_u8(nAccumLane, nCurrLane);
        
        nCurrLane = vld1q_u8(pSrc + nLine_3);
        nAccumLane = vpadalq_u8(nAccumLane, nCurrLane);
        
        nCurrLane = vld1q_u8(pSrc + nLine_4);
        nAccumLane = vpadalq_u8(nAccumLane, nCurrLane);
        
        pSrc += nLine_5;
    }
    
    q3 = vpaddlq_u32(vpaddlq_u16(nAccumLane));
    d5 = vadd_u32(vreinterpret_u32_u64(vget_low_u64(q3)), vreinterpret_u32_u64(vget_high_u64(q3)));
    
    return vget_lane_u32(d5, 0);
}


static INT32 _Util_Cal_Mean16x16_NEON(UINT8 *pSrc, INT32 nStride)
{
    return _Util_Cal_Accum16x16_NEON(pSrc, nStride) / 256;
}


static INT32 _Util_Cal_Variance16x16_NEON(IN UINT8 *pSrc, IN const INT32 nMean, IN const INT32 nStride)
{
    uint16x8_t              nAccumLane;
    uint8x16_t              nCurrLane, nAbsLane, nMeanLane;
    uint64x2_t              q3;
    uint32x2_t              d5;
    INT32                   n = 0;
    
    nMeanLane = vdupq_n_u8(nMean);
    nAccumLane = vdupq_n_u16(0);
    
    for(n=3 ; n>=0 ; n--)
    {
        UINT32          nLine_2 = nStride;
        UINT32          nLine_3 = 2 * nStride;
        UINT32          nLine_4 = 3 * nStride;
        UINT32          nLine_5 = 4 * nStride;
        
        nCurrLane = vld1q_u8(pSrc);
        nAbsLane = vabdq_u8(nCurrLane, nMeanLane);
        nAccumLane = vpadalq_u8(nAccumLane, nAbsLane);
        
        nCurrLane = vld1q_u8(pSrc + nLine_2);
        nAbsLane = vabdq_u8(nCurrLane, nMeanLane);
        nAccumLane = vpadalq_u8(nAccumLane, nAbsLane);
        
        nCurrLane = vld1q_u8(pSrc + nLine_3);
        nAbsLane = vabdq_u8(nCurrLane, nMeanLane);
        nAccumLane = vpadalq_u8(nAccumLane, nAbsLane);
        
        nCurrLane = vld1q_u8(pSrc + nLine_4);
        nAbsLane = vabdq_u8(nCurrLane, nMeanLane);
        nAccumLane = vpadalq_u8(nAccumLane, nAbsLane);
        
        pSrc += nLine_5;
    }
    
    q3 = vpaddlq_u32(vpaddlq_u16(nAccumLane));
    d5 = vadd_u32(vreinterpret_u32_u64(vget_low_u64(q3)), vreinterpret_u32_u64(vget_high_u64(q3)));
    
    return vget_lane_u32(d5, 0) / 256;
}


static Kakao_Status _Util_FramewiseToMBwise_NEON(IN OUT Img_T *pSrcImg, IN const BinaryMapType nBinaryMap, IN const UINT32 nThreshold)
{
    INT32                   i = 0, j = 0;
    INT32                   nLocalPos = 0;
    INT32                   nMBPos = 0;
    INT32                   nWidth = 0;
    INT32                   nHeight = 0;
    INT32                   nStride = 0;
    INT32                   nTh = (100 * (INT32)nThreshold) / 255;
    UINT8                   *pBinaryMap = NULL;
    UINT8                   *pMBwiseMap = NULL;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER)

    if(NULL == (pBinaryMap = pSrcImg->GetImgBinaryMap(pSrcImg, nBinaryMap)))
        return nRet;
    
    pMBwiseMap = pSrcImg->GetImgMBWiseMap(pSrcImg) + (PADDING_SIZE >> 4);
    
    // Kakao_ImgProc_Util_FramewiseToMBwise
    nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    for(j=0 ; j<nHeight ; j+=MACRO_BLOCK_SIZE)
    {
        nLocalPos = j * nStride;
        
        for(i=0 ; i<nWidth ; i+=MACRO_BLOCK_SIZE)
        {
            INT32           nAccum16x16 = 0;
            
            nMBPos = (nLocalPos >> 8);
            
            nAccum16x16 = _Util_Cal_Accum16x16_NEON(&(pBinaryMap[nLocalPos]), nStride);
            
            if(nAccum16x16 > nTh)
                pMBwiseMap[nMBPos] = INDICATE_VALUE;
            else
                pMBwiseMap[nMBPos] = 0;
            
            nLocalPos += MACRO_BLOCK_SIZE;
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _Util_ScaleBinaryMap_NEON(IN OUT Img_T *pSrcImg, IN const BinaryMapType nBinaryMap, IN const UINT32 nScaleFactor)
{
    INT32                   i = 0, j = 0;
    INT32                   nWidth = 0;
    INT32                   nHeight = 0;
    INT32                   nStride = 0;
    UINT8                   *pBinaryMap = NULL;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER)
    
    if(NULL == (pBinaryMap = pSrcImg->GetImgBinaryMap(pSrcImg, nBinaryMap)))
        return nRet;
    
    uint8x16_t      nMask;
    
    nMask = vdupq_n_u8(nScaleFactor);
    
    nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    for(j=0 ; j<nHeight ; j++)
    {
        UINT8           *pTmpSrc = &(pBinaryMap[j * nStride]);
        
        for(i=0 ; i<nWidth ; i+=MACRO_BLOCK_SIZE)
        {
            uint8x16_t          nCurrLane;
            
            nCurrLane = vld1q_u8(pTmpSrc);
            
            vst1q_u8(pTmpSrc, vmulq_u8(nCurrLane, nMask));
            
            pTmpSrc += MACRO_BLOCK_SIZE;
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _Util_MergeImgs_WithBinaryMap_OneRow_NEON(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, OUT UINT8 *pDst, IN const UINT8 *pMaskMap, IN const INT32 nWidth, IN const INT32 nStride)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   i = 0;
    INT32                   nLoopCnt = 0;
    INT32                   nOffset = 0;
    uint8x16_t              nNum2 = vdupq_n_u8(2);
    
    nLoopCnt = nWidth >> 4;
    
    for(i=nLoopCnt ; i>0 ; i--)
    {
        uint8x16_t          nSrc0, nSrc1, nDst, nMaskMap, nTmpMaskMap, nNotMaskMap;
        uint8x16_t          nMulSrc0, nMulSrc1, nOrrDst;
        
        nSrc0 = vld1q_u8(pSrc0 + nOffset);
        nSrc1 = vld1q_u8(pSrc1 + nOffset);
        nDst = vld1q_u8(pDst + nOffset);
        nMaskMap = vld1q_u8(pMaskMap + nOffset);
        
        nTmpMaskMap = vmvnq_u8(nMaskMap);
        nNotMaskMap = vaddq_u8(nTmpMaskMap, nNum2);

        nMulSrc0 = vmulq_u8(nSrc0, nMaskMap);
        nMulSrc1 = vmulq_u8(nSrc1, nNotMaskMap);
        nOrrDst = vorrq_u8(nMulSrc0, nMulSrc1);
        
        vst1q_u8(pDst + nOffset, nOrrDst);
        nOffset += 16;
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API INT32 Kakao_ImgProc_Util_Print_8x8(uint8x8_t Src)
{
    printf("%03d %03d %03d %03d %03d %03d %03d %03d\n",
           vget_lane_u8(Src, 0), vget_lane_u8(Src, 1), vget_lane_u8(Src, 2), vget_lane_u8(Src, 3),
           vget_lane_u8(Src, 4), vget_lane_u8(Src, 5), vget_lane_u8(Src, 6), vget_lane_u8(Src, 7));
    
    return 0;
}


KAKAO_EXPORT_API INT32 Kakao_ImgProc_Util_Print_8x16(uint8x16_t Src)
{
    printf("%03d %03d %03d %03d %03d %03d %03d %03d %03d %03d %03d %03d %03d %03d %03d %03d\n",
           vgetq_lane_u8(Src, 0), vgetq_lane_u8(Src, 1), vgetq_lane_u8(Src, 2), vgetq_lane_u8(Src, 3),
           vgetq_lane_u8(Src, 4), vgetq_lane_u8(Src, 5), vgetq_lane_u8(Src, 6), vgetq_lane_u8(Src, 7),
           vgetq_lane_u8(Src, 8), vgetq_lane_u8(Src, 9), vgetq_lane_u8(Src, 10), vgetq_lane_u8(Src, 11),
           vgetq_lane_u8(Src, 12), vgetq_lane_u8(Src, 13), vgetq_lane_u8(Src, 14), vgetq_lane_u8(Src, 15));
    
    return 0;
}


KAKAO_EXPORT_API INT32 Kakao_ImgProc_Util_Print_16x4(uint16x4_t Src)
{
    printf("%03d %03d %03d %03d\n",
           vget_lane_u16(Src, 0), vget_lane_u16(Src, 1), vget_lane_u16(Src, 2), vget_lane_u16(Src, 3));
    
    return 0;
}


KAKAO_EXPORT_API INT32 Kakao_ImgProc_Util_Print_16x8(uint16x8_t Src)
{
    printf("%03d %03d %03d %03d %03d %03d %03d %03d\n",
           vgetq_lane_u16(Src, 0), vgetq_lane_u16(Src, 1), vgetq_lane_u16(Src, 2), vgetq_lane_u16(Src, 3),
           vgetq_lane_u16(Src, 4), vgetq_lane_u16(Src, 5), vgetq_lane_u16(Src, 6), vgetq_lane_u16(Src, 7));
    
    return 0;
}


KAKAO_EXPORT_API INT32 Kakao_ImgProc_Util_Print_U32x4(uint32x4_t Src)
{
    printf("%03d %03d %03d %03d\n",
           vgetq_lane_u32(Src, 0), vgetq_lane_u32(Src, 1), vgetq_lane_u32(Src, 2), vgetq_lane_u32(Src, 3));
    
    return 0;
}


KAKAO_EXPORT_API INT32 Kakao_ImgProc_Util_Print_S32x4(int32x4_t Src)
{
    printf("%03d %03d %03d %03d\n",
           vgetq_lane_s32(Src, 0), vgetq_lane_s32(Src, 1), vgetq_lane_s32(Src, 2), vgetq_lane_s32(Src, 3));
    
    return 0;
}

#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_SSE4) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_SSE4)
static INT32 _Util_Cal_SAD16x16_SSE4(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, IN const INT32 nStride)
{
    INT32           nVal = 0;
    INT32           n = 0;
    
    for(n=15 ; n>=0 ; n--)
    {
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        
        pSrc0 += (nStride - 16);
        pSrc1 += (nStride - 16);
    }
    
    return nVal;
}


static INT32 _Util_Cal_Accum16x16_SSE4(UINT8 *pSrc, INT32 nStride)
{
    INT32           nVal = 0;
    INT32           n = 0;
    
    for(n=15 ; n>=0 ; n--)
    {
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        
        pSrc += (nStride - 16);
    }
    
    return nVal;
}


static INT32 _Util_Cal_Mean16x16_SSE4(IN UINT8 *pSrc, IN const INT32 nStride)
{
    INT32           nVal = 0;
    INT32           n = 0;
    
    for(n=15 ; n>=0 ; n--)
    {
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        
        pSrc += (nStride - 16);
    }
    
    return nVal / 256;
}


static INT32 _Util_Cal_Variance16x16_SSE4(IN UINT8* pSrc, IN const INT32 nMean, IN const INT32 nStride)
{
    INT32           nVal = 0;
    INT32           n = 0;
    
    for(n=15 ; n>=0 ; n--)
    {
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        
        pSrc += (nStride - 16);
    }
    
    return nVal / 256;
}


static Kakao_Status _Util_FramewiseToMBwise_SSE4(IN OUT Img_T *pSrcImg, IN const BinaryMapType nBinaryMap, IN const UINT32 nThreshold)
{
    INT32                   i = 0, j = 0, m = 0, n = 0;
    INT32                   nLocalPos = 0, nInPos = 0;
    INT32                   nMBPos = 0;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    INT32                   nTh = (100 * (INT32)nThreshold) / 255;
    INT32                   nCountFlag = 0;
    UINT8                   *pBinaryMap = NULL;
    UINT8                   *pMBwiseMap = NULL;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER)
    
    if(NULL == (pBinaryMap = pSrcImg->GetImgBinaryMap(pSrcImg, nBinaryMap)))
        return nRet;
    
    pMBwiseMap = pSrcImg->GetImgMBWiseMap(pSrcImg) + (PADDING_SIZE >> 4);
    
    // Kakao_ImgProc_Util_FramewiseToMBwise
    for(j=0 ; j<nHeight ; j+=MACRO_BLOCK_SIZE)
    {
        for(i=0 ; i<nWidth ; i+=MACRO_BLOCK_SIZE)
        {
            nCountFlag = nTh;
            
            nLocalPos = (j * nStride) + i;
            nMBPos = (nLocalPos >> 8);
            
            for(n=0 ; n<MACRO_BLOCK_SIZE ; n++)
            {
                nInPos = nLocalPos + (n * nStride);
                
                for(m=0 ; m<MACRO_BLOCK_SIZE ; m++)
                    nCountFlag -= !(!pBinaryMap[nInPos++]);
            }
            
            if(0 > nCountFlag)
                pMBwiseMap[nMBPos] = INDICATE_VALUE;
            else
                pMBwiseMap[nMBPos] = 0;
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _Util_ScaleBinaryMap_SSE4(IN OUT Img_T *pSrcImg, IN const BinaryMapType nBinaryMap, IN const UINT32 nScaleFactor)
{
    INT32                   i = 0, j = 0;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    INT32                   nLocalPos = 0;
    UINT8                   *pBinaryMap = NULL;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER)
    
    if(NULL == (pBinaryMap = pSrcImg->GetImgBinaryMap(pSrcImg, nBinaryMap)))
        return nRet;
    
    for(j=0 ; j<nHeight ; j++)
    {
        nLocalPos = j * nStride;
        for(i=0 ; i<nWidth ; i++)
            pBinaryMap[nLocalPos++] = pBinaryMap[nLocalPos] ? nScaleFactor : 0;
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _Util_MergeImgs_WithBinaryMap_OneRow_SSE4(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, OUT UINT8 *pDst, IN const UINT8 *pMaskMap, IN const INT32 nWidth, IN const INT32 nStride)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   i = 0;
    
    for(i=0 ; i<nWidth ; i++)
        pDst[i] = pMaskMap[i] ? pSrc0[i] : pSrc1[i];
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_SSE3) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_SSE3)
static INT32 _Util_Cal_SAD16x16_SSE3(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, IN const INT32 nStride)
{
    INT32           nVal = 0;
    INT32           n = 0;
    
    for(n=15 ; n>=0 ; n--)
    {
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        
        pSrc0 += (nStride - 16);
        pSrc1 += (nStride - 16);
    }
    
    return nVal;
}


static INT32 _Util_Cal_Accum16x16_SSE3(UINT8 *pSrc, INT32 nStride)
{
    INT32           nVal = 0;
    INT32           n = 0;
    
    for(n=15 ; n>=0 ; n--)
    {
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        
        pSrc += (nStride - 16);
    }
    
    return nVal;
}


static INT32 _Util_Cal_Mean16x16_SSE3(IN UINT8 *pSrc, IN const INT32 nStride)
{
    INT32           nVal = 0;
    INT32           n = 0;
    
    for(n=15 ; n>=0 ; n--)
    {
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        
        pSrc += (nStride - 16);
    }
    
    return nVal / 256;
}


static INT32 _Util_Cal_Variance16x16_SSE3(IN UINT8* pSrc, IN const INT32 nMean, IN const INT32 nStride)
{
    INT32           nVal = 0;
    INT32           n = 0;
    
    for(n=15 ; n>=0 ; n--)
    {
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        
        pSrc += (nStride - 16);
    }
    
    return nVal / 256;
}


static Kakao_Status _Util_FramewiseToMBwise_SSE3(IN OUT Img_T *pSrcImg, IN const BinaryMapType nBinaryMap, IN const UINT32 nThreshold)
{
    INT32                   i = 0, j = 0, m = 0, n = 0;
    INT32                   nLocalPos = 0, nInPos = 0;
    INT32                   nMBPos = 0;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    INT32                   nTh = (100 * (INT32)nThreshold) / 255;
    INT32                   nCountFlag = 0;
    UINT8                   *pBinaryMap = NULL;
    UINT8                   *pMBwiseMap = NULL;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER)
    
    if(NULL == (pBinaryMap = pSrcImg->GetImgBinaryMap(pSrcImg, nBinaryMap)))
        return nRet;
    
    pMBwiseMap = pSrcImg->GetImgMBWiseMap(pSrcImg) + (PADDING_SIZE >> 4);
    
    // Kakao_ImgProc_Util_FramewiseToMBwise
    for(j=0 ; j<nHeight ; j+=MACRO_BLOCK_SIZE)
    {
        for(i=0 ; i<nWidth ; i+=MACRO_BLOCK_SIZE)
        {
            nCountFlag = nTh;
            
            nLocalPos = (j * nStride) + i;
            nMBPos = (nLocalPos >> 8);
            
            for(n=0 ; n<MACRO_BLOCK_SIZE ; n++)
            {
                nInPos = nLocalPos + (n * nStride);
                
                for(m=0 ; m<MACRO_BLOCK_SIZE ; m++)
                    nCountFlag -= !(!pBinaryMap[nInPos++]);
            }
            
            if(0 > nCountFlag)
                pMBwiseMap[nMBPos] = INDICATE_VALUE;
            else
                pMBwiseMap[nMBPos] = 0;
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _Util_ScaleBinaryMap_SSE3(IN OUT Img_T *pSrcImg, IN const BinaryMapType nBinaryMap, IN const UINT32 nScaleFactor)
{
    INT32                   i = 0, j = 0;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    INT32                   nLocalPos = 0;
    UINT8                   *pBinaryMap = NULL;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER)
    
    if(NULL == (pBinaryMap = pSrcImg->GetImgBinaryMap(pSrcImg, nBinaryMap)))
        return nRet;
    
    for(j=0 ; j<nHeight ; j++)
    {
        nLocalPos = j * nStride;
        for(i=0 ; i<nWidth ; i++)
            pBinaryMap[nLocalPos++] = pBinaryMap[nLocalPos] ? nScaleFactor : 0;
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _Util_MergeImgs_WithBinaryMap_OneRow_SSE3(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, OUT UINT8 *pDst, IN const UINT8 *pMaskMap, IN const INT32 nWidth, IN const INT32 nStride)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   i = 0;
    
    for(i=0 ; i<nWidth ; i++)
        pDst[i] = pMaskMap[i] ? pSrc0[i] : pSrc1[i];
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_SSE2) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_SSE2)
static INT32 _Util_Cal_SAD16x16_SSE2(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, IN const INT32 nStride)
{
    INT32           nVal = 0;
    INT32           n = 0;
    
    for(n=15 ; n>=0 ; n--)
    {
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        
        //_mm_sad_epu8(*(__m128i *)pSrc0, *(__m128i *)pSrc1);
        
        pSrc0 += (nStride - 16);
        pSrc1 += (nStride - 16);
    }
    
    return nVal;
}


static INT32 _Util_Cal_Accum16x16_SSE2(UINT8 *pSrc, INT32 nStride)
{
    INT32           nVal = 0;
    INT32           n = 0;
    
    for(n=15 ; n>=0 ; n--)
    {
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        
        pSrc += (nStride - 16);
    }
    
    return nVal;
}


static INT32 _Util_Cal_Mean16x16_SSE2(IN UINT8 *pSrc, IN const INT32 nStride)
{
    INT32           nVal = 0;
    INT32           n = 0;
    
    for(n=15 ; n>=0 ; n--)
    {
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        
        pSrc += (nStride - 16);
    }
    
    return nVal / 256;
}


static INT32 _Util_Cal_Variance16x16_SSE2(IN UINT8* pSrc, IN const INT32 nMean, IN const INT32 nStride)
{
    INT32           nVal = 0;
    INT32           n = 0;
    
    for(n=15 ; n>=0 ; n--)
    {
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        
        pSrc += (nStride - 16);
    }
    
    return nVal / 256;
}


static Kakao_Status _Util_FramewiseToMBwise_SSE2(IN OUT Img_T *pSrcImg, IN const BinaryMapType nBinaryMap, IN const UINT32 nThreshold)
{
    INT32                   i = 0, j = 0, m = 0, n = 0;
    INT32                   nLocalPos = 0, nInPos = 0;
    INT32                   nMBPos = 0;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    INT32                   nTh = (100 * (INT32)nThreshold) / 255;
    INT32                   nCountFlag = 0;
    UINT8                   *pBinaryMap = NULL;
    UINT8                   *pMBwiseMap = NULL;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER)
    
    if(NULL == (pBinaryMap = pSrcImg->GetImgBinaryMap(pSrcImg, nBinaryMap)))
        return nRet;
    
    pMBwiseMap = pSrcImg->GetImgMBWiseMap(pSrcImg) + (PADDING_SIZE >> 4);
    
    // Kakao_ImgProc_Util_FramewiseToMBwise
    for(j=0 ; j<nHeight ; j+=MACRO_BLOCK_SIZE)
    {
        for(i=0 ; i<nWidth ; i+=MACRO_BLOCK_SIZE)
        {
            nCountFlag = nTh;
            
            nLocalPos = (j * nStride) + i;
            nMBPos = (nLocalPos >> 8);
            
            for(n=0 ; n<MACRO_BLOCK_SIZE ; n++)
            {
                nInPos = nLocalPos + (n * nStride);
                
                for(m=0 ; m<MACRO_BLOCK_SIZE ; m++)
                    nCountFlag -= !(!pBinaryMap[nInPos++]);
            }
            
            if(0 > nCountFlag)
                pMBwiseMap[nMBPos] = INDICATE_VALUE;
            else
                pMBwiseMap[nMBPos] = 0;
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _Util_ScaleBinaryMap_SSE2(IN OUT Img_T *pSrcImg, IN const BinaryMapType nBinaryMap, IN const UINT32 nScaleFactor)
{
    INT32                   i = 0, j = 0;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    INT32                   nLocalPos = 0;
    UINT8                   *pBinaryMap = NULL;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER)
    
    if(NULL == (pBinaryMap = pSrcImg->GetImgBinaryMap(pSrcImg, nBinaryMap)))
        return nRet;
    
    for(j=0 ; j<nHeight ; j++)
    {
        nLocalPos = j * nStride;
        for(i=0 ; i<nWidth ; i++)
            pBinaryMap[nLocalPos++] = pBinaryMap[nLocalPos] ? nScaleFactor : 0;
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _Util_MergeImgs_WithBinaryMap_OneRow_SSE2(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, OUT UINT8 *pDst, IN const UINT8 *pMaskMap, IN const INT32 nWidth, IN const INT32 nStride)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   i = 0;
    
    for(i=0 ; i<nWidth ; i++)
        pDst[i] = pMaskMap[i] ? pSrc0[i] : pSrc1[i];
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_SSE) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_SSE)
static INT32 _Util_Cal_SAD16x16_SSE(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, IN const INT32 nStride)
{
    INT32           nVal = 0;
    INT32           n = 0;
    
    for(n=15 ; n>=0 ; n--)
    {
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        
        pSrc0 += (nStride - 16);
        pSrc1 += (nStride - 16);
    }
    
    return nVal;
}


static INT32 _Util_Cal_Accum16x16_SSE(UINT8 *pSrc, INT32 nStride)
{
    INT32           nVal = 0;
    INT32           n = 0;
    
    for(n=15 ; n>=0 ; n--)
    {
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        
        pSrc += (nStride - 16);
    }
    
    return nVal;
}


static INT32 _Util_Cal_Mean16x16_SSE(IN UINT8 *pSrc, IN const INT32 nStride)
{
    INT32           nVal = 0;
    INT32           n = 0;
    
    for(n=15 ; n>=0 ; n--)
    {
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        
        pSrc += (nStride - 16);
    }
    
    return nVal / 256;
}


static INT32 _Util_Cal_Variance16x16_SSE(IN UINT8* pSrc, IN const INT32 nMean, IN const INT32 nStride)
{
    INT32           nVal = 0;
    INT32           n = 0;
    
    for(n=15 ; n>=0 ; n--)
    {
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        
        pSrc += (nStride - 16);
    }
    
    return nVal / 256;
}


static Kakao_Status _Util_FramewiseToMBwise_SSE(IN OUT Img_T *pSrcImg, IN const BinaryMapType nBinaryMap, IN const UINT32 nThreshold)
{
    INT32                   i = 0, j = 0, m = 0, n = 0;
    INT32                   nLocalPos = 0, nInPos = 0;
    INT32                   nMBPos = 0;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    INT32                   nTh = (100 * (INT32)nThreshold) / 255;
    INT32                   nCountFlag = 0;
    UINT8                   *pBinaryMap = NULL;
    UINT8                   *pMBwiseMap = NULL;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER)
    
    if(NULL == (pBinaryMap = pSrcImg->GetImgBinaryMap(pSrcImg, nBinaryMap)))
        return nRet;

    pMBwiseMap = pSrcImg->GetImgMBWiseMap(pSrcImg) + (PADDING_SIZE >> 4);
    
    // Kakao_ImgProc_Util_FramewiseToMBwise
    for(j=0 ; j<nHeight ; j+=MACRO_BLOCK_SIZE)
    {
        for(i=0 ; i<nWidth ; i+=MACRO_BLOCK_SIZE)
        {
            nCountFlag = nTh;
            
            nLocalPos = (j * nStride) + i;
            nMBPos = (nLocalPos >> 8);
            
            for(n=0 ; n<MACRO_BLOCK_SIZE ; n++)
            {
                nInPos = nLocalPos + (n * nStride);
                
                for(m=0 ; m<MACRO_BLOCK_SIZE ; m++)
                    nCountFlag -= !(!pBinaryMap[nInPos++]);
            }
            
            if(0 > nCountFlag)
                pMBwiseMap[nMBPos] = INDICATE_VALUE;
            else
                pMBwiseMap[nMBPos] = 0;
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _Util_ScaleBinaryMap_SSE(IN OUT Img_T *pSrcImg, IN const BinaryMapType nBinaryMap, IN const UINT32 nScaleFactor)
{
    INT32                   i = 0, j = 0;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    INT32                   nLocalPos = 0;
    UINT8                   *pBinaryMap = NULL;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER)
    
    if(NULL == (pBinaryMap = pSrcImg->GetImgBinaryMap(pSrcImg, nBinaryMap)))
        return nRet;
    
    for(j=0 ; j<nHeight ; j++)
    {
        nLocalPos = j * nStride;
        for(i=0 ; i<nWidth ; i++)
            pBinaryMap[nLocalPos++] = pBinaryMap[nLocalPos] ? nScaleFactor : 0;
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _Util_MergeImgs_WithBinaryMap_OneRow_SSE(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, OUT UINT8 *pDst, IN const UINT8 *pMaskMap, IN const INT32 nWidth, IN const INT32 nStride)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   i = 0;
    
    for(i=0 ; i<nWidth ; i++)
        pDst[i] = pMaskMap[i] ? pSrc0[i] : pSrc1[i];
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_NONE) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_NONE)
static INT32 _Util_Cal_SAD16x16_C(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, IN const INT32 nStride)
{
    INT32                   nVal = 0;
    INT32                   n = 0;
    
    for(n=15 ; n>=0 ; n--)
    {
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        
        pSrc0 += (nStride - 16);
        pSrc1 += (nStride - 16);
    }
    
    return nVal;
}


static INT32 _Util_Cal_Accum16x16_C(UINT8 *pSrc, INT32 nStride)
{
    INT32           nVal = 0;
    INT32           n = 0;
    
    for(n=15 ; n>=0 ; n--)
    {
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        
        pSrc += (nStride - 16);
    }
    
    return nVal;
}


static INT32 _Util_Cal_Mean16x16_C(IN UINT8 *pSrc, IN const INT32 nStride)
{
    INT32                   nVal = 0;
    INT32                   n = 0;
    
    for(n=15 ; n>=0 ; n--)
    {
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        
        pSrc += (nStride - 16);
    }
    
    return nVal / 256;
}


static INT32 _Util_Cal_Variance16x16_C(IN UINT8* pSrc, IN const INT32 nMean, IN const INT32 nStride)
{
    INT32                   nVal = 0;
    INT32                   n = 0;
    
    for(n=15 ; n>=0 ; n--)
    {
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        
        pSrc += (nStride - 16);
    }
    
    return nVal / 256;
}


static Kakao_Status _Util_FramewiseToMBwise_C(IN OUT Img_T *pSrcImg, IN const BinaryMapType nBinaryMap, IN const UINT32 nThreshold)
{
    INT32                   i = 0, j = 0, m = 0, n = 0;
    INT32                   nLocalPos = 0, nInPos = 0;
    INT32                   nMBPos = 0;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    INT32                   nTh = (100 * (INT32)nThreshold) / 255;
    INT32                   nCountFlag = 0;
    UINT8                   *pBinaryMap = NULL;
    UINT8                   *pMBwiseMap = NULL;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER)
    
    if(NULL == (pBinaryMap = pSrcImg->GetImgBinaryMap(pSrcImg, nBinaryMap)))
        return nRet;
    
    pMBwiseMap = pSrcImg->GetImgMBWiseMap(pSrcImg) + (PADDING_SIZE >> 4);
    
    // Kakao_ImgProc_Util_FramewiseToMBwise
    for(j=0 ; j<nHeight ; j+=MACRO_BLOCK_SIZE)
    {
        for(i=0 ; i<nWidth ; i+=MACRO_BLOCK_SIZE)
        {
            nCountFlag = nTh;
            
            nLocalPos = (j * nStride) + i;
            nMBPos = (nLocalPos >> 8);
            
            for(n=0 ; n<MACRO_BLOCK_SIZE ; n++)
            {
                nInPos = nLocalPos + (n * nStride);
                
                for(m=0 ; m<MACRO_BLOCK_SIZE ; m++)
                    nCountFlag -= !(!pBinaryMap[nInPos++]);
            }
            
            if(0 > nCountFlag)
                pMBwiseMap[nMBPos] = INDICATE_VALUE;
            else
                pMBwiseMap[nMBPos] = 0;
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _Util_ScaleBinaryMap_C(IN OUT Img_T *pSrcImg, IN const BinaryMapType nBinaryMap, IN const UINT32 nScaleFactor)
{
    INT32                   i = 0, j = 0;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    INT32                   nLocalPos = 0;
    UINT8                   *pBinaryMap = NULL;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER)
    
    if(NULL == (pBinaryMap = pSrcImg->GetImgBinaryMap(pSrcImg, nBinaryMap)))
        return nRet;
    
    for(j=0 ; j<nHeight ; j++)
    {
        nLocalPos = j * nStride;
        for(i=0 ; i<nWidth ; i++)
        {
            pBinaryMap[nLocalPos] = pBinaryMap[nLocalPos] ? nScaleFactor : 0;
            nLocalPos++;
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _Util_MergeImgs_WithBinaryMap_OneRow_C(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, OUT UINT8 *pDst, IN const UINT8 *pMaskMap, IN const INT32 nWidth, IN const INT32 nStride)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   i = 0;
    
    for(i=0 ; i<nWidth ; i++)
        pDst[i] = pMaskMap[i] ? pSrc0[i] : pSrc1[i];
    
    nRet = KAKAO_STAT_OK;
    return nRet;    
}
#else
static INT32 _Util_Cal_SAD16x16_C(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, IN const INT32 nStride)
{
    INT32           nVal = 0;
    INT32           n = 0;
    
    for(n=15 ; n>=0 ; n--)
    {
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        nVal += ABSM(*pSrc0++ - *pSrc1++);
        
        pSrc0 += (nStride - 16);
        pSrc1 += (nStride - 16);
    }
    
    return nVal;
}


static INT32 _Util_Cal_Accum16x16_C(UINT8 *pSrc, INT32 nStride)
{
    INT32           nVal = 0;
    INT32           n = 0;
    
    for(n=15 ; n>=0 ; n--)
    {
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        
        pSrc += (nStride - 16);
    }
    
    return nVal;
}


static INT32 _Util_Cal_Mean16x16_C(IN UINT8 *pSrc, IN const INT32 nStride)
{
    INT32                   nVal = 0;
    INT32                   n = 0;
    
    for(n=15 ; n>=0 ; n--)
    {
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        nVal += *pSrc++;
        
        pSrc += (nStride - 16);
    }
    
    return nVal / 256;
}


static INT32 _Util_Cal_Variance16x16_C(IN UINT8* pSrc, IN const INT32 nMean, IN const INT32 nStride)
{
    INT32                   nVal = 0;
    INT32                   n = 0;
    
    for(n=15 ; n>=0 ; n--)
    {
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        nVal += ABSM(*pSrc++ - nMean);
        
        pSrc += (nStride - 16);
    }
    
    return nVal / 256;
}


static Kakao_Status _Util_FramewiseToMBwise_C(IN OUT Img_T *pSrcImg, IN const BinaryMapType nBinaryMap, IN const UINT32 nThreshold)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER)
    
    {
        INT32                   i = 0, j = 0, m = 0, n = 0;
        INT32                   nLocalPos = 0, nInPos = 0;
        INT32                   nMBPos = 0;
        INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
        INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
        INT32                   nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
        INT32                   nTh = (100 * (INT32)nThreshold) / 255;
        INT32                   nCountFlag = 0;
        UINT8                   *pBinaryMap = NULL;
        UINT8                   *pMBwiseMap = NULL;

        if(NULL == (pBinaryMap = pSrcImg->GetImgBinaryMap(pSrcImg, nBinaryMap)))
            return nRet;
        
        pMBwiseMap = pSrcImg->GetImgMBWiseMap(pSrcImg) + (PADDING_SIZE >> 4);
        
        // Kakao_ImgProc_Util_FramewiseToMBwise
        for(j=0 ; j<nHeight ; j+=MACRO_BLOCK_SIZE)
        {
            for(i=0 ; i<nWidth ; i+=MACRO_BLOCK_SIZE)
            {
                nCountFlag = nTh;
                
                nLocalPos = (j * nStride) + i;
                nMBPos = (nLocalPos >> 8);
                
                for(n=0 ; n<MACRO_BLOCK_SIZE ; n++)
                {
                    nInPos = nLocalPos + (n * nStride);
                    
                    for(m=0 ; m<MACRO_BLOCK_SIZE ; m++)
                        nCountFlag -= !(!pBinaryMap[nInPos++]);
                }
                
                if(0 > nCountFlag)
                    pMBwiseMap[nMBPos] = INDICATE_VALUE;
                else
                    pMBwiseMap[nMBPos] = 0;
            }
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _Util_ScaleBinaryMap_C(IN OUT Img_T *pSrcImg, IN const BinaryMapType nBinaryMap, IN const UINT32 nScaleFactor)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER)

    {
        INT32                   i = 0, j = 0;
        INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
        INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
        INT32                   nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
        INT32                   nLocalPos = 0;
        UINT8                   *pBinaryMap = NULL;
        Kakao_Status            nRet = KAKAO_STAT_FAIL;

        if(NULL == (pBinaryMap = pSrcImg->GetImgBinaryMap(pSrcImg, nBinaryMap)))
            return nRet;
        
        for(j=0 ; j<nHeight ; j++)
        {
            nLocalPos = j * nStride;
            for(i=0 ; i<nWidth ; i++)
                pBinaryMap[nLocalPos++] = pBinaryMap[nLocalPos] ? nScaleFactor : 0;
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _Util_MergeImgs_WithBinaryMap_OneRow_C(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, OUT UINT8 *pDst, IN const UINT8 *pMaskMap, IN const INT32 nWidth, IN const INT32 nStride)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   i = 0;
    
    for(i=0 ; i<nWidth ; i++)
        pDst[i] = pMaskMap[i] ? pSrc0[i] : pSrc1[i];
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}
#endif


__INLINE INT32 _Util_GetImgWidth(IN const Img_T *pSrcImg, IN const INT32 nPlaneIdx)
{
    BaseImg_T           *pBaseImg = (BaseImg_T *)(pSrcImg);
    
    if(NULL == pBaseImg)
        return KAKAO_STAT_FAIL;
    
    return pBaseImg->nImgSize.nWidth[nPlaneIdx];
}


__INLINE INT32 _Util_SetImgWidth(IN Img_T *pSrcImg, IN const INT32 nPlaneIdx, IN const INT32 nWidth)
{
    BaseImg_T           *pBaseImg = (BaseImg_T *)(pSrcImg);
    
    if(NULL == pBaseImg)
        return KAKAO_STAT_FAIL;
    
    pBaseImg->nImgSize.nWidth[nPlaneIdx] = nWidth;
    
    return KAKAO_STAT_OK;
}


__INLINE INT32 _Util_GetImgHeight(IN const Img_T *pSrcImg, IN const INT32 nPlaneIdx)
{
    BaseImg_T           *pBaseImg = (BaseImg_T *)(pSrcImg);
    
    if(NULL == pBaseImg)
        return KAKAO_STAT_FAIL;
    
    return pBaseImg->nImgSize.nHeight[nPlaneIdx];
}


__INLINE INT32 _Util_SetImgHeight(IN Img_T *pSrcImg, IN const INT32 nPlaneIdx, IN const INT32 nHeight)
{
    BaseImg_T           *pBaseImg = (BaseImg_T *)(pSrcImg);
    
    if(NULL == pBaseImg)
        return KAKAO_STAT_FAIL;
    
    pBaseImg->nImgSize.nHeight[nPlaneIdx] = nHeight;
    
    return KAKAO_STAT_OK;
}


__INLINE INT32 _Util_GetImgStride(IN const Img_T *pSrcImg, IN const INT32 nPlaneIdx)
{
    BaseImg_T           *pBaseImg = (BaseImg_T *)(pSrcImg);
    
    if(NULL == pBaseImg)
        return KAKAO_STAT_FAIL;
    
    return pBaseImg->nImgSize.nStride[nPlaneIdx];
}


__INLINE INT32 _Util_SetImgStride(IN Img_T *pSrcImg, IN const INT32 nPlaneIdx, IN const INT32 nStride)
{
    BaseImg_T           *pBaseImg = (BaseImg_T *)(pSrcImg);
    
    if(NULL == pBaseImg)
        return KAKAO_STAT_FAIL;
    
    pBaseImg->nImgSize.nStride[nPlaneIdx] = nStride;
    
    return KAKAO_STAT_OK;
}


__INLINE KISize _Util_GetImgSize(IN const Img_T *pSrcImg)
{
    BaseImg_T           *pBaseImg = (BaseImg_T *)(pSrcImg);
    KISize              nDummyImgSize = {-1, };
    
    if(NULL == pBaseImg)
        return nDummyImgSize;
    
    return pBaseImg->nImgSize;
}


__INLINE ImgFormat _Util_GetImgFormat(IN const Img_T *pSrcImg)
{
    BaseImg_T           *pBaseImg = (BaseImg_T *)(pSrcImg);
    
    if(NULL == pBaseImg)
        return IMG_FORMAT_INVALID;
    
    return pBaseImg->nImgFormat;
}


__INLINE INT32 _Util_SetImgFormat(IN const Img_T *pSrcImg, IN const ImgFormat nImgFormat)
{
    BaseImg_T           *pBaseImg = (BaseImg_T *)(pSrcImg);
    
    if(NULL == pBaseImg)
        return KAKAO_STAT_FAIL;
    
    pBaseImg->nImgFormat = nImgFormat;
    
    return KAKAO_STAT_OK;
}


__INLINE ImgDepth _Util_GetImgDepth(IN const Img_T *pSrcImg)
{
    BaseImg_T           *pBaseImg = (BaseImg_T *)(pSrcImg);
    
    if(NULL == pBaseImg)
        return IMG_DEPTH_INVALID;
    
    return pBaseImg->nImgDepth;
}


__INLINE INT32 _Util_SetImgDepth(IN const Img_T *pSrcImg, IN const ImgDepth nImgDepth)
{
    BaseImg_T           *pBaseImg = (BaseImg_T *)(pSrcImg);
    
    if(NULL == pBaseImg)
        return KAKAO_STAT_FAIL;
    
    pBaseImg->nImgDepth = nImgDepth;
    
    return KAKAO_STAT_OK;
}


__INLINE INT32 _Util_GetImgNumofPlanes(IN const Img_T *pSrcImg)
{
    BaseImg_T           *pBaseImg = (BaseImg_T *)(pSrcImg);
    
    if(NULL == pBaseImg)
        return KAKAO_STAT_FAIL;
    
    return pBaseImg->nNumofPlanes;
}


__INLINE INT32 _Util_SetImgNumofPlanes(IN const Img_T *pSrcImg, IN const INT32 nNumofPlanes)
{
    BaseImg_T           *pBaseImg = (BaseImg_T *)(pSrcImg);
    
    if(NULL == pBaseImg)
        return KAKAO_STAT_FAIL;
    
    pBaseImg->nNumofPlanes = nNumofPlanes;
    
    return KAKAO_STAT_OK;
}


__INLINE ROI _Util_GetImgROI(IN const Img_T *pSrcImg)
{
    BaseImg_T           *pBaseImg = (BaseImg_T *)(pSrcImg);
    ROI                 nDummyROI = {-1, };
    
    if(NULL == pBaseImg)
        return nDummyROI;
    
    return pBaseImg->nROI;
}


__INLINE INT32 _Util_SetImgROI(IN Img_T *pSrcImg, IN const INT32 nStartX, IN const INT32 nEndX, IN const INT32 nStartY, IN const INT32 nEndY)
{
    BaseImg_T           *pBaseImg = (BaseImg_T *)(pSrcImg);
    UINT32              nPixelSize = 0;
    INT32               nWidth = 0;
    INT32               nHeight = 0;
    INT32               nAddrOffset = 0, nAddrOffsetUV = 0;
    INT32               i = 0;
    INT32               nNumofPlanes = -1;

    if(NULL == pBaseImg)
        return KAKAO_STAT_FAIL;
    
    nPixelSize = GET_PIXEL_SIZE(pSrcImg->GetImgDepth(pSrcImg));
    nNumofPlanes = pSrcImg->GetNumofImgPlanes(pSrcImg);
    
    if(0 != (nStartX + nStartY + nEndX + nEndY))
    {
        pBaseImg->nROI.nStartX = nStartX;
        pBaseImg->nROI.nEndX = nEndX;
        pBaseImg->nROI.nStartY = nStartY;
        pBaseImg->nROI.nEndY = nEndY;
        
        pSrcImg->SetImgROIStatus(pSrcImg, IMGPROC_TRUE);
        
        nWidth = (nEndX - nStartX);
        nHeight = (nEndY - nStartY);
    }
    else
    {
        pBaseImg->nROI.nStartX = 0;
        pBaseImg->nROI.nEndX = pBaseImg->nImgSize.nBaseWidth;
        pBaseImg->nROI.nStartY = 0;
        pBaseImg->nROI.nEndY = pBaseImg->nImgSize.nBaseHeight;
        
        pSrcImg->SetImgROIStatus(pSrcImg, IMGPROC_FALSE);
        
        nWidth = pBaseImg->nImgSize.nBaseWidth;
        nHeight = pBaseImg->nImgSize.nBaseHeight;
    }
    
    switch(pSrcImg->GetImgFormat(pSrcImg))
    {
        case IMG_FORMAT_GRAY:
            nAddrOffset = nPixelSize * ((pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0) * nStartY) + nStartX);
            
            for(i=0 ; i<nNumofPlanes ; i++)
            {
                pSrcImg->SetImgWidth(pSrcImg, i, nWidth);
                pSrcImg->SetImgHeight(pSrcImg, i, nHeight);
                pSrcImg->SetImgStride(pSrcImg, i, pSrcImg->GetImgStride(pSrcImg, i));
                pSrcImg->SetImgPlanes(pSrcImg, i, (HANDLE)((UINT8 *)(pBaseImg->pOrgImgPointer[i]) + nAddrOffset));
            }
            
            pBaseImg->pRealImgData = pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
            break;
        case IMG_FORMAT_RGB24:
        case IMG_FORMAT_BGR24:
        case IMG_FORMAT_RGBA32:
        case IMG_FORMAT_BGRA32:
            nAddrOffset = nPixelSize * ((pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0) * nStartY) + (nNumofPlanes * nStartX));
            
            for(i=0 ; i<nNumofPlanes ; i++)
            {
                pSrcImg->SetImgWidth(pSrcImg, i, nWidth);
                pSrcImg->SetImgHeight(pSrcImg, i, nHeight);
                pSrcImg->SetImgStride(pSrcImg, i, pSrcImg->GetImgStride(pSrcImg, i));
                pSrcImg->SetImgPlanes(pSrcImg, i, (HANDLE)((UINT8 *)(pBaseImg->pOrgImgPointer[i]) + nAddrOffset));
            }
            
            pBaseImg->pRealImgData = pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
            break;
        case IMG_FORMAT_HSV:
        case IMG_FORMAT_HSL:
            nAddrOffset = nPixelSize * ((pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0) * nStartY) + nStartX);
            
            for(i=0 ; i<nNumofPlanes ; i++)
            {
                pSrcImg->SetImgWidth(pSrcImg, i, nWidth);
                pSrcImg->SetImgHeight(pSrcImg, i, nHeight);
                pSrcImg->SetImgStride(pSrcImg, i, pSrcImg->GetImgStride(pSrcImg, i));
                pSrcImg->SetImgPlanes(pSrcImg, i, (HANDLE)((UINT8 *)(pBaseImg->pOrgImgPointer[i]) + nAddrOffset));
            }
            
            pBaseImg->pRealImgData = pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
            break;
        case IMG_FORMAT_I420:
            nAddrOffset = nPixelSize * ((pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0) * nStartY) + nStartX);
            nAddrOffsetUV = nPixelSize * ((pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_1) * (nStartY / 2)) + (nStartX / 2));
            
            for(i=0 ; i<nNumofPlanes ; i++)
            {
                pSrcImg->SetImgWidth(pSrcImg, i, nWidth >> !(!(i)));
                pSrcImg->SetImgHeight(pSrcImg, i, nHeight >> !(!(i)));
                pSrcImg->SetImgStride(pSrcImg, i, pSrcImg->GetImgStride(pSrcImg, i));
                pSrcImg->SetImgPlanes(pSrcImg, i, (HANDLE)((UINT8 *)(pBaseImg->pOrgImgPointer[i]) + ((0 == i) ? nAddrOffset : nAddrOffsetUV)));
            }
            
            pBaseImg->pRealImgData = pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
            break;
        default:
            break;
    }
    
    return KAKAO_STAT_OK;
}


__INLINE UINT8* _Util_GetImgMBWiseMap(IN const Img_T *pSrcImg)
{
    BaseImg_T           *pBaseImg = (BaseImg_T *)(pSrcImg);
    
    if(NULL == pBaseImg)
        return NULL;
    
    if(NULL == pBaseImg->pMBwiseMap)
        SAFEALLOC_NO_RET(pBaseImg->pMBwiseMap, ((MAKE_STRIDE(pBaseImg->nImgSize.nAllocatedWidth) * pBaseImg->nImgSize.nAllocatedHeight) >> 8), 32, UINT8);
    
    return pBaseImg->pMBwiseMap;
}


__INLINE INT32* _Util_GetImgMBWiseSADMap(IN const Img_T *pSrcImg)
{
    BaseImg_T           *pBaseImg = (BaseImg_T *)(pSrcImg);
    
    if(NULL == pBaseImg)
        return NULL;
    
    if(NULL == pBaseImg->pMBwiseSADMap)
        SAFEALLOC_NO_RET(pBaseImg->pMBwiseSADMap, ((MAKE_STRIDE(pBaseImg->nImgSize.nAllocatedWidth) * pBaseImg->nImgSize.nAllocatedHeight) >> 8), 32, INT32);
    
    return pBaseImg->pMBwiseSADMap;
}


__INLINE UINT8* _Util_GetBinaryMap(IN Img_T *pSrcImg, IN const BinaryMapType nBinaryMap)
{
    BaseImg_T           *pBaseImg = (BaseImg_T *)(pSrcImg);
    UINT8               *pSrc = NULL;
    UINT32              nAddOffset = 0;
    
    if(NULL == pBaseImg)
        return NULL;
    
    if(NULL == pBaseImg->pBinaryMap[nBinaryMap])
        _Util_AllocBinaryMap(pSrcImg, nBinaryMap);
    
    pSrc = pBaseImg->pBinaryMap[nBinaryMap];
    
    if(IMGPROC_TRUE == pSrcImg->GetImgROIStatus(pSrcImg))
    {
        const ROI           nROI = pSrcImg->GetImgROI(pSrcImg);
        
        nAddOffset = nROI.nStartY * pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0) + nROI.nStartX;
    }
    
    return pSrc + PADDING_SIZE + nAddOffset;
}


__INLINE F32* _Util_GetImgHistMap(IN const Img_T *pSrcImg, IN const INT32 nPlaneIdx, IN const INT32 nDimension)
{
    BaseImg_T           *pBaseImg = (BaseImg_T *)(pSrcImg);
    F32                 *pHistMap = NULL;
    
    if(NULL == pBaseImg)
        return NULL;
    
    if(0 == nDimension)
    {
        pHistMap = pBaseImg->p1DHist[nPlaneIdx];
    }
    else
        pHistMap = pBaseImg->p2DHist;
    
    return pHistMap;
}


__INLINE void* _Util_GetImgPlanes(IN const Img_T *pSrcImg, IN const INT32 nPlaneIdx)
{
    BaseImg_T           *pBaseImg = (BaseImg_T *)(pSrcImg);
    
    if(NULL == pBaseImg)
        return NULL;
    
    return pBaseImg->pPlanes[nPlaneIdx];
}


__INLINE INT32 _Util_SetImgPlanes(IN Img_T *pSrcImg, IN const INT32 nPlaneIdx, IN HANDLE pPtr)
{
    BaseImg_T           *pBaseImg = (BaseImg_T *)(pSrcImg);
    
    if(NULL == pBaseImg)
        return KAKAO_STAT_FAIL;
    
    pBaseImg->pPlanes[nPlaneIdx] = pPtr;
    
    return KAKAO_STAT_OK;
}


__INLINE INT32 _Util_GetImgBins(IN const Img_T *pSrcImg)
{
    BaseImg_T           *pBaseImg = (BaseImg_T *)(pSrcImg);
    
    if(NULL == pBaseImg)
        return KAKAO_STAT_FAIL;
    
    return pBaseImg->nBins;
}


__INLINE INT32 _Util_SetImgBins(IN Img_T *pSrcImg, IN const INT32 nBins)
{
    BaseImg_T           *pBaseImg = (BaseImg_T *)(pSrcImg);
    
    if(NULL == pBaseImg)
        return KAKAO_STAT_FAIL;
    
    pBaseImg->nBins = nBins;
    
    return KAKAO_STAT_OK;
}


__INLINE INT32 _Util_GetImgROIStatus(IN const Img_T *pSrcImg)
{
    BaseImg_T           *pBaseImg = (BaseImg_T *)(pSrcImg);
    
    if(NULL == pBaseImg)
        return KAKAO_STAT_FAIL;
    
    return pBaseImg->bIsROIEnabled;
}


__INLINE IMGPROC_BOOL _Util_SetImgROIStatus(IN Img_T *pSrcImg, IN const IMGPROC_BOOL nROIStatus)
{
    BaseImg_T           *pBaseImg = (BaseImg_T *)(pSrcImg);
    
    if(NULL == pBaseImg)
        return KAKAO_STAT_FAIL;
    
    pBaseImg->bIsROIEnabled = nROIStatus;
    
    return KAKAO_STAT_OK;
}


static Kakao_Status _Util_AllocBinaryMap(IN OUT Img_T *pSrcImg, IN const BinaryMapType nBinaryMap)
{
    Kakao_Status        nRet = KAKAO_STAT_FAIL;
    BaseImg_T           *pBaseImg = (BaseImg_T *)pSrcImg;
    
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER)
    
    SAFEALLOC(pBaseImg->pBinaryMap[nBinaryMap], (MAKE_STRIDE(pBaseImg->nImgSize.nAllocatedWidth) * pBaseImg->nImgSize.nAllocatedHeight), 32, UINT8);
    
    nRet = KAKAO_STAT_OK;

    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _Util_Padding_Gray(IN Img_T *pSrcImg)
{
    INT32                   j = 0;
    INT32                   nWidth = 0;
    INT32                   nHeight = 0;
    INT32                   nStride = 0;
    INT32                   nLocalPos = 0;
    UINT8                   *pSrc = NULL;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER)

    nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    pSrc = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
    /* ----------- Y  -----------*/
    // Left-Side & Right-Side
    for(j=0 ; j<nHeight ; j++)
    {
        nLocalPos = j * nStride;
        
        MEMSET(&(pSrc[nLocalPos - PADDING_SIZE]), pSrc[nLocalPos], PADDING_SIZE);
        MEMSET(&(pSrc[nLocalPos + nWidth]), pSrc[nLocalPos + nWidth - 1], PADDING_SIZE);
    }
    
    // Top-Side
    pSrc -= PADDING_SIZE;
    for(j=0 ; j<PADDING_SIZE ; j++)
        OneRow_Copy(&(pSrc[-((j + 1) * nStride)]), &(pSrc[0]), nStride);
    
    // Bottom-Side
    pSrc += (nStride * (nHeight - 1));
    for(j=0 ; j<PADDING_SIZE ; j++)
        OneRow_Copy(&(pSrc[((j + 1) * nStride)]), &(pSrc[0]), nStride);
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _Util_Copy_YUV(OUT Img_T *pDstImg, IN const Img_T *pSrcImg)
{
    INT32                   nWidth = 0;
    INT32                   nHeight = 0;
    INT32                   nSrcStride = 0;
    INT32                   nDstStride = 0;
    UINT8                   *pSrc = NULL;
    UINT8                   *pDst = NULL;
    UINT32                  nPixelSize = 0;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER)

    /* ----------- Y  -----------*/
    // Left-Side & Right-Side
    nPixelSize = GET_PIXEL_SIZE(pDstImg->GetImgDepth(pDstImg));
    nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    nSrcStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    nDstStride = pDstImg->GetImgStride(pDstImg, IMG_PLANE_0);
    
    pSrc = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
    pDst = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0);
    _Util_CopyPlane(pDst, (const UINT8 *)pSrc, nWidth, nHeight, nSrcStride, nDstStride, nPixelSize);
    
    /* ----------- U  -----------*/
    // Left-Side & Right-Side
    nWidth = (nWidth >> 1);
    nHeight = (nHeight >> 1);
    nSrcStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_1);
    nDstStride = pDstImg->GetImgStride(pDstImg, IMG_PLANE_1);

    pSrc = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_1);
    pDst = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_1);
    _Util_CopyPlane(pDst, (const UINT8 *)pSrc, nWidth, nHeight, nSrcStride, nDstStride, nPixelSize);
    
    /* ----------- V  -----------*/
    // Left-Side & Right-Side
    pSrc = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_2);
    pDst = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_2);
    _Util_CopyPlane(pDst, (const UINT8 *)pSrc, nWidth, nHeight, nSrcStride, nDstStride, nPixelSize);

    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _Util_Padding_YUV(IN Img_T *pSrcImg)
{
    INT32                   nWidth = 0;
    INT32                   nHeight = 0;
    INT32                   nStride = 0;
    UINT8                   *pSrc = NULL;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER)

    /* ----------- Y  -----------*/
    // Left-Side & Right-Side
    nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    pSrc = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
    if(KAKAO_STAT_OK != (nRet = _Util_Padding_Plane(pSrc, nWidth, nHeight, nStride)))
        goto Error;
    
    /* ----------- U  -----------*/
    // Left-Side & Right-Side
    nWidth = (nWidth >> 1);
    nHeight = (nHeight >> 1);
    nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_1);
    
    pSrc = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_1);
    if(KAKAO_STAT_OK != (nRet = _Util_Padding_Plane(pSrc, nWidth, nHeight, nStride)))
        goto Error;
    
    /* ----------- V  -----------*/
    // Left-Side & Right-Side
    pSrc = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_2);
    if(KAKAO_STAT_OK != (nRet = _Util_Padding_Plane(pSrc, nWidth, nHeight, nStride)))
        goto Error;
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _Util_Padding_Plane(UINT8 *pSrc, INT32 nWidth, INT32 nHeight, INT32 nStride)
{
    INT32                   j = 0;
    INT32                   nLocalPos = 0;
    UINT8                   *pLocalSrc = NULL;
    UINT8                   *pLocalDst = NULL;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pSrc, KAKAO_STAT_NULL_POINTER);
    
    for(j=0 ; j<nHeight ; j++)
    {
        nLocalPos = j * nStride;
        
        MEMSET(&(pSrc[nLocalPos - PADDING_SIZE]), pSrc[nLocalPos], PADDING_SIZE);
        MEMSET(&(pSrc[nLocalPos + nWidth]), pSrc[nLocalPos + nWidth - 1], PADDING_SIZE);
    }

    // Top-Side
    pLocalSrc = pSrc - PADDING_SIZE;
    pLocalDst = pSrc - ((nStride + 1) * PADDING_SIZE);
    for(j=0 ; j<PADDING_SIZE ; j++)
    {
        OneRow_Copy(pLocalDst, pLocalSrc, nStride);
        pLocalDst += nStride;
    }
    
    // Bottom-Side
    pLocalSrc = (UINT8 *)pSrc + nStride * (nHeight - 1) - PADDING_SIZE;
    pLocalDst = (UINT8 *)pSrc + nStride * (nHeight) - PADDING_SIZE;
    for(j=0 ; j<PADDING_SIZE ; j++)
    {
        OneRow_Copy(pLocalDst, pLocalSrc, nStride);
        pLocalDst += nStride;
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


__INLINE void _ImgProc_Util_Flip(IN UINT8 *pSrc, OUT UINT8 *pDst, OUT UINT8 *pTmp, IN const INT32 nWidth)
{
    OneRow_Copy(pTmp, pSrc, nWidth);
    OneRow_Copy(pSrc, pDst, nWidth);
    OneRow_Copy(pDst, pTmp, nWidth);
}


static Kakao_Status _Util_CopyPlane(UINT8 *pDst, const UINT8 *pSrc, IN const INT32 nWidth, IN const INT32 nHeight, IN const INT32 nSrcStride, IN const INT32 nDstStride, IN const UINT32 nPixelSize)
{
    INT32                   i = 0;
    UINT8                   *pSrc1 = NULL, *pSrc2 = NULL;
    UINT8                   *pDst1 = NULL, *pDst2 = NULL;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    pSrc1 = (UINT8 *)pSrc;
    pSrc2 = (UINT8 *)pSrc + nWidth - 1;
    pDst1 = pDst - PADDING_SIZE;
    pDst2 = pDst + nWidth;
    for(i=0 ; i<nHeight ; i++)
    {
        MEMSET(pDst1, pSrc1[0], PADDING_SIZE);
        OneRow_Copy(pDst1 + PADDING_SIZE, pSrc1, nWidth);
        MEMSET(pDst2, pSrc2[0], PADDING_SIZE);
        pSrc1 += nSrcStride;
        pSrc2 += nSrcStride;
        pDst1 += nDstStride;
        pDst2 += nDstStride;
    }
    
    pSrc1 = pDst - PADDING_SIZE;
    pDst1 = pDst - (nDstStride + 1) * PADDING_SIZE;
    for(i=0 ; i<PADDING_SIZE ; i++)
    {
        OneRow_Copy(pDst1, pSrc1, nDstStride);
        pDst1 += nDstStride;
    }
    
    pSrc2 = pDst + nDstStride * (nHeight - 1) - PADDING_SIZE;
    pDst2 = pDst + nDstStride * (nHeight) - PADDING_SIZE;
    for(i=0 ; i<PADDING_SIZE ; i++)
    {
        OneRow_Copy(pDst2, pSrc2, nDstStride);
        pDst2 += nDstStride;
    }
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
}


static Kakao_Status _Util_MergeImgs_WithBinaryMap(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, OUT UINT8 *pDst, IN const UINT8 *pMaskMap, IN const INT32 nWidth, IN const INT32 nHeight, IN const INT32 nStride)
{
    INT32                   j = 0;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    for(j=0 ; j<nHeight ; j++)
    {
        _Util_MergeImgs_WithBinaryMap_OneRow(pSrc0, pSrc1, pDst, pMaskMap, nWidth, nStride);
        pSrc0 += nStride;
        pSrc1 += nStride;
        pDst += nStride;
        pMaskMap += nStride;
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _Util_UpdateImgSize(IN OUT Img_T *pSrcImg, IN const INT32 nWidth, IN const INT32 nHeight, IN const UINT32 *pStride, IN const ImgFormat nImgFormat)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   i = 0;
    INT32                   nAlignedWidth = GET_ALIGNED_LENGTH(nWidth, MACRO_BLOCK_SIZE);
    const INT32             nNumofPlanes = pSrcImg->GetNumofImgPlanes(pSrcImg);
    
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER);
    
    if(NULL == pStride)
    {
        switch(nImgFormat)
        {
            case IMG_FORMAT_GRAY:
            case IMG_FORMAT_HSL:
            case IMG_FORMAT_HSV:
                for(i=0 ; i<nNumofPlanes ; i++)
                {
                    pSrcImg->SetImgWidth(pSrcImg, i, nWidth);
                    pSrcImg->SetImgHeight(pSrcImg, i, nHeight);
                    pSrcImg->SetImgStride(pSrcImg, i, MAKE_STRIDE(nAlignedWidth));
                }
                break;
            case IMG_FORMAT_RGB24:
            case IMG_FORMAT_BGR24:
            case IMG_FORMAT_RGBA32:
            case IMG_FORMAT_BGRA32:
                for(i=0 ; i<nNumofPlanes ; i++)
                {
                    pSrcImg->SetImgWidth(pSrcImg, i, nWidth);
                    pSrcImg->SetImgHeight(pSrcImg, i, nHeight);
                    pSrcImg->SetImgStride(pSrcImg, i, ((((nNumofPlanes * nAlignedWidth) + 2 * (nNumofPlanes * PADDING_SIZE)) + (PADDING_SIZE - 1)) & ~(PADDING_SIZE - 1)));
                }
                break;
            case IMG_FORMAT_I420:
                for(i=0 ; i<nNumofPlanes ; i++)
                {
                    pSrcImg->SetImgWidth(pSrcImg, i, nWidth >> !(!(i)));
                    pSrcImg->SetImgHeight(pSrcImg, i, nHeight >> !(!(i)));
                    pSrcImg->SetImgStride(pSrcImg, i, MAKE_STRIDE(nAlignedWidth >> !(!(i))));
                }
                break;
            default:
                for(i=0 ; i<4 ; i++)
                {
                    pSrcImg->SetImgWidth(pSrcImg, i, nWidth);
                    pSrcImg->SetImgHeight(pSrcImg, i, nHeight);
                    pSrcImg->SetImgStride(pSrcImg, i, MAKE_STRIDE(nAlignedWidth));
                }
                break;
        }
    }
    else
    {
        for(i=0 ; i<4 ; i++)
        {
            pSrcImg->SetImgWidth(pSrcImg, i, nWidth);
            pSrcImg->SetImgHeight(pSrcImg, i, nHeight);
            pSrcImg->SetImgStride(pSrcImg, i, pStride[i]);
        }
    }
    
    ((BaseImg_T *)pSrcImg)->nImgSize.nBaseWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    ((BaseImg_T *)pSrcImg)->nImgSize.nBaseHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _Util_UpdateSize(IN OUT KISize *pSize, IN const INT32 nWidth, IN const INT32 nHeight, IN const UINT32 *pStride, IN const INT32 nNumofPlanes, IN const ImgFormat nImgFormat)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   i = 0;
    INT32                   nAlignedWidth = GET_ALIGNED_LENGTH(nWidth, MACRO_BLOCK_SIZE);
    
    CHECK_POINTER_VALIDATION(pSize, KAKAO_STAT_NULL_POINTER);
    
    if(NULL == pStride)
    {
        switch(nImgFormat)
        {
            case IMG_FORMAT_GRAY:
            case IMG_FORMAT_HSL:
            case IMG_FORMAT_HSV:
                for(i=0 ; i<nNumofPlanes ; i++)
                {
                    pSize->nWidth[i] = nWidth;
                    pSize->nHeight[i] = nHeight;
                    pSize->nStride[i] = MAKE_STRIDE(nAlignedWidth);
                }
                break;
            case IMG_FORMAT_RGB24:
            case IMG_FORMAT_BGR24:
            case IMG_FORMAT_RGBA32:
            case IMG_FORMAT_BGRA32:
                for(i=0 ; i<nNumofPlanes ; i++)
                {
                    pSize->nWidth[i] = nWidth;
                    pSize->nHeight[i] = nHeight;
                    pSize->nStride[i] = (((nNumofPlanes * nAlignedWidth) + 2 * (nNumofPlanes * PADDING_SIZE)) + (PADDING_SIZE - 1)) & ~(PADDING_SIZE - 1);
                }
                break;
            case IMG_FORMAT_I420:
                for(i=0 ; i<nNumofPlanes ; i++)
                {
                    pSize->nWidth[i] = nWidth >> !(!(i));
                    pSize->nHeight[i] = nHeight >> !(!(i));
                    pSize->nStride[i] = MAKE_STRIDE(nAlignedWidth) >> !(!(i));
                }
                break;
            default:
                for(i=0 ; i<4 ; i++)
                {
                    pSize->nWidth[i] = nWidth;
                    pSize->nHeight[i] = nHeight;
                    pSize->nStride[i] = MAKE_STRIDE(nAlignedWidth);
                }
                break;
        }
    }
    else
    {
        for(i=0 ; i<4 ; i++)
        {
            pSize->nWidth[i] = nWidth;
            pSize->nHeight[i] = nHeight;
            pSize->nStride[i] = pStride[i];
        }
    }
    
    pSize->nBaseWidth = pSize->nWidth[0];
    pSize->nBaseHeight = pSize->nHeight[0];
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


static INT32 _Util_Get_CpuCores(void)
{
    INT32 nNumofCPUCore = 32;
    
    #if HAVE_UNISTD_H && !defined(__OS2__)
        #if defined(_SC_NPROCESSORS_ONLN)
            nNumofCPUCore = (INT32)(sysconf(_SC_NPROCESSORS_ONLN));
        #elif defined(_SC_NPROC_ONLN)
            nNumofCPUCore = (INT32)(sysconf(_SC_NPROCESSORS_ONLN));
        #endif
    #elif defined(_WIN32)
    {
        PGNSI pGNSI;
        SYSTEM_INFO sysinfo;
        
        /* Call GetNativeSystemInfo if supported or
         * GetSystemInfo otherwise. */
        
        pGNSI = (PGNSI) GetProcAddress(
                                       GetModuleHandle(TEXT("kernel32.dll")), "GetNativeSystemInfo");
        if(NULL != pGNSI)
            pGNSI(&sysinfo);
        else
            GetSystemInfo(&sysinfo);
        
        nNumofCPUCore = sysinfo.dwNumberOfProcessors;
    }
    #elif defined(__OS2__)
    {
        ULONG proc_id;
        ULONG status;
        
        nNumofCPUCore = 0;
        for (proc_id = 1; ; proc_id++)
        {
            if (DosGetProcessorStatus(proc_id, &status))
                break;
            
            if (status == PROC_ONLINE)
                nNumofCPUCore++;
        }
    }
    #else
        /* other platforms */
    #endif

    return (nNumofCPUCore > 0) ? ((32 != nNumofCPUCore) ? nNumofCPUCore : 1) : 1;
}


static UINT64 _Util_Get_CpuFrequency(void)
{
    UINT64   nCPUFrequency = 0;
    
    #if ENABLE_OS(OS_IOS)
    {
        //
    }
    #elif ENABLE_OS(OS_ANDROID)
    {
        //
    }
    #elif ENABLE_OS(OS_OSX)
    {
        INT32       nConfig[2];
        size_t      nLen = sizeof(nCPUFrequency);

        nConfig[0] = CTL_HW;
        nConfig[1] = HW_CPU_FREQ;
        
        sysctl(nConfig, 2, &nCPUFrequency, &nLen, NULL, 0);
    }
    #elif ENABLE_OS(OS_WIN32 || OS_WIN64) || defined(_WIN32)
    {
        unsigned    __int64 Start;
        LARGE_INTEGER nWait, nStart, nCurrent;
        
        QueryPerformanceCounter(&nStart);
        QueryPerformanceFrequency(&nWait);
        nWait.QuadPart >>= 5;
        Start = __rdtsc();
        
        do
        {
            QueryPerformanceCounter(&nCurrent);
        }while(nCurrent.QuadPart - nStart.QuadPart < nWait.QuadPart);
        
        nCPUFrequency = ((__rdtsc() - Start) << 5);
    }
    #else
    {
        //
    }
    #endif
    
    return nCPUFrequency / 1000000;
}
