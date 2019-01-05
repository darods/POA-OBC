


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
typedef Kakao_Status (* fpFilters)(Thread_Param *pThread_Param);
typedef Kakao_Status (* fpMosaic)(Thread_Param *pThread_Param);


struct _Thread_Param
{
    Filter_T            *pFilter;
    fpFilters           pFilterFunc;
    
    Img_T               *pSrcImg;
    Img_T               *pDstImg;

    UINT8               *pSrc0;
    UINT8               *pSrc1;
    UINT8               *pDst;
    INT32               nWidth;
    INT32               nHeight;
    INT32               nSrcStride;
    INT32               nDstStride;
    
    UINT32              nThreadIdx;

    QuantLevel          nQuantLevel;
    INT32               nNbyNMeanMaskSize;
};


/*----------------------------------------------------------------------------------------
 Static Function
 ----------------------------------------------------------------------------------------*/
#if ENABLE_ISET(CPU_ARMX | ISET_NEON)
    static void _Filter_Erosion_NEON(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg,
                                                    IN const BinaryMapType nBinaryMap, IN const INT32 nThick);
    static void _Filter_Dilation_NEON(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg,
                                                    IN const BinaryMapType nBinaryMap);
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_SSE4) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_SSE4)
    static void _Filter_Erosion_SSE4(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg,
                                                    IN const BinaryMapType nBinaryMap, IN const INT32 nThick);
    static void _Filter_Dilation_SSE4(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg,
                                                    IN const BinaryMapType nBinaryMap);
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_SSE3) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_SSE3)
    static void _Filter_Erosion_SSE3(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg,
                                                    IN const BinaryMapType nBinaryMap, IN const INT32 nThick);
    static void _Filter_Dilation_SSE3(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg,
                                                    IN const BinaryMapType nBinaryMap);
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_SSE2) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_SSE2)
    static void _Filter_Erosion_SSE2(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg,
                                                    IN const BinaryMapType nBinaryMap, IN const INT32 nThick);
    static void _Filter_Dilation_SSE2(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg,
                                                    IN const BinaryMapType nBinaryMap);
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_SSE) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_SSE)
    static void _Filter_Erosion_SSE(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg,
                                                    IN const BinaryMapType nBinaryMap, IN const INT32 nThick);
    static void _Filter_Dilation_SSE(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg,
                                                    IN const BinaryMapType nBinaryMap);
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_NONE) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_NONE)
    static void _Filter_Erosion_C(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg,
                                                    IN const BinaryMapType nBinaryMap, IN const INT32 nThick);
    static void _Filter_Dilation_C(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg,
                                                    IN const BinaryMapType nBinaryMap);
#else
    static void _Filter_Erosion_C(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg,
                                                    IN const BinaryMapType nBinaryMap, IN const INT32 nThick);
    static void _Filter_Dilation_C(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg,
                                                    IN const BinaryMapType nBinaryMap);
#endif


static Kakao_Status _Filter_Blur_Gaussian_1DFilter(IN Filter_T *pFilter, IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, OUT UINT8 *pDst,
                                                    IN const INT32 nWidth, IN const INT32 nHeight, IN const INT32 nStride);
static Kakao_Status _Filter_Blur_Gaussian_2DFilter(Thread_Param *pThread_Param);
static Kakao_Status _Filter_Blur_MeanFilter(Thread_Param *pThread_Param);
static Kakao_Status _Filter_Blur_NoiseReductionFilter(Thread_Param *pThread_Param);
static Kakao_Status _Filter_Blur_UnsharpenFilter(Thread_Param *pThread_Param);
static Kakao_Status _Filter_Blur_NbyNMeanFilter(Thread_Param *pThread_Param);
static Kakao_Status _Filter_Quantize_Plane(Thread_Param *pThread_Param);
static Kakao_Status _Filter_Mosaic4(Thread_Param *pThread_Param);
static Kakao_Status _Filter_Mosaic8(Thread_Param *pThread_Param);
static Kakao_Status _Filter_Mosaic16(Thread_Param *pThread_Param);
static Kakao_Status _Filter_Median_Square(Thread_Param *pThread_Param);
static Kakao_Status _Filter_Median_Horizon(Thread_Param *pThread_Param);
static Kakao_Status _Filter_Median_Vertical(Thread_Param *pThread_Param);
static Kakao_Status _Filter_Mean_Horizon(Thread_Param *pThread_Param);
static Kakao_Status _Filter_Mean_Vertical(Thread_Param *pThread_Param);
static Kakao_Status _Filter_Inverse(Thread_Param *pThread_Param);
static Kakao_Status _Filter_Clip(Thread_Param *pThread_Param);
static Kakao_Status _Filter_DiffImg(Thread_Param *pThread_Param);
static Kakao_Status _Filter_Multiply(Thread_Param *pThread_Param);
static Kakao_Status _Filter_ContrastShift(Thread_Param *pThread_Param);
static Kakao_Status _Filter_PropMixY(Thread_Param *pThread_Param);
static Kakao_Status _Filter_PropMixUV(Thread_Param *pThread_Param);



static Kakao_Status _Filter_Make_BulgePosTransMap(IN Filter_T *pFilter, IN const F32 nAspectRatio, IN const F32 nScaleFactor, IN const F32 nRadius);
static Kakao_Status _Filter_Bulge_Plane(IN Filter_T *pFilter, IN Img_T *pSrcImg, OUT Img_T *pDstImg, IN const Vec2 nCenterPos);
static Kakao_Status _Do_BubbleSort(IN UINT8 *pSrcArr, IN const UINT32 nSize);
static Kakao_Status _Filter_Make_Blur_Gaussian_1DCoeffi(IN const F32 nSigma, OUT F32 *pFilterCoeffi, OUT UINT32 *pMaskSize);
static Kakao_Status _Filter_Make_Blur_Gaussian_2DCoeffi(IN const F32 nSigma, OUT F32 *pFilterCoeffi, OUT UINT32 *pMaskSize);
static Kakao_Status _Filter_Get_Mean_Variance(IN const Img_T *pSrcImg, OUT F32 *pMean, OUT F32 *pVariance);
static Kakao_Status _Filter_Make_QuantizationTable(IN OUT const Filter_T *pFilter);
static Kakao_Status _Filter_Make_IntegralImg(IN OUT Filter_T *pFilter, IN const Img_T *pSrcImg);
static F32 _EdgeStopFunc(IN const INT32 x, IN const F32 nESigma);
static F32 _GaussianFunc(IN const INT32 m, IN const INT32 n, IN const F32 nGSigma);
#if USE_MULTI_THREAD
static void _Filter_Thread(void *pParamStruct);
#endif

#define _Filter_Erosion                     MAKE_ACCEL_FUNC(_Filter_Erosion)
#define _Filter_Dilation                    MAKE_ACCEL_FUNC(_Filter_Dilation)


/*----------------------------------------------------------------------------------------
 Static Variable
 ----------------------------------------------------------------------------------------*/
static INT32                nVerticalMask[3][3] = {{-1,0,1},{-2,0,2},{-1,0,1}};
static INT32                nHorizontalMask[3][3] = {{1,2,1},{0,0,0},{-1,-2,-1}};
static const fpMosaic       fpMosiacFunc[3] = {_Filter_Mosaic4, _Filter_Mosaic8, _Filter_Mosaic16};


/*----------------------------------------------------------------------------------------
 Global Variable
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Function Implementation
 ----------------------------------------------------------------------------------------*/
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_Create(IN OUT Filter_T **ppFilter, IN const INT32 nWidth, IN const INT32 nHeight)
{
    UINT32                  nWindowSize = 5;
    INT32                   nRangeSizeF = 0, nRangeSizeE = 0;
    INT32                   nMask = (nWindowSize - 1) >> 1;
    INT32                   i = 0, j = 0, k = 0;
    Filter_T                *pTmpFilter = NULL;
    INT32                   nAlignedWidth = GET_ALIGNED_LENGTH(nWidth, MACRO_BLOCK_SIZE);
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    SAFEALLOC(pTmpFilter, 1, 32, Filter_T);
    
    Kakao_ImgProc_Util_UpdateSize(&pTmpFilter->nBaseSize, nWidth, nHeight, NULL, IMG_FORMAT_MAX, 4);
    
    pTmpFilter->nSigmaD_Flat = 4.0f;
    pTmpFilter->nSigmaD_Edge = 2.0f;
    pTmpFilter->nSigmaR_Flat = 10.0f;
    pTmpFilter->nSigmaR_Edge = 50.f;
    pTmpFilter->nAlpha = 0.4f;
    pTmpFilter->nSigma = 0.6f;
    pTmpFilter->bIsBulgePosTransMapMade = IMGPROC_FALSE;
    pTmpFilter->pPropMixCoeffi[0] = NULL;
    pTmpFilter->pPropMixCoeffi[1] = NULL;
    
    nRangeSizeF = 2 * (INT32)(pTmpFilter->nSigmaR_Flat + 0.5);
    nRangeSizeE = 2 * (INT32)(pTmpFilter->nSigmaR_Edge + 0.5);
    
    SAFEALLOC(pTmpFilter->pMask_Edge, (nWindowSize * nWindowSize), 32, F32);
    SAFEALLOC(pTmpFilter->pMask_Flat, (nWindowSize * nWindowSize), 32, F32);
    SAFEALLOC(pTmpFilter->pRangeMask_Edge, (nRangeSizeE + 1), 32, F32);
    SAFEALLOC(pTmpFilter->pRangeMask_Flat, (nRangeSizeF + 1), 32, F32);
    SAFEALLOC(pTmpFilter->pLookUptable, 256, 32, UINT8);
    
    for(i=0 ; i<QUANT_LEVEL_MAX ; i++)
        SAFEALLOC(pTmpFilter->pQuantTable[i], 256, 32, UINT8);
    _Filter_Make_QuantizationTable(pTmpFilter);
    
    Kakao_ImgProc_Scaler_Create(&(pTmpFilter->pScaler), (nWidth>>3), (nHeight>>3));
    
    Kakao_ImgProc_Util_CreateImg(&(pTmpFilter->pTmpF32Img), nWidth, nHeight, IMG_FORMAT_GRAY, IMG_DEPTH_F32);
    Kakao_ImgProc_Util_CreateImg(&(pTmpFilter->pTmpU8Img), nWidth, nHeight, IMG_FORMAT_GRAY, IMG_DEPTH_U8);
    Kakao_ImgProc_Util_CreateImg(&(pTmpFilter->pIntegralImg), nWidth, nHeight, IMG_FORMAT_GRAY, IMG_DEPTH_U32);

    
    for(j=-nMask; j<=nMask; j++)
    {
        for(i=-nMask; i<=nMask; i++, k++)
        {
            pTmpFilter->pMask_Edge[k] = (2*pTmpFilter->nSigmaD_Edge - abs(i))*(2*pTmpFilter->nSigmaD_Edge - abs(j));
            pTmpFilter->pMask_Flat[k] = (2*pTmpFilter->nSigmaD_Flat - abs(i))*(2*pTmpFilter->nSigmaD_Flat - abs(j));
        }
    }
    
    for(j=0; j<=nRangeSizeE; j++)
        pTmpFilter->pRangeMask_Edge[j]=2 * pTmpFilter->nSigmaR_Edge - j;
    
    for(j=0; j<=nRangeSizeF; j++)
        pTmpFilter->pRangeMask_Flat[j]=2 * pTmpFilter->nSigmaR_Flat - j;
    
    _Filter_Make_Blur_Gaussian_1DCoeffi(pTmpFilter->nSigma, &pTmpFilter->nGaussian1DFltCoeffi[0], &pTmpFilter->nGaussianMask1DSize);
    _Filter_Make_Blur_Gaussian_2DCoeffi(pTmpFilter->nSigma, &pTmpFilter->nGaussian2DFltCoeffi[0], &pTmpFilter->nGaussianMask2DSize);
    
    #if USE_MULTI_THREAD
    {
        INT32           nNumofCPUCore = 0;
        
        Kakao_ImgProc_Util_Get_CPUCores(&nNumofCPUCore);
        pTmpFilter->nNumofCPUCore = nNumofCPUCore = GET_PROPER_THREAD_NUM(nNumofCPUCore);
        
        if(1 < nNumofCPUCore)
        {
            SAFEALLOC(pTmpFilter->pThreadID, nNumofCPUCore * sizeof(ThreadID), 32, ThreadID);
            SAFEALLOC(pTmpFilter->pThreadParam, nNumofCPUCore * sizeof(Thread_Param), 32, Thread_Param);
            
            pTmpFilter->bIsMultiThreadEnabled = IMGPROC_TRUE;
            for(i=0 ; i<nNumofCPUCore ; i++)
            {
                Thread_Param            *pThread_Param = &(pTmpFilter->pThreadParam[i]);
                
                MEMSET(pThread_Param, 0, sizeof(Thread_Param));
                pThread_Param->pFilter = pTmpFilter;
                pThread_Param->nThreadIdx = i;

                SEMA_CREATE(&(pTmpFilter->nStartSema[i]), 0, 0);
                SEMA_CREATE(&(pTmpFilter->nEndSema[i]), 0, 0);
                
                THREAD_CREATE((pTmpFilter->pThreadID[i]), NULL, &_Filter_Thread, pThread_Param)
            }
        }
        else
            pTmpFilter->bIsMultiThreadEnabled = IMGPROC_FALSE;
    }
    #else
        pTmpFilter->nNumofCPUCore = 1;
    #endif
    
    *ppFilter = pTmpFilter;

    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    Kakao_ImgProc_Filter_Destroy(&pTmpFilter);
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_Destroy(IN OUT Filter_T **ppFilter)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   i = 0;
    
    CHECK_POINTER_VALIDATION((*ppFilter), KAKAO_STAT_NULL_POINTER);

    Kakao_ImgProc_Scaler_Destroy(&((*ppFilter)->pScaler));
    Kakao_ImgProc_Edge_Destroy(&((*ppFilter)->pEdgeDetector));
    
    Kakao_ImgProc_Util_ReleaseImg(&((*ppFilter)->pTmpF32Img));
    Kakao_ImgProc_Util_ReleaseImg(&((*ppFilter)->pTmpU8Img));
    Kakao_ImgProc_Util_ReleaseImg(&((*ppFilter)->pIntegralImg));
    Kakao_ImgProc_Util_ReleaseImg(&((*ppFilter)->pScaledImg));
    Kakao_ImgProc_Util_ReleaseImg(&((*ppFilter)->pPosTransMap));
    for(i=0 ; i<4 ; i++)
        Kakao_ImgProc_Util_ReleaseImg(&((*ppFilter)->pSBTmpBuf[i]));
    
    SAFEFREE((*ppFilter)->pMask_Edge);
    SAFEFREE((*ppFilter)->pMask_Flat);
    SAFEFREE((*ppFilter)->pRangeMask_Edge);
    SAFEFREE((*ppFilter)->pRangeMask_Flat);
    SAFEFREE((*ppFilter)->pLookUptable);
    SAFEFREE((*ppFilter)->pPropMixCoeffi[0]);
    SAFEFREE((*ppFilter)->pPropMixCoeffi[1]);
    
    for(i=0 ; i<QUANT_LEVEL_MAX ; i++)
        SAFEFREE((*ppFilter)->pQuantTable[i]);

    #if USE_MULTI_THREAD
    {
        const UINT32          nNumofCPUCore = (*ppFilter)->nNumofCPUCore;
        
        if(IMGPROC_TRUE == (*ppFilter)->bIsMultiThreadEnabled)
        {
            (*ppFilter)->bIsMultiThreadEnabled = IMGPROC_FALSE;
            for(i=0 ; i<nNumofCPUCore ; i++)
            {
                SEMA_POST(&((*ppFilter)->nStartSema[i]));
                SEMA_POST(&((*ppFilter)->nEndSema[i]));
                THREAD_JOIN((*ppFilter)->pThreadID[i]);
                SEMA_DESTROY(&((*ppFilter)->nStartSema[i]));
                SEMA_DESTROY(&((*ppFilter)->nEndSema[i]));
            }
            
            SAFEFREE((*ppFilter)->pThreadID);
            SAFEFREE((*ppFilter)->pThreadParam);
        }
    }
    #endif
    
    SAFEFREE((*ppFilter));
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_UpdateImgSize(IN OUT Filter_T *pFilter, IN const INT32 nWidth, IN const INT32 nHeight)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   i = 0;
    
    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    
    Kakao_ImgProc_Util_UpdateSize(&pFilter->nBaseSize, nWidth, nHeight, NULL, IMG_FORMAT_MAX, 4);
    Kakao_ImgProc_Util_UpdateImgSize(pFilter->pTmpF32Img, nWidth, nHeight, NULL, IMG_FORMAT_GRAY);
    Kakao_ImgProc_Util_UpdateImgSize(pFilter->pTmpU8Img, nWidth, nHeight, NULL, IMG_FORMAT_GRAY);
    Kakao_ImgProc_Util_UpdateImgSize(pFilter->pIntegralImg, nWidth, nHeight, NULL, IMG_FORMAT_GRAY);
    
    // Allocate Image Buffer
    for(i=0 ; i<3 ; i++)
    {
        if(NULL != pFilter->pSBTmpBuf[i])
            Kakao_ImgProc_Util_ReleaseImg(&(pFilter->pSBTmpBuf[i]));
        
        if(NULL == pFilter->pSBTmpBuf[i])
            Kakao_ImgProc_Util_CreateImg(&(pFilter->pSBTmpBuf[i]), nWidth, nHeight, IMG_FORMAT_I420, IMG_DEPTH_U8);
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_TrilateralFlt(IN const Filter_T *pFilter, Img_T *pSrcImg0, Img_T *pSrcImg1, UINT8 *input0, UINT8 *input1)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg0, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg1, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(input0, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(input1, KAKAO_STAT_NULL_POINTER);
    
    {
        EdgeDet_T               *pEdgeDetector = pFilter->pEdgeDetector;
        //INT32                   i = 0, n = 0;
        INT32                   nWidth = pSrcImg0->GetImgWidth(pSrcImg0, IMG_PLANE_0);
        INT32                   nHeight = pSrcImg0->GetImgHeight(pSrcImg0, IMG_PLANE_0);
        INT32                   nStride = pSrcImg0->GetImgStride(pSrcImg0, IMG_PLANE_0);
        F32                     *pMask_Edge = pFilter->pMask_Edge;
        F32                     *pRangeMask_Edge = pFilter->pRangeMask_Edge;
        F32                     *pMask_Flat = pFilter->pMask_Flat;
        F32                     *pRangeMask_Flat = pFilter->pRangeMask_Flat;
        UINT8                   *pIn0 = input0;
        UINT8                   *pIn1 = input1;
        UINT8                   *pOut0 = NULL;
        UINT8                   *pOut1 = NULL;
        INT32                   winsize;
        INT32                   nRangeSizeE,nRangeSizeF;
        INT32                   r, c, k, l;
        INT32                   Mask_n;
        F32                     sum1;
        F32                     sum_temp;
        INT32                   dy;
        F32                     norm;
        INT32                   x, y;
        F32                     nSigmaR_Flat = 0.0f;
        F32                     nSigmaR_Edge = 0.0f;
    
        CHECK_POINTER_VALIDATION(pEdgeDetector, KAKAO_STAT_NULL_POINTER);
        CHECK_POINTER_VALIDATION(pMask_Edge, KAKAO_STAT_NULL_POINTER);
        CHECK_POINTER_VALIDATION(pRangeMask_Edge, KAKAO_STAT_NULL_POINTER);
        CHECK_POINTER_VALIDATION(pMask_Flat, KAKAO_STAT_NULL_POINTER);
        CHECK_POINTER_VALIDATION(pRangeMask_Flat, KAKAO_STAT_NULL_POINTER);
        CHECK_POINTER_VALIDATION(pIn0, KAKAO_STAT_NULL_POINTER);
        CHECK_POINTER_VALIDATION(pIn1, KAKAO_STAT_NULL_POINTER);
        
        //////////////////////////////////////////////////////
        //		 Bilateral Filter¿« Parameter ¡∂¿˝«œ±‚		//
        //////////////////////////////////////////////////////
        
        // Domain filter sigma
        //F32 nSigmaD_Flat = pFilter->nSigmaD_Flat;
        //F32 nSigmaD_Edge = pFilter->nSigmaD_Edge;
        
        // Range filter sigma
        nSigmaR_Flat = pFilter->nSigmaR_Flat;
        nSigmaR_Edge = pFilter->nSigmaR_Edge;
        
        // Mask size
        winsize = 5;
        nRangeSizeF = (2*(INT32)(nSigmaR_Flat+0.5));
        nRangeSizeE = (2*(INT32)(nSigmaR_Edge+0.5));
        Mask_n=(winsize-1)>>1;
        
        
        // Motion Adaptive Bilateral Mask
        for(r=0 ; r<nHeight ; r++)
        {
            for(c=0 ; c<nWidth ; c++)
            {
                dy = 0;
                sum1 = 0.0;
                norm = 0.0;
                
                if((UINT8)(ABSM(pIn0[r*nStride + c]-pIn1[r*nStride + c])) >= 2)
                {
                    for(l=-Mask_n ; l<=Mask_n ; l++)
                    {
                        for(k=-Mask_n ; k<=Mask_n ; k++)
                        {
                            dy = (INT32)abs(pIn0[(r+l)*nStride+(c+k)] - pIn0[r*nStride+c]);
                            
                            if (dy <= nRangeSizeE)
                            {
                                sum1 += pIn0[(r+l)*nStride + (c+k)] * pMask_Edge[(l+Mask_n)*winsize+(k+Mask_n)] * pRangeMask_Edge[dy];
                                norm += pMask_Edge[(l+Mask_n)*winsize+(k+Mask_n)]*pRangeMask_Edge[dy];
                            }
                        }
                    }
                    sum1=sum1/norm;
                    if(sum1<0)	pOut0[r*nStride + c]=0;
                    else if (sum1>255) pOut0[r*nStride + c]=255;
                    else pOut0[r*nStride + c] = (UINT8)sum1;
                }
                else
                {
                    for(l=-Mask_n ; l<=Mask_n ; l++)
                    {
                        for(k=-Mask_n ; k<=Mask_n ; k++)
                        {
                            dy = (INT32)abs(pIn0[(r+l)*nStride+(c+k)] - pIn0[(r*nStride)+c]);
                            if (dy <= nRangeSizeF)
                            {
                                sum1 += pIn0[(r+l)*nStride + (c+k)]*pMask_Flat[(l+Mask_n)*winsize+(k+Mask_n)]*pRangeMask_Flat[dy];
                                norm += pMask_Flat[(l+Mask_n)*winsize+(k+Mask_n)]*pRangeMask_Flat[dy];
                            }
                        }
                    }
                    sum1=sum1/norm;
                    if(sum1<0)	pOut0[r*nStride + c]=0;
                    else if (sum1>255) pOut0[r*nStride + c]=255;
                    else pOut0[r*nStride + c] = (UINT8)sum1;
                }
            }
        }
        
        // Edge Adaptive High Pass Filter By Sobel filter
        {
            F32       alpha = 0;//pFilter->alpha;
            UINT8   *pEdgeDetectFramewiseMap0 = NULL;//&pEdgeDetector->pEdgeDetectFramewiseMap0[nStride*nPaddingSize + nPaddingSize];
            
            for(x=0 ; x<nHeight ; x++)
            {
                for(y=0 ; y<nWidth ; y++)
                {
                    #if 0
                    INT32       vertical_var = 0, horizontal_var = 0, ret_var = 0;
                    
                    for(m=-1 ; m<=1 ; m++)
                    {
                        for(n=-1 ; n<=1 ; n++)
                        {
                            vertical_var += pOut0[((x+m) * nStride) + (y+n)] * nVerticalMask[m+1][n+1];
                            horizontal_var += pOut0[((x+m) * nStride) + (y+n)] * nHorizontalMask[m+1][n+1];
                        }
                    }
                    vertical_var = abs(vertical_var);
                    horizontal_var = abs(horizontal_var);
                    ret_var = vertical_var + horizontal_var;
                    #endif
                    
                    if(pEdgeDetectFramewiseMap0[y*nStride + x])
                    {
                        sum1=0.0;
                        sum_temp = 0.0;
                        
                        sum1 =                                                                  -(pOut0[(x-1)*nStride + (y)] * alpha) +
                        -(pOut0[(x)*nStride + (y-1)] * alpha) + (pOut0[(x)*nStride + (y)] * (4*alpha +1.0f)) - (pOut0[(x)*nStride + (y+1)] *alpha) +
                        -(pOut0[(x+1)*nStride + (y)] * alpha);
                        
                        sum_temp = (F32)(fabs(sum1));
                        if(sum_temp<0)	pOut1[x*nStride + y]=0;
                        else if (sum_temp>255) pOut1[x*nStride + y]=255;
                        else pOut1[x*nStride + y] = (UINT8)sum_temp;
                    }
                    else
                        pOut1[x*nStride + y] = pOut0[x*nStride + y];
                }
            }
        }
    }

    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_BilateralFlt(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg, IN const UINT32 nMaskSize, IN const F32 nGSigma, IN const F32 nESigma)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER);
    
    {
        INT32                   i = 0, j = 0;
        INT32                   m = 0, n = 0;
        INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
        INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
        INT32                   nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
        INT32                   Ip = 0, Is = 0;
        INT32                   Xsize = 0, Ysize = 0;
        F32                     nSum = 0.0, nNormTerm = 0.0;
        UINT8                   *pSrc = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
        UINT8                   *pDst = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0);
        
        CHECK_POINTER_VALIDATION(pSrc, KAKAO_STAT_NULL_POINTER);
        CHECK_POINTER_VALIDATION(pDst, KAKAO_STAT_NULL_POINTER);

        Xsize = (INT32)(nMaskSize/2);
        Ysize = (INT32)(nMaskSize/2);
        
        for(j=0 ; j<nHeight ; j++)
        {
            for(i=0 ; i<nWidth ; i++)
            {
                nNormTerm=0.0;
                Is = pSrc[j*nStride + i];
                
                // Normalization term
                for(n=-Ysize ; n<=Ysize ; n++)
                {
                    for(m=-Xsize ; m<=Xsize ; m++)
                    {
                        Ip = pSrc[(j + n) * nStride + (i + m)];
                        nNormTerm += _GaussianFunc(m, n, nGSigma) * _EdgeStopFunc((Ip-Is), nESigma);
                    }
                }
                
                nSum = 0.0;
                // bilateral filtering
                for(n=-Ysize ; n<=Ysize ; n++)
                {
                    for(m=-Xsize ; m<=Xsize ; m++)
                    {
                        Ip = pSrc[(j + n) * nStride + (i + m)];
                        nSum += _GaussianFunc(m, n, nGSigma) * _EdgeStopFunc((Ip-Is), nESigma) * Ip;
                    }
                }
                
                nSum /= nNormTerm;
                
                pDst[(j*nStride) + i] = (UINT8)(CLIP3(PIXEL_MIN, PIXEL_MAX, nSum));
            }
        }
    }

    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_Gaussian1DBlur(IN Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   i = 0;
    
    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER);
    
    for(i=0 ; i<pSrcImg->GetNumofImgPlanes(pSrcImg) ; i++)
    {
        const UINT8         *pSrc0 = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, i);
        const UINT8         *pSrc1 = NULL;
        UINT8               *pDst = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, i);
        const INT32         nWidth = pSrcImg->GetImgWidth(pSrcImg, i);
        const INT32         nHeight = pSrcImg->GetImgHeight(pSrcImg, i);
        const INT32         nStride = pSrcImg->GetImgStride(pSrcImg, i);
        
        _Filter_Blur_Gaussian_1DFilter(pFilter, pSrc0, pSrc1, pDst, nWidth, nHeight, nStride);
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_Gaussian2DBlur(IN Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER);
    
    #if USE_MULTI_THREAD
    {
        if(IMGPROC_TRUE == pFilter->bIsMultiThreadEnabled)
        {
            INT32           i = 0, j = 0;
            UINT32          nNumofCPUCore = pFilter->nNumofCPUCore;
            
            for(i=0 ; i<pSrcImg->GetNumofImgPlanes(pSrcImg) ; i++)
            {
                for(j=0 ; j<nNumofCPUCore ; j++)
                {
                    Thread_Param        *pThread_Param = (Thread_Param *)(&pFilter->pThreadParam[j]);
                    
                    pThread_Param->pFilterFunc      = _Filter_Blur_Gaussian_2DFilter;
                    pThread_Param->pFilter          = pFilter;
                    pThread_Param->pSrc0            = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, i);
                    pThread_Param->pSrc1            = NULL;
                    pThread_Param->pDst             = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, i);
                    pThread_Param->nWidth           = pSrcImg->GetImgWidth(pSrcImg, i);
                    pThread_Param->nHeight          = pSrcImg->GetImgHeight(pSrcImg, i);
                    pThread_Param->nSrcStride       = pSrcImg->GetImgStride(pSrcImg, i);
                    pThread_Param->nDstStride       = pDstImg->GetImgStride(pDstImg, i);

                    SEMA_POST(&(pFilter->nStartSema[j]));
                }
                
                // Wait Until Done All Processes of Threads
                for(j=0 ; j<nNumofCPUCore ; j++)
                    SEMA_WAIT(&(pFilter->nEndSema[j]));
            }
        }
        else
        {
            INT32           i = 0;
            
            for(i=0 ; i<pSrcImg->GetNumofImgPlanes(pSrcImg) ; i++)
            {
                Thread_Param        nThread_Param = {pFilter, };
                
                nThread_Param.pSrc0             = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, i);
                nThread_Param.pSrc1             = NULL;
                nThread_Param.pDst              = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, i);
                nThread_Param.nWidth            = pSrcImg->GetImgWidth(pSrcImg, i);
                nThread_Param.nHeight           = pSrcImg->GetImgHeight(pSrcImg, i);
                nThread_Param.nSrcStride        = pSrcImg->GetImgStride(pSrcImg, i);
                nThread_Param.nDstStride        = pDstImg->GetImgStride(pDstImg, i);

                _Filter_Blur_Gaussian_2DFilter(&nThread_Param);
            }
        }
    }
    #else
    {
        INT32           i = 0;
        
        for(i=0 ; i<pSrcImg->GetNumofImgPlanes(pSrcImg) ; i++)
        {
            Thread_Param        nThread_Param = {pFilter, };
            
            nThread_Param.pSrc0             = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, i);
            nThread_Param.pSrc1             = NULL;
            nThread_Param.pDst              = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, i);
            nThread_Param.nWidth            = pSrcImg->GetImgWidth(pSrcImg, i);
            nThread_Param.nHeight           = pSrcImg->GetImgHeight(pSrcImg, i);
            nThread_Param.nSrcStride        = pSrcImg->GetImgStride(pSrcImg, i);
            nThread_Param.nDstStride        = pDstImg->GetImgStride(pDstImg, i);
            
            _Filter_Blur_Gaussian_2DFilter(&nThread_Param);
        }
    }
    #endif
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_MeanBlur(IN Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER);
    
    ___START_PROFILE(FilterImg)
    
    #if USE_MULTI_THREAD
    {
        if(IMGPROC_TRUE == pFilter->bIsMultiThreadEnabled)
        {
            INT32           i = 0, j = 0;
            UINT32          nNumofCPUCore = pFilter->nNumofCPUCore;
        
            for(i=0 ; i<pSrcImg->GetNumofImgPlanes(pSrcImg) ; i++)
            {
                for(j=0 ; j<nNumofCPUCore ; j++)
                {
                    Thread_Param        *pThread_Param = (Thread_Param *)(&pFilter->pThreadParam[j]);
                    
                    pThread_Param->pFilterFunc      = _Filter_Blur_MeanFilter;
                    pThread_Param->pFilter          = pFilter;
                    pThread_Param->pSrc0            = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, i);
                    pThread_Param->pSrc1            = NULL;
                    pThread_Param->pDst             = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, i);
                    pThread_Param->nWidth           = pSrcImg->GetImgWidth(pSrcImg, i);
                    pThread_Param->nHeight          = pSrcImg->GetImgHeight(pSrcImg, i);
                    pThread_Param->nSrcStride       = pSrcImg->GetImgStride(pSrcImg, i);
                    pThread_Param->nDstStride       = pDstImg->GetImgStride(pDstImg, i);
                    
                    SEMA_POST(&(pFilter->nStartSema[j]));
                }
                
                // Wait Until Done All Processes of Threads
                for(j=0 ; j<nNumofCPUCore ; j++)
                    SEMA_WAIT(&(pFilter->nEndSema[j]));
            }
        }
        else
        {
            INT32           i = 0;
            
            for(i=0 ; i<pSrcImg->GetNumofImgPlanes(pSrcImg) ; i++)
            {
                Thread_Param        nThread_Param = {pFilter, };
                
                nThread_Param.pSrc0             = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, i);
                nThread_Param.pSrc1             = NULL;
                nThread_Param.pDst              = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, i);
                nThread_Param.nWidth            = pSrcImg->GetImgWidth(pSrcImg, i);
                nThread_Param.nHeight           = pSrcImg->GetImgHeight(pSrcImg, i);
                nThread_Param.nSrcStride        = pSrcImg->GetImgStride(pSrcImg, i);
                nThread_Param.nDstStride        = pDstImg->GetImgStride(pDstImg, i);

                _Filter_Blur_MeanFilter(&nThread_Param);
            }
        }
    }
    #else
    {
        INT32           i = 0;
        
        for(i=0 ; i<pSrcImg->GetNumofImgPlanes(pSrcImg) ; i++)
        {
            Thread_Param        nThread_Param = {pFilter, };
            
            nThread_Param.pSrc0             = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, i);
            nThread_Param.pSrc1             = NULL;
            nThread_Param.pDst              = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, i);
            nThread_Param.nWidth            = pSrcImg->GetImgWidth(pSrcImg, i);
            nThread_Param.nHeight           = pSrcImg->GetImgHeight(pSrcImg, i);
            nThread_Param.nSrcStride        = pSrcImg->GetImgStride(pSrcImg, i);
            nThread_Param.nDstStride        = pDstImg->GetImgStride(pDstImg, i);
            
            _Filter_Blur_MeanFilter(&nThread_Param);
        }
    }
    #endif
    
    ___STOP_PROFILE(FilterImg)
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_MeanBlur_Plane(IN Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg, IN const UINT32 nPlaneIdx)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER);
    
    ___START_PROFILE(FilterImg)
    
    #if USE_MULTI_THREAD
    {
        if(IMGPROC_TRUE == pFilter->bIsMultiThreadEnabled)
        {
            INT32           j = 0;
            UINT32          nNumofCPUCore = pFilter->nNumofCPUCore;

            for(j=0 ; j<nNumofCPUCore ; j++)
            {
                Thread_Param        *pThread_Param = (Thread_Param *)(&pFilter->pThreadParam[j]);
                
                pThread_Param->pFilterFunc          = _Filter_Blur_MeanFilter;
                pThread_Param->pSrc0                = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, nPlaneIdx);
                pThread_Param->pSrc1                = NULL;
                pThread_Param->pDst                 = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, nPlaneIdx);
                pThread_Param->nWidth               = pSrcImg->GetImgWidth(pSrcImg, nPlaneIdx);
                pThread_Param->nHeight              = pSrcImg->GetImgHeight(pSrcImg, nPlaneIdx);
                pThread_Param->nSrcStride           = pSrcImg->GetImgStride(pSrcImg, nPlaneIdx);
                pThread_Param->nDstStride           = pDstImg->GetImgStride(pDstImg, nPlaneIdx);
                
                SEMA_POST(&(pFilter->nStartSema[j]));
            }
            
            // Wait Until Done All Processes of Threads
            for(j=0 ; j<nNumofCPUCore ; j++)
                SEMA_WAIT(&(pFilter->nEndSema[j]));
        }
        else
        {
            Thread_Param        nThread_Param = {pFilter, };
            
            nThread_Param.pSrc0                 = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, nPlaneIdx);
            nThread_Param.pSrc1                 = NULL;
            nThread_Param.pDst                  = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, nPlaneIdx);
            nThread_Param.nWidth                = pSrcImg->GetImgWidth(pSrcImg, nPlaneIdx);
            nThread_Param.nHeight               = pSrcImg->GetImgHeight(pSrcImg, nPlaneIdx);
            nThread_Param.nSrcStride            = pSrcImg->GetImgStride(pSrcImg, nPlaneIdx);
            nThread_Param.nDstStride            = pDstImg->GetImgStride(pDstImg, nPlaneIdx);

            _Filter_Blur_MeanFilter(&nThread_Param);
        }
    }
    #else
    {
        Thread_Param        nThread_Param = {pFilter, };
        
        nThread_Param.pSrc0                 = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, nPlaneIdx);
        nThread_Param.pSrc1                 = NULL;
        nThread_Param.pDst                  = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, nPlaneIdx);
        nThread_Param.nWidth                = pSrcImg->GetImgWidth(pSrcImg, nPlaneIdx);
        nThread_Param.nHeight               = pSrcImg->GetImgHeight(pSrcImg, nPlaneIdx);
        nThread_Param.nSrcStride            = pSrcImg->GetImgStride(pSrcImg, nPlaneIdx);
        nThread_Param.nDstStride            = pDstImg->GetImgStride(pDstImg, nPlaneIdx);
        
        _Filter_Blur_MeanFilter(&nThread_Param);
    }
    #endif
    
    ___STOP_PROFILE(FilterImg)
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_MeanBlur_Horizon(IN Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER);
    
    ___START_PROFILE(FilterImg)
    
    #if USE_MULTI_THREAD
    {
        if(IMGPROC_TRUE == pFilter->bIsMultiThreadEnabled)
        {
            INT32           i = 0, j = 0;
            UINT32          nNumofCPUCore = pFilter->nNumofCPUCore;
            
            for(i=0 ; i<pSrcImg->GetNumofImgPlanes(pSrcImg) ; i++)
            {
                for(j=0 ; j<nNumofCPUCore ; j++)
                {
                    Thread_Param        *pThread_Param = (Thread_Param *)(&pFilter->pThreadParam[j]);
                    
                    pThread_Param->pFilterFunc = _Filter_Mean_Horizon;
                    pThread_Param->pSrc0 = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, i);
                    pThread_Param->pSrc1 = NULL;
                    pThread_Param->pDst = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, i);
                    pThread_Param->nWidth = pSrcImg->GetImgWidth(pSrcImg, i);
                    pThread_Param->nHeight = pSrcImg->GetImgHeight(pSrcImg, i);
                    pThread_Param->nSrcStride = pSrcImg->GetImgStride(pSrcImg, i);
                    pThread_Param->nDstStride = pDstImg->GetImgStride(pDstImg, i);
                    
                    SEMA_POST(&(pFilter->nStartSema[j]));
                }
                
                // Wait Until Done All Processes of Threads
                for(j=0 ; j<nNumofCPUCore ; j++)
                    SEMA_WAIT(&(pFilter->nEndSema[j]));
            }
        }
        else
        {
            INT32           i = 0;
            
            for(i=0 ; i<pSrcImg->GetNumofImgPlanes(pSrcImg) ; i++)
            {
                Thread_Param        nThread_Param = {pFilter, };
                
                nThread_Param.pSrc0             = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, i);
                nThread_Param.pSrc1             = NULL;
                nThread_Param.pDst              = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, i);
                nThread_Param.nWidth            = pSrcImg->GetImgWidth(pSrcImg, i);
                nThread_Param.nHeight           = pSrcImg->GetImgHeight(pSrcImg, i);
                nThread_Param.nSrcStride        = pSrcImg->GetImgStride(pSrcImg, i);
                nThread_Param.nDstStride        = pDstImg->GetImgStride(pDstImg, i);
                
                _Filter_Mean_Horizon(&nThread_Param);
            }
        }
    }
    #else
    {
        INT32           i = 0;
        
        for(i=0 ; i<pSrcImg->GetNumofImgPlanes(pSrcImg) ; i++)
        {
            Thread_Param        nThread_Param = {pFilter, };
            
            nThread_Param.pSrc0             = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, i);
            nThread_Param.pSrc1             = NULL;
            nThread_Param.pDst              = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, i);
            nThread_Param.nWidth            = pSrcImg->GetImgWidth(pSrcImg, i);
            nThread_Param.nHeight           = pSrcImg->GetImgHeight(pSrcImg, i);
            nThread_Param.nSrcStride        = pSrcImg->GetImgStride(pSrcImg, i);
            nThread_Param.nDstStride        = pDstImg->GetImgStride(pDstImg, i);
            
            _Filter_Mean_Horizon(&nThread_Param);
        }
    }
    #endif
    
    ___STOP_PROFILE(FilterImg)
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_MeanBlur_Vertical(IN Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER);
    
    ___START_PROFILE(FilterImg)
    
    #if USE_MULTI_THREAD
    {
        if(IMGPROC_TRUE == pFilter->bIsMultiThreadEnabled)
        {
            INT32           i = 0, j = 0;
            UINT32          nNumofCPUCore = pFilter->nNumofCPUCore;
            
            for(i=0 ; i<pSrcImg->GetNumofImgPlanes(pSrcImg) ; i++)
            {
                for(j=0 ; j<nNumofCPUCore ; j++)
                {
                    Thread_Param        *pThread_Param = (Thread_Param *)(&pFilter->pThreadParam[j]);
                    
                    pThread_Param->pFilterFunc      = _Filter_Mean_Vertical;
                    pThread_Param->pFilter          = pFilter;
                    pThread_Param->pSrc0            = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, i);
                    pThread_Param->pSrc1            = NULL;
                    pThread_Param->pDst             = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, i);
                    pThread_Param->nWidth           = pSrcImg->GetImgWidth(pSrcImg, i);
                    pThread_Param->nHeight          = pSrcImg->GetImgHeight(pSrcImg, i);
                    pThread_Param->nSrcStride       = pSrcImg->GetImgStride(pSrcImg, i);
                    pThread_Param->nDstStride       = pDstImg->GetImgStride(pDstImg, i);
                    
                    SEMA_POST(&(pFilter->nStartSema[j]));
                }
                
                // Wait Until Done All Processes of Threads
                for(j=0 ; j<nNumofCPUCore ; j++)
                    SEMA_WAIT(&(pFilter->nEndSema[j]));
            }
        }
        else
        {
            INT32               i = 0;
            
            for(i=0 ; i<pSrcImg->GetNumofImgPlanes(pSrcImg) ; i++)
            {
                Thread_Param        nThread_Param = {pFilter, };
                
                nThread_Param.pSrc0             = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, i);
                nThread_Param.pSrc1             = NULL;
                nThread_Param.pDst              = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, i);
                nThread_Param.nWidth            = pSrcImg->GetImgWidth(pSrcImg, i);
                nThread_Param.nHeight           = pSrcImg->GetImgHeight(pSrcImg, i);
                nThread_Param.nSrcStride        = pSrcImg->GetImgStride(pSrcImg, i);
                nThread_Param.nDstStride        = pDstImg->GetImgStride(pDstImg, i);
                
                _Filter_Mean_Vertical(&nThread_Param);
            }
        }
    }
    #else
    {
        INT32           i = 0;
        
        for(i=0 ; i<pSrcImg->GetNumofImgPlanes(pSrcImg) ; i++)
        {
            Thread_Param        nThread_Param = {pFilter, };
            
            nThread_Param.pSrc0             = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, i);
            nThread_Param.pSrc1             = NULL;
            nThread_Param.pDst              = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, i);
            nThread_Param.nWidth            = pSrcImg->GetImgWidth(pSrcImg, i);
            nThread_Param.nHeight           = pSrcImg->GetImgHeight(pSrcImg, i);
            nThread_Param.nSrcStride        = pSrcImg->GetImgStride(pSrcImg, i);
            nThread_Param.nDstStride        = pDstImg->GetImgStride(pDstImg, i);
            
            _Filter_Mean_Vertical(&nThread_Param);
        }
    }
    #endif
    
    ___STOP_PROFILE(FilterImg)
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_NbyNMeanBlur(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg, IN const INT32 nMaskSize)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   i = 0;
    
    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER);
    
    for(i=0 ; i<pSrcImg->GetNumofImgPlanes(pSrcImg) ; i++)
    {
        Thread_Param        nThread_Param = {(Filter_T *)pFilter, };
        
        nThread_Param.pSrc0             = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, i);
        nThread_Param.pSrc1             = NULL;
        nThread_Param.pDst              = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, i);
        nThread_Param.nWidth            = pSrcImg->GetImgWidth(pSrcImg, i);
        nThread_Param.nHeight           = pSrcImg->GetImgHeight(pSrcImg, i);
        nThread_Param.nSrcStride        = pSrcImg->GetImgStride(pSrcImg, i);
        nThread_Param.nDstStride        = pDstImg->GetImgStride(pDstImg, i);
        nThread_Param.nNbyNMeanMaskSize = nMaskSize;

        _Filter_Blur_NbyNMeanFilter(&nThread_Param);
    }

    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_SharpenFlt(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER);
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_SharpenFlt_Plane(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg, IN const UINT32 nPlaneIdx)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER);
    
    #if USE_MULTI_THREAD
    {
        if(IMGPROC_TRUE == pFilter->bIsMultiThreadEnabled)
        {
            INT32           j = 0;
            UINT32          nNumofCPUCore = pFilter->nNumofCPUCore;
            
            for(j=0 ; j<nNumofCPUCore ; j++)
            {
                Thread_Param        *pThread_Param = (Thread_Param *)(&pFilter->pThreadParam[j]);
                
                pThread_Param->pFilterFunc      = _Filter_Blur_UnsharpenFilter;
                pThread_Param->pFilter          = pFilter;
                pThread_Param->pSrc0            = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, nPlaneIdx);
                pThread_Param->pSrc1            = NULL;
                pThread_Param->pDst             = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, nPlaneIdx);
                pThread_Param->nWidth           = pSrcImg->GetImgWidth(pSrcImg, nPlaneIdx);
                pThread_Param->nHeight          = pSrcImg->GetImgHeight(pSrcImg, nPlaneIdx);
                pThread_Param->nSrcStride       = pSrcImg->GetImgStride(pSrcImg, nPlaneIdx);
                pThread_Param->nDstStride       = pDstImg->GetImgStride(pDstImg, nPlaneIdx);
                
                SEMA_POST(&(pFilter->nStartSema[j]));
            }
            
            // Wait Until Done All Processes of Threads
            for(j=0 ; j<nNumofCPUCore ; j++)
                SEMA_WAIT(&(pFilter->nEndSema[j]));
        }
        else
        {
            Thread_Param        nThread_Param = {(Filter_T *)pFilter, };
            
            nThread_Param.pSrc0                 = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, nPlaneIdx);
            nThread_Param.pSrc1                 = NULL;
            nThread_Param.pDst                  = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, nPlaneIdx);
            nThread_Param.nWidth                = pSrcImg->GetImgWidth(pSrcImg, nPlaneIdx);
            nThread_Param.nHeight               = pSrcImg->GetImgHeight(pSrcImg, nPlaneIdx);
            nThread_Param.nSrcStride            = pSrcImg->GetImgStride(pSrcImg, nPlaneIdx);
            nThread_Param.nDstStride            = pDstImg->GetImgStride(pDstImg, nPlaneIdx);
            
            _Filter_Blur_UnsharpenFilter(&nThread_Param);
        }
    }
    #else
    {
        Thread_Param        nThread_Param = {(Filter_T *)pFilter, };
        
        nThread_Param.pSrc0                 = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, nPlaneIdx);
        nThread_Param.pSrc1                 = NULL;
        nThread_Param.pDst                  = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, nPlaneIdx);
        nThread_Param.nWidth                = pSrcImg->GetImgWidth(pSrcImg, nPlaneIdx);
        nThread_Param.nHeight               = pSrcImg->GetImgHeight(pSrcImg, nPlaneIdx);
        nThread_Param.nSrcStride            = pSrcImg->GetImgStride(pSrcImg, nPlaneIdx);
        nThread_Param.nDstStride            = pDstImg->GetImgStride(pDstImg, nPlaneIdx);
        
        _Filter_Blur_UnsharpenFilter(&nThread_Param);
    }
    #endif

    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_NoiseReduction_Plane(IN Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg, IN const UINT32 nPlaneIdx)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER);
    
    ___START_PROFILE(FilterImg)
    
    #if USE_MULTI_THREAD
    {
        if(IMGPROC_TRUE == pFilter->bIsMultiThreadEnabled)
        {
            INT32           j = 0;
            UINT32          nNumofCPUCore = pFilter->nNumofCPUCore;
            
            for(j=0 ; j<nNumofCPUCore ; j++)
            {
                Thread_Param        *pThread_Param = (Thread_Param *)(&pFilter->pThreadParam[j]);
                
                pThread_Param->pFilterFunc = _Filter_Blur_MeanFilter;
                pThread_Param->pFilter          = pFilter;
                pThread_Param->pSrc0            = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, nPlaneIdx);
                pThread_Param->pSrc1            = NULL;
                pThread_Param->pDst             = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, nPlaneIdx);
                pThread_Param->nWidth           = pSrcImg->GetImgWidth(pSrcImg, nPlaneIdx);
                pThread_Param->nHeight          = pSrcImg->GetImgHeight(pSrcImg, nPlaneIdx);
                pThread_Param->nSrcStride       = pSrcImg->GetImgStride(pSrcImg, nPlaneIdx);
                pThread_Param->nDstStride       = pDstImg->GetImgStride(pDstImg, nPlaneIdx);
                
                SEMA_POST(&(pFilter->nStartSema[j]));
            }
            
            // Wait Until Done All Processes of Threads
            for(j=0 ; j<nNumofCPUCore ; j++)
                SEMA_WAIT(&(pFilter->nEndSema[j]));
        }
        else
        {
            Thread_Param        nThread_Param = {pFilter, };
            
            nThread_Param.pSrc0                 = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, nPlaneIdx);
            nThread_Param.pSrc1                 = NULL;
            nThread_Param.pDst                  = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, nPlaneIdx);
            nThread_Param.nWidth                = pSrcImg->GetImgWidth(pSrcImg, nPlaneIdx);
            nThread_Param.nHeight               = pSrcImg->GetImgHeight(pSrcImg, nPlaneIdx);
            nThread_Param.nSrcStride            = pSrcImg->GetImgStride(pSrcImg, nPlaneIdx);
            nThread_Param.nDstStride            = pDstImg->GetImgStride(pDstImg, nPlaneIdx);

            _Filter_Blur_MeanFilter(&nThread_Param);
        }
    }
    #else
    {
        Thread_Param        nThread_Param = {pFilter, };
        
        nThread_Param.pSrc0                 = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, nPlaneIdx);
        nThread_Param.pSrc1                 = NULL;
        nThread_Param.pDst                  = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, nPlaneIdx);
        nThread_Param.nWidth                = pSrcImg->GetImgWidth(pSrcImg, nPlaneIdx);
        nThread_Param.nHeight               = pSrcImg->GetImgHeight(pSrcImg, nPlaneIdx);
        nThread_Param.nSrcStride            = pSrcImg->GetImgStride(pSrcImg, nPlaneIdx);
        nThread_Param.nDstStride            = pDstImg->GetImgStride(pDstImg, nPlaneIdx);
        
        _Filter_Blur_NoiseReductionFilter(&nThread_Param);
    }
    #endif
    
    ___STOP_PROFILE(FilterImg)
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}



KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_Quantize(IN Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg, IN const QuantLevel nQuantLevel)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   i = 0;
    
    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER);
    
    for(i=0 ; i<pSrcImg->GetNumofImgPlanes(pSrcImg) ; i++)
    {
        Thread_Param        nThread_Param = {pFilter, };
        
        nThread_Param.pSrc0             = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, i);
        nThread_Param.pSrc1             = NULL;
        nThread_Param.pDst              = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, i);
        nThread_Param.nWidth            = pSrcImg->GetImgWidth(pSrcImg, i);
        nThread_Param.nHeight           = pSrcImg->GetImgHeight(pSrcImg, i);
        nThread_Param.nSrcStride        = pSrcImg->GetImgStride(pSrcImg, i);
        nThread_Param.nDstStride        = pDstImg->GetImgStride(pDstImg, i);
        nThread_Param.nQuantLevel       = nQuantLevel;
        
        _Filter_Quantize_Plane(&nThread_Param);
    }

    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_Mosaic(IN Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg, IN const MosiacLevel nMosiacLevel)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER);
    
    // Make Integral Image
    if(MOSAIC_LEVEL_4 < nMosiacLevel)
        _Filter_Make_IntegralImg((Filter_T *)pFilter, pSrcImg);
    
    ___START_PROFILE(FilterImg)
    
    #if USE_MULTI_THREAD
    {
        if(IMGPROC_TRUE == pFilter->bIsMultiThreadEnabled)
        {
            INT32           i = 0, j = 0;
            UINT32          nNumofCPUCore = pFilter->nNumofCPUCore;
            
            for(i=0 ; i<pSrcImg->GetNumofImgPlanes(pSrcImg) ; i++)
            {
                for(j=0 ; j<nNumofCPUCore ; j++)
                {
                    Thread_Param        *pThread_Param = (Thread_Param *)(&pFilter->pThreadParam[j]);
                    
                    pThread_Param->pFilterFunc      = fpMosiacFunc[nMosiacLevel];
                    pThread_Param->pFilter          = pFilter;
                    pThread_Param->pSrc0            = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, i);
                    pThread_Param->pSrc1            = NULL;
                    pThread_Param->pDst             = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, i);
                    pThread_Param->nWidth           = pSrcImg->GetImgWidth(pSrcImg, i);
                    pThread_Param->nHeight          = pSrcImg->GetImgHeight(pSrcImg, i);
                    pThread_Param->nSrcStride       = pSrcImg->GetImgStride(pSrcImg, i);
                    pThread_Param->nDstStride       = pDstImg->GetImgStride(pDstImg, i);

                    SEMA_POST(&(pFilter->nStartSema[j]));
                }
                
                // Wait Until Done All Processes of Threads
                for(j=0 ; j<nNumofCPUCore ; j++)
                    SEMA_WAIT(&(pFilter->nEndSema[j]));
            }
        }
        else
        {
            INT32               i = 0;
            
            for(i=0 ; i<pSrcImg->GetNumofImgPlanes(pSrcImg) ; i++)
            {
                Thread_Param        nThread_Param = {pFilter, };
                
                nThread_Param.pSrc0             = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, i);
                nThread_Param.pSrc1             = NULL;
                nThread_Param.pDst              = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, i);
                nThread_Param.nWidth            = pSrcImg->GetImgWidth(pSrcImg, i);
                nThread_Param.nHeight           = pSrcImg->GetImgHeight(pSrcImg, i);
                nThread_Param.nSrcStride        = pSrcImg->GetImgStride(pSrcImg, i);
                nThread_Param.nDstStride        = pDstImg->GetImgStride(pDstImg, i);
                
                fpMosiacFunc[nMosiacLevel](&nThread_Param);
            }
        }
    }
    #else
    {
        INT32               i = 0;
        
        for(i=0 ; i<pSrcImg->GetNumofImgPlanes(pSrcImg) ; i++)
        {
            Thread_Param        nThread_Param = {pFilter, };
            
            nThread_Param.pSrc0             = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, i);
            nThread_Param.pSrc1             = NULL;
            nThread_Param.pDst              = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, i);
            nThread_Param.nWidth            = pSrcImg->GetImgWidth(pSrcImg, i);
            nThread_Param.nHeight           = pSrcImg->GetImgHeight(pSrcImg, i);
            nThread_Param.nSrcStride        = pSrcImg->GetImgStride(pSrcImg, i);
            nThread_Param.nDstStride        = pDstImg->GetImgStride(pDstImg, i);
            
            fpMosiacFunc[nMosiacLevel](&nThread_Param);
        }
    }
    #endif
    
    ___STOP_PROFILE(FilterImg)
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_MedianSquare(IN Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER);
    
    ___START_PROFILE(FilterImg)
    
    #if USE_MULTI_THREAD
    {
        if(IMGPROC_TRUE == pFilter->bIsMultiThreadEnabled)
        {
            INT32           i = 0, j = 0;
            UINT32          nNumofCPUCore = pFilter->nNumofCPUCore;
            
            for(i=0 ; i<pSrcImg->GetNumofImgPlanes(pSrcImg) ; i++)
            {
                for(j=0 ; j<nNumofCPUCore ; j++)
                {
                    Thread_Param        *pThread_Param = (Thread_Param *)(&pFilter->pThreadParam[j]);
                    
                    pThread_Param->pFilterFunc      = _Filter_Median_Square;
                    pThread_Param->pFilter          = pFilter;
                    pThread_Param->pSrc0            = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, i);
                    pThread_Param->pSrc1            = NULL;
                    pThread_Param->pDst             = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, i);
                    pThread_Param->nWidth           = pSrcImg->GetImgWidth(pSrcImg, i);
                    pThread_Param->nHeight          = pSrcImg->GetImgHeight(pSrcImg, i);
                    pThread_Param->nSrcStride       = pSrcImg->GetImgStride(pSrcImg, i);
                    pThread_Param->nDstStride       = pDstImg->GetImgStride(pDstImg, i);
                    
                    SEMA_POST(&(pFilter->nStartSema[j]));
                }
                
                // Wait Until Done All Processes of Threads
                for(j=0 ; j<nNumofCPUCore ; j++)
                    SEMA_WAIT(&(pFilter->nEndSema[j]));
            }
        }
        else
        {
            INT32               i = 0;
            
            for(i=0 ; i<pSrcImg->GetNumofImgPlanes(pSrcImg) ; i++)
            {
                Thread_Param        nThread_Param = {pFilter, };
                
                nThread_Param.pSrc0             = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, i);
                nThread_Param.pSrc1             = NULL;
                nThread_Param.pDst              = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, i);
                nThread_Param.nWidth            = pSrcImg->GetImgWidth(pSrcImg, i);
                nThread_Param.nHeight           = pSrcImg->GetImgHeight(pSrcImg, i);
                nThread_Param.nSrcStride        = pSrcImg->GetImgStride(pSrcImg, i);
                nThread_Param.nDstStride        = pDstImg->GetImgStride(pDstImg, i);
                
                _Filter_Median_Square(&nThread_Param);
            }
        }
    }
    #else
    {
        INT32           i = 0;
        
        for(i=0 ; i<pSrcImg->GetNumofImgPlanes(pSrcImg) ; i++)
        {
            Thread_Param        nThread_Param = {pFilter, };
            
            nThread_Param.pSrc0             = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, i);
            nThread_Param.pSrc1             = NULL;
            nThread_Param.pDst              = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, i);
            nThread_Param.nWidth            = pSrcImg->GetImgWidth(pSrcImg, i);
            nThread_Param.nHeight           = pSrcImg->GetImgHeight(pSrcImg, i);
            nThread_Param.nSrcStride        = pSrcImg->GetImgStride(pSrcImg, i);
            nThread_Param.nDstStride        = pDstImg->GetImgStride(pDstImg, i);
            
            _Filter_Median_Square(&nThread_Param);
        }
    }
    #endif

    ___STOP_PROFILE(FilterImg)
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_MedianSquare_Plane(IN Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg, IN const UINT32 nPlaneIdx)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER);
    
    ___START_PROFILE(FilterImg)
    
    #if USE_MULTI_THREAD
    {
        if(IMGPROC_TRUE == pFilter->bIsMultiThreadEnabled)
        {
            INT32           j = 0;
            UINT32          nNumofCPUCore = pFilter->nNumofCPUCore;
            
            for(j=0 ; j<nNumofCPUCore ; j++)
            {
                Thread_Param        *pThread_Param = (Thread_Param *)(&pFilter->pThreadParam[j]);
                
                pThread_Param->pFilterFunc      = _Filter_Median_Square;
                pThread_Param->pFilter          = pFilter;
                pThread_Param->pSrc0            = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, nPlaneIdx);
                pThread_Param->pSrc1            = NULL;
                pThread_Param->pDst             = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, nPlaneIdx);
                pThread_Param->nWidth           = pSrcImg->GetImgWidth(pSrcImg, nPlaneIdx);
                pThread_Param->nHeight          = pSrcImg->GetImgHeight(pSrcImg, nPlaneIdx);
                pThread_Param->nSrcStride       = pSrcImg->GetImgStride(pSrcImg, nPlaneIdx);
                pThread_Param->nDstStride       = pDstImg->GetImgStride(pDstImg, nPlaneIdx);
                
                SEMA_POST(&(pFilter->nStartSema[j]));
            }
            
            // Wait Until Done All Processes of Threads
            for(j=0 ; j<nNumofCPUCore ; j++)
                SEMA_WAIT(&(pFilter->nEndSema[j]));
        }
        else
        {
            Thread_Param        nThread_Param = {pFilter, };
            
            nThread_Param.pSrc0                 = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, nPlaneIdx);
            nThread_Param.pSrc1                 = NULL;
            nThread_Param.pDst                  = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, nPlaneIdx);
            nThread_Param.nWidth                = pSrcImg->GetImgWidth(pSrcImg, nPlaneIdx);
            nThread_Param.nHeight               = pSrcImg->GetImgHeight(pSrcImg, nPlaneIdx);
            nThread_Param.nSrcStride            = pSrcImg->GetImgStride(pSrcImg, nPlaneIdx);
            nThread_Param.nDstStride            = pDstImg->GetImgStride(pDstImg, nPlaneIdx);
            
            _Filter_Median_Square(&nThread_Param);
        }
    }
    #else
    {
        Thread_Param        nThread_Param = {pFilter, };
        
        nThread_Param.pSrc0                 = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, nPlaneIdx);
        nThread_Param.pSrc1                 = NULL;
        nThread_Param.pDst                  = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, nPlaneIdx);
        nThread_Param.nWidth                = pSrcImg->GetImgWidth(pSrcImg, nPlaneIdx);
        nThread_Param.nHeight               = pSrcImg->GetImgHeight(pSrcImg, nPlaneIdx);
        nThread_Param.nSrcStride            = pSrcImg->GetImgStride(pSrcImg, nPlaneIdx);
        nThread_Param.nDstStride            = pDstImg->GetImgStride(pDstImg, nPlaneIdx);
        
        _Filter_Median_Square(&nThread_Param);
    }
    #endif

    ___STOP_PROFILE(FilterImg)
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_MedianHorizon(IN Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER);

    ___START_PROFILE(FilterImg)
    
    #if USE_MULTI_THREAD
    {
        if(IMGPROC_TRUE == pFilter->bIsMultiThreadEnabled)
        {
            INT32           i = 0, j = 0;
            UINT32          nNumofCPUCore = pFilter->nNumofCPUCore;
            
            for(i=0 ; i<pSrcImg->GetNumofImgPlanes(pSrcImg) ; i++)
            {
                for(j=0 ; j<nNumofCPUCore ; j++)
                {
                    Thread_Param        *pThread_Param = (Thread_Param *)(&pFilter->pThreadParam[j]);
                    
                    pThread_Param->pFilterFunc      = _Filter_Median_Horizon;
                    pThread_Param->pFilter          = pFilter;
                    pThread_Param->pSrc0            = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, i);
                    pThread_Param->pSrc1            = NULL;
                    pThread_Param->pDst             = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, i);
                    pThread_Param->nWidth           = pSrcImg->GetImgWidth(pSrcImg, i);
                    pThread_Param->nHeight          = pSrcImg->GetImgHeight(pSrcImg, i);
                    pThread_Param->nSrcStride       = pSrcImg->GetImgStride(pSrcImg, i);
                    pThread_Param->nDstStride       = pDstImg->GetImgStride(pDstImg, i);
                    
                    SEMA_POST(&(pFilter->nStartSema[j]));
                }
                
                // Wait Until Done All Processes of Threads
                for(j=0 ; j<nNumofCPUCore ; j++)
                    SEMA_WAIT(&(pFilter->nEndSema[j]));
            }
        }
        else
        {
            INT32           i = 0;
            
            for(i=0 ; i<pSrcImg->GetNumofImgPlanes(pSrcImg) ; i++)
            {
                Thread_Param        nThread_Param = {pFilter, };
                
                nThread_Param.pSrc0             = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, i);
                nThread_Param.pSrc1             = NULL;
                nThread_Param.pDst              = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, i);
                nThread_Param.nWidth            = pSrcImg->GetImgWidth(pSrcImg, i);
                nThread_Param.nHeight           = pSrcImg->GetImgHeight(pSrcImg, i);
                nThread_Param.nSrcStride        = pSrcImg->GetImgStride(pSrcImg, i);
                nThread_Param.nDstStride        = pDstImg->GetImgStride(pDstImg, i);
                
                _Filter_Median_Horizon(&nThread_Param);
            }
        }
    }
    #else
    {
        INT32           i = 0;
        
        for(i=0 ; i<pSrcImg->GetNumofImgPlanes(pSrcImg) ; i++)
        {
            Thread_Param        nThread_Param = {pFilter, };
            
            nThread_Param.pSrc0             = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, i);
            nThread_Param.pSrc1             = NULL;
            nThread_Param.pDst              = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, i);
            nThread_Param.nWidth            = pSrcImg->GetImgWidth(pSrcImg, i);
            nThread_Param.nHeight           = pSrcImg->GetImgHeight(pSrcImg, i);
            nThread_Param.nSrcStride        = pSrcImg->GetImgStride(pSrcImg, i);
            nThread_Param.nDstStride        = pDstImg->GetImgStride(pDstImg, i);
            
            _Filter_Median_Horizon(&nThread_Param);
        }
    }
    #endif
    
    ___STOP_PROFILE(FilterImg)
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_MedianVertical(IN Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER);

    ___START_PROFILE(FilterImg)
    
    #if USE_MULTI_THREAD
    {
        if(IMGPROC_TRUE == pFilter->bIsMultiThreadEnabled)
        {
            INT32           i = 0, j = 0;
            UINT32          nNumofCPUCore = pFilter->nNumofCPUCore;
            
            for(i=0 ; i<pSrcImg->GetNumofImgPlanes(pSrcImg) ; i++)
            {
                for(j=0 ; j<nNumofCPUCore ; j++)
                {
                    Thread_Param        *pThread_Param = (Thread_Param *)(&pFilter->pThreadParam[j]);
                    
                    pThread_Param->pFilterFunc      = _Filter_Median_Vertical;
                    pThread_Param->pFilter          = pFilter;
                    pThread_Param->pSrc0            = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, i);
                    pThread_Param->pSrc1            = NULL;
                    pThread_Param->pDst             = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, i);
                    pThread_Param->nWidth           = pSrcImg->GetImgWidth(pSrcImg, i);
                    pThread_Param->nHeight          = pSrcImg->GetImgHeight(pSrcImg, i);
                    pThread_Param->nSrcStride       = pSrcImg->GetImgStride(pSrcImg, i);
                    pThread_Param->nDstStride       = pDstImg->GetImgStride(pDstImg, i);
                    
                    SEMA_POST(&(pFilter->nStartSema[j]));
                }
                
                // Wait Until Done All Processes of Threads
                for(j=0 ; j<nNumofCPUCore ; j++)
                    SEMA_WAIT(&(pFilter->nEndSema[j]));
            }
        }
        else
        {
            INT32           i = 0;
            
            for(i=0 ; i<pSrcImg->GetNumofImgPlanes(pSrcImg) ; i++)
            {
                Thread_Param        nThread_Param = {pFilter, };
                
                nThread_Param.pSrc0             = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, i);
                nThread_Param.pSrc1             = NULL;
                nThread_Param.pDst              = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, i);
                nThread_Param.nWidth            = pSrcImg->GetImgWidth(pSrcImg, i);
                nThread_Param.nHeight           = pSrcImg->GetImgHeight(pSrcImg, i);
                nThread_Param.nSrcStride        = pSrcImg->GetImgStride(pSrcImg, i);
                nThread_Param.nDstStride        = pDstImg->GetImgStride(pDstImg, i);
                
                _Filter_Median_Vertical(&nThread_Param);
            }
        }
    }
    #else
    {
        INT32           i = 0;
        
        for(i=0 ; i<pSrcImg->GetNumofImgPlanes(pSrcImg) ; i++)
        {
            Thread_Param        nThread_Param = {pFilter, };
            
            nThread_Param.pSrc0             = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, i);
            nThread_Param.pSrc1             = NULL;
            nThread_Param.pDst              = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, i);
            nThread_Param.nWidth            = pSrcImg->GetImgWidth(pSrcImg, i);
            nThread_Param.nHeight           = pSrcImg->GetImgHeight(pSrcImg, i);
            nThread_Param.nSrcStride        = pSrcImg->GetImgStride(pSrcImg, i);
            nThread_Param.nDstStride        = pDstImg->GetImgStride(pDstImg, i);
            
            _Filter_Median_Vertical(&nThread_Param);
        }
    }
    #endif
    
    ___STOP_PROFILE(FilterImg);
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_NormalizeByMean(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER);
    
    {
        INT32               i = 0, j = 0;
        INT32               nLocalPos = 0;
        const INT32         nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
        const INT32         nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
        const INT32         nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
        const UINT8         *pSrc = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
        UINT8               *pDst = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0);
        F32                 nMean = 0.0;
        F32                 nVariance = 0.0;
        F32                 nStandardDiv = 0.0;
        
        CHECK_POINTER_VALIDATION(pSrc, KAKAO_STAT_NULL_POINTER);
        CHECK_POINTER_VALIDATION(pDst, KAKAO_STAT_NULL_POINTER);

        _Filter_Get_Mean_Variance(pSrcImg, &nMean, &nVariance);
        
        nStandardDiv = (F32)(SQRT(nVariance));
        for(j=0 ; j<nHeight ; j++)
        {
            nLocalPos = j * nStride;
            
            for(i=0 ; i<nWidth ; i++)
            {
                pDst[nLocalPos] = (UINT8)(256.0 * (D64)(pSrc[nLocalPos] - nMean) / nStandardDiv);
                nLocalPos++;
            }
        }
    }

    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_Equalization(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER);

    {
        INT32               i = 0, j = 0;
        INT32               nLocalPos = 0;
        UINT32              nSum = 0;
        UINT32              nHistogram[256] = {0, };
        UINT32              nSum_of_Histogram[256] = {0, };
        const INT32         nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
        const INT32         nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
        const INT32         nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
        const UINT8         *pSrc = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
        UINT8               *pDst = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0);
        
        // Calculate histogram bin
        for(j=0 ; j<nHeight ; j++)
        {
            nLocalPos = j * nStride;
            
            for(i=0 ; i<nWidth ; i++)
                nHistogram[pSrc[nLocalPos++]]++;
        }
        
        // Calculate accumulated histogram
        for(i=0 ; i<256 ; i++)
        {
            nSum += nHistogram[i];
            nSum_of_Histogram[i] = nSum;
        }
        
        // Normalization
        {
            UINT32     nTotalPixs = nWidth * nHeight;
            
            for(j=0 ; j<nHeight ; j++)
            {
                nLocalPos = j * nStride;
                
                for(i=0 ; i<nWidth ; i++)
                {
                    pDst[nLocalPos] = (UINT8)((255 * nSum_of_Histogram[pSrc[nLocalPos]]) / nTotalPixs);
                    nLocalPos++;
                }
            }
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_HistogrmaStretch(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER);
    
    ___START_PROFILE(FilterImg);
    
    {
        INT32               i = 0, j = 0;
        INT32               nLocalPos = 0;
        INT32               nLow_value = 0, nHigh_value = 255;
        UINT32              nHistogram[256] = {0, };
        UINT8               nLookUptable[256] = {0, };
        const INT32         nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
        const INT32         nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
        const INT32         nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
        const UINT8         *pSrc = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
        UINT8               *pDst = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0);
        
        CHECK_POINTER_VALIDATION(pSrc, KAKAO_STAT_NULL_POINTER);
        CHECK_POINTER_VALIDATION(pDst, KAKAO_STAT_NULL_POINTER);

        // Calculate histogram bin
        for(j=0 ; j<nHeight ; j++)
        {
            nLocalPos = j * nStride;
            
            for(i=0 ; i<nWidth ; i++)
                nHistogram[pSrc[nLocalPos++]]++;
        }
        
        // Find lowest pixel value
        for(i=0 ; i<256 ; i++)
        {
            if(nHistogram[i])
            {
                nLow_value = i;
                break;
            }
        }
        
        // Find highest pixel value
        for(i=255 ; i>0 ; i--)
        {
            if(nHistogram[i])
            {
                nHigh_value = i;
                break;
            }
        }
        
        for(i=0 ; i<nLow_value ; i++)
            nLookUptable[i] = 0;
        
        for(i=255 ; i>=nHigh_value ; i--)
            nLookUptable[i] = 255;
        
        {
            const F32           nScaleFactor = (F32)(255.0f / (nHigh_value - nLow_value));
            
            for(i=nLow_value; i<nHigh_value ; i++)
                nLookUptable[i] = (UINT8)((i - nLow_value) * nScaleFactor);
            
            for(i=1; i<254 ; i++)
                nLookUptable[i] = (nLookUptable[i - 1] + nLookUptable[i] + nLookUptable[i + 1]) / 3;
        }
        
        // Histogram stretching
        for(j=0 ; j<nHeight ; j++)
        {
            nLocalPos = j * nStride;
            
            for(i=0 ; i<nWidth ; i++)
            {
                pDst[nLocalPos] = nLookUptable[pSrc[nLocalPos]];
                nLocalPos++;
            }
        }
    }

    ___STOP_PROFILE(FilterImg);

    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_CustomHistogramStretch(IN Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER);
    
    ___START_PROFILE(FilterImg);
    
    {
        INT32                   i = 0, j = 0;
        INT32                   nLowValue = 0, nHighValue = 255;
        UINT32                  nHistogram[256] = {0, };
        UINT8                   nLookUptable[256] = {0, };
        UINT8                   *pPrevLookUptable = pFilter->pLookUptable;
        F32                     nHistogramBin[4] = {0.0f, };
        const INT32             nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
        const INT32             nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
        
        CHECK_POINTER_VALIDATION(pPrevLookUptable, KAKAO_STAT_NULL_POINTER);

        if(NULL == pFilter->pScaledImg)
            Kakao_ImgProc_Util_CreateImg(&(pFilter->pScaledImg), (nWidth>>3), (nHeight>>3), IMG_FORMAT_I420, IMG_DEPTH_U32);
        
        Kakao_ImgProc_Scaler_Subsample((const ImgScaler_T *)pFilter->pScaler, (const Img_T *)pSrcImg, pFilter->pScaledImg);
        
        // Calculate histogram bin
        {
            const Img_T         *pSrcImg = pFilter->pScaledImg;
            const UINT8         *pSrc = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
            const INT32         nLocalWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
            const INT32         nLocalHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
            const INT32         nLocalStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
            
            CHECK_POINTER_VALIDATION(pSrc, KAKAO_STAT_NULL_POINTER);

            for(j=0 ; j<nLocalHeight ; j++)
            {
                const UINT8               *pLocalSrc = pSrc + (j * nLocalStride);
                
                for(i=nLocalWidth-1 ; i>=0 ; i-=8)
                {
                    nHistogram[pLocalSrc[0]]++;
                    nHistogram[pLocalSrc[1]]++;
                    nHistogram[pLocalSrc[2]]++;
                    nHistogram[pLocalSrc[3]]++;
                    nHistogram[pLocalSrc[4]]++;
                    nHistogram[pLocalSrc[5]]++;
                    nHistogram[pLocalSrc[6]]++;
                    nHistogram[pLocalSrc[7]]++;
                    
                    pLocalSrc += 8;
                }
            }
        }
        
        // Find lowest pixel value
        for(i=0 ; i<256 ; i++)
        {
            if(nHistogram[i])
            {
                nLowValue = i;
                break;
            }
        }
        
        // Find highest pixel value
        for(i=255 ; i>0 ; i--)
        {
            if(nHistogram[i])
            {
                nHighValue = i;
                break;
            }
        }
        
        if(0 != pFilter->nPrevMinVal)
            nLowValue = ((pFilter->nPrevMinVal * 90) + (nLowValue * 10)) / 100;
        
        if(0 != pFilter->nPrevMaxVal)
            nHighValue = ((pFilter->nPrevMaxVal * 90) + (nHighValue * 10)) / 100;
                            
        for(i=0 ; i<nLowValue ; i++)
            nLookUptable[i] = 0;
        
        for(i=255 ; i>=nHighValue ; i--)
            nLookUptable[i] = 255;

        {
            F32                 nMaxHistoBin = 0;
            INT32               nBiasOffset = 0;
            const Img_T         *pScaledImg = pFilter->pScaledImg;
            const F32           nImgSize = (F32)(pScaledImg->GetImgWidth(pScaledImg, IMG_PLANE_0) * pScaledImg->GetImgHeight(pScaledImg, IMG_PLANE_0));
            
            for(j=0 ; j<4 ; j++)
            {
                const INT32         nRangeMin = (256 >> 2) * j;
                const INT32         nRangeMax = (256 >> 2) * (j + 1);
                
                for(i=nRangeMin ; i<nRangeMax ; i++)
                    nHistogramBin[j] += nHistogram[i];
                
                nHistogramBin[j] /= nImgSize;
                
                if(nMaxHistoBin < nHistogramBin[j])
                {
                    nMaxHistoBin = nHistogramBin[j];
                    nBiasOffset = j;
                }
            }
            
            {
                const F32           nScaleFactor = (F32)(255.0f / (nHighValue - nLowValue));
                
                for(i=nLowValue ; i<nHighValue ; i++)
                    nLookUptable[i] = CLIP3(0, 255, (UINT32)((i - nLowValue) * nScaleFactor));

                if((0 == nBiasOffset) && (0.4 <= nHistogramBin[nBiasOffset]))
                {
                    const F32       nWeight = 255.0f / (F32)((nLowValue - nHighValue) * (nLowValue - nHighValue));

                    // y = -nWeight * (i-nHeight)^2 + 255;
                    for(i=nLowValue ; i<nHighValue ; i++)
                        nLookUptable[i] = (UINT8)(CLIP3(0, 255, 255 - nWeight * (((i-nHighValue) * (i-nHighValue)))));
                }
            }
            
            for(i=1; i<255 ; i++)
                nLookUptable[i] = (nLookUptable[i - 1] + nLookUptable[i] + nLookUptable[i + 1]) / 3;
            
            if(0 != pPrevLookUptable[128])
                for(i=0; i<256 ; i++)
                    nLookUptable[i] = (UINT8)((((UINT32)(nLookUptable[i]) * 10) + ((UINT32)(pPrevLookUptable[i]) * 90)) / 100);
        }

        // Histogram stretching
        {
            const UINT8         *pSrc = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
            UINT8               *pDst = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0);
            const INT32         nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
            const INT32         nOffset = nStride - nWidth;

            for(j=0 ; j<nHeight ; j++)
            {
                for(i=0 ; i<nWidth ; i++)
                    *pDst++ = nLookUptable[*pSrc++];
                
                pSrc += nOffset;
                pDst += nOffset;
            }
        }
        
        pFilter->nPrevMinVal = nLowValue;
        pFilter->nPrevMaxVal = nHighValue;
        MEMCPY(pPrevLookUptable, &(nLookUptable[0]), 256 * sizeof(UINT8));
    }
    
    ___STOP_PROFILE(FilterImg);
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_ScreenBlend(IN Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg, IN const F32 nStartRadius, IN const F32 nEndRadius)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER);
    
    ___START_PROFILE(FilterImg);

    {
        int                 i = 0;
        Img_T               *pSBTmpBuf[4] = {NULL, };
        
        #if 0
        {
            // Allocate Image Buffer
            for(i=0 ; i<4 ; i++)
            {
                if(NULL == pFilter->pSBTmpBuf[i])
                    Kakao_ImgProc_Util_CreateImg(&(pFilter->pSBTmpBuf[i]), pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0), pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0), IMG_FORMAT_I420, IMG_DEPTH_U8);
                
                pSBTmpBuf[i] = pFilter->pSBTmpBuf[i];
            }

            // Make Blurred Image
            Kakao_ImgProc_Filter_MeanBlur_Plane(pFilter, pSrcImg, pSBTmpBuf[1], IMG_PLANE_0);
            
            // Make Difference Image White - Original
            Kakao_ImgProc_Filter_Inverse_Plane(pFilter, pSrcImg, pSBTmpBuf[2], IMG_PLANE_0);
            
            // Make Difference Image White - Blurred
            Kakao_ImgProc_Filter_Inverse_Plane(pFilter, pSBTmpBuf[1], pSBTmpBuf[3], IMG_PLANE_0);
            
            // Make Multiplied Image
            Kakao_ImgProc_Filter_Multiply_Plane(pFilter, pSBTmpBuf[2], pSBTmpBuf[3], pSBTmpBuf[2], IMG_PLANE_0);
            
            // Make Difference Image White - Multiplied
            Kakao_ImgProc_Filter_Inverse_Plane(pFilter, pSBTmpBuf[2], pSBTmpBuf[3], IMG_PLANE_0);
            
            // Tune Contrast Curve
            Kakao_ImgProc_Filter_ContrastShift_Plane(pFilter, pSBTmpBuf[3], pSBTmpBuf[2], pSBTmpBuf[3], IMG_PLANE_0);
            
            // Make Mixed Image
            Kakao_ImgProc_Filter_ProportionalMixImg_Plane(pFilter, pSBTmpBuf[3], pSBTmpBuf[1], pDstImg, IMG_PLANE_0);
        }
        #else
        {
            // Allocate Image Buffer
            for(i=0 ; i<3 ; i++)
            {
                if(NULL == pFilter->pSBTmpBuf[i])
                    Kakao_ImgProc_Util_CreateImg(&(pFilter->pSBTmpBuf[i]), pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0), pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0), IMG_FORMAT_I420, IMG_DEPTH_U8);

                pSBTmpBuf[i] = pFilter->pSBTmpBuf[i];
            }

            // Make Difference Image White - Original
            Kakao_ImgProc_Filter_Inverse_Plane(pFilter, pSrcImg, pSBTmpBuf[1], IMG_PLANE_0);
            
            // Make Multiplied Image
            Kakao_ImgProc_Filter_Multiply_Plane(pFilter, pSBTmpBuf[1], pSBTmpBuf[1], pSBTmpBuf[2], IMG_PLANE_0);
            
            // Make Difference Image White - Multiplied
            Kakao_ImgProc_Filter_Inverse_Plane(pFilter, pSBTmpBuf[2], pSBTmpBuf[0], IMG_PLANE_0);
            
            // Tune Contrast Curve
            Kakao_ImgProc_Filter_ContrastShift_Plane(pFilter, pSBTmpBuf[0], pSBTmpBuf[2], pSBTmpBuf[1], IMG_PLANE_0);
            
            // Make Mixed Image
            Kakao_ImgProc_Filter_ProportionalMixImg_Plane(pFilter, pSBTmpBuf[1], pSrcImg, pDstImg, IMG_PLANE_0, nStartRadius, nEndRadius);
        }
        #endif
    }

    ___STOP_PROFILE(FilterImg);

    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_Logarithm(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER);
    
    {
        INT32                   i = 0, j = 0;
        INT32                   nLocalPos = 0;
        const INT32             nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
        const INT32             nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
        const INT32             nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
        const UINT8             *pSrc = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
        UINT8                   *pDst = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0);
        D64                     nValue = 0.0;
        const D64               Coeffi = 255.0 / log((D64)(1+255));
        
        for(j=0 ; j<nHeight ; j++)
        {
            nLocalPos = j * nStride;
            
            for(i=0 ; i<nWidth ; i++)
            {
                nValue = Coeffi * log((D64)(1 + (pSrc[nLocalPos])));
                
                pDst[nLocalPos++] = (nValue > 255) ? 255 :(UINT8)nValue;
            }
        }
    }

    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_Inverse(IN Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER);
    
    ___START_PROFILE(FilterImg)
    
    #if USE_MULTI_THREAD
    {
        if(IMGPROC_TRUE == pFilter->bIsMultiThreadEnabled)
        {
            INT32           i = 0, j = 0;
            UINT32          nNumofCPUCore = pFilter->nNumofCPUCore;
            
            for(i=0 ; i<pSrcImg->GetNumofImgPlanes(pSrcImg) ; i++)
            {
                for(j=0 ; j<nNumofCPUCore ; j++)
                {
                    Thread_Param        *pThread_Param = (Thread_Param *)(&pFilter->pThreadParam[j]);
                    
                    pThread_Param->pFilterFunc      = _Filter_Inverse;
                    pThread_Param->pFilter          = pFilter;
                    pThread_Param->pSrc0            = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, i);
                    pThread_Param->pSrc1            = NULL;
                    pThread_Param->pDst             = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, i);
                    pThread_Param->nWidth           = pSrcImg->GetImgWidth(pSrcImg, i);
                    pThread_Param->nHeight          = pSrcImg->GetImgHeight(pSrcImg, i);
                    pThread_Param->nSrcStride       = pSrcImg->GetImgStride(pSrcImg, i);
                    pThread_Param->nDstStride       = pDstImg->GetImgStride(pDstImg, i);
                    
                    SEMA_POST(&(pFilter->nStartSema[j]));
                }
                
                // Wait Until Done All Processes of Threads
                for(j=0 ; j<nNumofCPUCore ; j++)
                    SEMA_WAIT(&(pFilter->nEndSema[j]));
            }
        }
        else
        {
            INT32           i = 0;
            
            for(i=0 ; i<pSrcImg->GetNumofImgPlanes(pSrcImg) ; i++)
            {
                Thread_Param        nThread_Param = {pFilter, };
                
                nThread_Param.pSrc0             = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, i);
                nThread_Param.pSrc1             = NULL;
                nThread_Param.pDst              = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, i);
                nThread_Param.nWidth            = pSrcImg->GetImgWidth(pSrcImg, i);
                nThread_Param.nHeight           = pSrcImg->GetImgHeight(pSrcImg, i);
                nThread_Param.nSrcStride        = pSrcImg->GetImgStride(pSrcImg, i);
                nThread_Param.nDstStride        = pDstImg->GetImgStride(pDstImg, i);
                
                _Filter_Inverse(&nThread_Param);
            }
        }
    }
    #else
    {
        INT32           i = 0;
        
        for(i=0 ; i<pSrcImg->GetNumofImgPlanes(pSrcImg) ; i++)
        {
            Thread_Param        nThread_Param = {pFilter, };
            
            nThread_Param.pSrc0             = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, i);
            nThread_Param.pSrc1             = NULL;
            nThread_Param.pDst              = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, i);
            nThread_Param.nWidth            = pSrcImg->GetImgWidth(pSrcImg, i);
            nThread_Param.nHeight           = pSrcImg->GetImgHeight(pSrcImg, i);
            nThread_Param.nSrcStride        = pSrcImg->GetImgStride(pSrcImg, i);
            nThread_Param.nDstStride        = pDstImg->GetImgStride(pDstImg, i);

            _Filter_Inverse(&nThread_Param);
        }
    }
    #endif
    
    ___STOP_PROFILE(FilterImg)
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_Inverse_Plane(IN Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg, IN const UINT32 nPlaneIdx)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER);
    
    //___START_PROFILE(FilterImg)
    
    #if USE_MULTI_THREAD
    {
        if(IMGPROC_TRUE == pFilter->bIsMultiThreadEnabled)
        {
            INT32           j = 0;
            UINT32          nNumofCPUCore = pFilter->nNumofCPUCore;
            
            for(j=0 ; j<nNumofCPUCore ; j++)
            {
                Thread_Param        *pThread_Param = (Thread_Param *)(&pFilter->pThreadParam[j]);
                
                pThread_Param->pFilterFunc      = _Filter_Inverse;
                pThread_Param->pFilter          = pFilter;
                pThread_Param->pSrc0            = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, nPlaneIdx);
                pThread_Param->pSrc1            = NULL;
                pThread_Param->pDst             = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, nPlaneIdx);
                pThread_Param->nWidth           = pSrcImg->GetImgWidth(pSrcImg, nPlaneIdx);
                pThread_Param->nHeight          = pSrcImg->GetImgHeight(pSrcImg, nPlaneIdx);
                pThread_Param->nSrcStride       = pSrcImg->GetImgStride(pSrcImg, nPlaneIdx);
                pThread_Param->nDstStride       = pDstImg->GetImgStride(pDstImg, nPlaneIdx);
                
                
                SEMA_POST(&(pFilter->nStartSema[j]));
            }
            
            // Wait Until Done All Processes of Threads
            for(j=0 ; j<nNumofCPUCore ; j++)
                SEMA_WAIT(&(pFilter->nEndSema[j]));
        }
        else
        {
            Thread_Param        nThread_Param = {pFilter, };
            
            nThread_Param.pSrc0                 = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, nPlaneIdx);
            nThread_Param.pSrc1                 = NULL;
            nThread_Param.pDst                  = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, nPlaneIdx);
            nThread_Param.nWidth                = pSrcImg->GetImgWidth(pSrcImg, nPlaneIdx);
            nThread_Param.nHeight               = pSrcImg->GetImgHeight(pSrcImg, nPlaneIdx);
            nThread_Param.nSrcStride            = pSrcImg->GetImgStride(pSrcImg, nPlaneIdx);
            nThread_Param.nDstStride            = pDstImg->GetImgStride(pDstImg, nPlaneIdx);
            
            _Filter_Inverse(&nThread_Param);
        }
    }
    #else
    {
        Thread_Param        nThread_Param = {pFilter, };
        
        nThread_Param.pSrc0                 = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, nPlaneIdx);
        nThread_Param.pSrc1                 = NULL;
        nThread_Param.pDst                  = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, nPlaneIdx);
        nThread_Param.nWidth                = pSrcImg->GetImgWidth(pSrcImg, nPlaneIdx);
        nThread_Param.nHeight               = pSrcImg->GetImgHeight(pSrcImg, nPlaneIdx);
        nThread_Param.nSrcStride            = pSrcImg->GetImgStride(pSrcImg, nPlaneIdx);
        nThread_Param.nDstStride            = pDstImg->GetImgStride(pDstImg, nPlaneIdx);
        
        _Filter_Inverse(&nThread_Param);
    }
    #endif
    
    //___STOP_PROFILE(FilterImg)
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_Binarization(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, IN const INT32 nThreshold)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER);
    
    {
        INT32                   i = 0, j = 0;
        const INT32             nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
        const INT32             nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
        const INT32             nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
        const UINT8             *pSrc = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
        UINT8                   *pSkinMap = NULL;
        
        if(NULL == (pSkinMap = pSrcImg->GetImgBinaryMap((Img_T *)pSrcImg, IMG_BINARYMAP_SKIN)))
            return nRet;
        
        for(j=0 ; j<nHeight ; j++)
        {
            INT32               nLocalPos = j * nStride;
            
            for(i=0 ; i<nWidth ; i++)
            {
                pSkinMap[nLocalPos] = (pSrc[nLocalPos] > nThreshold) ? 0 : INDICATE_VALUE;
                nLocalPos++;
            }
        }
    }

    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_ClipImg(IN Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg, IN const INT32 nMinClipVal, IN const INT32 nMaxClipVal)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER);
    
    pFilter->nMinClipVal = nMinClipVal;
    pFilter->nMaxClipVal = nMaxClipVal;

    ___START_PROFILE(FilterImg)
    
    #if USE_MULTI_THREAD
    {
        if(IMGPROC_TRUE == pFilter->bIsMultiThreadEnabled)
        {
            INT32           i = 0, j = 0;
            UINT32          nNumofCPUCore = pFilter->nNumofCPUCore;
            
            for(i=0 ; i<pSrcImg->GetNumofImgPlanes(pSrcImg) ; i++)
            {
                for(j=0 ; j<nNumofCPUCore ; j++)
                {
                    Thread_Param        *pThread_Param = (Thread_Param *)(&pFilter->pThreadParam[j]);
                    
                    pThread_Param->pFilterFunc      = _Filter_Median_Square;
                    pThread_Param->pFilter          = pFilter;
                    pThread_Param->pSrc0            = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, i);
                    pThread_Param->pSrc1            = NULL;
                    pThread_Param->pDst             = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, i);
                    pThread_Param->nWidth           = pSrcImg->GetImgWidth(pSrcImg, i);
                    pThread_Param->nHeight          = pSrcImg->GetImgHeight(pSrcImg, i);
                    pThread_Param->nSrcStride       = pSrcImg->GetImgStride(pSrcImg, i);
                    pThread_Param->nDstStride       = pDstImg->GetImgStride(pDstImg, i);
                    
                    SEMA_POST(&(pFilter->nStartSema[j]));
                }
                
                // Wait Until Done All Processes of Threads
                for(j=0 ; j<nNumofCPUCore ; j++)
                    SEMA_WAIT(&(pFilter->nEndSema[j]));
            }
        }
        else
        {
            INT32           i = 0;
            
            for(i=0 ; i<pSrcImg->GetNumofImgPlanes(pSrcImg) ; i++)
            {
                Thread_Param        nThread_Param = {pFilter, };
                
                nThread_Param.pSrc0             = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, i);
                nThread_Param.pSrc1             = NULL;
                nThread_Param.pDst              = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, i);
                nThread_Param.nWidth            = pSrcImg->GetImgWidth(pSrcImg, i);
                nThread_Param.nHeight           = pSrcImg->GetImgHeight(pSrcImg, i);
                nThread_Param.nSrcStride        = pSrcImg->GetImgStride(pSrcImg, i);
                nThread_Param.nDstStride        = pDstImg->GetImgStride(pDstImg, i);
                
                _Filter_Clip(&nThread_Param);
            }
        }
    }
    #else
    {
        INT32           i = 0;
        
        for(i=0 ; i<pSrcImg->GetNumofImgPlanes(pSrcImg) ; i++)
        {
            Thread_Param        nThread_Param = {pFilter, };
            
            nThread_Param.pSrc0             = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, i);
            nThread_Param.pSrc1             = NULL;
            nThread_Param.pDst              = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, i);
            nThread_Param.nWidth            = pSrcImg->GetImgWidth(pSrcImg, i);
            nThread_Param.nHeight           = pSrcImg->GetImgHeight(pSrcImg, i);
            nThread_Param.nSrcStride        = pSrcImg->GetImgStride(pSrcImg, i);
            nThread_Param.nDstStride        = pDstImg->GetImgStride(pDstImg, i);
            
            _Filter_Clip(&nThread_Param);
        }
    }
    #endif
    
    ___STOP_PROFILE(FilterImg)
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_ClipPlane(IN Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg,
                                                             IN const INT32 nMinClipVal, IN const INT32 nMaxClipVal, IN const UINT32 nPlaneIdx)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER);
    
    pFilter->nMinClipVal = nMinClipVal;
    pFilter->nMaxClipVal = nMaxClipVal;
    
    ___START_PROFILE(FilterImg)
    
    #if USE_MULTI_THREAD
    {
        if(IMGPROC_TRUE == pFilter->bIsMultiThreadEnabled)
        {
            INT32           j = 0;
            UINT32          nNumofCPUCore = pFilter->nNumofCPUCore;
            
            for(j=0 ; j<nNumofCPUCore ; j++)
            {
                Thread_Param        *pThread_Param = (Thread_Param *)(&pFilter->pThreadParam[j]);
                
                pThread_Param->pFilterFunc      = _Filter_Clip;
                pThread_Param->pFilter          = pFilter;
                pThread_Param->pSrc0            = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, nPlaneIdx);
                pThread_Param->pSrc1            = NULL;
                pThread_Param->pDst             = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, nPlaneIdx);
                pThread_Param->nWidth           = pSrcImg->GetImgWidth(pSrcImg, nPlaneIdx);
                pThread_Param->nHeight          = pSrcImg->GetImgHeight(pSrcImg, nPlaneIdx);
                pThread_Param->nSrcStride       = pSrcImg->GetImgStride(pSrcImg, nPlaneIdx);
                pThread_Param->nDstStride       = pDstImg->GetImgStride(pDstImg, nPlaneIdx);

                SEMA_POST(&(pFilter->nStartSema[j]));
            }
            
            // Wait Until Done All Processes of Threads
            for(j=0 ; j<nNumofCPUCore ; j++)
                SEMA_WAIT(&(pFilter->nEndSema[j]));
        }
        else
        {
            Thread_Param        nThread_Param = {pFilter, };
            
            nThread_Param.pSrc0                 = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, nPlaneIdx);
            nThread_Param.pSrc1                 = NULL;
            nThread_Param.pDst                  = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, nPlaneIdx);
            nThread_Param.nWidth                = pSrcImg->GetImgWidth(pSrcImg, nPlaneIdx);
            nThread_Param.nHeight               = pSrcImg->GetImgHeight(pSrcImg, nPlaneIdx);
            nThread_Param.nSrcStride            = pSrcImg->GetImgStride(pSrcImg, nPlaneIdx);
            nThread_Param.nDstStride            = pDstImg->GetImgStride(pDstImg, nPlaneIdx);
            
            _Filter_Clip(&nThread_Param);
        }
    }
    #else
    {
        Thread_Param        nThread_Param = {pFilter, };
        
        nThread_Param.pSrc0                 = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, nPlaneIdx);
        nThread_Param.pSrc1                 = NULL;
        nThread_Param.pDst                  = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, nPlaneIdx);
        nThread_Param.nWidth                = pSrcImg->GetImgWidth(pSrcImg, nPlaneIdx);
        nThread_Param.nHeight               = pSrcImg->GetImgHeight(pSrcImg, nPlaneIdx);
        nThread_Param.nSrcStride            = pSrcImg->GetImgStride(pSrcImg, nPlaneIdx);
        nThread_Param.nDstStride            = pDstImg->GetImgStride(pDstImg, nPlaneIdx);

        _Filter_Clip(&nThread_Param);
    }
    #endif
    
    ___STOP_PROFILE(FilterImg)
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_Erosion(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg, IN const BinaryMapType nBinaryMap, IN const INT32 nThick)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER);
    
    _Filter_Erosion(pFilter, pSrcImg, pDstImg, nBinaryMap, nThick);
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_Dilation(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg, IN const BinaryMapType nBinaryMap)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER);
    
    _Filter_Dilation(pFilter, pSrcImg, pDstImg, nBinaryMap);
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_Open(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg, IN const BinaryMapType nBinaryMap, IN const INT32 nThick)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER);
    
    {
        Img_T                   *pTmpImg = pFilter->pTmpU8Img;
        
        Kakao_ImgProc_Filter_Erosion(pFilter, pSrcImg, pTmpImg, nBinaryMap, nThick);
        
        Kakao_ImgProc_Filter_Dilation(pFilter, pTmpImg, pDstImg, nBinaryMap);
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_Close(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg, IN const BinaryMapType nBinaryMap, IN const INT32 nThick)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER);
    
    {
        Img_T                   *pTmpImg = pFilter->pTmpU8Img;
        
        Kakao_ImgProc_Filter_Dilation(pFilter, pSrcImg, pTmpImg, nBinaryMap);
        
        Kakao_ImgProc_Filter_Erosion(pFilter, pTmpImg, pDstImg, nBinaryMap, nThick);
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_Bulge(IN Filter_T *pFilter, IN Img_T *pSrcImg, OUT Img_T *pDstImg,
                                                        IN Vec2 nCenterPos, IN const F32 nAspectRatio, IN const F32 nScaleFactor, IN const F32 nRadius, IN const IMGPROC_BOOL bIsNeedUpdate)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER);

    if((IMGPROC_FALSE == pFilter->bIsBulgePosTransMapMade) || (IMGPROC_TRUE == bIsNeedUpdate))
    {
        // Free If Allocated
        SAFEFREE(pFilter->pPosTransMap);
        
        if(NULL == pFilter->pPosTransMap)
            Kakao_ImgProc_Util_CreateImg(&(pFilter->pPosTransMap), pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0), pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0), IMG_FORMAT_I420, IMG_DEPTH_U32);
        
        // Make Position Transformed Map
        _Filter_Make_BulgePosTransMap(pFilter, nAspectRatio, nScaleFactor, nRadius);
        
        pFilter->bIsBulgePosTransMapMade = IMGPROC_TRUE;
    }
    else
    {
        _Filter_Bulge_Plane(pFilter, pSrcImg, pDstImg, nCenterPos);
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_MeanShift(IN Filter_T *pFilter, IN Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER);
    
    {
        INT32               i = 0, j = 0;
        const INT32         nRad = 5;
        const F32           nSqRad = (F32)(nRad * nRad);
        const F32           nColorDistTh = 1000.0f;
        const INT32         nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
        const INT32         nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
        const INT32         nStrideY = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
        const INT32         nStrideUV = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_1);
        const UINT8         *pSrcY = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
        const UINT8         *pSrcU = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_1);
        const UINT8         *pSrcV = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_2);
        UINT8               *pDstY = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0);
        UINT8               *pDstU = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_1);
        UINT8               *pDstV = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_2);

        CHECK_POINTER_VALIDATION(pSrcY, KAKAO_STAT_NULL_POINTER);
        CHECK_POINTER_VALIDATION(pSrcU, KAKAO_STAT_NULL_POINTER);
        CHECK_POINTER_VALIDATION(pSrcV, KAKAO_STAT_NULL_POINTER);
        CHECK_POINTER_VALIDATION(pDstY, KAKAO_STAT_NULL_POINTER);
        CHECK_POINTER_VALIDATION(pDstU, KAKAO_STAT_NULL_POINTER);
        CHECK_POINTER_VALIDATION(pDstV, KAKAO_STAT_NULL_POINTER);
        
        for(j=0 ; j<nHeight ; j++)
        {
            INT32           nBasePosY = j * nStrideY;
            INT32           nBasePosUV = (j>>1) * nStrideUV;

            for(i=0 ; i<nWidth ; i++)
            {
                INT32       nLoopCnt = 0;
                INT32       nCenterX = i, nCenterY = j;
                INT32       nSumY = 0, nSumU = 0, nSumV = 0;
                INT32       nSumCnt = 0;
                
                do
                {
                    const INT32 nStartX = GET_MAX(0, (nCenterX - nRad));
                    const INT32 nEndX = GET_MIN(nWidth, (nCenterX + nRad));
                    const INT32 nStartY = GET_MAX(0, (nCenterY - nRad));
                    const INT32 nEndY = GET_MIN(nHeight, (nCenterY + nRad));
                    const UINT8 nBaseY = pSrcY[((nCenterY * nStrideY) + nCenterX)];
                    const UINT8 nBaseU = pSrcU[(((nCenterY/2) * nStrideUV) + (nCenterX/2))];
                    const UINT8 nBaseV = pSrcV[(((nCenterY/2) * nStrideUV) + (nCenterX/2))];
                    INT32       k = 0, l = 0;
                    INT32       nDistX = 0, nDistY = 0;
                    INT32       nEstCenterX = 0, nEstCenterY = 0;

                    nSumY = 0;
                    nSumU = 0;
                    nSumV = 0;
                    nEstCenterX = 0;
                    nEstCenterY = 0;
                    nSumCnt = 0;
                    
                    nDistY = (nStartY - nCenterY);
                    for(l=nStartY ; l<nEndY ; l++, nDistY++)
                    {
                        INT32           nLocalPosY = 0;
                        INT32           nLocalPosUV = 0;

                        nDistX = (nStartX - nCenterX);
                        for(k=nStartX, nLocalPosY=(l*nStrideY+k), nLocalPosUV=((l>>1)*nStrideUV+(k>>1)) ; k<nEndX ; k++, nDistX++)
                        {
                            if(((nDistX * nDistX) + (nDistY * nDistY)) <= nSqRad)
                            {
                                const UINT8     nLocalY = pSrcY[nLocalPosY];
                                const UINT8     nLocalU = pSrcU[nLocalPosUV];
                                const UINT8     nLocalV = pSrcV[nLocalPosUV];
                                const INT32     nDiffY = nBaseY - nLocalY;
                                const INT32     nDiffU = nBaseU - nLocalU;
                                const INT32     nDiffV = nBaseV - nLocalV;
                                
                                if((F32)((nDiffY * nDiffY) + (nDiffU * nDiffU) + (nDiffV * nDiffV)) <= nColorDistTh)
                                {
                                    nSumY += nLocalY;
                                    nSumU += nLocalU;
                                    nSumV += nLocalV;
                                    
                                    nEstCenterX += k;
                                    nEstCenterY += l;
                                    
                                    nSumCnt++;
                                }
                            }
                            
                            nLocalPosY++;
                            nLocalPosUV += (k & 0x01);
                        }
                    }
                    
                    if(0.0f != nSumCnt)
                    {
                        nSumY /= nSumCnt;
                        nSumU /= nSumCnt;
                        nSumV /= nSumCnt;
                        nEstCenterX /= nSumCnt;
                        nEstCenterY /= nSumCnt;
                    }
                    else
                    {
                        nSumY = nBaseY;
                        nSumU = nBaseU;
                        nSumV = nBaseV;
                        break;
                    }

                    if((ABSM(nEstCenterX - nCenterX) <= 1) && (ABSM(nEstCenterY - nCenterY) <= 1))
                        break;

                    nCenterX = nEstCenterX;
                    nCenterY = nEstCenterY;

                    nLoopCnt++;
                }while(nLoopCnt < 10);
                
                pDstY[nBasePosY] = (UINT8)(nSumY);
                pDstU[nBasePosUV] = (UINT8)(nSumU);
                pDstV[nBasePosUV] = (UINT8)(nSumV);
                
                nBasePosY++;
                nBasePosUV += (i & 0x01);
            }
        }
    }

    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_DiffImg(IN Filter_T *pFilter, IN const Img_T *pSrcImg0, IN const Img_T *pSrcImg1, OUT Img_T *pDstImg)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg1, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pSrcImg0, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER)
    
    ___START_PROFILE(FilterImg)
    
    #if USE_MULTI_THREAD
    {
        if(IMGPROC_TRUE == pFilter->bIsMultiThreadEnabled)
        {
            INT32           i = 0, j = 0;
            UINT32          nNumofCPUCore = pFilter->nNumofCPUCore;
            
            for(i=0 ; i<pSrcImg0->GetNumofImgPlanes(pSrcImg0) ; i++)
            {
                for(j=0 ; j<nNumofCPUCore ; j++)
                {
                    Thread_Param        *pThread_Param = (Thread_Param *)(&pFilter->pThreadParam[j]);
                    
                    pThread_Param->pFilterFunc      = _Filter_DiffImg;
                    pThread_Param->pFilter          = pFilter;
                    pThread_Param->pSrc0            = (UINT8 *)pSrcImg0->GetImgPlanes(pSrcImg0, i);
                    pThread_Param->pSrc1            = (UINT8 *)pSrcImg1->GetImgPlanes(pSrcImg1, i);
                    pThread_Param->pDst             = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, i);
                    pThread_Param->nWidth           = pSrcImg0->GetImgWidth(pSrcImg0, i);
                    pThread_Param->nHeight          = pSrcImg0->GetImgHeight(pSrcImg0, i);
                    pThread_Param->nSrcStride       = pSrcImg0->GetImgStride(pSrcImg0, i);
                    pThread_Param->nDstStride       = pDstImg->GetImgStride(pDstImg, i);
                    
                    SEMA_POST(&(pFilter->nStartSema[j]));
                }
                
                // Wait Until Done All Processes of Threads
                for(j=0 ; j<nNumofCPUCore ; j++)
                    SEMA_WAIT(&(pFilter->nEndSema[j]));
            }
        }
        else
        {
            INT32               i = 0;
            
            for(i=0 ; i<pSrcImg0->GetNumofImgPlanes(pSrcImg0) ; i++)
            {
                Thread_Param        nThread_Param = {pFilter, };
                
                nThread_Param.pSrc0             = (UINT8 *)pSrcImg0->GetImgPlanes(pSrcImg0, i);
                nThread_Param.pSrc1             = (UINT8 *)pSrcImg1->GetImgPlanes(pSrcImg1, i);
                nThread_Param.pDst              = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, i);
                nThread_Param.nWidth            = pSrcImg0->GetImgWidth(pSrcImg0, i);
                nThread_Param.nHeight           = pSrcImg0->GetImgHeight(pSrcImg0, i);
                nThread_Param.nSrcStride        = pSrcImg0->GetImgStride(pSrcImg0, i);
                nThread_Param.nDstStride        = pDstImg->GetImgStride(pDstImg, i);
                
                _Filter_DiffImg(&nThread_Param);
            }
        }
    }
    #else
    {
        INT32           i = 0;
        
        for(i=0 ; i<pSrcImg0->GetNumofImgPlanes(pSrcImg0) ; i++)
        {
            Thread_Param        nThread_Param = {pFilter, };
            
            nThread_Param.pSrc0             = (UINT8 *)pSrcImg0->GetImgPlanes(pSrcImg0, i);
            nThread_Param.pSrc1             = (UINT8 *)pSrcImg1->GetImgPlanes(pSrcImg1, i);
            nThread_Param.pDst              = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, i);
            nThread_Param.nWidth            = pSrcImg0->GetImgWidth(pSrcImg0, i);
            nThread_Param.nHeight           = pSrcImg0->GetImgHeight(pSrcImg0, i);
            nThread_Param.nSrcStride        = pSrcImg0->GetImgStride(pSrcImg0, i);
            nThread_Param.nDstStride        = pDstImg->GetImgStride(pDstImg, i);
            
            _Filter_DiffImg(&nThread_Param);
        }
    }
    #endif
    
    ___STOP_PROFILE(FilterImg)
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_DiffImg_Plane(IN Filter_T *pFilter, IN const Img_T *pSrcImg0, IN const Img_T *pSrcImg1, OUT Img_T *pDstImg, IN const UINT32 nPlaneIdx)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg1, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pSrcImg0, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER)
    
    ___START_PROFILE(FilterImg)
    
    #if USE_MULTI_THREAD
    {
        if(IMGPROC_TRUE == pFilter->bIsMultiThreadEnabled)
        {
            INT32           j = 0;
            UINT32          nNumofCPUCore = pFilter->nNumofCPUCore;
            
            for(j=0 ; j<nNumofCPUCore ; j++)
            {
                Thread_Param        *pThread_Param = (Thread_Param *)(&pFilter->pThreadParam[j]);
                
                pThread_Param->pFilterFunc      = _Filter_DiffImg;
                pThread_Param->pFilter          = pFilter;
                pThread_Param->pSrc0            = (UINT8 *)pSrcImg0->GetImgPlanes(pSrcImg0, nPlaneIdx);
                pThread_Param->pSrc1            = (UINT8 *)pSrcImg1->GetImgPlanes(pSrcImg1, nPlaneIdx);
                pThread_Param->pDst             = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, nPlaneIdx);
                pThread_Param->nWidth           = pSrcImg0->GetImgWidth(pSrcImg0, nPlaneIdx);
                pThread_Param->nHeight          = pSrcImg0->GetImgHeight(pSrcImg0, nPlaneIdx);
                pThread_Param->nSrcStride       = pSrcImg0->GetImgStride(pSrcImg0, nPlaneIdx);
                pThread_Param->nDstStride       = pDstImg->GetImgStride(pDstImg, nPlaneIdx);
                
                SEMA_POST(&(pFilter->nStartSema[j]));
            }
            
            // Wait Until Done All Processes of Threads
            for(j=0 ; j<nNumofCPUCore ; j++)
                SEMA_WAIT(&(pFilter->nEndSema[j]));
        }
        else
        {
            Thread_Param        nThread_Param = {pFilter, };
            
            nThread_Param.pSrc0                 = (UINT8 *)pSrcImg0->GetImgPlanes(pSrcImg0, nPlaneIdx);
            nThread_Param.pSrc1                 = (UINT8 *)pSrcImg1->GetImgPlanes(pSrcImg1, nPlaneIdx);
            nThread_Param.pDst                  = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, nPlaneIdx);
            nThread_Param.nWidth                = pSrcImg0->GetImgWidth(pSrcImg0, nPlaneIdx);
            nThread_Param.nHeight               = pSrcImg0->GetImgHeight(pSrcImg0, nPlaneIdx);
            nThread_Param.nSrcStride            = pSrcImg0->GetImgStride(pSrcImg0, nPlaneIdx);
            nThread_Param.nDstStride            = pDstImg->GetImgStride(pDstImg, nPlaneIdx);

            _Filter_DiffImg(&nThread_Param);
        }
    }
    #else
    {
        Thread_Param        nThread_Param = {pFilter, };
        
        nThread_Param.pSrc0                 = (UINT8 *)pSrcImg0->GetImgPlanes(pSrcImg0, nPlaneIdx);
        nThread_Param.pSrc1                 = (UINT8 *)pSrcImg1->GetImgPlanes(pSrcImg1, nPlaneIdx);
        nThread_Param.pDst                  = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, nPlaneIdx);
        nThread_Param.nWidth                = pSrcImg0->GetImgWidth(pSrcImg0, nPlaneIdx);
        nThread_Param.nHeight               = pSrcImg0->GetImgHeight(pSrcImg0, nPlaneIdx);
        nThread_Param.nSrcStride            = pSrcImg0->GetImgStride(pSrcImg0, nPlaneIdx);
        nThread_Param.nDstStride            = pDstImg->GetImgStride(pDstImg, nPlaneIdx);
        
        _Filter_DiffImg(&nThread_Param);
    }
    #endif
    
    ___STOP_PROFILE(FilterImg)
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_ABSImg(IN const Filter_T *pFilter, IN const Img_T *pSrcImg0, IN const Img_T *pSrcImg1, OUT Img_T *pDstImg)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg1, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pSrcImg0, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER)
    
    {
        INT32               i = 0, j = 0;
        INT32               nLocalPos = 0;
        const INT32         nWidth = pSrcImg0->GetImgWidth(pSrcImg0, IMG_PLANE_0);
        const INT32         nHeight = pSrcImg0->GetImgHeight(pSrcImg0, IMG_PLANE_0);
        const INT32         nStride = pSrcImg0->GetImgStride(pSrcImg0, IMG_PLANE_0);
        const UINT8         *pSrc0 = (UINT8 *)pSrcImg0->GetImgPlanes(pSrcImg0, IMG_PLANE_0);
        const UINT8         *pSrc1 = (UINT8 *)pSrcImg1->GetImgPlanes(pSrcImg1, IMG_PLANE_0);
        UINT8               *pDst = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0);
        
        for(j=0 ; j<nHeight ; j++)
        {
            nLocalPos = j * nStride;
            
            for(i=0 ; i<nWidth ; i++)
            {
                pDst[nLocalPos] = ABSM(pSrc0[nLocalPos] - pSrc1[nLocalPos]);
                nLocalPos++;
            }
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}



KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_AccumImg(IN const Filter_T *pFilter, IN OUT Img_T *pAccumImg, IN const Img_T *pSrcImg)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pAccumImg, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER)
    
    {
        INT32               i = 0, j = 0;
        INT32               nLocalPos = 0;
        const INT32         nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
        const INT32         nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
        const INT32         nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
        const UINT8         *pSrc = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
        UINT8               *pDst = (UINT8 *)pAccumImg->GetImgPlanes(pAccumImg, IMG_PLANE_0);
        static UINT32       nLoopCnt = 0;
        
        for(j=0 ; j<nHeight ; j++)
        {
            nLocalPos = j * nStride;
            for(i=0 ; i<nWidth ; i++)
            {
                pDst[nLocalPos] = (UINT32)((D64)((pDst[nLocalPos] * (nLoopCnt - 1)) + pSrc[nLocalPos]) / (D64)nLoopCnt);
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


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_Multiply(IN Filter_T *pFilter, IN const Img_T *pSrcImg0, IN const Img_T *pSrcImg1, OUT const Img_T *pDstImg)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg0, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pSrcImg1, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER)
    
    ___START_PROFILE(FilterImg)
    
    #if USE_MULTI_THREAD
    {
        if(IMGPROC_TRUE == pFilter->bIsMultiThreadEnabled)
        {
            INT32           i = 0, j = 0;
            UINT32          nNumofCPUCore = pFilter->nNumofCPUCore;
            
            for(i=0 ; i<pSrcImg0->GetNumofImgPlanes(pSrcImg0) ; i++)
            {
                for(j=0 ; j<nNumofCPUCore ; j++)
                {
                    Thread_Param        *pThread_Param = (Thread_Param *)(&pFilter->pThreadParam[j]);
                    
                    pThread_Param->pFilterFunc      = _Filter_Multiply;
                    pThread_Param->pFilter          = pFilter;
                    pThread_Param->pSrc0            = (UINT8 *)pSrcImg0->GetImgPlanes(pSrcImg0, i);
                    pThread_Param->pSrc1            = (UINT8 *)pSrcImg1->GetImgPlanes(pSrcImg1, i);
                    pThread_Param->pDst             = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, i);
                    pThread_Param->nWidth           = pSrcImg0->GetImgWidth(pSrcImg0, i);
                    pThread_Param->nHeight          = pSrcImg0->GetImgHeight(pSrcImg0, i);
                    pThread_Param->nSrcStride       = pSrcImg0->GetImgStride(pSrcImg0, i);
                    pThread_Param->nDstStride       = pDstImg->GetImgStride(pDstImg, i);
                    
                    SEMA_POST(&(pFilter->nStartSema[j]));
                }
                
                // Wait Until Done All Processes of Threads
                for(j=0 ; j<nNumofCPUCore ; j++)
                    SEMA_WAIT(&(pFilter->nEndSema[j]));
            }
        }
        else
        {
            INT32               i = 0;
            
            for(i=0 ; i<pSrcImg0->GetNumofImgPlanes(pSrcImg0) ; i++)
            {
                Thread_Param        nThread_Param = {pFilter, };
                
                nThread_Param.pSrc0             = (UINT8 *)pSrcImg0->GetImgPlanes(pSrcImg0, i);
                nThread_Param.pSrc1             = (UINT8 *)pSrcImg1->GetImgPlanes(pSrcImg1, i);
                nThread_Param.pDst              = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, i);
                nThread_Param.nWidth            = pSrcImg0->GetImgWidth(pSrcImg0, i);
                nThread_Param.nHeight           = pSrcImg0->GetImgHeight(pSrcImg0, i);
                nThread_Param.nSrcStride        = pSrcImg0->GetImgStride(pSrcImg0, i);
                nThread_Param.nDstStride        = pDstImg->GetImgStride(pDstImg, i);
                
                _Filter_Multiply(&nThread_Param);
            }
        }
    }
    #else
    {
        INT32           i = 0;
        
        for(i=0 ; i<pSrcImg0->GetNumofImgPlanes(pSrcImg0) ; i++)
        {
            Thread_Param        nThread_Param = {pFilter, };
            
            nThread_Param.pSrc0             = (UINT8 *)pSrcImg0->GetImgPlanes(pSrcImg0, i);
            nThread_Param.pSrc1             = (UINT8 *)pSrcImg1->GetImgPlanes(pSrcImg1, i);
            nThread_Param.pDst              = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, i);
            nThread_Param.nWidth            = pSrcImg0->GetImgWidth(pSrcImg0, i);
            nThread_Param.nHeight           = pSrcImg0->GetImgHeight(pSrcImg0, i);
            nThread_Param.nSrcStride        = pSrcImg0->GetImgStride(pSrcImg0, i);
            nThread_Param.nDstStride        = pDstImg->GetImgStride(pDstImg, i);
            
            _Filter_Multiply(&nThread_Param);
        }
    }
    #endif
    
    ___STOP_PROFILE(FilterImg)
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_Multiply_Plane(IN Filter_T *pFilter, IN const Img_T *pSrcImg0, IN const Img_T *pSrcImg1, OUT const Img_T *pDstImg, IN const UINT32 nPlaneIdx)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg0, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pSrcImg1, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER)
    
    //___START_PROFILE(FilterImg)
    
    #if USE_MULTI_THREAD
    {
        if(IMGPROC_TRUE == pFilter->bIsMultiThreadEnabled)
        {
            INT32           j = 0;
            UINT32          nNumofCPUCore = pFilter->nNumofCPUCore;
            
            for(j=0 ; j<nNumofCPUCore ; j++)
            {
                Thread_Param        *pThread_Param = (Thread_Param *)(&pFilter->pThreadParam[j]);
                
                pThread_Param->pFilterFunc      = _Filter_Multiply;
                pThread_Param->pFilter          = pFilter;
                pThread_Param->pSrc0            = (UINT8 *)pSrcImg0->GetImgPlanes(pSrcImg0, nPlaneIdx);
                pThread_Param->pSrc1            = (UINT8 *)pSrcImg1->GetImgPlanes(pSrcImg1, nPlaneIdx);
                pThread_Param->pDst             = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, nPlaneIdx);
                pThread_Param->nWidth           = pSrcImg0->GetImgWidth(pSrcImg0, nPlaneIdx);
                pThread_Param->nHeight          = pSrcImg0->GetImgHeight(pSrcImg0, nPlaneIdx);
                pThread_Param->nSrcStride       = pSrcImg0->GetImgStride(pSrcImg0, nPlaneIdx);
                pThread_Param->nDstStride       = pDstImg->GetImgStride(pDstImg, nPlaneIdx);
                
                SEMA_POST(&(pFilter->nStartSema[j]));
            }
            
            // Wait Until Done All Processes of Threads
            for(j=0 ; j<nNumofCPUCore ; j++)
                SEMA_WAIT(&(pFilter->nEndSema[j]));
        }
        else
        {
            Thread_Param        nThread_Param = {pFilter, };
            
            nThread_Param.pSrc0                 = (UINT8 *)pSrcImg0->GetImgPlanes(pSrcImg0, nPlaneIdx);
            nThread_Param.pSrc1                 = (UINT8 *)pSrcImg1->GetImgPlanes(pSrcImg1, nPlaneIdx);
            nThread_Param.pDst                  = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, nPlaneIdx);
            nThread_Param.nWidth                = pSrcImg0->GetImgWidth(pSrcImg0, nPlaneIdx);
            nThread_Param.nHeight               = pSrcImg0->GetImgHeight(pSrcImg0, nPlaneIdx);
            nThread_Param.nSrcStride            = pSrcImg0->GetImgStride(pSrcImg0, nPlaneIdx);
            nThread_Param.nDstStride            = pDstImg->GetImgStride(pDstImg, nPlaneIdx);
            
            _Filter_Multiply(&nThread_Param);
        }
    }
    #else
    {
        Thread_Param        nThread_Param = {pFilter, };
        
        nThread_Param.pSrc0                 = (UINT8 *)pSrcImg0->GetImgPlanes(pSrcImg0, nPlaneIdx);
        nThread_Param.pSrc1                 = (UINT8 *)pSrcImg1->GetImgPlanes(pSrcImg1, nPlaneIdx);
        nThread_Param.pDst                  = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, nPlaneIdx);
        nThread_Param.nWidth                = pSrcImg0->GetImgWidth(pSrcImg0, nPlaneIdx);
        nThread_Param.nHeight               = pSrcImg0->GetImgHeight(pSrcImg0, nPlaneIdx);
        nThread_Param.nSrcStride            = pSrcImg0->GetImgStride(pSrcImg0, nPlaneIdx);
        nThread_Param.nDstStride            = pDstImg->GetImgStride(pDstImg, nPlaneIdx);
        
        _Filter_Multiply(&nThread_Param);
    }
    #endif
    
    //___STOP_PROFILE(FilterImg)
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_MixImg(IN const Filter_T *pFilter, IN const Img_T *pSrcImg0, IN const Img_T *pSrcImg1, OUT Img_T *pDstImg, IN const F32 nMixRatio)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   i = 0, j = 0, k = 0;

    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg1, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pSrcImg0, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER)
    
    for(k=0 ; k<pSrcImg0->GetNumofImgPlanes(pSrcImg0) ; k++)
    {
        INT32               nLocalPos = 0;
        const UINT8         *pSrc0 = (UINT8 *)pSrcImg0->GetImgPlanes(pSrcImg0, k);
        const UINT8         *pSrc1 = (UINT8 *)pSrcImg1->GetImgPlanes(pSrcImg1, k);
        UINT8               *pDst = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, k);
        const INT32         nWidth = pSrcImg0->GetImgWidth(pSrcImg0, k);
        const INT32         nHeight = pSrcImg0->GetImgHeight(pSrcImg0, k);
        const INT32         nStride = pSrcImg0->GetImgStride(pSrcImg0, k);
        
        for(j=0 ; j<nHeight ; j++)
        {
            nLocalPos = j * nStride;
            
            for(i=0 ; i<nWidth ; i++)
            {
                pDst[nLocalPos] = (UINT8)((nMixRatio * (F32)pSrc0[nLocalPos]) - ((1.0f - nMixRatio) * (F32)pSrc1[nLocalPos]));
                nLocalPos++;
            }
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_MixImg_Plane(IN const Filter_T *pFilter, IN const Img_T *pSrcImg0, IN const Img_T *pSrcImg1, OUT Img_T *pDstImg, IN const F32 nMixRatio, IN const UINT32 nPlaneIdx)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg1, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pSrcImg0, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER)
    
    {
        INT32               i = 0, j = 0;
        INT32               nLocalPos = 0;
        const INT32         nWidth = pSrcImg0->GetImgWidth(pSrcImg0, nPlaneIdx);
        const INT32         nHeight = pSrcImg0->GetImgHeight(pSrcImg0, nPlaneIdx);
        const INT32         nStride = pSrcImg0->GetImgStride(pSrcImg0, nPlaneIdx);
        const UINT8         *pSrc0 = (UINT8 *)pSrcImg0->GetImgPlanes(pSrcImg0, nPlaneIdx);
        const UINT8         *pSrc1 = (UINT8 *)pSrcImg1->GetImgPlanes(pSrcImg1, nPlaneIdx);
        UINT8               *pDst = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, nPlaneIdx);
        
        for(j=0 ; j<nHeight ; j++)
        {
            nLocalPos = j * nStride;
            
            for(i=0 ; i<nWidth ; i++)
            {
                pDst[nLocalPos] = (UINT8)((nMixRatio * (F32)pSrc0[nLocalPos]) + ((1.0f - nMixRatio) * (F32)pSrc1[nLocalPos]));
                nLocalPos++;
            }
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_ProportionalMixImg(IN Filter_T *pFilter, IN const Img_T *pSrcImg0, IN const Img_T *pSrcImg1, OUT Img_T *pDstImg,
                                                                            IN const F32 nStartRadius, IN const F32 nEndRadius)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg0, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pSrcImg1, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER)
    
    ___START_PROFILE(FilterImg)
    
    #if USE_MULTI_THREAD
    {
        if(IMGPROC_TRUE == pFilter->bIsMultiThreadEnabled)
        {
            INT32           i = 0, j = 0;
            UINT32          nNumofCPUCore = pFilter->nNumofCPUCore;
            
            for(i=0 ; i<pSrcImg0->GetNumofImgPlanes(pSrcImg0) ; i++)
            {
                KISize          *pPropMixFltPrevSize = &(pFilter->nPropMixFltPrevSize[(!(!i))]);
                
                // Generate Proportional Filter Coefficient If Needed
                if((pPropMixFltPrevSize->nWidth[IMG_PLANE_0] != (pSrcImg0->GetImgWidth(pSrcImg0, i))) ||
                   (pPropMixFltPrevSize->nHeight[IMG_PLANE_0] != (pSrcImg0->GetImgHeight(pSrcImg0, i))))
                {
                    Kakao_ImgProc_Filter_Generate_PropMixCoeffi(pFilter, pSrcImg0->GetImgSize(pSrcImg0), (!(!i)), nStartRadius, nEndRadius);
                    pPropMixFltPrevSize->nWidth[IMG_PLANE_0] = pSrcImg0->GetImgWidth(pSrcImg0, i);
                    pPropMixFltPrevSize->nHeight[IMG_PLANE_0] = pSrcImg0->GetImgHeight(pSrcImg0, i);
                }

                for(j=0 ; j<nNumofCPUCore ; j++)
                {
                    Thread_Param        *pThread_Param = (Thread_Param *)(&pFilter->pThreadParam[j]);
 
                    pThread_Param->pFilterFunc      = (!i) ? _Filter_PropMixY : _Filter_PropMixUV;
                    pThread_Param->pFilter          = pFilter;
                    pThread_Param->pSrc0            = (UINT8 *)pSrcImg0->GetImgPlanes(pSrcImg0, i);
                    pThread_Param->pSrc1            = (UINT8 *)pSrcImg1->GetImgPlanes(pSrcImg1, i);
                    pThread_Param->pDst             = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, i);
                    pThread_Param->nWidth           = pSrcImg0->GetImgWidth(pSrcImg0, i);
                    pThread_Param->nHeight          = pSrcImg0->GetImgHeight(pSrcImg0, i);
                    pThread_Param->nSrcStride       = pSrcImg0->GetImgStride(pSrcImg0, i);
                    pThread_Param->nDstStride       = pDstImg->GetImgStride(pDstImg, i);
                    
                    SEMA_POST(&(pFilter->nStartSema[j]));
                }
                
                // Wait Until Done All Processes of Threads
                for(j=0 ; j<nNumofCPUCore ; j++)
                    SEMA_WAIT(&(pFilter->nEndSema[j]));
            }
        }
        else
        {
            INT32           i = 0;
            
            for(i=0 ; i<pSrcImg0->GetNumofImgPlanes(pSrcImg0) ; i++)
            {
                Thread_Param        nThread_Param = {pFilter, };
                KISize              *pPropMixFltPrevSize = &(pFilter->nPropMixFltPrevSize[(!(!i))]);
                
                nThread_Param.pSrc0             = (UINT8 *)pSrcImg0->GetImgPlanes(pSrcImg0, i);
                nThread_Param.pSrc1             = (UINT8 *)pSrcImg1->GetImgPlanes(pSrcImg1, i);
                nThread_Param.pDst              = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, i);
                nThread_Param.nWidth            = pSrcImg0->GetImgWidth(pSrcImg0, i);
                nThread_Param.nHeight           = pSrcImg0->GetImgHeight(pSrcImg0, i);
                nThread_Param.nSrcStride        = pSrcImg0->GetImgStride(pSrcImg0, i);
                nThread_Param.nDstStride        = pDstImg->GetImgStride(pDstImg, i);
                
                // Generate Proportional Filter Coefficient If Needed
                if((pPropMixFltPrevSize->nWidth[IMG_PLANE_0] != nThread_Param.nWidth) || (pPropMixFltPrevSize->nHeight[IMG_PLANE_0] != nThread_Param.nWidth))
                {
                    Kakao_ImgProc_Filter_Generate_PropMixCoeffi(pFilter, pSrcImg0->GetImgSize(pSrcImg0), (!(!i)), nStartRadius, nEndRadius);
                    pPropMixFltPrevSize->nWidth[IMG_PLANE_0] = nThread_Param.nWidth;
                    pPropMixFltPrevSize->nHeight[IMG_PLANE_0] = nThread_Param.nHeight;
                }
                
                if(0 == i)
                    _Filter_PropMixY(&nThread_Param);
                else
                    _Filter_PropMixUV(&nThread_Param);
            }
        }
    }
    #else
    {
        INT32           i = 0;
        
        for(i=0 ; i<pSrcImg0->GetNumofImgPlanes(pSrcImg0) ; i++)
        {
            Thread_Param        nThread_Param = {pFilter, };
            KISize              *pPropMixFltPrevSize = &(pFilter->nPropMixFltPrevSize[(!(!i))]);
            
            nThread_Param.pSrc0             = (UINT8 *)pSrcImg0->GetImgPlanes(pSrcImg0, i);
            nThread_Param.pSrc1             = (UINT8 *)pSrcImg1->GetImgPlanes(pSrcImg1, i);
            nThread_Param.pDst              = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, i);
            nThread_Param.nWidth            = pSrcImg0->GetImgWidth(pSrcImg0, i);
            nThread_Param.nHeight           = pSrcImg0->GetImgHeight(pSrcImg0, i);
            nThread_Param.nSrcStride        = pSrcImg0->GetImgStride(pSrcImg0, i);
            nThread_Param.nDstStride        = pDstImg->GetImgStride(pDstImg, i);
            
            // Generate Proportional Filter Coefficient If Needed
            if((pPropMixFltPrevSize->nWidth[IMG_PLANE_0] != nThread_Param.nWidth) || (pPropMixFltPrevSize->nHeight[IMG_PLANE_0] != nThread_Param.nHeight))
            {
                Kakao_ImgProc_Filter_Generate_PropMixCoeffi(pFilter, pSrcImg0->GetImgSize(pSrcImg0), (!(!i)), nStartRadius, nEndRadius);
                pPropMixFltPrevSize->nWidth[IMG_PLANE_0] = nThread_Param.nWidth;
                pPropMixFltPrevSize->nHeight[IMG_PLANE_0] = nThread_Param.nHeight;
            }
            
            if(0 == i)
                _Filter_PropMixY(&nThread_Param);
            else
                _Filter_PropMixUV(&nThread_Param);
        }
    }
    #endif
    
    ___STOP_PROFILE(FilterImg)
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_ProportionalMixImg_Plane(IN Filter_T *pFilter, IN const Img_T *pSrcImg0, IN const Img_T *pSrcImg1, OUT Img_T *pDstImg,
                                                                            IN const UINT32 nPlaneIdx, IN const F32 nStartRadius, IN const F32 nEndRadius)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg0, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg1, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER);
    
    //___START_PROFILE(FilterImg)
    
    #if USE_MULTI_THREAD
    {
        if(IMGPROC_TRUE == pFilter->bIsMultiThreadEnabled)
        {
            INT32           j = 0;
            UINT32          nNumofCPUCore = pFilter->nNumofCPUCore;
            KISize          *pPropMixFltPrevSize = &(pFilter->nPropMixFltPrevSize[(!(!nPlaneIdx))]);
            
            // Generate Proportional Filter Coefficient If Needed
            if((pPropMixFltPrevSize->nWidth[IMG_PLANE_0] != (pSrcImg0->GetImgWidth(pSrcImg0, nPlaneIdx))) ||
               (pPropMixFltPrevSize->nHeight[IMG_PLANE_0] != (pSrcImg0->GetImgHeight(pSrcImg0, nPlaneIdx))))
            {
                Kakao_ImgProc_Filter_Generate_PropMixCoeffi(pFilter, pSrcImg0->GetImgSize(pSrcImg0), (!(!nPlaneIdx)), nStartRadius, nEndRadius);
                pPropMixFltPrevSize->nWidth[IMG_PLANE_0] = pSrcImg0->GetImgWidth(pSrcImg0, nPlaneIdx);
                pPropMixFltPrevSize->nHeight[IMG_PLANE_0] = pSrcImg0->GetImgHeight(pSrcImg0, nPlaneIdx);
            }
            
            for(j=0 ; j<nNumofCPUCore ; j++)
            {
                Thread_Param        *pThread_Param = (Thread_Param *)(&pFilter->pThreadParam[j]);
                
                pThread_Param->pFilterFunc      = (!nPlaneIdx) ? _Filter_PropMixY : _Filter_PropMixUV;
                pThread_Param->pFilter          = pFilter;
                pThread_Param->pSrc0            = (UINT8 *)pSrcImg0->GetImgPlanes(pSrcImg0, nPlaneIdx);
                pThread_Param->pSrc1            = (UINT8 *)pSrcImg1->GetImgPlanes(pSrcImg1, nPlaneIdx);
                pThread_Param->pDst             = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, nPlaneIdx);
                pThread_Param->nWidth           = pSrcImg0->GetImgWidth(pSrcImg0, nPlaneIdx);
                pThread_Param->nHeight          = pSrcImg0->GetImgHeight(pSrcImg0, nPlaneIdx);
                pThread_Param->nSrcStride       = pSrcImg0->GetImgStride(pSrcImg0, nPlaneIdx);
                pThread_Param->nDstStride       = pDstImg->GetImgStride(pDstImg, nPlaneIdx);
                
                SEMA_POST(&(pFilter->nStartSema[j]));
            }
            
            // Wait Until Done All Processes of Threads
            for(j=0 ; j<nNumofCPUCore ; j++)
                SEMA_WAIT(&(pFilter->nEndSema[j]));
        }
        else
        {
            Thread_Param        nThread_Param = {pFilter, };
            KISize              *pPropMixFltPrevSize = &(pFilter->nPropMixFltPrevSize[(!(!nPlaneIdx))]);
            
            nThread_Param.pSrc0                 = (UINT8 *)pSrcImg0->GetImgPlanes(pSrcImg0, nPlaneIdx);
            nThread_Param.pSrc1                 = (UINT8 *)pSrcImg1->GetImgPlanes(pSrcImg1, nPlaneIdx);
            nThread_Param.pDst                  = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, nPlaneIdx);
            nThread_Param.nWidth                = pSrcImg0->GetImgWidth(pSrcImg0, nPlaneIdx);
            nThread_Param.nHeight               = pSrcImg0->GetImgHeight(pSrcImg0, nPlaneIdx);
            nThread_Param.nSrcStride            = pSrcImg0->GetImgStride(pSrcImg0, nPlaneIdx);
            nThread_Param.nDstStride            = pDstImg->GetImgStride(pDstImg, nPlaneIdx);
            
            
            // Generate Proportional Filter Coefficient If Needed
            if((pPropMixFltPrevSize->nWidth[IMG_PLANE_0] != nThread_Param.nWidth) || (pPropMixFltPrevSize->nHeight[IMG_PLANE_0] != nThread_Param.nHeight))
            {
                Kakao_ImgProc_Filter_Generate_PropMixCoeffi(pFilter, pSrcImg0->GetImgSize(pSrcImg0), (!(!nPlaneIdx)), nStartRadius, nEndRadius);
                pPropMixFltPrevSize->nWidth[IMG_PLANE_0] = nThread_Param.nWidth;
                pPropMixFltPrevSize->nHeight[IMG_PLANE_0] = nThread_Param.nHeight;
            }
            
            if(0 == nPlaneIdx)
                _Filter_PropMixY(&nThread_Param);
            else
                _Filter_PropMixUV(&nThread_Param);
        }
    }
    #else
    {
        Thread_Param        nThread_Param = {pFilter, };
        KISize              *pPropMixFltPrevSize = &(pFilter->nPropMixFltPrevSize[(!(!nPlaneIdx))]);
        
        nThread_Param.pSrc0                 = (UINT8 *)pSrcImg0->GetImgPlanes(pSrcImg0, nPlaneIdx);
        nThread_Param.pSrc1                 = (UINT8 *)pSrcImg1->GetImgPlanes(pSrcImg1, nPlaneIdx);
        nThread_Param.pDst                  = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, nPlaneIdx);
        nThread_Param.nWidth                = pSrcImg0->GetImgWidth(pSrcImg0, nPlaneIdx);
        nThread_Param.nHeight               = pSrcImg0->GetImgHeight(pSrcImg0, nPlaneIdx);
        nThread_Param.nSrcStride            = pSrcImg0->GetImgStride(pSrcImg0, nPlaneIdx);
        nThread_Param.nDstStride            = pDstImg->GetImgStride(pDstImg, nPlaneIdx);

        
        // Generate Proportional Filter Coefficient If Needed
        if((pPropMixFltPrevSize->nWidth[IMG_PLANE_0] != nThread_Param.nWidth) || (pPropMixFltPrevSize->nHeight[IMG_PLANE_0] != nThread_Param.nHeight))
        {
            Kakao_ImgProc_Filter_Generate_PropMixCoeffi(pFilter, pSrcImg0->GetImgSize(pSrcImg0), (!(!nPlaneIdx)), nStartRadius, nEndRadius);
            pPropMixFltPrevSize->nWidth[IMG_PLANE_0] = nThread_Param.nWidth;
            pPropMixFltPrevSize->nHeight[IMG_PLANE_0] = nThread_Param.nHeight;
        }
        
        if(0 == nPlaneIdx)
            _Filter_PropMixY(&nThread_Param);
        else
            _Filter_PropMixUV(&nThread_Param);
    }
    #endif
    
    //___STOP_PROFILE(FilterImg)
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_Generate_PropMixCoeffi(IN Filter_T *pFilter, IN const KISize nImgSize, IN const UINT32 nPlaneIdx, IN const F32 nStartRadius, IN const F32 nEndRadius)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    
    {
        INT32               i = 0, j = 0;
        INT32               nLocalPos = 0;
        F32                 *pPropMixCoeffi = NULL;
        const INT32         nWidth = nImgSize.nWidth[IMG_PLANE_0];
        const INT32         nHeight = nImgSize.nHeight[IMG_PLANE_0];
        const INT32         nHalfWidth = nWidth >> 1;
        const INT32         nHalfHeight = nHeight >> 1;
        const INT32         nStride = nImgSize.nStride[IMG_PLANE_0];
        F32                 nRatio = 0.0f;
        const F32           nRelativeRangeDist = nEndRadius - nStartRadius;
        
        if(NULL == pFilter->pPropMixCoeffi[nPlaneIdx])
            SAFEALLOC(pFilter->pPropMixCoeffi[nPlaneIdx], (nStride * nHeight * sizeof(F32)), 32, F32);
        
        pPropMixCoeffi = pFilter->pPropMixCoeffi[nPlaneIdx];
        
        for(j=0 ; j<nHalfHeight ; j++)
        {
            F32             nYDistOffset = (F32)ABSM(j - nHalfHeight) / (F32)(nHalfHeight);
            
            nYDistOffset *= nYDistOffset;
            nLocalPos = j * nStride;
            
            for(i=0 ; i<nHalfWidth ; i++)
            {
                
                F32         nXDistOffset = (F32)ABSM(i - nHalfWidth) / (F32)(nHalfWidth);
                F32         nDist = 0.0f;
                
                nXDistOffset *= nXDistOffset;
                nDist = (F32)(SQRT(nXDistOffset + nYDistOffset));
                
                if(nDist <= nStartRadius)
                    nRatio = 1.0f;
                else if((nStartRadius < nDist) && (nDist <= nEndRadius))
                    nRatio = 1.0f - (nDist - nStartRadius) / nRelativeRangeDist;
                else
                    nRatio = 0.0f;
                
                pPropMixCoeffi[nLocalPos++] = nRatio;
            }
        }
        
        for(j=0 ; j<nHalfHeight ; j++)
        {
            INT32           nXPosOffset = 0;
            
            nLocalPos = j * nStride + nHalfWidth;
            nXPosOffset = nLocalPos - 1;
            
            for(i=0 ; i<nHalfWidth ; i++)
            {
                pPropMixCoeffi[nLocalPos++] = pPropMixCoeffi[nXPosOffset--];
            }
        }
        
        for(j=0 ; j<nHalfHeight ; j++)
        {
            INT32           nSrcOffset = (nHalfHeight - j - 1) * nStride;
            INT32           nDstOffset = (nHalfHeight + j) * nStride;
            F32             *pSrc = &(pPropMixCoeffi[nSrcOffset]);
            F32             *pDst = &(pPropMixCoeffi[nDstOffset]);
            
            OneRow_Copy((UINT8 *)pDst, (UINT8 *)pSrc, nWidth * sizeof(F32));
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_ContrastShift(IN Filter_T *pFilter, IN const Img_T *pSrcImg0, IN const Img_T *pSrcImg1, OUT Img_T *pDstImg, IN const UINT32 nPlaneIdx)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg0, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg1, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER);
    
    //___START_PROFILE(FilterImg)
    
    #if USE_MULTI_THREAD
    {
        if(IMGPROC_TRUE == pFilter->bIsMultiThreadEnabled)
        {
            INT32           i = 0, j = 0;
            UINT32          nNumofCPUCore = pFilter->nNumofCPUCore;
            
            for(i=0 ; i<pSrcImg0->GetNumofImgPlanes(pSrcImg0) ; i++)
            {
                for(j=0 ; j<nNumofCPUCore ; j++)
                {
                    Thread_Param        *pThread_Param = (Thread_Param *)(&pFilter->pThreadParam[j]);
                    
                    pThread_Param->pFilterFunc      = _Filter_ContrastShift;
                    pThread_Param->pFilter          = pFilter;
                    pThread_Param->pSrc0            = (UINT8 *)pSrcImg0->GetImgPlanes(pSrcImg0, i);
                    pThread_Param->pSrc1            = (UINT8 *)pSrcImg1->GetImgPlanes(pSrcImg1, i);
                    pThread_Param->pDst             = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, i);
                    pThread_Param->nWidth           = pSrcImg0->GetImgWidth(pSrcImg0, i);
                    pThread_Param->nHeight          = pSrcImg0->GetImgHeight(pSrcImg0, i);
                    pThread_Param->nSrcStride       = pSrcImg0->GetImgStride(pSrcImg0, i);
                    pThread_Param->nDstStride       = pDstImg->GetImgStride(pDstImg, i);
                    
                    SEMA_POST(&(pFilter->nStartSema[j]));
                }
                
                // Wait Until Done All Processes of Threads
                for(j=0 ; j<nNumofCPUCore ; j++)
                    SEMA_WAIT(&(pFilter->nEndSema[j]));
            }
        }
        else
        {
            INT32           i = 0;
            
            for(i=0 ; i<pSrcImg0->GetNumofImgPlanes(pSrcImg0) ; i++)
            {
                Thread_Param        nThread_Param = {pFilter, };
                
                nThread_Param.pSrc0             = (UINT8 *)pSrcImg0->GetImgPlanes(pSrcImg0, i);
                nThread_Param.pSrc1             = (UINT8 *)pSrcImg1->GetImgPlanes(pSrcImg1, i);
                nThread_Param.pDst              = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, i);
                nThread_Param.nWidth            = pSrcImg0->GetImgWidth(pSrcImg0, i);
                nThread_Param.nHeight           = pSrcImg0->GetImgHeight(pSrcImg0, i);
                nThread_Param.nSrcStride        = pSrcImg0->GetImgStride(pSrcImg0, i);
                nThread_Param.nDstStride        = pDstImg->GetImgStride(pDstImg, i);
                
                _Filter_ContrastShift(&nThread_Param);
            }
        }
    }
    #else
    {
        INT32           i = 0;
        
        for(i=0 ; i<pSrcImg0->GetNumofImgPlanes(pSrcImg0) ; i++)
        {
            Thread_Param        nThread_Param = {pFilter, };
            
            nThread_Param.pSrc0             = (UINT8 *)pSrcImg0->GetImgPlanes(pSrcImg0, i);
            nThread_Param.pSrc1             = (UINT8 *)pSrcImg1->GetImgPlanes(pSrcImg1, i);
            nThread_Param.pDst              = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, i);
            nThread_Param.nWidth            = pSrcImg0->GetImgWidth(pSrcImg0, i);
            nThread_Param.nHeight           = pSrcImg0->GetImgHeight(pSrcImg0, i);
            nThread_Param.nSrcStride        = pSrcImg0->GetImgStride(pSrcImg0, i);
            nThread_Param.nDstStride        = pDstImg->GetImgStride(pDstImg, i);
            
            _Filter_ContrastShift(&nThread_Param);
        }
    }
    #endif
    
    //___STOP_PROFILE(FilterImg)
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_ContrastShift_Plane(IN Filter_T *pFilter, IN const Img_T *pSrcImg0, IN const Img_T *pSrcImg1, OUT Img_T *pDstImg, IN const UINT32 nPlaneIdx)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg0, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg1, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER);
    
    //___START_PROFILE(FilterImg)
    
    #if USE_MULTI_THREAD
    {
        if(IMGPROC_TRUE == pFilter->bIsMultiThreadEnabled)
        {
            INT32           j = 0;
            UINT32          nNumofCPUCore = pFilter->nNumofCPUCore;
            
            for(j=0 ; j<nNumofCPUCore ; j++)
            {
                Thread_Param        *pThread_Param = (Thread_Param *)(&pFilter->pThreadParam[j]);
                
                pThread_Param->pFilterFunc      = _Filter_ContrastShift;
                pThread_Param->pFilter          = pFilter;
                pThread_Param->pSrc0            = (UINT8 *)pSrcImg0->GetImgPlanes(pSrcImg0, nPlaneIdx);
                pThread_Param->pSrc1            = (UINT8 *)pSrcImg1->GetImgPlanes(pSrcImg1, nPlaneIdx);
                pThread_Param->pDst             = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, nPlaneIdx);
                pThread_Param->nWidth           = pSrcImg0->GetImgWidth(pSrcImg0, nPlaneIdx);
                pThread_Param->nHeight          = pSrcImg0->GetImgHeight(pSrcImg0, nPlaneIdx);
                pThread_Param->nSrcStride       = pSrcImg0->GetImgStride(pSrcImg0, nPlaneIdx);
                pThread_Param->nDstStride       = pDstImg->GetImgStride(pDstImg, nPlaneIdx);
                
                SEMA_POST(&(pFilter->nStartSema[j]));
            }
            
            // Wait Until Done All Processes of Threads
            for(j=0 ; j<nNumofCPUCore ; j++)
                SEMA_WAIT(&(pFilter->nEndSema[j]));
        }
        else
        {
            Thread_Param        nThread_Param = {pFilter, };
            
            nThread_Param.pSrc0                 = (UINT8 *)pSrcImg0->GetImgPlanes(pSrcImg0, nPlaneIdx);
            nThread_Param.pSrc1                 = (UINT8 *)pSrcImg1->GetImgPlanes(pSrcImg1, nPlaneIdx);
            nThread_Param.pDst                  = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, nPlaneIdx);
            nThread_Param.nWidth                = pSrcImg0->GetImgWidth(pSrcImg0, nPlaneIdx);
            nThread_Param.nHeight               = pSrcImg0->GetImgHeight(pSrcImg0, nPlaneIdx);
            nThread_Param.nSrcStride            = pSrcImg0->GetImgStride(pSrcImg0, nPlaneIdx);
            nThread_Param.nDstStride            = pDstImg->GetImgStride(pDstImg, nPlaneIdx);
            
            _Filter_ContrastShift(&nThread_Param);
        }
    }
    #else
    {
        Thread_Param        nThread_Param = {pFilter, };
        
        nThread_Param.pSrc0                 = (UINT8 *)pSrcImg0->GetImgPlanes(pSrcImg0, nPlaneIdx);
        nThread_Param.pSrc1                 = (UINT8 *)pSrcImg1->GetImgPlanes(pSrcImg1, nPlaneIdx);
        nThread_Param.pDst                  = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, nPlaneIdx);
        nThread_Param.nWidth                = pSrcImg0->GetImgWidth(pSrcImg0, nPlaneIdx);
        nThread_Param.nHeight               = pSrcImg0->GetImgHeight(pSrcImg0, nPlaneIdx);
        nThread_Param.nSrcStride            = pSrcImg0->GetImgStride(pSrcImg0, nPlaneIdx);
        nThread_Param.nDstStride            = pDstImg->GetImgStride(pDstImg, nPlaneIdx);
        
        _Filter_ContrastShift(&nThread_Param);
    }
    #endif
    
    //___STOP_PROFILE(FilterImg)
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_Div(IN const Filter_T *pFilter, IN OUT Img_T *pSrcImg, IN const BinaryMapType nBinaryMap, IN const UINT32 nScaleFactor)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER)
    
    {
        INT32               i = 0, j = 0;
        const INT32         nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
        const INT32         nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
        const INT32         nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
        UINT32              nLocalPos = 0;
        UINT8               *pSrc = NULL;
        
        pSrc = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
        for(j=0 ; j<nHeight ; j++)
        {
            nLocalPos = j * nStride;
            for(i=0 ; i<nWidth ; i++)
            {
                pSrc[nLocalPos] = (UINT8)((D64)pSrc[nLocalPos] / (D64)nScaleFactor);
                nLocalPos++;
            }
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_Rotate(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg, IN const UINT32 nDegree)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   i = 0, j = 0;
    //INT32                   left, right;
    F32                     x = 0, y = 0, x2 = 0, y2 = 0;
    F32                     seta = 0;
    
    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);

    //MEMSET(&m_ResultImg[0][0], 0, 256*256*sizeof(UINT8));
    
    // Just rotate
    seta = PI / 4;
    for(j=0 ; j<256 ; j++)
    {
        for(i=0 ; i<256 ; i++)
        {
            x = -((j - 128) * (F32)(sin(seta))) + 128;
            y = (i - 128) * (F32)(cos(seta));
            x2 = x + y;
            x2 = x2 > 255 ? 255 : (x2 < 0 ? 0 : x2);
            
            x = ((i - 128) * (F32)(sin(seta))) + 128;
            y = (j - 128) * (F32)(cos(seta));
            y2 = x + y;
            y2 = y2 > 255 ? 255 : (y2 < 0 ? 0 : y2);
            
            //            m_ResultImg[(INT32)y2][(INT32)x2] = m_OpenImg[j][i];
        }
    }
    
    // Interpolation
    //    for(j=1 ; j<256 ; j++)
    //    {
    //        for(i=1 ; i<256 ; i++)
    //        {
    //            left = m_ResultImg[j][i-1];
    //            right = m_ResultImg[j][i+1];
    //
    //            if(left!=0 && right!=0 && m_ResultImg[j][i]==0)
    //            m_ResultImg[j][i] = (left + right) / 2;
    //        }
    //    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


#if ENABLE_ISET(CPU_ARMX | ISET_NEON)
static void _Filter_Erosion_NEON(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg, IN const BinaryMapType nBinaryMap, IN const INT32 nThick)
{
    INT32                   i = 0, j = 0;
    const INT32             nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    const INT32             nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    const INT32             nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    const UINT32            nHalfMBSize = MACRO_BLOCK_SIZE / 2;
    UINT8                   *pSrc = NULL;
    UINT8                   *pDst = NULL;
    uint8x16_t              nZeroLane;
    
    if(NULL == (pSrc = pSrcImg->GetImgBinaryMap((Img_T *)pSrcImg, nBinaryMap)))
        return;
    
    if(NULL == (pDst = pDstImg->GetImgBinaryMap(pDstImg, nBinaryMap)))
        return;

    
    nZeroLane = vdupq_n_u8(0);

    for(j=nThick ; j<nHeight-nThick ; j++)
    {
        uint8x16_t      nPrevLane0, nPrevLane1, nPrevLane2, nPrevMultiLane;
        uint8x16_t      nLeftShift, nRightShift;
        const UINT8     *pTmpSrc = pSrc + (j * nStride);
        UINT8           *pTmpDst = pDst + (j * nStride);
        
        nPrevLane0 = vld1q_u8(pTmpSrc - nStride);
        nPrevLane1 = vld1q_u8(pTmpSrc);
        nPrevLane2 = vld1q_u8(pTmpSrc + nStride);
        
        nPrevMultiLane = vmulq_u8(nPrevLane0, nPrevLane1);
        nPrevMultiLane = vmulq_u8(nPrevMultiLane, nPrevLane2);
        
        nLeftShift = vextq_u8(nPrevMultiLane, nZeroLane, 1);
        nRightShift = vextq_u8(nZeroLane, nPrevMultiLane, 15);
        
        nLeftShift = vandq_u8(nLeftShift, nPrevMultiLane);
        nRightShift = vandq_u8(nRightShift, nPrevMultiLane);
        nPrevMultiLane = vandq_u8(nLeftShift, nRightShift);
        
        vst1_u8(pTmpDst, vget_low_u8(nPrevMultiLane));
        
        pTmpSrc += MACRO_BLOCK_SIZE;
        pTmpDst += nHalfMBSize;
        for(i=nThick ; i<nWidth-nThick ; i+=nHalfMBSize)
        {
            uint8x8_t       nCurrLane0, nCurrLane1, nCurrLane2;

            nCurrLane0 = vld1_u8(pTmpSrc - nStride);
            nCurrLane1 = vld1_u8(pTmpSrc);
            nCurrLane2 = vld1_u8(pTmpSrc + nStride);

            nPrevLane0 = vcombine_u8(vget_high_u8(nPrevLane0), nCurrLane0);
            nPrevLane1 = vcombine_u8(vget_high_u8(nPrevLane1), nCurrLane1);
            nPrevLane2 = vcombine_u8(vget_high_u8(nPrevLane2), nCurrLane2);
            
            nPrevMultiLane = vmulq_u8(nPrevLane0, nPrevLane1);
            nPrevMultiLane = vmulq_u8(nPrevMultiLane, nPrevLane2);
            
            nLeftShift = vextq_u8(nPrevMultiLane, nZeroLane, 1);
            nRightShift = vextq_u8(nZeroLane, nPrevMultiLane, 15);
            
            nLeftShift = vandq_u8(nLeftShift, nPrevMultiLane);
            nRightShift = vandq_u8(nRightShift, nPrevMultiLane);
            nPrevMultiLane = vandq_u8(nLeftShift, nRightShift);
            
            vst1_u8(pTmpDst, vget_low_u8(nPrevMultiLane));
            
            pTmpSrc += nHalfMBSize;
            pTmpDst += nHalfMBSize;
        }
    }

    return;
}


static void _Filter_Dilation_NEON(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg, IN const BinaryMapType nBinaryMap)
{
    INT32                   i = 0, j = 0;
    INT32                   nThick = 2;
    const INT32             nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    const INT32             nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    const INT32             nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    UINT8                   *pSrc = NULL;
    UINT8                   *pDst = NULL;
    uint8x16_t              nZeroLane;
    
    if(NULL == (pSrc = pSrcImg->GetImgBinaryMap((Img_T *)pSrcImg, nBinaryMap)))
        return;
    
    if(NULL == (pDst = pDstImg->GetImgBinaryMap(pDstImg, nBinaryMap)))
        return;
    
    nZeroLane = vdupq_n_u8(0);
    
    // Dilation
    for(j=nThick ; j<nHeight-nThick ; j++)
    {
        UINT8           *pTmpSrc = pSrc + (j * nStride);
        UINT8           *pTmpDstLane0 = pDst + ((j - 1) * nStride);
        UINT8           *pTmpDstLane1 = pDst + (j * nStride);
        UINT8           *pTmpDstLane2 = pDst + ((j + 1) * nStride);
        
        for(i=nThick ; i<nWidth-nThick ; i+=MACRO_BLOCK_SIZE)
        {
            uint8x16_t      nCurrSrcLane;
            uint8x16_t      nLeftShift, nRightShift;
            
            __builtin_prefetch(pTmpSrc + (nWidth << 4));
            nCurrSrcLane = vld1q_u8(pTmpSrc);
            
            nLeftShift = vextq_u8(nCurrSrcLane, nZeroLane, 1);
            nRightShift = vextq_u8(nZeroLane, nCurrSrcLane, 15);
            
            nLeftShift = vorrq_u8(nLeftShift, nCurrSrcLane);
            nRightShift = vorrq_u8(nRightShift, nCurrSrcLane);
            nCurrSrcLane = vorrq_u8(nLeftShift, nRightShift);
            
            vst1q_u8(pTmpDstLane0, nCurrSrcLane);
            vst1q_u8(pTmpDstLane1, nCurrSrcLane);
            vst1q_u8(pTmpDstLane2, nCurrSrcLane);
            
            pTmpSrc += MACRO_BLOCK_SIZE;
            pTmpDstLane0 += MACRO_BLOCK_SIZE;
            pTmpDstLane1 += MACRO_BLOCK_SIZE;
            pTmpDstLane2 += MACRO_BLOCK_SIZE;
        }
    }

    return;
}
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_SSE4) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_SSE4)
static void _Filter_Erosion_SSE4(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg, IN const BinaryMapType nBinaryMap, IN const INT32 nThick)
{
    INT32                   i = 0, j = 0;
    INT32                   nLocalPos = 0, nInPos = 0;
    UINT32                  nCountFlag = 0;
    const UINT32            nThreshold = (2 * nThick + 1) * (2 * nThick + 1);
    const INT32             nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    const INT32             nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    const INT32             nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    UINT8                   *pSrc = NULL;
    UINT8                   *pDst = NULL;
    
    if(NULL == (pSrc = pSrcImg->GetImgBinaryMap((Img_T *)pSrcImg, nBinaryMap)))
        return;
    
    if(NULL == (pDst = pDstImg->GetImgBinaryMap(pDstImg, nBinaryMap)))
        return;

    // Kakao_ImgProc_Util_Erosion
    for(j=nThick ; j<nHeight-nThick ; j++)
    {
        nLocalPos = j * nStride;
        for(i=nThick ; i<nWidth-nThick ; i++)
        {
            nCountFlag = nThreshold;
            
            {
                nInPos = nLocalPos - nStride;
                nCountFlag -= !(!pSrc[nInPos++]);
                nCountFlag -= !(!pSrc[nInPos++]);
                nCountFlag -= !(!pSrc[nInPos]);
                
                nInPos = nLocalPos;
                nCountFlag -= !(!pSrc[nInPos++]);
                nCountFlag -= !(!pSrc[nInPos++]);
                nCountFlag -= !(!pSrc[nInPos]);
                
                nInPos = nLocalPos + nStride;
                nCountFlag -= !(!pSrc[nInPos++]);
                nCountFlag -= !(!pSrc[nInPos++]);
                nCountFlag -= !(!pSrc[nInPos]);
            }
            
            pDst[nLocalPos++] = (!nCountFlag) * INDICATE_VALUE;
        }
    }
    
    return;
}


static void _Filter_Dilation_SSE4(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg, IN const BinaryMapType nBinaryMap)
{
    INT32                   i = 0, j = 0;
    INT32                   nLocalPos = 0, nInPos = 0;
    const INT32             nThick = 2;
    const INT32             nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    const INT32             nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    const INT32             nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    UINT8                   *pSrc = NULL;
    UINT8                   *pDst = NULL;
    
    if(NULL == (pSrc = pSrcImg->GetImgBinaryMap((Img_T *)pSrcImg, nBinaryMap)))
        return;
    
    if(NULL == (pDst = pDstImg->GetImgBinaryMap(pDstImg, nBinaryMap)))
        return;
    
    // Dilation
    for(j=nThick ; j<nHeight-nThick ; j++)
    {
        nLocalPos = j * nStride;
        for(i=nThick ; i<nWidth-nThick ; i++)
        {
            if(0 != pSrc[nLocalPos])
            {
                nInPos = nLocalPos - nStride;
                pDst[nInPos++] = INDICATE_VALUE;
                pDst[nInPos++] = INDICATE_VALUE;
                pDst[nInPos] = INDICATE_VALUE;
                
                nInPos = nLocalPos;
                pDst[nInPos++] = INDICATE_VALUE;
                pDst[nInPos++] = INDICATE_VALUE;
                pDst[nInPos] = INDICATE_VALUE;
                
                nInPos = nLocalPos + nStride;
                pDst[nInPos++] = INDICATE_VALUE;
                pDst[nInPos++] = INDICATE_VALUE;
                pDst[nInPos] = INDICATE_VALUE;
            }
            else
                pDst[nLocalPos] = 0;
            
            nLocalPos++;
        }
    }
    
    return;
}
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_SSE3) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_SSE3)
static void _Filter_Erosion_SSE3(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg, IN const BinaryMapType nBinaryMap, IN const INT32 nThick)
{
    INT32                   i = 0, j = 0;
    INT32                   nLocalPos = 0, nInPos = 0;
    UINT32                  nCountFlag = 0;
    const UINT32            nThreshold = (2 * nThick + 1) * (2 * nThick + 1);
    const INT32             nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    const INT32             nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    const INT32             nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    UINT8                   *pSrc = NULL;
    UINT8                   *pDst = NULL;
    
    if(NULL == (pSrc = pSrcImg->GetImgBinaryMap((Img_T *)pSrcImg, nBinaryMap)))
        return;
    
    if(NULL == (pDst = pDstImg->GetImgBinaryMap(pDstImg, nBinaryMap)))
        return;

    // Kakao_ImgProc_Util_Erosion
    for(j=nThick ; j<nHeight-nThick ; j++)
    {
        nLocalPos = j * nStride;
        for(i=nThick ; i<nWidth-nThick ; i++)
        {
            nCountFlag = nThreshold;
            
            {
                nInPos = nLocalPos - nStride;
                nCountFlag -= !(!pSrc[nInPos++]);
                nCountFlag -= !(!pSrc[nInPos++]);
                nCountFlag -= !(!pSrc[nInPos]);
                
                nInPos = nLocalPos;
                nCountFlag -= !(!pSrc[nInPos++]);
                nCountFlag -= !(!pSrc[nInPos++]);
                nCountFlag -= !(!pSrc[nInPos]);
                
                nInPos = nLocalPos + nStride;
                nCountFlag -= !(!pSrc[nInPos++]);
                nCountFlag -= !(!pSrc[nInPos++]);
                nCountFlag -= !(!pSrc[nInPos]);
            }
            
            pDst[nLocalPos++] = (!nCountFlag) * INDICATE_VALUE;
        }
    }
    
    return;
}


static void _Filter_Dilation_SSE3(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg, IN const BinaryMapType nBinaryMap)
{
    INT32                   i = 0, j = 0;
    INT32                   nLocalPos = 0, nInPos = 0;
    const INT32             nThick = 2;
    const INT32             nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    const INT32             nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    const INT32             nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    UINT8                   *pSrc = NULL;
    UINT8                   *pDst = NULL;
    
    if(NULL == (pSrc = pSrcImg->GetImgBinaryMap((Img_T *)pSrcImg, nBinaryMap)))
        return;
    
    if(NULL == (pDst = pDstImg->GetImgBinaryMap(pDstImg, nBinaryMap)))
        return;
    
    // Dilation
    for(j=nThick ; j<nHeight-nThick ; j++)
    {
        nLocalPos = j * nStride;
        for(i=nThick ; i<nWidth-nThick ; i++)
        {
            if(0 != pSrc[nLocalPos])
            {
                nInPos = nLocalPos - nStride;
                pDst[nInPos++] = INDICATE_VALUE;
                pDst[nInPos++] = INDICATE_VALUE;
                pDst[nInPos] = INDICATE_VALUE;
                
                nInPos = nLocalPos;
                pDst[nInPos++] = INDICATE_VALUE;
                pDst[nInPos++] = INDICATE_VALUE;
                pDst[nInPos] = INDICATE_VALUE;
                
                nInPos = nLocalPos + nStride;
                pDst[nInPos++] = INDICATE_VALUE;
                pDst[nInPos++] = INDICATE_VALUE;
                pDst[nInPos] = INDICATE_VALUE;
            }
            else
                pDst[nLocalPos] = 0;
            
            nLocalPos++;
        }
    }
    
    return;
}
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_SSE2) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_SSE2)
static void _Filter_Erosion_SSE2(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg, IN const BinaryMapType nBinaryMap, IN const INT32 nThick)
{
    INT32                   i = 0, j = 0;
    INT32                   nLocalPos = 0, nInPos = 0;
    UINT32                  nCountFlag = 0;
    const UINT32            nThreshold = (2 * nThick + 1) * (2 * nThick + 1);
    const INT32             nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    const INT32             nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    const INT32             nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    UINT8                   *pSrc = NULL;
    UINT8                   *pDst = NULL;
    
    if(NULL == (pSrc = pSrcImg->GetImgBinaryMap((Img_T *)pSrcImg, nBinaryMap)))
        return;
    
    if(NULL == (pDst = pDstImg->GetImgBinaryMap(pDstImg, nBinaryMap)))
        return;

    // Kakao_ImgProc_Util_Erosion
    for(j=nThick ; j<nHeight-nThick ; j++)
    {
        nLocalPos = j * nStride;
        for(i=nThick ; i<nWidth-nThick ; i++)
        {
            nCountFlag = nThreshold;
            
            {
                nInPos = nLocalPos - nStride;
                nCountFlag -= !(!pSrc[nInPos++]);
                nCountFlag -= !(!pSrc[nInPos++]);
                nCountFlag -= !(!pSrc[nInPos]);
                
                nInPos = nLocalPos;
                nCountFlag -= !(!pSrc[nInPos++]);
                nCountFlag -= !(!pSrc[nInPos++]);
                nCountFlag -= !(!pSrc[nInPos]);
                
                nInPos = nLocalPos + nStride;
                nCountFlag -= !(!pSrc[nInPos++]);
                nCountFlag -= !(!pSrc[nInPos++]);
                nCountFlag -= !(!pSrc[nInPos]);
            }
            
            pDst[nLocalPos++] = (!nCountFlag) * INDICATE_VALUE;
        }
    }
    
    return;
}


static void _Filter_Dilation_SSE2(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg, IN const BinaryMapType nBinaryMap)
{
    INT32                   i = 0, j = 0;
    INT32                   nLocalPos = 0, nInPos = 0;
    const INT32             nThick = 2;
    const INT32             nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    const INT32             nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    const INT32             nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    UINT8                   *pSrc = NULL;
    UINT8                   *pDst = NULL;
    
    if(NULL == (pSrc = pSrcImg->GetImgBinaryMap((Img_T *)pSrcImg, nBinaryMap)))
        return;
    
    if(NULL == (pDst = pDstImg->GetImgBinaryMap(pDstImg, nBinaryMap)))
        return;
    
    // Dilation
    for(j=nThick ; j<nHeight-nThick ; j++)
    {
        nLocalPos = j * nStride;
        for(i=nThick ; i<nWidth-nThick ; i++)
        {
            if(0 != pSrc[nLocalPos])
            {
                nInPos = nLocalPos - nStride;
                pDst[nInPos++] = INDICATE_VALUE;
                pDst[nInPos++] = INDICATE_VALUE;
                pDst[nInPos] = INDICATE_VALUE;
                
                nInPos = nLocalPos;
                pDst[nInPos++] = INDICATE_VALUE;
                pDst[nInPos++] = INDICATE_VALUE;
                pDst[nInPos] = INDICATE_VALUE;
                
                nInPos = nLocalPos + nStride;
                pDst[nInPos++] = INDICATE_VALUE;
                pDst[nInPos++] = INDICATE_VALUE;
                pDst[nInPos] = INDICATE_VALUE;
            }
            else
                pDst[nLocalPos] = 0;
            
            nLocalPos++;
        }
    }
    
    return;
}
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_SSE) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_SSE)
static void _Filter_Erosion_SSE(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg, IN const BinaryMapType nBinaryMap, IN const INT32 nThick)
{
    INT32                   i = 0, j = 0;
    INT32                   nLocalPos = 0, nInPos = 0;
    UINT32                  nCountFlag = 0;
    const UINT32            nThreshold = (2 * nThick + 1) * (2 * nThick + 1);
    const INT32             nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    const INT32             nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    const INT32             nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    UINT8                   *pSrc = NULL;
    UINT8                   *pDst = NULL;
    
    if(NULL == (pSrc = pSrcImg->GetImgBinaryMap((Img_T *)pSrcImg, nBinaryMap)))
        return;
    
    if(NULL == (pDst = pDstImg->GetImgBinaryMap(pDstImg, nBinaryMap)))
        return;

    // Kakao_ImgProc_Util_Erosion
    for(j=nThick ; j<nHeight-nThick ; j++)
    {
        nLocalPos = j * nStride;
        for(i=nThick ; i<nWidth-nThick ; i++)
        {
            nCountFlag = nThreshold;
            
            {
                nInPos = nLocalPos - nStride;
                nCountFlag -= !(!pSrc[nInPos++]);
                nCountFlag -= !(!pSrc[nInPos++]);
                nCountFlag -= !(!pSrc[nInPos]);
                
                nInPos = nLocalPos;
                nCountFlag -= !(!pSrc[nInPos++]);
                nCountFlag -= !(!pSrc[nInPos++]);
                nCountFlag -= !(!pSrc[nInPos]);
                
                nInPos = nLocalPos + nStride;
                nCountFlag -= !(!pSrc[nInPos++]);
                nCountFlag -= !(!pSrc[nInPos++]);
                nCountFlag -= !(!pSrc[nInPos]);
            }
            
            pDst[nLocalPos++] = (!nCountFlag) * INDICATE_VALUE;
        }
    }
    
    return;
}


static void _Filter_Dilation_SSE(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg, IN const BinaryMapType nBinaryMap)
{
    INT32                   i = 0, j = 0;
    INT32                   nLocalPos = 0, nInPos = 0;
    const INT32             nThick = 2;
    const INT32             nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    const INT32             nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    const INT32             nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    UINT8                   *pSrc = NULL;
    UINT8                   *pDst = NULL;
    
    if(NULL == (pSrc = pSrcImg->GetImgBinaryMap((Img_T *)pSrcImg, nBinaryMap)))
        return;
    
    if(NULL == (pDst = pDstImg->GetImgBinaryMap(pDstImg, nBinaryMap)))
        return;
    
    // Dilation
    for(j=nThick ; j<nHeight-nThick ; j++)
    {
        nLocalPos = j * nStride;
        for(i=nThick ; i<nWidth-nThick ; i++)
        {
            if(0 != pSrc[nLocalPos])
            {
                nInPos = nLocalPos - nStride;
                pDst[nInPos++] = INDICATE_VALUE;
                pDst[nInPos++] = INDICATE_VALUE;
                pDst[nInPos] = INDICATE_VALUE;
                
                nInPos = nLocalPos;
                pDst[nInPos++] = INDICATE_VALUE;
                pDst[nInPos++] = INDICATE_VALUE;
                pDst[nInPos] = INDICATE_VALUE;
                
                nInPos = nLocalPos + nStride;
                pDst[nInPos++] = INDICATE_VALUE;
                pDst[nInPos++] = INDICATE_VALUE;
                pDst[nInPos] = INDICATE_VALUE;
            }
            else
                pDst[nLocalPos] = 0;
            
            nLocalPos++;
        }
    }
    
    return;
}
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_NONE) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_NONE)
static void _Filter_Erosion_C(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg, IN const BinaryMapType nBinaryMap, IN const INT32 nThick)
{
    INT32                   i = 0, j = 0;
    INT32                   nLocalPos = 0, nInPos = 0;
    UINT32                  nCountFlag = 0;
    const UINT32            nThreshold = (2 * nThick + 1) * (2 * nThick + 1);
    const INT32             nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    const INT32             nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    const INT32             nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    UINT8                   *pSrc = NULL;
    UINT8                   *pDst = NULL;
    
    if(NULL == (pSrc = pSrcImg->GetImgBinaryMap((Img_T *)pSrcImg, nBinaryMap)))
        return;
    
    if(NULL == (pDst = pDstImg->GetImgBinaryMap(pDstImg, nBinaryMap)))
        return;

    // Kakao_ImgProc_Util_Erosion
    for(j=nThick ; j<nHeight-nThick ; j++)
    {
        nLocalPos = j * nStride;
        for(i=nThick ; i<nWidth-nThick ; i++)
        {
            nCountFlag = nThreshold;
            
            {
                nInPos = nLocalPos - nStride;
                nCountFlag -= !(!pSrc[nInPos++]);
                nCountFlag -= !(!pSrc[nInPos++]);
                nCountFlag -= !(!pSrc[nInPos]);
                
                nInPos = nLocalPos;
                nCountFlag -= !(!pSrc[nInPos++]);
                nCountFlag -= !(!pSrc[nInPos++]);
                nCountFlag -= !(!pSrc[nInPos]);
                
                nInPos = nLocalPos + nStride;
                nCountFlag -= !(!pSrc[nInPos++]);
                nCountFlag -= !(!pSrc[nInPos++]);
                nCountFlag -= !(!pSrc[nInPos]);
            }
            
            pDst[nLocalPos++] = (!nCountFlag) * INDICATE_VALUE;
        }
    }
    
    return;
}


static void _Filter_Dilation_C(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg, IN const BinaryMapType nBinaryMap)
{
    INT32                   i = 0, j = 0;
    INT32                   nLocalPos = 0, nInPos = 0;
    const INT32             nThick = 2;
    const INT32             nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    const INT32             nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    const INT32             nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    UINT8                   *pSrc = NULL;
    UINT8                   *pDst = NULL;
    
    if(NULL == (pSrc = pSrcImg->GetImgBinaryMap((Img_T *)pSrcImg, nBinaryMap)))
        return;
    
    if(NULL == (pDst = pDstImg->GetImgBinaryMap(pDstImg, nBinaryMap)))
        return;
    
    // Dilation
    for(j=nThick ; j<nHeight-nThick ; j++)
    {
        nLocalPos = j * nStride;
        for(i=nThick ; i<nWidth-nThick ; i++)
        {
            if(0 != pSrc[nLocalPos])
            {
                nInPos = nLocalPos - nStride;
                pDst[nInPos++] = INDICATE_VALUE;
                pDst[nInPos++] = INDICATE_VALUE;
                pDst[nInPos] = INDICATE_VALUE;
                
                nInPos = nLocalPos;
                pDst[nInPos++] = INDICATE_VALUE;
                pDst[nInPos++] = INDICATE_VALUE;
                pDst[nInPos] = INDICATE_VALUE;
                
                nInPos = nLocalPos + nStride;
                pDst[nInPos++] = INDICATE_VALUE;
                pDst[nInPos++] = INDICATE_VALUE;
                pDst[nInPos] = INDICATE_VALUE;
            }
            else
                pDst[nLocalPos] = 0;
            
            nLocalPos++;
        }
    }
    
    return;
}
#else
static void _Filter_Erosion_C(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg, IN const BinaryMapType nBinaryMap, IN const INT32 nThick)
{
    INT32                   i = 0, j = 0;
    INT32                   nLocalPos = 0, nInPos = 0;
    UINT32                  nCountFlag = 0;
    UINT32                  nThreshold = (2 * nThick + 1) * (2 * nThick + 1);
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    UINT8                   *pSrc = NULL;
    UINT8                   *pDst = NULL;
    
    if(NULL == (pSrc = pSrcImg->GetImgBinaryMap((Img_T *)pSrcImg, nBinaryMap)))
        return;
    
    if(NULL == (pDst = pDstImg->GetImgBinaryMap(pDstImg, nBinaryMap)))
        return;

    // Kakao_ImgProc_Util_Erosion
    for(j=nThick ; j<nHeight-nThick ; j++)
    {
        nLocalPos = j * nStride;
        for(i=nThick ; i<nWidth-nThick ; i++)
        {
            nCountFlag = nThreshold;
            
            {
                nInPos = nLocalPos - nStride;
                nCountFlag -= !(!pSrc[nInPos++]);
                nCountFlag -= !(!pSrc[nInPos++]);
                nCountFlag -= !(!pSrc[nInPos]);
                
                nInPos = nLocalPos;
                nCountFlag -= !(!pSrc[nInPos++]);
                nCountFlag -= !(!pSrc[nInPos++]);
                nCountFlag -= !(!pSrc[nInPos]);
                
                nInPos = nLocalPos + nStride;
                nCountFlag -= !(!pSrc[nInPos++]);
                nCountFlag -= !(!pSrc[nInPos++]);
                nCountFlag -= !(!pSrc[nInPos]);
            }
            
            pDst[nLocalPos++] = (!nCountFlag) * INDICATE_VALUE;
        }
    }
    
    return;
}


static void _Filter_Dilation_C(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg, IN const BinaryMapType nBinaryMap)
{
    INT32                   i = 0, j = 0;
    INT32                   nLocalPos = 0, nInPos = 0;
    INT32                   nThick = 2;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    UINT8                   *pSrc = NULL;
    UINT8                   *pDst = NULL;
    
    if(NULL == (pSrc = pSrcImg->GetImgBinaryMap((Img_T *)pSrcImg, nBinaryMap)))
        return;
    
    if(NULL == (pDst = pDstImg->GetImgBinaryMap(pDstImg, nBinaryMap)))
        return;
    
    // Dilation
    for(j=nThick ; j<nHeight-nThick ; j++)
    {
        nLocalPos = j * nStride;
        for(i=nThick ; i<nWidth-nThick ; i++)
        {
            if(0 != pSrc[nLocalPos])
            {
                nInPos = nLocalPos - nStride;
                pDst[nInPos++] = INDICATE_VALUE;
                pDst[nInPos++] = INDICATE_VALUE;
                pDst[nInPos] = INDICATE_VALUE;
                
                nInPos = nLocalPos;
                pDst[nInPos++] = INDICATE_VALUE;
                pDst[nInPos++] = INDICATE_VALUE;
                pDst[nInPos] = INDICATE_VALUE;
                
                nInPos = nLocalPos + nStride;
                pDst[nInPos++] = INDICATE_VALUE;
                pDst[nInPos++] = INDICATE_VALUE;
                pDst[nInPos] = INDICATE_VALUE;
            }
            else
                pDst[nLocalPos] = 0;
            
            nLocalPos++;
        }
    }
    
    return;
}
#endif


static Kakao_Status _Filter_Blur_Gaussian_1DFilter(IN Filter_T *pFilter, IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, OUT UINT8 *pDst, IN const INT32 nWidth, IN const INT32 nHeight, IN const INT32 nStride)
{
    INT32                   i = 0, j = 0;
    INT32                   m = 0;
    INT32                   nLocalPos = 0, nCoeffiPos = 0;
    const INT32             nMaskSize = pFilter->nGaussianMask1DSize;
    const F32               *pGaussian1DFltCoeffi = pFilter->nGaussian1DFltCoeffi;
    F32                     *pTmpBuf = (F32 *)pFilter->pTmpF32Img->GetImgPlanes(pFilter->pTmpF32Img, IMG_PLANE_0);
    UINT32                  nValue = 0;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    for(j=0 ; j<nHeight ; j++)
    {
        nLocalPos = j * nStride;

        for(i=0 ; i<nWidth ; i++)
        {
            nValue = 0;
            nCoeffiPos = 0;
            for(m=-nMaskSize ; m<=nMaskSize ; m++)
                nValue += (UINT32)(pGaussian1DFltCoeffi[nCoeffiPos++]) * pSrc0[nLocalPos + m];
            
            pTmpBuf[nLocalPos++] = (UINT8)(CLIP3(PIXEL_MIN, PIXEL_MAX, (nValue >> FLOATFACTOR)));
        }
    }

    for(j=0 ; j<nHeight ; j++)
    {
        nLocalPos = j * nStride;
        
        for(i=0 ; i<nWidth ; i++)
        {
            nValue = 0;
            nCoeffiPos = 0;
            for(m=-nMaskSize ; m<=nMaskSize ; m++)
                nValue += (UINT32)(pGaussian1DFltCoeffi[nCoeffiPos++] * pTmpBuf[nLocalPos + (m * nStride)]);
            
            pDst[nLocalPos++] = (UINT8)(CLIP3(PIXEL_MIN, PIXEL_MAX, (nValue >> FLOATFACTOR)));
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _Filter_Blur_Gaussian_2DFilter(Thread_Param *pThread_Param)
{
    Filter_T                *pFilter = pThread_Param->pFilter;
    const UINT8             *pSrc0 = pThread_Param->pSrc0;
    const UINT8             *pSrc1 = pThread_Param->pSrc1;
    UINT8                   *pDst = pThread_Param->pDst;
    const INT32             nWidth = pThread_Param->nWidth;
    const INT32             nHeight = pThread_Param->nHeight;
    const INT32             nSrcStride = pThread_Param->nSrcStride;
    const INT32             nDstStride = pThread_Param->nDstStride;
    const UINT32            nThreadIdx = pThread_Param->nThreadIdx;
    INT32                   i = 0, j = 0, k = 0;
    const F32               *pCoeffi = pFilter->nGaussian2DFltCoeffi;
    UINT32                  nValue = 0;
    const UINT32            nVerticalOffset = pFilter->nNumofCPUCore;
    const UINT32            nShiftOffset = (nHeight & 0x08) ? 3 : 4;
    const INT32             nLocalHeight = (1 << nShiftOffset);
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    for(k=nThreadIdx ; k<(nHeight>>nShiftOffset) ; k+=nVerticalOffset)
    {
        for(j=0 ; j<nLocalHeight ; j++)
        {
            for(i=0 ; i<nWidth ; i++)
            {
                const INT32         nLocalSrcPos = ((k<<nShiftOffset) + j) * nSrcStride + i;
                const INT32         nLocalDstPos = ((k<<nShiftOffset) + j) * nDstStride + i;
                const UINT8         *pLocalSrc0 = &(pSrc0[nLocalSrcPos - 1]);
                const UINT8         *pLocalSrc1 = pLocalSrc0 + nSrcStride;
                const UINT8         *pLocalSrc2 = pLocalSrc0 + (nSrcStride << 1);
                
                nValue = 0;
                nValue += (UINT32)((pCoeffi[0] * pLocalSrc0[0])
                            + (pCoeffi[1] * pLocalSrc0[1])
                            + (pCoeffi[2] * pLocalSrc0[2])
                            + (pCoeffi[3] * pLocalSrc1[0])
                            + (pCoeffi[4] * pLocalSrc1[1])
                            + (pCoeffi[5] * pLocalSrc1[2])
                            + (pCoeffi[6] * pLocalSrc2[0])
                            + (pCoeffi[7] * pLocalSrc2[1])
                            + (pCoeffi[8] * pLocalSrc2[2]));
                
                pDst[nLocalDstPos] = (UINT8)(CLIP3(PIXEL_MIN, PIXEL_MAX, (nValue >> FLOATFACTOR)));
            }
        }
    }
    
    #if USE_MULTI_THREAD
    if(IMGPROC_TRUE == pFilter->bIsMultiThreadEnabled)
        SEMA_POST(&(pFilter->nEndSema[nThreadIdx]));
    #endif

    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _Filter_Blur_MeanFilter(Thread_Param *pThread_Param)
{
    Filter_T                *pFilter = pThread_Param->pFilter;
    const UINT8             *pSrc0 = pThread_Param->pSrc0;
    const UINT8             *pSrc1 = pThread_Param->pSrc1;
    UINT8                   *pDst = pThread_Param->pDst;
    const INT32             nWidth = pThread_Param->nWidth;
    const INT32             nHeight = pThread_Param->nHeight;
    const INT32             nSrcStride = pThread_Param->nSrcStride;
    const INT32             nDstStride = pThread_Param->nDstStride;
    const UINT32            nThreadIdx = pThread_Param->nThreadIdx;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   j = 0, k = 0;
    const UINT32            nVerticalOffset = pFilter->nNumofCPUCore;
    const UINT32            nShiftOffset = (nHeight & 0x08) ? 3 : 4;
    const INT32             nLocalHeight = (1 << nShiftOffset);

    // Copy First Top Line of Src to Dst due to Character of Mean filter Algorithm
    if(0 == nThreadIdx)
        OneRow_Copy(pDst - nDstStride - MACRO_BLOCK_SIZE, pSrc0 - MACRO_BLOCK_SIZE, nDstStride);
    
    for(k=nThreadIdx ; k<(nHeight>>nShiftOffset) ; k+=nVerticalOffset)
    {
        for(j=0 ; j<=nLocalHeight ; j+=3)
        {
            INT32           nSrcOffset = ((k<<nShiftOffset) + j) * nSrcStride;
            INT32           nDstOffset = ((k<<nShiftOffset) + j) * nDstStride;
            
            OneRow_MeanBlur(pSrc0 + nSrcOffset, pDst + nDstOffset, nWidth, nSrcStride, nDstStride);
        }
    }
    
    #if USE_MULTI_THREAD
    if(IMGPROC_TRUE == pFilter->bIsMultiThreadEnabled)
        SEMA_POST(&(pFilter->nEndSema[nThreadIdx]));
    #endif
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _Filter_Blur_NoiseReductionFilter(Thread_Param *pThread_Param)
{
    Filter_T                *pFilter = pThread_Param->pFilter;
    const UINT8             *pSrc0 = pThread_Param->pSrc0;
    const UINT8             *pSrc1 = pThread_Param->pSrc1;
    UINT8                   *pDst = pThread_Param->pDst;
    const INT32             nWidth = pThread_Param->nWidth;
    const INT32             nHeight = pThread_Param->nHeight;
    const INT32             nSrcStride = pThread_Param->nSrcStride;
    const INT32             nDstStride = pThread_Param->nDstStride;
    const UINT32            nThreadIdx = pThread_Param->nThreadIdx;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   j = 0, k = 0;
    const UINT32            nVerticalOffset = pFilter->nNumofCPUCore;
    const UINT32            nShiftOffset = (nHeight & 0x08) ? 3 : 4;
    const INT32             nLocalHeight = (1 << nShiftOffset);
    
    // Copy First Top Line of Src to Dst due to Character of Mean filter Algorithm
    if(0 == nThreadIdx)
        OneRow_Copy(pDst - nDstStride - MACRO_BLOCK_SIZE, pSrc0 - MACRO_BLOCK_SIZE, nDstStride);
    
    for(k=nThreadIdx ; k<(nHeight>>nShiftOffset) ; k+=nVerticalOffset)
    {
        for(j=0 ; j<=nLocalHeight ; j+=3)
        {
            INT32           nSrcOffset = ((k<<nShiftOffset) + j) * nSrcStride;
            INT32           nDstOffset = ((k<<nShiftOffset) + j) * nDstStride;
            
            OneRow_BlurNoiseReduction(pSrc0 + nSrcOffset, pDst + nDstOffset, nWidth, nSrcStride, nDstStride);
        }
    }
    
    #if USE_MULTI_THREAD
    if(IMGPROC_TRUE == pFilter->bIsMultiThreadEnabled)
        SEMA_POST(&(pFilter->nEndSema[nThreadIdx]));
    #endif
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _Filter_Blur_UnsharpenFilter(Thread_Param *pThread_Param)
{
    Filter_T                *pFilter = pThread_Param->pFilter;
    const UINT8             *pSrc0 = pThread_Param->pSrc0;
    const UINT8             *pSrc1 = pThread_Param->pSrc1;
    UINT8                   *pDst = pThread_Param->pDst;
    const INT32             nWidth = pThread_Param->nWidth;
    const INT32             nHeight = pThread_Param->nHeight;
    const INT32             nSrcStride = pThread_Param->nSrcStride;
    const INT32             nDstStride = pThread_Param->nDstStride;
    const UINT32            nThreadIdx = pThread_Param->nThreadIdx;
    INT32                   i = 0, j = 0;
    INT32                   m = 0, n = 0;
    INT32                   nSrcPos = 0, nDstPos = 0;
    INT32                   nMaskSize = 1;
    F32                     nValue = 0.0f;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    for(j=0 ; j<nHeight ; j++)
    {
        for(i=nWidth/2 ; i<nWidth ; i++)
        {
//            const F32       nUnSharpMask[3][3] = {{-1, -3, -1},
//                                                  {-3,  17, -3},
//                                                  {-1, -3, -1}};
            const F32       nUnSharpMask[3][3] = {{-0.1, -0.5, -0.1},
                                                    {-0.5,  3.4, -0.5},
                                                    {-0.1, -0.5, -0.1}};
//            const F32       nUnSharpMask[3][3] = {{-0.1, -0.3, -0.1},
//                                                  {-0.3,  2.6, -0.3},
//                                                  {-0.1, -0.3, -0.1}};
            
            nDstPos = j * nDstStride + i;
            
            nValue = 0.0f;
            for(n=-nMaskSize ; n<=nMaskSize ; n++)
            {
                nSrcPos = (j + n) * nSrcStride + i;
                
                for(m=-nMaskSize ; m<=nMaskSize ; m++)
                    nValue += (F32)(pSrc0[nSrcPos + m]) * nUnSharpMask[n+1][m+1];
            }
            
            pDst[nDstPos] = (UINT8)(CLIP3(16.0, 235.0, nValue));
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _Filter_Blur_NbyNMeanFilter(Thread_Param *pThread_Param)
{
    Filter_T                *pFilter = pThread_Param->pFilter;
    const UINT8             *pSrc0 = pThread_Param->pSrc0;
    const UINT8             *pSrc1 = pThread_Param->pSrc1;
    UINT8                   *pDst = pThread_Param->pDst;
    const INT32             nWidth = pThread_Param->nWidth;
    const INT32             nHeight = pThread_Param->nHeight;
    const INT32             nSrcStride = pThread_Param->nSrcStride;
    const INT32             nDstStride = pThread_Param->nDstStride;
    const UINT32            nThreadIdx = pThread_Param->nThreadIdx;
    const INT32             nMaskSize = CLIP3(1, 5, pThread_Param->nNbyNMeanMaskSize);
    INT32                   i = 0, j = 0;
    INT32                   m = 0, n = 0;
    INT32                   nSrcPos = 0, nDstPos = 0;
    UINT32                  nBase = 0;
    UINT32                  nValue = 0;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    nBase = (2 * nMaskSize + 1) * (2 * nMaskSize + 1);
    
    for(j=0 ; j<nHeight ; j++)
    {
        for(i=0 ; i<nWidth ; i++)
        {
            nDstPos = j * nDstStride + i;
            
            nValue = 0;
            for(n=-nMaskSize ; n<=nMaskSize ; n++)
            {
                nSrcPos = (j + n) * nSrcStride + i;
                
                for(m=-nMaskSize ; m<=nMaskSize ; m++)
                {
                    nValue += (UINT32)(pSrc0[nSrcPos + m]);
                }
            }
            
            pDst[nDstPos] = (UINT8)(CLIP3(PIXEL_MIN, PIXEL_MAX, (nValue / nBase)));
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _Filter_Quantize_Plane(Thread_Param *pThread_Param)
{
    Filter_T                *pFilter = pThread_Param->pFilter;
    const UINT8             *pSrc0 = pThread_Param->pSrc0;
    const UINT8             *pSrc1 = pThread_Param->pSrc1;
    UINT8                   *pDst = pThread_Param->pDst;
    const INT32             nWidth = pThread_Param->nWidth;
    const INT32             nHeight = pThread_Param->nHeight;
    const INT32             nSrcStride = pThread_Param->nSrcStride;
    const INT32             nDstStride = pThread_Param->nDstStride;
    const UINT32            nThreadIdx = pThread_Param->nThreadIdx;
    const UINT8             *pQuantTable = pFilter->pQuantTable[pThread_Param->nQuantLevel];
    INT32                   i = 0, j = 0;
    INT32                   nSrcPos = 0, nDstPos = 0;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    for(j=0 ; j<nHeight ; j++)
    {
        nSrcPos = j * nSrcStride;
        nDstPos = j * nDstStride;
        
        for(i=0 ; i<nWidth ; i++, nSrcPos++)
            pDst[nDstPos] = pQuantTable[pSrc0[nSrcPos]];
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _Filter_Mosaic4(Thread_Param *pThread_Param)
{
    Filter_T                *pFilter = pThread_Param->pFilter;
    const UINT8             *pSrc0 = pThread_Param->pSrc0;
    const UINT8             *pSrc1 = pThread_Param->pSrc1;
    UINT8                   *pDst = pThread_Param->pDst;
    const INT32             nWidth = pThread_Param->nWidth;
    const INT32             nHeight = pThread_Param->nHeight;
    const INT32             nSrcStride = pThread_Param->nSrcStride;
    const INT32             nDstStride = pThread_Param->nDstStride;
    const UINT32            nThreadIdx = pThread_Param->nThreadIdx;
    INT32                   j = 0, k = 0;
    const INT32             nMosaicSize = 4;
    const UINT32            nShiftOffset = (nHeight & 0x08) ? 3 : 4;
    const UINT32            nVerticalOffset = pFilter->nNumofCPUCore;
    const INT32             nLocalHeight = (1 << nShiftOffset);
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    for(k=nThreadIdx ; k<(nHeight>>nShiftOffset) ; k+=nVerticalOffset)
    {
        for(j=0 ; j<nLocalHeight ; j+=nMosaicSize)
        {
            UINT32          nSrcOffset = ((k<<nShiftOffset) + j) * nSrcStride;
            UINT32          nDstOffset = ((k<<nShiftOffset) + j) * nDstStride;
            
            OneRow_Mosaic4(pSrc0 + nSrcOffset, pDst + nDstOffset, nWidth, nSrcStride, nDstStride, NULL);
        }
    }
    
    #if USE_MULTI_THREAD
    if(IMGPROC_TRUE == pFilter->bIsMultiThreadEnabled)
        SEMA_POST(&(pFilter->nEndSema[nThreadIdx]));
    #endif
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _Filter_Mosaic8(Thread_Param *pThread_Param)
{
    Filter_T                *pFilter = pThread_Param->pFilter;
    const UINT8             *pSrc0 = pThread_Param->pSrc0;
    const UINT8             *pSrc1 = pThread_Param->pSrc1;
    UINT8                   *pDst = pThread_Param->pDst;
    const INT32             nWidth = pThread_Param->nWidth;
    const INT32             nHeight = pThread_Param->nHeight;
    const INT32             nSrcStride = pThread_Param->nSrcStride;
    const INT32             nDstStride = pThread_Param->nDstStride;
    const UINT32            nThreadIdx = pThread_Param->nThreadIdx;
    INT32                   j = 0, k = 0;
    const INT32             nMosaicSize = 8;
    const UINT32            nShiftOffset = (nHeight & 0x08) ? 3 : 4;
    const UINT32            nVerticalOffset = pFilter->nNumofCPUCore;
    const INT32             nLocalHeight = (1 << nShiftOffset);
    RectImgAddr             nBaseRectAddr = {NULL, };
    UINT32                  *pIntegralImg = (UINT32 *)pFilter->pIntegralImg->GetImgPlanes(pFilter->pIntegralImg, IMG_PLANE_0);
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    nBaseRectAddr.pTL = GET_INTEGRAL_PIX_POS(pIntegralImg, 0, 0, nSrcStride);
    nBaseRectAddr.pTR = GET_INTEGRAL_PIX_POS(pIntegralImg, nMosaicSize, 0, nSrcStride);
    nBaseRectAddr.pBL = GET_INTEGRAL_PIX_POS(pIntegralImg, 0, nMosaicSize, nSrcStride);
    nBaseRectAddr.pBR = GET_INTEGRAL_PIX_POS(pIntegralImg, nMosaicSize, nMosaicSize, nSrcStride);
    
    for(k=nThreadIdx ; k<(nHeight>>nShiftOffset) ; k+=nVerticalOffset)
    {
        for(j=0 ; j<nLocalHeight ; j+=nMosaicSize)
        {
            UINT32              nSrcOffset = ((k<<nShiftOffset) + j) * nSrcStride;
            UINT32              nDstOffset = ((k<<nShiftOffset) + j) * nDstStride;
            RectImgAddr         nTmpRectAddr = {NULL, };
            
            nTmpRectAddr.pTL = nBaseRectAddr.pTL + nSrcOffset;
            nTmpRectAddr.pTR = nBaseRectAddr.pTR + nSrcOffset;
            nTmpRectAddr.pBL = nBaseRectAddr.pBL + nSrcOffset;
            nTmpRectAddr.pBR = nBaseRectAddr.pBR + nSrcOffset;
            
            OneRow_Mosaic8(pSrc0 + nSrcOffset, pDst + nDstOffset, nWidth, nDstStride, nDstOffset, &nTmpRectAddr);
        }
    }
    
    #if USE_MULTI_THREAD
    if(IMGPROC_TRUE == pFilter->bIsMultiThreadEnabled)
        SEMA_POST(&(pFilter->nEndSema[nThreadIdx]));
    #endif
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _Filter_Mosaic16(Thread_Param *pThread_Param)
{
    Filter_T                *pFilter = pThread_Param->pFilter;
    const UINT8             *pSrc0 = pThread_Param->pSrc0;
    const UINT8             *pSrc1 = pThread_Param->pSrc1;
    UINT8                   *pDst = pThread_Param->pDst;
    const INT32             nWidth = pThread_Param->nWidth;
    const INT32             nHeight = pThread_Param->nHeight;
    const INT32             nSrcStride = pThread_Param->nSrcStride;
    const INT32             nDstStride = pThread_Param->nDstStride;
    const UINT32            nThreadIdx = pThread_Param->nThreadIdx;
    INT32                   j = 0, k = 0;
    const INT32             nMosaicSize = 16;
    const UINT32            nShiftOffset = (nHeight & 0x08) ? 3 : 4;
    const UINT32            nVerticalOffset = pFilter->nNumofCPUCore;
    const INT32             nLocalHeight = (1 << nShiftOffset);
    RectImgAddr             nBaseRectAddr = {NULL, };
    UINT32                  *pIntegralImg = (UINT32 *)pFilter->pIntegralImg->GetImgPlanes(pFilter->pIntegralImg, IMG_PLANE_0);
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    nBaseRectAddr.pTL = GET_INTEGRAL_PIX_POS(pIntegralImg, 0, 0, nSrcStride);
    nBaseRectAddr.pTR = GET_INTEGRAL_PIX_POS(pIntegralImg, nMosaicSize, 0, nSrcStride);
    nBaseRectAddr.pBL = GET_INTEGRAL_PIX_POS(pIntegralImg, 0, nMosaicSize, nSrcStride);
    nBaseRectAddr.pBR = GET_INTEGRAL_PIX_POS(pIntegralImg, nMosaicSize, nMosaicSize, nSrcStride);
    
    for(k=nThreadIdx ; k<(nHeight>>nShiftOffset) ; k+=nVerticalOffset)
    {
        for(j=0 ; j<nLocalHeight ; j+=nMosaicSize)
        {
            UINT32              nSrcOffset = ((k<<nShiftOffset) + j) * nSrcStride;
            UINT32              nDstOffset = ((k<<nShiftOffset) + j) * nDstStride;
            
            OneRow_Mosaic16(pSrc0 + nSrcOffset, pDst + nDstOffset, nWidth, nDstStride, nDstOffset, &nBaseRectAddr);
        }
    }
    
    #if USE_MULTI_THREAD
    if(IMGPROC_TRUE == pFilter->bIsMultiThreadEnabled)
        SEMA_POST(&(pFilter->nEndSema[nThreadIdx]));
    #endif
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _Filter_Median_Square(Thread_Param *pThread_Param)
{
    Filter_T                *pFilter = pThread_Param->pFilter;
    const UINT8             *pSrc0 = pThread_Param->pSrc0;
    const UINT8             *pSrc1 = pThread_Param->pSrc1;
    UINT8                   *pDst = pThread_Param->pDst;
    const INT32             nWidth = pThread_Param->nWidth;
    const INT32             nHeight = pThread_Param->nHeight;
    const INT32             nSrcStride = pThread_Param->nSrcStride;
    const INT32             nDstStride = pThread_Param->nDstStride;
    const UINT32            nThreadIdx = pThread_Param->nThreadIdx;
    INT32                   i = 0, j = 0;
    UINT8                   nPixArr[9] = {0, };
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    for(j=0 ; j<nHeight ; j++)
    {
        for(i=0 ; i<nWidth ; i++)
        {
            INT32           nLocalPos = 0;
            
            nLocalPos = j * nSrcStride + i - (nSrcStride + 1);

            nPixArr[0] = pSrc0[nLocalPos];
            nPixArr[1] = pSrc0[nLocalPos + 1];
            nPixArr[2] = pSrc0[nLocalPos + 2];

            nLocalPos += nSrcStride;
            nPixArr[3] = pSrc0[nLocalPos];
            nPixArr[4] = pSrc0[nLocalPos + 1];
            nPixArr[5] = pSrc0[nLocalPos + 2];
            
            nLocalPos += nSrcStride;
            nPixArr[6] = pSrc0[nLocalPos];
            nPixArr[7] = pSrc0[nLocalPos + 1];
            nPixArr[8] = pSrc0[nLocalPos + 2];
            
            _Do_BubbleSort(nPixArr, 9);
            
            pDst[j * nDstStride + i] = nPixArr[4];
        }
    }
    
    #if USE_MULTI_THREAD
    if(IMGPROC_TRUE == pFilter->bIsMultiThreadEnabled)
        SEMA_POST(&(pFilter->nEndSema[nThreadIdx]));
    #endif

    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _Filter_Median_Horizon(Thread_Param *pThread_Param)
{
    Filter_T                *pFilter = pThread_Param->pFilter;
    const UINT8             *pSrc0 = pThread_Param->pSrc0;
    const UINT8             *pSrc1 = pThread_Param->pSrc1;
    UINT8                   *pDst = pThread_Param->pDst;
    const INT32             nWidth = pThread_Param->nWidth;
    const INT32             nHeight = pThread_Param->nHeight;
    const INT32             nSrcStride = pThread_Param->nSrcStride;
    const INT32             nDstStride = pThread_Param->nDstStride;
    const UINT32            nThreadIdx = pThread_Param->nThreadIdx;
    INT32                   i = 0, j = 0;
    UINT8                   nPixArr[3] = {0, };
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    for(j=0 ; j<nHeight ; j++)
    {
        for(i=0 ; i<nWidth ; i++)
        {
            const INT32     nLocalSrcPos = j * nSrcStride + i;
            const INT32     nLocalDstPos = j * nDstStride + i;
            
            nPixArr[0] = pSrc0[nLocalSrcPos - 1];
            nPixArr[1] = pSrc0[nLocalSrcPos];
            nPixArr[2] = pSrc0[nLocalSrcPos + 1];
            
            _Do_BubbleSort(nPixArr, 3);
            
            pDst[nLocalDstPos] = nPixArr[1];
        }
    }
    
    #if USE_MULTI_THREAD
    if(IMGPROC_TRUE == pFilter->bIsMultiThreadEnabled)
        SEMA_POST(&(pFilter->nEndSema[nThreadIdx]));
    #endif
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _Filter_Median_Vertical(Thread_Param *pThread_Param)
#if 1
{
    Filter_T                *pFilter = pThread_Param->pFilter;
    const UINT8             *pSrc0 = pThread_Param->pSrc0;
    const UINT8             *pSrc1 = pThread_Param->pSrc1;
    UINT8                   *pDst = pThread_Param->pDst;
    const INT32             nWidth = pThread_Param->nWidth;
    const INT32             nHeight = pThread_Param->nHeight;
    const INT32             nSrcStride = pThread_Param->nSrcStride;
    const INT32             nDstStride = pThread_Param->nDstStride;
    const UINT32            nThreadIdx = pThread_Param->nThreadIdx;
    INT32                   i = 0, j = 0;
    UINT8                   nPixArr[3] = {0, };
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    for(j=0 ; j<nHeight ; j++)
    {
        for(i=0 ; i<nWidth ; i++)
        {
            const INT32     nLocalSrcPos = (j - 1) * nSrcStride + i;
            const INT32     nLocalDstPos = j * nDstStride + i;

            nPixArr[0] = pSrc0[nLocalSrcPos];
            nPixArr[1] = pSrc0[nLocalSrcPos + nSrcStride];
            nPixArr[2] = pSrc0[nLocalSrcPos + 2 * nSrcStride];

            _Do_BubbleSort(nPixArr, 3);

            pDst[nLocalDstPos] = nPixArr[1];
        }
    }

    #if USE_MULTI_THREAD
    if(IMGPROC_TRUE == pFilter->bIsMultiThreadEnabled)
        SEMA_POST(&(pFilter->nEndSema[nThreadIdx]));
    #endif

    nRet = KAKAO_STAT_OK;
    return nRet;
}
#else
{
    Filter_T                *pFilter = pThread_Param->pFilter;
    const UINT8             *pSrc0 = pThread_Param->pSrc0;
    const UINT8             *pSrc1 = pThread_Param->pSrc1;
    UINT8                   *pDst = pThread_Param->pDst;
    const INT32             nWidth = pThread_Param->nWidth;
    const INT32             nHeight = pThread_Param->nHeight;
    const INT32             nSrcStride = pThread_Param->nSrcStride;
    const INT32             nDstStride = pThread_Param->nDstStride;
    const UINT32            nThreadIdx = pThread_Param->nThreadIdx;
    INT32                   i = 0, j = 0;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    UINT8                   *pCurrLine = (UINT8 *)&(pSrc0[0]);
    UINT8                   *pPrevLine = pCurrLine - nStride;
    UINT8                   *pNextLine = pCurrLine + nStride;
    
    for(j=nHeight/2 ; j<nHeight ; j++)
    {
        INT32           nLocalPos = j * nStride;
        
        for(i=0 ; i<nWidth ; i++)
        {
            #define GET_SCALE_IDX(a, b, nIdx0, nIdx1)           if(a < b)\
                                                                    {nPixIdx[nIdx1]++; nPixIdx[nIdx0]--;}\
                                                                else\
                                                                    {nPixIdx[nIdx0]++; nPixIdx[nIdx1]--;}
            
            const UINT8     nPixArr[3] = {pPrevLine[nLocalPos], pCurrLine[nLocalPos], pNextLine[nLocalPos]};
            UINT8           nPixIdx[3] = {0, };
            INT32           k = 0;
            
            GET_SCALE_IDX(nPixArr[0], nPixArr[1], 0, 1);
            GET_SCALE_IDX(nPixArr[1], nPixArr[2], 1, 2);
            GET_SCALE_IDX(nPixArr[0], nPixArr[2], 0, 2);
            
            for(k=0 ; k<3 ; k++)
                if(0 == nPixIdx[k])
                    break;
            
            //_Do_BubbleSort(nPixArr, 3);
            
            pDst[nLocalPos++] = nPixArr[k];
        }
    }
    
    #if USE_MULTI_THREAD
    if(IMGPROC_TRUE == pFilter->bIsMultiThreadEnabled)
        SEMA_POST(&(pFilter->nEndSema[nThreadIdx]));
    #endif
        
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}
#endif


static Kakao_Status _Filter_Mean_Horizon(Thread_Param *pThread_Param)
{
    Filter_T                *pFilter = pThread_Param->pFilter;
    const UINT8             *pSrc0 = pThread_Param->pSrc0;
    const UINT8             *pSrc1 = pThread_Param->pSrc1;
    UINT8                   *pDst = pThread_Param->pDst;
    const INT32             nWidth = pThread_Param->nWidth;
    const INT32             nHeight = pThread_Param->nHeight;
    const INT32             nSrcStride = pThread_Param->nSrcStride;
    const INT32             nDstStride = pThread_Param->nDstStride;
    const UINT32            nThreadIdx = pThread_Param->nThreadIdx;
    INT32                   j = 0, k = 0;
    const UINT32            nShiftOffset = (nHeight & 0x08) ? 3 : 4;
    const UINT32            nVerticalOffset = pFilter->nNumofCPUCore;
    const INT32             nLocalHeight = (1 << nShiftOffset);
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    for(k=nThreadIdx ; k<(nHeight>>nShiftOffset) ; k+=nVerticalOffset)
    {
        for(j=0 ; j<nLocalHeight ; j++)
        {
            UINT32              nSrcOffset = ((k<<nShiftOffset) + j) * nSrcStride;
            UINT32              nDstOffset = ((k<<nShiftOffset) + j) * nDstStride;
            
            OneRow_MeanBlur_Hor(pSrc0 + nSrcOffset, pDst + nDstOffset, nWidth);
        }
    }
    
    #if USE_MULTI_THREAD
    if(IMGPROC_TRUE == pFilter->bIsMultiThreadEnabled)
        SEMA_POST(&(pFilter->nEndSema[nThreadIdx]));
    #endif

    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _Filter_Mean_Vertical(Thread_Param *pThread_Param)
{
    Filter_T                *pFilter = pThread_Param->pFilter;
    const UINT8             *pSrc0 = pThread_Param->pSrc0;
    const UINT8             *pSrc1 = pThread_Param->pSrc1;
    UINT8                   *pDst = pThread_Param->pDst;
    const INT32             nWidth = pThread_Param->nWidth;
    const INT32             nHeight = pThread_Param->nHeight;
    const INT32             nSrcStride = pThread_Param->nSrcStride;
    const INT32             nDstStride = pThread_Param->nDstStride;
    const UINT32            nThreadIdx = pThread_Param->nThreadIdx;
    INT32                   j = 0, k = 0;
    const UINT32            nShiftOffset = (nHeight & 0x08) ? 3 : 4;
    const UINT32            nVerticalOffset = pFilter->nNumofCPUCore;
    const INT32             nLocalHeight = (1 << nShiftOffset);
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    for(k=nThreadIdx ; k<(nHeight>>nShiftOffset) ; k+=nVerticalOffset)
    {
        for(j=0 ; j<nLocalHeight ; j++)
        {
            UINT32              nSrcOffset = ((k<<nShiftOffset) + j) * nSrcStride;
            UINT32              nDstOffset = ((k<<nShiftOffset) + j) * nDstStride;
            
            OneRow_MeanBlur_Ver(pSrc0 + nSrcOffset, pDst + nDstOffset, nWidth, nSrcStride, nDstStride);
        }
    }

    #if USE_MULTI_THREAD
    if(IMGPROC_TRUE == pFilter->bIsMultiThreadEnabled)
        SEMA_POST(&(pFilter->nEndSema[nThreadIdx]));
    #endif

    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _Filter_Inverse(Thread_Param *pThread_Param)
{
    Filter_T                *pFilter = pThread_Param->pFilter;
    const UINT8             *pSrc0 = pThread_Param->pSrc0;
    const UINT8             *pSrc1 = pThread_Param->pSrc1;
    UINT8                   *pDst = pThread_Param->pDst;
    const INT32             nWidth = pThread_Param->nWidth;
    const INT32             nHeight = pThread_Param->nHeight;
    const INT32             nSrcStride = pThread_Param->nSrcStride;
    const INT32             nDstStride = pThread_Param->nDstStride;
    const UINT32            nThreadIdx = pThread_Param->nThreadIdx;
    INT32                   j = 0, k = 0;
    const UINT32            nShiftOffset = (nHeight & 0x08) ? 3 : 4;
    const UINT32            nVerticalOffset = pFilter->nNumofCPUCore;
    const INT32             nLocalHeight = (1 << nShiftOffset);
    
    for(k=nThreadIdx ; k<(nHeight>>nShiftOffset) ; k+=nVerticalOffset)
    {
        for(j=0 ; j<nLocalHeight ; j++)
        {
            UINT32              nSrcOffset = ((k<<nShiftOffset) + j) * nSrcStride;
            UINT32              nDstOffset = ((k<<nShiftOffset) + j) * nDstStride;
            
            OneRow_Inverse(pSrc0 + nSrcOffset, pDst + nDstOffset, nWidth);
        }
    }
        
    #if USE_MULTI_THREAD
    if(IMGPROC_TRUE == pFilter->bIsMultiThreadEnabled)
        SEMA_POST(&(pFilter->nEndSema[nThreadIdx]));
    #endif

    return KAKAO_STAT_OK;
}


static Kakao_Status _Filter_Clip(Thread_Param *pThread_Param)
{
    Filter_T                *pFilter = pThread_Param->pFilter;
    const UINT8             *pSrc0 = pThread_Param->pSrc0;
    const UINT8             *pSrc1 = pThread_Param->pSrc1;
    UINT8                   *pDst = pThread_Param->pDst;
    const INT32             nWidth = pThread_Param->nWidth;
    const INT32             nHeight = pThread_Param->nHeight;
    const INT32             nSrcStride = pThread_Param->nSrcStride;
    const INT32             nDstStride = pThread_Param->nDstStride;
    const UINT32            nThreadIdx = pThread_Param->nThreadIdx;
    INT32                   j = 0, k = 0;
    const UINT32            nShiftOffset = (nHeight & 0x08) ? 3 : 4;
    const UINT32            nVerticalOffset = pFilter->nNumofCPUCore;
    const INT32             nLocalHeight = (1 << nShiftOffset);
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    for(k=nThreadIdx ; k<(nHeight>>nShiftOffset) ; k+=nVerticalOffset)
    {
        for(j=0 ; j<nLocalHeight ; j++)
        {
            UINT32              nSrcOffset = ((k<<nShiftOffset) + j) * nSrcStride;
            UINT32              nDstOffset = ((k<<nShiftOffset) + j) * nDstStride;
            
            OneRow_Clip(pSrc0 + nSrcOffset, pDst + nDstOffset, nWidth, (const INT32)(pFilter->nMinClipVal), (const INT32)(pFilter->nMaxClipVal));
        }
    }
    
    #if USE_MULTI_THREAD
    if(IMGPROC_TRUE == pFilter->bIsMultiThreadEnabled)
        SEMA_POST(&(pFilter->nEndSema[nThreadIdx]));
    #endif
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _Filter_DiffImg(Thread_Param *pThread_Param)
{
    Filter_T                *pFilter = pThread_Param->pFilter;
    const UINT8             *pSrc0 = pThread_Param->pSrc0;
    const UINT8             *pSrc1 = pThread_Param->pSrc1;
    UINT8                   *pDst = pThread_Param->pDst;
    const INT32             nWidth = pThread_Param->nWidth;
    const INT32             nHeight = pThread_Param->nHeight;
    const INT32             nSrcStride = pThread_Param->nSrcStride;
    const INT32             nDstStride = pThread_Param->nDstStride;
    const UINT32            nThreadIdx = pThread_Param->nThreadIdx;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   j = 0, k = 0;
    const UINT32            nVerticalOffset = pFilter->nNumofCPUCore;
    const UINT32            nShiftOffset = (nHeight & 0x08) ? 3 : 4;
    const INT32             nLocalHeight = (1 << nShiftOffset);
    
    for(k=nThreadIdx ; k<(nHeight>>nShiftOffset) ; k+=nVerticalOffset)
    {
        for(j=0 ; j<nLocalHeight ; j++)
        {
            UINT32              nSrcOffset = ((k<<nShiftOffset) + j) * nSrcStride;
            UINT32              nDstOffset = ((k<<nShiftOffset) + j) * nDstStride;
            
            OneRow_DiffImg(pSrc0 + nSrcOffset, pSrc1 + nSrcOffset, pDst + nDstOffset, nWidth);
        }
    }
    
    #if USE_MULTI_THREAD
    if(IMGPROC_TRUE == pFilter->bIsMultiThreadEnabled)
        SEMA_POST(&(pFilter->nEndSema[nThreadIdx]));
    #endif
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
}


static Kakao_Status _Filter_Multiply(Thread_Param *pThread_Param)
{
    Filter_T                *pFilter = pThread_Param->pFilter;
    const UINT8             *pSrc0 = pThread_Param->pSrc0;
    const UINT8             *pSrc1 = pThread_Param->pSrc1;
    UINT8                   *pDst = pThread_Param->pDst;
    const INT32             nWidth = pThread_Param->nWidth;
    const INT32             nHeight = pThread_Param->nHeight;
    const INT32             nSrcStride = pThread_Param->nSrcStride;
    const INT32             nDstStride = pThread_Param->nDstStride;
    const UINT32            nThreadIdx = pThread_Param->nThreadIdx;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   j = 0, k = 0;
    const UINT32            nVerticalOffset = pFilter->nNumofCPUCore;
    const UINT32            nShiftOffset = (nHeight & 0x08) ? 3 : 4;
    const INT32             nLocalHeight = (1 << nShiftOffset);
    
    for(k=nThreadIdx ; k<(nHeight>>nShiftOffset) ; k+=nVerticalOffset)
    {
        for(j=0 ; j<nLocalHeight ; j++)
        {
            UINT32              nSrcOffset = ((k<<nShiftOffset) + j) * nSrcStride;
            UINT32              nDstOffset = ((k<<nShiftOffset) + j) * nDstStride;
            
            OneRow_Multiply(pSrc0 + nSrcOffset, pSrc1 + nSrcOffset, pDst + nDstOffset, nWidth);
        }
    }
    
    #if USE_MULTI_THREAD
    if(IMGPROC_TRUE == pFilter->bIsMultiThreadEnabled)
        SEMA_POST(&(pFilter->nEndSema[nThreadIdx]));
    #endif
    
    nRet = KAKAO_STAT_OK;

    return nRet;
}


static Kakao_Status _Filter_ContrastShift(Thread_Param *pThread_Param)
{
    Filter_T                *pFilter = pThread_Param->pFilter;
    const UINT8             *pSrc0 = pThread_Param->pSrc0;
    const UINT8             *pSrc1 = pThread_Param->pSrc1;
    UINT8                   *pDst = pThread_Param->pDst;
    const INT32             nWidth = pThread_Param->nWidth;
    const INT32             nHeight = pThread_Param->nHeight;
    const INT32             nSrcStride = pThread_Param->nSrcStride;
    const INT32             nDstStride = pThread_Param->nDstStride;
    const UINT32            nThreadIdx = pThread_Param->nThreadIdx;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   j = 0, k = 0;
    const UINT32            nVerticalOffset = pFilter->nNumofCPUCore;
    const UINT32            nShiftOffset = (nHeight & 0x08) ? 3 : 4;
    const INT32             nLocalHeight = (1 << nShiftOffset);
    
    for(k=nThreadIdx ; k<(nHeight>>nShiftOffset) ; k+=nVerticalOffset)
    {
        INT32               nSrcOffset = (k << nShiftOffset) * nSrcStride;
        INT32               nDstOffset = (k << nShiftOffset) * nDstStride;
        const UINT8         *pTmpSrc0 = pSrc0 + nSrcOffset;
        const UINT8         *pTmpSrc1 = pSrc1 + nSrcOffset;
        UINT8               *pTmpDst = pDst + nDstOffset;
        
        for(j=0 ; j<nLocalHeight ; j++)
        {
            OneRow_ContrastShift(pTmpSrc0, pTmpSrc1, pTmpDst, nWidth);
            
            pTmpSrc0 += nSrcStride;
            pTmpSrc1 += nSrcStride;
            pTmpDst += nDstStride;
        }
    }
    
    #if USE_MULTI_THREAD
    if(IMGPROC_TRUE == pFilter->bIsMultiThreadEnabled)
        SEMA_POST(&(pFilter->nEndSema[nThreadIdx]));
    #endif
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
}


static Kakao_Status _Filter_PropMixY(Thread_Param *pThread_Param)
{
    Filter_T                *pFilter = pThread_Param->pFilter;
    const UINT8             *pSrc0 = pThread_Param->pSrc0;
    const UINT8             *pSrc1 = pThread_Param->pSrc1;
    UINT8                   *pDst = pThread_Param->pDst;
    const INT32             nWidth = pThread_Param->nWidth;
    const INT32             nHeight = pThread_Param->nHeight;
    const INT32             nSrcStride = pThread_Param->nSrcStride;
    const INT32             nDstStride = pThread_Param->nDstStride;
    const UINT32            nThreadIdx = pThread_Param->nThreadIdx;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   j = 0, k = 0;
    const UINT32            nVerticalOffset = pFilter->nNumofCPUCore;
    const UINT32            nShiftOffset = (nHeight & 0x08) ? 3 : 4;
    const INT32             nLocalHeight = (1 << nShiftOffset);
    const F32               *pPropMixCoeffi = pFilter->pPropMixCoeffi[IMG_PLANE_0];
    
    for(k=nThreadIdx ; k<(nHeight>>nShiftOffset) ; k+=nVerticalOffset)
    {
        INT32               nSrcOffset = (k << nShiftOffset) * nSrcStride;
        INT32               nDstOffset = (k << nShiftOffset) * nDstStride;
        const UINT8         *pTmpSrc0 = pSrc0 + nSrcOffset;
        const UINT8         *pTmpSrc1 = pSrc1 + nSrcOffset;
        const F32           *pTmpPropMixCoeffi = pPropMixCoeffi + nSrcOffset;
        UINT8               *pTmpDst = pDst + nDstOffset;
        
        for(j=0 ; j<nLocalHeight ; j++)
        {
            OneRow_PropMix(pTmpSrc0, pTmpSrc1, pTmpPropMixCoeffi, pTmpDst, nWidth);
            
            pTmpSrc0 += nSrcStride;
            pTmpSrc1 += nSrcStride;
            pTmpPropMixCoeffi += nSrcStride;
            pTmpDst += nDstStride;
        }
    }
    
    #if USE_MULTI_THREAD
    if(IMGPROC_TRUE == pFilter->bIsMultiThreadEnabled)
        SEMA_POST(&(pFilter->nEndSema[nThreadIdx]));
    #endif
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
}


static Kakao_Status _Filter_PropMixUV(Thread_Param *pThread_Param)
{
    Filter_T                *pFilter = pThread_Param->pFilter;
    const UINT8             *pSrc0 = pThread_Param->pSrc0;
    const UINT8             *pSrc1 = pThread_Param->pSrc1;
    UINT8                   *pDst = pThread_Param->pDst;
    const INT32             nWidth = pThread_Param->nWidth;
    const INT32             nHeight = pThread_Param->nHeight;
    const INT32             nSrcStride = pThread_Param->nSrcStride;
    const INT32             nDstStride = pThread_Param->nDstStride;
    const UINT32            nThreadIdx = pThread_Param->nThreadIdx;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   j = 0, k = 0;
    const UINT32            nVerticalOffset = pFilter->nNumofCPUCore;
    const UINT32            nShiftOffset = (nHeight & 0x08) ? 3 : 4;
    const INT32             nLocalHeight = (1 << nShiftOffset);
    const F32               *pPropMixCoeffi = pFilter->pPropMixCoeffi[IMG_PLANE_1];
    
    for(k=nThreadIdx ; k<(nHeight>>nShiftOffset) ; k+=nVerticalOffset)
    {
        INT32               nSrcOffset = (k << nShiftOffset) * nSrcStride;
        INT32               nDstOffset = (k << nShiftOffset) * nDstStride;
        const UINT8         *pTmpSrc0 = pSrc0 + nSrcOffset;
        const UINT8         *pTmpSrc1 = pSrc1 + nSrcOffset;
        const F32           *pTmpPropMixCoeffi = pPropMixCoeffi + nSrcOffset;
        UINT8               *pTmpDst = pDst + nDstOffset;

        for(j=0 ; j<nLocalHeight ; j++)
        {
            OneRow_PropMix(pTmpSrc0, pTmpSrc1, pTmpPropMixCoeffi, pTmpDst, nWidth);
            
            pTmpSrc0 += nSrcStride;
            pTmpSrc1 += nSrcStride;
            pTmpPropMixCoeffi += nSrcStride;
            pTmpDst += nDstStride;
        }
    }
    
    #if USE_MULTI_THREAD
    if(IMGPROC_TRUE == pFilter->bIsMultiThreadEnabled)
        SEMA_POST(&(pFilter->nEndSema[nThreadIdx]));
    #endif
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
}


static Kakao_Status _Filter_Make_BulgePosTransMap(IN Filter_T *pFilter, IN const F32 nAspectRatio,
                                                  IN const F32 nScaleFactor, IN const F32 nRadius)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pFilter->pPosTransMap, KAKAO_STAT_NULL_POINTER);
    
    {
        Img_T                   *pPosTransMap = pFilter->pPosTransMap;
        const INT32             nWidth = pPosTransMap->GetImgWidth(pPosTransMap, IMG_PLANE_0);
        const INT32             nHeight = pPosTransMap->GetImgHeight(pPosTransMap, IMG_PLANE_0);
        const INT32             nStrideY = pPosTransMap->GetImgStride(pPosTransMap, IMG_PLANE_0);
        const INT32             nStrideUV = pPosTransMap->GetImgStride(pPosTransMap, IMG_PLANE_1);
        Vec2                    nCenterPos = {0, };
        INT32                   i = 0, j = 0;
        
        // Set Center Value as Center of Image
        nCenterPos.nPointF2.x = (F32)(nWidth / 2);
        nCenterPos.nPointF2.y = (F32)(nHeight / 2);
        
        // Set Radius for Bulge Filter
        pFilter->nBulgeRadius = nRadius;
        
        for(j=0 ; j<nHeight ; j++)
        {
            const INT32         nPosOffsetY = j * nStrideY;
            const INT32         nPosOffsetUV = (j / 2) * nStrideUV;
            UINT32              *pPosTransMapY = (UINT32 *)pPosTransMap->GetImgPlanes(pPosTransMap, IMG_PLANE_0) + nPosOffsetY;
            UINT32              *pPosTransMapU = (UINT32 *)pPosTransMap->GetImgPlanes(pPosTransMap, IMG_PLANE_1) + nPosOffsetUV;
            UINT32              *pPosTransMapV = (UINT32 *)pPosTransMap->GetImgPlanes(pPosTransMap, IMG_PLANE_2) + nPosOffsetUV;
            
            for(i=0 ; i<nWidth ; i++)
            {
                Vec2            nCurrPos = {0, };
                F32             nDist = 0.0f;
                INT32           nTransPosY = 0, nTransPosUV = 0;
                
                nCurrPos.nPointF2.x = (F32)i;
                nCurrPos.nPointF2.y = (F32)(j - nHeight/2) * nAspectRatio + nHeight/2;
                nDist = (F32)(SQRT(((nCenterPos.nPointF2.x - nCurrPos.nPointF2.x) * (nCenterPos.nPointF2.x - nCurrPos.nPointF2.x))
                             + ((nCenterPos.nPointF2.y - nCurrPos.nPointF2.y) * (nCenterPos.nPointF2.y - nCurrPos.nPointF2.y))));
                
                nCurrPos.nPointF2.x = (F32)i;
                nCurrPos.nPointF2.y = (F32)j;
                
                if(nDist > nRadius)
                {
                    nTransPosY = 0;//nPosOffsetY + i;
                    nTransPosUV = 0;//nPosOffsetUV + (i / 2);
                }
                else
                {
                    F32         nPercent = 1.0f - ((nRadius - nDist) / nRadius) * nScaleFactor;
                    
                    nCurrPos.nPointF2.x -= nCenterPos.nPointF2.x;
                    nCurrPos.nPointF2.y -= nCenterPos.nPointF2.y;
                    
                    nPercent = nPercent * nPercent;
                    
                    nCurrPos.nPointF2.x *= nPercent;
                    nCurrPos.nPointF2.y *= nPercent;
                    nCurrPos.nPointF2.x += nCenterPos.nPointF2.x;
                    nCurrPos.nPointF2.y += nCenterPos.nPointF2.y;
                    
                    nTransPosY = (INT32)((INT32)nCurrPos.nPointF2.y * nStrideY + (INT32)nCurrPos.nPointF2.x) - (nPosOffsetY + i);
                    nTransPosUV = (INT32)((INT32)nCurrPos.nPointF2.y/2 * nStrideUV + (INT32)nCurrPos.nPointF2.x/2) - (nPosOffsetUV + i/2);
                }
                
                *pPosTransMapY++ = nTransPosY;
                if((0 == (i & 0x1)) && (0 == (j & 0x1)))
                    *pPosTransMapU++ = *pPosTransMapV++ = nTransPosUV;
            }
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _Filter_Bulge_Plane(IN Filter_T *pFilter, IN Img_T *pSrcImg, OUT Img_T *pDstImg, IN const Vec2 nCenterPos)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    Vec2                    nLocalCenterPos;
    ROI                     nROI = {0, };
    
    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER);

    // Set ROI for Filtering Bulge
    {
        const Img_T         *pPosTransMap = pFilter->pPosTransMap;
        
        nLocalCenterPos.nPointF2.x = (0 == nCenterPos.nPointF2.x) ? pPosTransMap->GetImgWidth(pPosTransMap, IMG_PLANE_0) / 2 : nCenterPos.nPointF2.x;
        nLocalCenterPos.nPointF2.y = (0 == nCenterPos.nPointF2.y) ? pPosTransMap->GetImgHeight(pPosTransMap, IMG_PLANE_0) / 2 : nCenterPos.nPointF2.y;
        
        nROI.nStartX = CLIP3(0, pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0), (INT32)(nLocalCenterPos.nPointF2.x - pFilter->nBulgeRadius));
        nROI.nStartY = CLIP3(0, pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0), (INT32)(nLocalCenterPos.nPointF2.y - pFilter->nBulgeRadius));
        nROI.nEndX = CLIP3(0, pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0), (INT32)(nROI.nStartX + (2 * pFilter->nBulgeRadius)));
        nROI.nEndY = CLIP3(0, pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0), (INT32)(nROI.nStartY + (2 * pFilter->nBulgeRadius)));
        Kakao_ImgProc_Util_SetROI(pSrcImg, nROI);

        nROI.nStartX = CLIP3(0, pDstImg->GetImgWidth(pDstImg, IMG_PLANE_0), (INT32)(nLocalCenterPos.nPointF2.x - pFilter->nBulgeRadius));
        nROI.nStartY = CLIP3(0, pDstImg->GetImgHeight(pDstImg, IMG_PLANE_0), (INT32)(nLocalCenterPos.nPointF2.y - pFilter->nBulgeRadius));
        nROI.nEndX = CLIP3(0, pDstImg->GetImgWidth(pDstImg, IMG_PLANE_0), (INT32)(nROI.nStartX + (2 * pFilter->nBulgeRadius)));
        nROI.nEndY = CLIP3(0, pDstImg->GetImgHeight(pDstImg, IMG_PLANE_0), (INT32)(nROI.nStartY + (2 * pFilter->nBulgeRadius)));
        Kakao_ImgProc_Util_SetROI(pDstImg, nROI);
        
        nROI.nStartX = CLIP3(0, pPosTransMap->GetImgWidth(pPosTransMap, IMG_PLANE_0), (INT32)((pPosTransMap->GetImgWidth(pPosTransMap, IMG_PLANE_0) / 2) - pFilter->nBulgeRadius));
        nROI.nStartY = CLIP3(0, pPosTransMap->GetImgHeight(pPosTransMap, IMG_PLANE_0), (INT32)((pPosTransMap->GetImgHeight(pPosTransMap, IMG_PLANE_0) / 2) - pFilter->nBulgeRadius));
        nROI.nEndX = CLIP3(0, pPosTransMap->GetImgWidth(pPosTransMap, IMG_PLANE_0), (INT32)(nROI.nStartX + (2 * pFilter->nBulgeRadius)));
        nROI.nEndY = CLIP3(0, pPosTransMap->GetImgHeight(pPosTransMap, IMG_PLANE_0), (INT32)(nROI.nStartY + (2 * pFilter->nBulgeRadius)));
        Kakao_ImgProc_Util_SetROI(pFilter->pPosTransMap, nROI);
    }
    
    {
        const INT32             nWidth = pDstImg->GetImgWidth(pDstImg, IMG_PLANE_0);
        const INT32             nHeight = pDstImg->GetImgHeight(pDstImg, IMG_PLANE_0);
        const INT32             nStrideY = pDstImg->GetImgStride(pDstImg, IMG_PLANE_0);
        const INT32             nStrideUV = pDstImg->GetImgStride(pDstImg, IMG_PLANE_1);
        INT32                   i = 0, j = 0, k = 0;
        
        // Transform Y-Plane
        {
            const UINT8         *pSrc = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, k);
            
            for(j=0 ; j<nHeight ; j++)
            {
                const INT32         nPosOffset = j * nStrideY;
                const UINT32        *pPosTransMap = (UINT32 *)pFilter->pPosTransMap->GetImgPlanes(pFilter->pPosTransMap, k) + nPosOffset;
                UINT8               *pDst = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, k) + nPosOffset;
                
                for(i=0 ; i<nWidth ; i++)
                    *pDst++ = pSrc[nPosOffset + i + *pPosTransMap++];
            }
        }
        
        // Transform U and V Plane
        for(k=1 ; k<pSrcImg->GetNumofImgPlanes(pSrcImg) ; k++)
        {
            const UINT8         *pSrc = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, k);
            
            for(j=0 ; j<nHeight/2 ; j++)
            {
                const INT32         nPosOffset = j * nStrideUV;
                const UINT32        *pPosTransMap = (UINT32 *)pFilter->pPosTransMap->GetImgPlanes(pFilter->pPosTransMap, k) + nPosOffset;
                UINT8               *pDst = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, k) + nPosOffset;
                
                for(i=0 ; i<nWidth/2 ; i++)
                    *pDst++ = pSrc[nPosOffset + i + *pPosTransMap++];
            }
        }
    }
    
    // Reset ROI of Images
    MEMSET(&nROI, 0, sizeof(ROI));
    Kakao_ImgProc_Util_SetROI(pSrcImg, nROI);
    Kakao_ImgProc_Util_SetROI(pDstImg, nROI);
    Kakao_ImgProc_Util_SetROI(pFilter->pPosTransMap, nROI);

    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _Do_BubbleSort(IN UINT8 *pPixArr, IN const UINT32 nSize)
{
    INT32                   nHold = 0;
    UINT32                  i = 0, j = 0;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    for(i=1 ; i<nSize ; i++)
    {
        for(j=0 ; j<(nSize - 1) ; j++)
        {
            if(pPixArr[j] > pPixArr[j+1])
            {
                nHold = pPixArr[j];
                pPixArr[j] = pPixArr[j+1];
                pPixArr[j+1] = nHold;
            }
        }
    }

    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _Filter_Make_Blur_Gaussian_1DCoeffi(IN const F32 nSigma, OUT F32 *pFilterCoeffi, OUT UINT32 *pMaskSize)
{
    INT32                   i = 0;
    INT32                   nGaussianMask1DSize = 0;
    INT32                   nMaskSize = 0;
    F32                     nFilter_Coeffi = 0.0f;
    F32                     nSIGMA = nSigma * nSigma;
    F32                     nSum = 0.0f;
    F32                     nRatio = 0.0f;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    nMaskSize = -1;
    
    // For Limitting Filter size
    if(nSigma == 0)
        nMaskSize++;
    else
    {
        do
        {
            nMaskSize++;
            nGaussianMask1DSize = nMaskSize^2;
            nFilter_Coeffi = (F32)(EXP(-(nGaussianMask1DSize / (2.0 * nSIGMA))) / SQRT((F32)(2.0 * PI * nSIGMA)));
        }while(nFilter_Coeffi >= 0.001);
    }
    
    // Determined filer size
    nGaussianMask1DSize = 2 * nMaskSize + 1;
    
    if(nMaskSize == 0)
        pFilterCoeffi[0] = FLOATPRECISION;
    else
    {
        UINT32                  nPos = 0;
        
        // Calculate filter coefficients
        for(i=-nMaskSize ; i<=nMaskSize ; i++)
        {
            pFilterCoeffi[nPos] = (F32)(EXP(-((i*i) / (2.0 * nSIGMA))) / SQRT((F32)(2.0 * PI * nSIGMA)));
            nSum += pFilterCoeffi[nPos];
            
            nPos++;
        }
        
        nRatio = 1 / nSum;
        nPos = 0;
        
        // Normalization
        for(i=0 ; i<nGaussianMask1DSize ; i++)
        {
            pFilterCoeffi[nPos]= pFilterCoeffi[nPos] * nRatio * FLOATPRECISION;
            nPos++;
        }
    }
    
    *pMaskSize = (UINT32)nMaskSize;
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _Filter_Make_Blur_Gaussian_2DCoeffi(IN const F32 nSigma, OUT F32 *pFilterCoeffi, OUT UINT32 *pMaskSize)
{
    INT32                   i = 0, j = 0;
    INT32                   nGaussianMask2DSize = 0;
    INT32                   nMaskSize = 0;
    F32                     nFilter_Coeffi = 0.0f;
    F32                     nSIGMA = nSigma * nSigma;
    F32                     nSum = 0.0f;
    F32                     nRatio = 0.0f;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    nMaskSize = -1;
    
    // For Limitting Filter size
    if(nSigma == 0)
        nMaskSize++;
    else
    {
        do
        {
            nMaskSize++;
            nGaussianMask2DSize = nMaskSize^2;
            nFilter_Coeffi = (F32)(EXP(-((2 * nGaussianMask2DSize) / (2.0 * nSIGMA))) / (F32)(2.0 * PI * nSIGMA));
        }while(nFilter_Coeffi >= 0.001);
    }
    
    // Determined filer size
    nGaussianMask2DSize = 2 * nMaskSize + 1;
    
    if(nMaskSize == 0)
        pFilterCoeffi[0] = FLOATPRECISION;
    else
    {
        UINT32                  nPos = 0;
        
        // Calculate filter coefficients
        for(j=-nMaskSize ; j<=nMaskSize ; j++)
        {
            for(i=-nMaskSize ; i<=nMaskSize ; i++)
            {
                pFilterCoeffi[nPos] = (F32)(EXP(-(((i*i) + (j*j)) / (2.0 * nSIGMA))) / (F32)(2.0 * PI * nSIGMA));
                nSum += pFilterCoeffi[nPos];
                
                nPos++;
            }
        }
        
        nRatio = 1 / nSum;
        nPos = 0;
        
        // Normalization
        for(j=0 ; j<nGaussianMask2DSize ; j++)
        {
            for(i=0 ; i<nGaussianMask2DSize ; i++)
            {
                pFilterCoeffi[nPos]= pFilterCoeffi[nPos] * nRatio * FLOATPRECISION;
                nPos++;
            }
        }
    }

    *pMaskSize = nMaskSize;
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _Filter_Get_Mean_Variance(IN const Img_T *pSrcImg, OUT F32 *pMean, OUT F32 *pVariance)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   i = 0, j = 0;
    INT32                   nLocalPos = 0;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    UINT8                   *pSrc = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
    UINT32                  nImgSize = nWidth * nHeight;
    F32                     nSum = 0.0;
    F32                     nSquareSum = 0.0;
    
    for(j=0 ; j<nHeight ; j++)
    {
        nLocalPos = j * nStride;
        
        for(i=0 ; i<nWidth ; i++)
        {
            nSum += pSrc[nLocalPos];
            nSquareSum += (pSrc[nLocalPos] * pSrc[nLocalPos]);
            nLocalPos++;
        }
    }
    
    *pMean = (nSum / (F32)nImgSize);
    *pVariance = (nSquareSum / (F32)nImgSize) - ((*pMean) * (*pMean));
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _Filter_Make_QuantizationTable(IN OUT const Filter_T *pFilter)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   i = 0, j = 0;
    UINT8                   nQuantBase = 0;
    UINT8                   nHalfQuantBase = 0;
    UINT8                   *pQuantTable = NULL;
    
    for(j=0 ; j<QUANT_LEVEL_MAX ; j++)
    {
        nQuantBase = ((j + 1) << 2);
        nHalfQuantBase = (nQuantBase >> 1);
        pQuantTable = pFilter->pQuantTable[j];
        
        for(i=0 ; i<256 ; i++)
            pQuantTable[i] = (i & (~nQuantBase)) + nHalfQuantBase;
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _Filter_Make_IntegralImg(IN OUT Filter_T *pFilter, IN const Img_T *pSrcImg)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    UINT8                   *pSrc = NULL;
    UINT32                  *pIntegralImg = NULL;
    UINT32                  nCurrVal = 0;
    UINT32                  nRowSum = 0;
    UINT32                  nLocalPos = 0;
    INT32                   i = 0, j = 0;
    
    CHECK_POINTER_VALIDATION(pFilter, KAKAO_STAT_NULL_POINTER);
    
    // Integral Image & Square Integral Image
    pSrc = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
    pIntegralImg = (UINT32 *)pFilter->pIntegralImg->GetImgPlanes(pFilter->pIntegralImg, IMG_PLANE_0);
    for(i=0 ; i<nWidth ; i++)
    {
        nCurrVal = pSrc[i];
        nRowSum += nCurrVal;
        pIntegralImg[i] = nRowSum;
    }
    
    for(j=1 ; j<nHeight ; j++)
    {
        UINT32              *pPrevLineIntegralImg = NULL;
        
        nRowSum = 0;
        nLocalPos = j * nStride;
        pPrevLineIntegralImg = &(pIntegralImg[nLocalPos - nStride]);
        
        for(i=0 ; i<nWidth ; i++)
        {
            nCurrVal = pSrc[nLocalPos];
            nRowSum += nCurrVal;
            pIntegralImg[nLocalPos] = nRowSum + pPrevLineIntegralImg[i];
            nLocalPos++;
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


static F32 _EdgeStopFunc(IN const INT32 x, IN const F32 nESigma)
{
    F32                     value = 0.0f;
    
    value = EXP_F(-((F32)x * (F32)x) / (2.0f * nESigma * nESigma));
    
    return value;
}


static F32 _GaussianFunc(IN const INT32 m, IN const INT32 n, IN const F32 nGSigma)
{
    F32                     value = 0.0f;
    
    value = EXP_F((F32)(-((m * m) + (n * n))) / (2 * nGSigma * nGSigma)) / (2 * PI * nGSigma * nGSigma);
    
    return value;
}


#if USE_MULTI_THREAD
static void _Filter_Thread(void *pParamStruct)
{
    Thread_Param        *pThread_Param = (Thread_Param *)pParamStruct;
    UINT32              nThreadIdx = pThread_Param->nThreadIdx;
    
    if(NULL == pThread_Param->pFilter)
        return;

    while (1)
    {
        SEMA_WAIT(&(pThread_Param->pFilter->nStartSema[nThreadIdx]));
        
        {
            Filter_T                *pFilter = pThread_Param->pFilter;
            const UINT8             *pSrc0 = pThread_Param->pSrc0;
            const UINT8             *pSrc1 = pThread_Param->pSrc1;
            UINT8                   *pDst = pThread_Param->pDst;
            const INT32             nWidth = pThread_Param->nWidth;
            const INT32             nHeight = pThread_Param->nHeight;
            const INT32             nSrcStride = pThread_Param->nSrcStride;
            const INT32             nDstStride = pThread_Param->nDstStride;
            
            if(IMGPROC_FALSE == pThread_Param->pFilter->bIsMultiThreadEnabled)
                break;

            if(NULL != pThread_Param->pFilterFunc)
                pThread_Param->pFilterFunc(pThread_Param);
        }
    };
    
    return;
}
#endif
