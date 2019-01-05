


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


/*----------------------------------------------------------------------------------------
 Macro Definitions
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Type Definitions
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Static Function
 ----------------------------------------------------------------------------------------*/
static Kakao_Status _Hist_GetHistBuf(IN OUT Img_T *pSrcImg, IN const INT32 nDimension, IN const UINT32 nPlaneIdx, OUT F32 **pBuf);
static Kakao_Status _Hist_Cal1DHist(IN OUT Img_T *pSrcImg, OUT F32 *pHist, IN const UINT32 nBins, IN const UINT32 nPlaneIdx);
static Kakao_Status _Hist_Cal1DNormHist(IN OUT Img_T *pSrcImg, OUT F32 *pHist, IN const UINT32 nBins, IN const UINT32 nPlaneIdx);
static Kakao_Status _Hist_Cal2DHist(IN OUT Img_T *pSrcImg, OUT F32 *pHist, IN const UINT32 nBins);
static Kakao_Status _Hist_Cal2DNormHist(IN OUT Img_T *pSrcImg, OUT F32 *pHist, IN const UINT32 nBins);
static Kakao_Status _Hist_Comp1DHist_Correlation(IN const F32 *pHist0, IN const F32 *pHist1, IN const UINT32 nBins, OUT F32 *pHIVal);
static Kakao_Status _Hist_Comp1DHist_Intersection(IN const F32 *pHist0, IN const F32 *pHist1, IN const UINT32 nBins, OUT F32 *pHIVal);
static Kakao_Status _Hist_Comp1DHist_ChiSqr(IN const F32 *pHist0, IN const F32 *pHist1, IN const UINT32 nBins, OUT F32 *pHIVal);
static Kakao_Status _Hist_Comp1DHist_Bhattacharyya(IN const F32 *pHist0, IN const F32 *pHist1, IN const UINT32 nBins, OUT F32 *pHIVal);
static Kakao_Status _Hist_Comp2DHist_Correlation(IN const F32 *pHist0, IN const F32 *pHist1, IN const UINT32 nBins, OUT F32 *pHIVal);
static Kakao_Status _Hist_Comp2DHist_Intersection(IN const F32 *pHist0, IN const F32 *pHist1, IN const UINT32 nBins, OUT F32 *pHIVal);
static Kakao_Status _Hist_Comp2DHist_ChiSqr(IN const F32 *pHist0, IN const F32 *pHist1, IN const UINT32 nBins, OUT F32 *pHIVal);
static Kakao_Status _Hist_Comp2DHist_Bhattacharyya(IN const F32 *pHist0, IN const F32 *pHist1, IN const UINT32 nBins, OUT F32 *pHIVal);
static Kakao_Status _Hist_Cal1DHistBP(IN OUT Img_T *pSrcImg, IN const F32 *pRefHist, IN const UINT32 nBins, IN const UINT32 nPlaneIdx);
static Kakao_Status _Hist_Cal2DHistBP(IN OUT Img_T *pSrcImg, IN const F32 *pRefHist, IN const UINT32 nBins);
static Kakao_Status _Hist_Cal1DRatioHistBP(IN OUT Img_T *pSrcImg, IN const F32 *pRefHist, IN const UINT32 nBins, IN const UINT32 nPlaneIdx);
static Kakao_Status _Hist_Cal2DRatioHistBP(IN OUT Img_T *pSrcImg, IN const F32 *pRefHist, IN const UINT32 nBins);


/*----------------------------------------------------------------------------------------
 Static Variable
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Global Variable
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Function Implementation
 ----------------------------------------------------------------------------------------*/
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Hist_Cal1DHist(IN OUT Img_T *pSrcImg, OUT F32 *pHist, IN const UINT32 nBins, IN const UINT32 nPlaneIdx, IN const IMGPROC_BOOL bNormalized)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   nLocalBins = 0;
    
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER)
    
    if((0 <= nBins) && (256 >= nBins))
        pSrcImg->SetImgBins(pSrcImg, nBins);
    else
        pSrcImg->SetImgBins(pSrcImg, 256);
    
    nLocalBins = pSrcImg->GetImgBins(pSrcImg);
    
    if(KAKAO_STAT_OK != (nRet = _Hist_GetHistBuf(pSrcImg, 1, nPlaneIdx, &pHist)))
        goto Error;
    
    if(IMGPROC_TRUE == bNormalized)
    {
        if(KAKAO_STAT_OK != (nRet = _Hist_Cal1DNormHist(pSrcImg, pHist, nLocalBins, nPlaneIdx)))
            goto Error;
    }
    else
    {
        if(KAKAO_STAT_OK != (nRet = _Hist_Cal1DHist(pSrcImg, pHist, nLocalBins, nPlaneIdx)))
            goto Error;
    }
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Hist_Cal2DHist(IN OUT Img_T *pSrcImg, OUT F32 *pHist, IN const UINT32 nBins, IN const IMGPROC_BOOL bNormalized)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    const INT32             nArraySize = 65536;             // 256 * 256
    INT32                   nLocalBins = 0;
    
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER)
    
    if((0 <= nBins) && (256 >= nBins))
        pSrcImg->SetImgBins(pSrcImg, nBins);
    else
        pSrcImg->SetImgBins(pSrcImg, 256);
    
    nLocalBins = pSrcImg->GetImgBins(pSrcImg);
    
    if(KAKAO_STAT_OK != (nRet = _Hist_GetHistBuf(pSrcImg, 2, 0, &pHist)))
        goto Error;
    
    if(IMGPROC_TRUE == bNormalized)
    {
        if(KAKAO_STAT_OK != (nRet = _Hist_Cal2DNormHist(pSrcImg, pHist, nLocalBins)))
            goto Error;
    }
    else
    {
        if(KAKAO_STAT_OK != (nRet = _Hist_Cal2DHist(pSrcImg, pHist, nLocalBins)))
            goto Error;
    }
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Hist_Comp1DHist(IN Img_T *pSrcImg0, IN Img_T *pSrcImg1, IN const UINT32 nBins, IN const ImgHistCompMode nHistCompMode, IN const UINT32 nPlaneIdx, OUT F32 *pHICompVal)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    F32                     *pHist0 = NULL, *pHist1 = NULL;
    INT32                   nLocalBins = 0;
    
    CHECK_POINTER_VALIDATION(pSrcImg0, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pSrcImg1, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pHICompVal, KAKAO_STAT_NULL_POINTER)
    
    if((0 <= nBins) && (256 >= nBins))
    {
        pSrcImg0->SetImgBins(pSrcImg0, nBins);
        pSrcImg1->SetImgBins(pSrcImg1, nBins);
        nLocalBins = nBins;
    }
    else
    {
        pSrcImg0->SetImgBins(pSrcImg0, 256);
        pSrcImg1->SetImgBins(pSrcImg1, 256);
        nLocalBins = 256;
    }
    
    if(KAKAO_STAT_OK != (nRet = _Hist_GetHistBuf(pSrcImg0, 1, nPlaneIdx, &pHist0)))
        goto Error;
    
    if(KAKAO_STAT_OK != (nRet = _Hist_Cal1DHist(pSrcImg0, pHist0, nLocalBins, nPlaneIdx)))
        goto Error;
    
    if(KAKAO_STAT_OK != (nRet = _Hist_GetHistBuf(pSrcImg1, 1, nPlaneIdx, &pHist1)))
        goto Error;
    
    if(KAKAO_STAT_OK != (nRet = _Hist_Cal1DHist(pSrcImg1, pHist1, nLocalBins, nPlaneIdx)))
        goto Error;
    
    switch(nHistCompMode)
    {
        case IMG_HISTCOMP_CORREL:
            if(KAKAO_STAT_OK != (nRet = _Hist_Comp1DHist_Correlation(pHist0, pHist1, nLocalBins, pHICompVal)))
                goto Error;
            break;
        case IMG_HISTCOMP_INTERSECTION:
            if(KAKAO_STAT_OK != (nRet = _Hist_Comp1DHist_Intersection(pHist0, pHist1, nLocalBins, pHICompVal)))
                goto Error;
            break;
        case IMG_HISTCOMP_CHISQR:
            if(KAKAO_STAT_OK != (nRet = _Hist_Comp1DHist_ChiSqr(pHist0, pHist1, nLocalBins, pHICompVal)))
                goto Error;
            break;
        case IMG_HISTCOMP_BHATTACHARYYA:
            if(KAKAO_STAT_OK != (nRet = _Hist_Comp1DHist_Bhattacharyya(pHist0, pHist1, nLocalBins, pHICompVal)))
                goto Error;
            break;
        default:
            break;
    }
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Hist_Comp2DHist(IN Img_T *pSrcImg0, IN Img_T *pSrcImg1, IN const UINT32 nBins, IN const ImgHistCompMode nHistCompMode, OUT F32 *pHICompVal)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    F32                     *pHist0 = NULL, *pHist1 = NULL;
    INT32                   nLocalBins = 0;
    
    CHECK_POINTER_VALIDATION(pSrcImg0, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pSrcImg1, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pHICompVal, KAKAO_STAT_NULL_POINTER)
    
    if((0 <= nBins) && (256 >= nBins))
    {
        pSrcImg0->SetImgBins(pSrcImg0, nBins);
        pSrcImg1->SetImgBins(pSrcImg1, nBins);
        nLocalBins = nBins;
    }
    else
    {
        pSrcImg0->SetImgBins(pSrcImg0, 256);
        pSrcImg1->SetImgBins(pSrcImg1, 256);
        nLocalBins = 256;
    }
    
    if(KAKAO_STAT_OK != (nRet = _Hist_GetHistBuf(pSrcImg0, 2, 0, &pHist0)))
        goto Error;
    
    if(KAKAO_STAT_OK != (nRet = _Hist_Cal2DHist(pSrcImg0, pHist0, nLocalBins)))
        goto Error;
    
    if(KAKAO_STAT_OK != (nRet = _Hist_GetHistBuf(pSrcImg1, 2, 0, &pHist1)))
        goto Error;
    
    if(KAKAO_STAT_OK != (nRet = _Hist_Cal2DHist(pSrcImg1, pHist1, nLocalBins)))
        goto Error;
    
    switch(nHistCompMode)
    {
        case IMG_HISTCOMP_CORREL:
            if(KAKAO_STAT_OK != (nRet = _Hist_Comp2DHist_Correlation(pHist0, pHist1, nLocalBins, pHICompVal)))
                goto Error;
            break;
        case IMG_HISTCOMP_INTERSECTION:
            if(KAKAO_STAT_OK != (nRet = _Hist_Comp2DHist_Intersection(pHist0, pHist1, nLocalBins, pHICompVal)))
                goto Error;
            break;
        case IMG_HISTCOMP_CHISQR:
            if(KAKAO_STAT_OK != (nRet = _Hist_Comp2DHist_ChiSqr(pHist0, pHist1, nLocalBins, pHICompVal)))
                goto Error;
            break;
        case IMG_HISTCOMP_BHATTACHARYYA:
            if(KAKAO_STAT_OK != (nRet = _Hist_Comp2DHist_Bhattacharyya(pHist0, pHist1, nLocalBins, pHICompVal)))
                goto Error;
            break;
        default:
            break;
    }
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Hist_Cal1DHistBP(IN OUT Img_T *pSrcImg, IN const F32 *pRefHist, IN const UINT32 nBins, IN const UINT32 nPlaneIdx, IN const ImgBPMode nHistBPMode)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    switch(nHistBPMode)
    {
        case IMG_BP_NORMAL:
            if(KAKAO_STAT_OK != (nRet = _Hist_Cal1DHistBP(pSrcImg, pRefHist, nBins, nPlaneIdx)))
                goto Error;
            break;
        case IMG_BP_RATIO:
            if(KAKAO_STAT_OK != (nRet = _Hist_Cal1DRatioHistBP(pSrcImg, pRefHist, nBins, nPlaneIdx)))
                goto Error;
            break;
        default:
            break;
    }
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Hist_Cal2DHistBP(IN OUT Img_T *pSrcImg, IN const F32 *pRefHist, IN const UINT32 nBins, IN const ImgBPMode nHistBPMode)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    switch(nHistBPMode)
    {
        case IMG_BP_NORMAL:
            if(KAKAO_STAT_OK != (nRet = _Hist_Cal2DHistBP(pSrcImg, pRefHist, nBins)))
                goto Error;
            break;
        case IMG_BP_RATIO:
            if(KAKAO_STAT_OK != (nRet = _Hist_Cal2DRatioHistBP(pSrcImg, pRefHist, nBins)))
                goto Error;
            break;
        default:
            break;
    }
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Hist_GetHistMap(IN Img_T *pSrcImg, IN const INT32 nDimension, IN const INT32 nPlaneIdx, OUT F32 **ppHistMapMap)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER)
    CHECK_PARAM_VALIDATION((nDimension <= IMG_PLANE_2), KAKAO_STAT_INVALID_PARAM)
    
    if(KAKAO_STAT_OK != (nRet = _Hist_GetHistBuf(pSrcImg, nDimension, nPlaneIdx, ppHistMapMap)))
        goto Error;
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _Hist_GetHistBuf(IN OUT Img_T *pSrcImg, IN const INT32 nDimension, IN const UINT32 nPlaneIdx, OUT F32 **pBuf)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    BaseImg_T               *pBaseImg = (BaseImg_T *)pSrcImg;
    
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER)
    
    if(1 == nDimension)
    {
        CHECK_PARAM_VALIDATION((nPlaneIdx < 4), KAKAO_STAT_INVALID_PARAM);
        
        if(NULL == (*pBuf))
        {
            SAFEALLOC(pBaseImg->p1DHist[nPlaneIdx], 256, 32, F32);
            *pBuf = pBaseImg->p1DHist[nPlaneIdx];
        }
        
        SAFEALLOC(pBaseImg->pTmp1DHist, 256, 32, INT32);
    }
    else if(2 == nDimension)
    {
        if(NULL == (*pBuf))
        {
            SAFEALLOC(pBaseImg->p2DHist, 65536, 32, F32);
            *pBuf = pBaseImg->p2DHist;
        }
        
        SAFEALLOC(pBaseImg->pTmp2DHist, 65536, 32, INT32);
    }
    
    if(NULL == (*pBuf))
        goto Error;
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _Hist_Cal1DHist(IN OUT Img_T *pSrcImg, OUT F32 *pHist, IN const UINT32 nBins, IN const UINT32 nPlaneIdx)
{
    const UINT8         *pSrc = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, nPlaneIdx);
    const INT32         nWidth = pSrcImg->GetImgWidth(pSrcImg, nPlaneIdx);
    const INT32         nHeight = pSrcImg->GetImgHeight(pSrcImg, nPlaneIdx);
    const INT32         nStride = pSrcImg->GetImgStride(pSrcImg, nPlaneIdx);
    const INT32         nBase = (256 / nBins);
    INT32               *pTmpHist = ((BaseImg_T *)pSrcImg)->pTmp1DHist;
    INT32               i = 0, j = 0;
    Kakao_Status        nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pSrc, KAKAO_STAT_NULL_POINTER);
    
    MEMSET(pTmpHist, 0, 256 * sizeof(INT32));
    
    // Calculate Histogram
    for(j=0 ; j<nHeight ; j++)
    {
        const UINT8               *pLocalSrc = pSrc + (j * nStride);
        
        for(i=0 ; i<nWidth ; i++)
            pTmpHist[pLocalSrc[i]]++;
    }
    
    // Divide Histogram by Target Bins
    for(i=0 ; i<nBins ; i++)
    {
        const INT32     nStartIdx = i * nBase;
        const INT32     nEndIdx = (i + 1) * nBase;
        INT32           nHistoBinSum = 0;
        
        for(j=nStartIdx ; j<nEndIdx ; j++)
            nHistoBinSum += pTmpHist[j];
        
        for(j=nStartIdx ; j<nEndIdx ; j++)
            pHist[j] = (F32)nHistoBinSum;
    }
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _Hist_Cal1DNormHist(IN OUT Img_T *pSrcImg, OUT F32 *pHist, IN const UINT32 nBins, IN const UINT32 nPlaneIdx)
{
    const UINT8         *pSrc = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, nPlaneIdx);
    const INT32         nWidth = pSrcImg->GetImgWidth(pSrcImg, nPlaneIdx);
    const INT32         nHeight = pSrcImg->GetImgHeight(pSrcImg, nPlaneIdx);
    const INT32         nStride = pSrcImg->GetImgStride(pSrcImg, nPlaneIdx);
    const INT32         nBase = (256 / nBins);
    INT32               *pTmpHist = ((BaseImg_T *)pSrcImg)->pTmp1DHist;
    INT32               i = 0, j = 0;
    INT32               nMin = 0x1FFFFFFF, nMax = 0;
    INT32               nGrayRegionSum = 0;
    Kakao_Status        nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pSrc, KAKAO_STAT_NULL_POINTER);
    
    MEMSET(pTmpHist, 0, 256 * sizeof(INT32));
    
    // Calculate Histogram
    for(j=0 ; j<nHeight ; j++)
    {
        const UINT8               *pLocalSrc = pSrc + (j * nStride);
        
        for(i=0 ; i<nWidth ; i++)
            pTmpHist[pLocalSrc[i]]++;
    }
    
    // Remove Gray Color Region
    if(0 != nPlaneIdx)
    {
        for(j=103 ; j<154 ; j++)
        {
            const INT32             nOffset = 256 * j;
            
            for(i=103 ; i<154 ; i++)
            {
                nGrayRegionSum += pTmpHist[nOffset + i];
                pTmpHist[nOffset + i] = 0;
            }
        }
    }
    
    // Divide Histogram by Target Bins
    for(i=0 ; i<nBins ; i++)
    {
        const INT32     nStartIdx = i * nBase;
        const INT32     nEndIdx = (i + 1) * nBase;
        INT32           nHistoBinSum = 0;
        
        for(j=nStartIdx ; j<nEndIdx ; j++)
            nHistoBinSum += pTmpHist[j];
        
        for(j=nStartIdx ; j<nEndIdx ; j++)
            pHist[j] = (F32)nHistoBinSum;
        
        // Find Min & Max Histogram Bins for Normalize
        if(nMin > nHistoBinSum)
            nMin = nHistoBinSum;
        
        if(nMax < nHistoBinSum)
            nMax = nHistoBinSum;
    }
    
    // Normalize Histogram
    {
        //const F32           nNormalizingScaleFactor = 255.0f / (F32)(nMax - nMin);
        const F32           nNormalizingScaleFactor = 255.0f * (F32)((nWidth * nHeight) - nGrayRegionSum);
        
        for(i=0 ; i<256 ; i++)
            pHist[i] *= nNormalizingScaleFactor;
    }
    
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _Hist_Cal2DHist(IN OUT Img_T *pSrcImg, OUT F32 *pHist, IN const UINT32 nBins)
{
    const UINT8         *pSrcU = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_1);
    const UINT8         *pSrcV = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_2);
    const INT32         nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_1);
    const INT32         nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_1);
    const INT32         nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_1);
    const INT32         nBase = (256 / nBins);
    const INT32         nArraySize = 65536;             // 256 * 256
    INT32               *pTmpHist = ((BaseImg_T *)pSrcImg)->pTmp2DHist;
    INT32               i = 0, j = 0, s = 0, t = 0;
    Kakao_Status        nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pSrcU, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcV, KAKAO_STAT_NULL_POINTER);
    
    MEMSET(pTmpHist, 0, nArraySize * sizeof(INT32));
    
    // Calculate Histogram
    for(j=0 ; j<nHeight ; j++)
    {
        const UINT8               *pLocalSrcU = pSrcU + (j * nStride);
        const UINT8               *pLocalSrcV = pSrcV + (j * nStride);
        
        for(i=0 ; i<nWidth ; i++)
            pTmpHist[((INT32)(pLocalSrcV[i]) * 256) + pLocalSrcU[i]]++;
    }
    
    // Remove Gray Color Region
    for(j=103 ; j<154 ; j++)
    {
        INT32           *pLocalTmpHist = pTmpHist + (256 * j);
        
        for(i=103 ; i<154 ; i++)
            pLocalTmpHist[i] = 0;
    }
    
    // Divide Histogram by Target Bins
    for(j=0 ; j<nBins ; j++)
    {
        for(i=0 ; i<nBins ; i++)
        {
            const INT32 nStartXIdx = i * nBase;
            const INT32 nEndXIdx = (i + 1) * nBase;
            const INT32 nStartYIdx = j * nBase;
            const INT32 nEndYIdx = (j + 1) * nBase;
            INT32       nHistoBinSum = 0;
            
            for(s=nStartYIdx ; s<nEndYIdx ; s++)
            {
                const INT32     *pLocalTmpHist = &(pTmpHist[256 * s]);
                
                for(t=nStartXIdx ; t<nEndXIdx ; t++)
                    nHistoBinSum += pLocalTmpHist[t];
            }
            
            for(s=nStartYIdx ; s<nEndYIdx ; s++)
            {
                F32           *pLocalHist = &(pHist[256 * s]);
                
                for(t=nStartXIdx ; t<nEndXIdx ; t++)
                    pLocalHist[t] = (F32)nHistoBinSum;
            }
        }
    }
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _Hist_Cal2DNormHist(IN OUT Img_T *pSrcImg, OUT F32 *pHist, IN const UINT32 nBins)
{
    const UINT8         *pSrcU = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_1);
    const UINT8         *pSrcV = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_2);
    const INT32         nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_1);
    const INT32         nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_1);
    const INT32         nStrideUV = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_1);
    const INT32         nBase = (256 / nBins);
    const INT32         nArraySize = 65536;             // 256 * 256
    INT32               *pTmpHist = ((BaseImg_T *)pSrcImg)->pTmp2DHist;
    INT32               i = 0, j = 0, s = 0, t = 0;
    INT32               nMin = 0x1FFFFFFF, nMax = 0;
    INT32               nGrayRegionSum = 0;
    Kakao_Status        nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pSrcU, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcV, KAKAO_STAT_NULL_POINTER);
    
    MEMSET(pTmpHist, 0, nArraySize * sizeof(INT32));
    
    // Calculate Histogram
    for(j=0 ; j<nHeight ; j++)
    {
        const UINT8               *pLocalSrcU = pSrcU + (j * nStrideUV);
        const UINT8               *pLocalSrcV = pSrcV + (j * nStrideUV);
        
        for(i=0 ; i<nWidth ; i++)
            pTmpHist[((INT32)(pLocalSrcV[i]) * 256) + pLocalSrcU[i]]++;
    }
    
    // Remove Gray Color Region
    for(j=103 ; j<154 ; j++)
    {
        INT32           *pLocalTmpHist = pTmpHist + (256 * j);
        
        for(i=103 ; i<154 ; i++)
        {
            nGrayRegionSum += pLocalTmpHist[i];
            pLocalTmpHist[i] = 0;
        }
    }
    
    // Divide Histogram by Target Bins
    for(j=0 ; j<nBins ; j++)
    {
        for(i=0 ; i<nBins ; i++)
        {
            const INT32 nStartXIdx = i * nBase;
            const INT32 nEndXIdx = (i + 1) * nBase;
            const INT32 nStartYIdx = j * nBase;
            const INT32 nEndYIdx = (j + 1) * nBase;
            INT32       nHistoBinSum = 0;
            
            for(s=nStartYIdx ; s<nEndYIdx ; s++)
            {
                const INT32     *pLocalTmpHist = &(pTmpHist[256 * s]);
                
                for(t=nStartXIdx ; t<nEndXIdx ; t++)
                    nHistoBinSum += pLocalTmpHist[t];
            }
            
            for(s=nStartYIdx ; s<nEndYIdx ; s++)
            {
                F32             *pLocalHist = &(pHist[256 * s]);
                
                for(t=nStartXIdx ; t<nEndXIdx ; t++)
                    pLocalHist[t] = (F32)nHistoBinSum;
            }
            
            // Find Min & Max Histogram Bins for Normalize
            if(nMin > nHistoBinSum)
                nMin = nHistoBinSum;
            
            if(nMax < nHistoBinSum)
                nMax = nHistoBinSum;
        }
    }
    
    // Normalize Histogram
    {
        //const F32           nNormalizingScaleFactor = 255.0f / (F32)(nMax - nMin);
        const F32           nNormalizingScaleFactor = 255.0f / (F32)((nWidth * nHeight) - nGrayRegionSum);
        
        for(i=0 ; i<65536 ; i++)
            pHist[i] *= nNormalizingScaleFactor;
    }
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _Hist_Comp1DHist_Correlation(IN const F32 *pHist0, IN const F32 *pHist1, IN const UINT32 nBins, OUT F32 *pHIVal)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    const INT32             nBase = (256 / nBins);
    INT32                   i = 0;
    F32                     nHist0Sum = 0.0f, nHist1Sum = 0.0f;
    F32                     nHist00Sum = 0.0f, nHist11Sum = 0.0f, nHist01Sum = 0.0f;
    F32                     nTargetHISum = 1;               // Set One to Avoid Dividing By Zero
    
    for(i=0 ; i<nBins ; i++)
    {
        const INT32         nOffset = i * nBase;
        const F32           nTmpH0 = pHist0[nOffset];
        const F32           nTmpH1 = pHist1[nOffset];
        
        nHist0Sum += nTmpH0;
        nHist1Sum += nTmpH1;
        nHist00Sum += nTmpH0 * nTmpH0;
        nHist11Sum += nTmpH1 * nTmpH1;
        nHist01Sum += nTmpH0 * nTmpH1;
    }
    
    *pHIVal = (nHist01Sum - nHist0Sum * nHist1Sum / (F32)nBins) /
    SQRT((nHist00Sum - nHist0Sum * nHist0Sum / (F32)nBins) * (nHist11Sum - nHist1Sum * nHist1Sum / (F32)nBins));
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _Hist_Comp1DHist_Intersection(IN const F32 *pHist0, IN const F32 *pHist1, IN const UINT32 nBins, OUT F32 *pHIVal)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    const INT32             nBase = (256 / nBins);
    INT32                   i = 0;
    F32                     nHistSum = 0;
    F32                     nTargetHistSum = 1;               // Set One to Avoid Dividing By Zero
    
    for(i=0 ; i<nBins ; i++)
    {
        const INT32         nOffset = i * nBase;
        
        nHistSum += GET_MIN(pHist0[nOffset], pHist1[nOffset]);
        nTargetHistSum += pHist1[nOffset];
    }
    
    *pHIVal = nHistSum / nTargetHistSum;
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _Hist_Comp1DHist_ChiSqr(IN const F32 *pHist0, IN const F32 *pHist1, IN const UINT32 nBins, OUT F32 *pHIVal)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    const INT32             nBase = (256 / nBins);
    INT32                   i = 0;
    
    *pHIVal = 0.0f;
    
    for(i=0 ; i<nBins ; i++)
    {
        const INT32         nOffset = i * nBase;
        const F32           nTmpH0 = pHist0[nOffset];
        const F32           nTmpH1 = nTmpH0 - pHist1[nOffset];
        
        *pHIVal += nTmpH1 * nTmpH1 / nTmpH0;
    }
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _Hist_Comp1DHist_Bhattacharyya(IN const F32 *pHist0, IN const F32 *pHist1, IN const UINT32 nBins, OUT F32 *pHIVal)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    const INT32             nBase = (256 / nBins);
    INT32                   i = 0;
    F32                     nHist0Sum = 0.0f, nHist1Sum = 0.0f;
    F32                     nHistSqrtSum = 0.0f;
    
    for(i=0 ; i<nBins ; i++)
    {
        const INT32         nOffset = i * nBase;
        const F32           nTmpH0 = pHist0[nOffset];
        const F32           nTmpH1 = nTmpH0 - pHist1[nOffset];
        
        nHistSqrtSum += SQRT(nTmpH0 * nTmpH1);
        nHist0Sum += nTmpH0;
        nHist1Sum += nTmpH1;
    }
    
    *pHIVal = GET_MAX(SQRT(1.0f - ((1.0f / SQRT(nHist0Sum * nHist1Sum)) * nHistSqrtSum)), 1.0f);
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _Hist_Comp2DHist_Correlation(IN const F32 *pHist0, IN const F32 *pHist1, IN const UINT32 nBins, OUT F32 *pHIVal)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    const INT32             nBase = (256 / nBins);
    INT32                   i = 0, j = 0;
    F32                     nHist0Sum = 0.0f, nHist1Sum = 0.0f;
    F32                     nHist00Sum = 0.0f, nHist11Sum = 0.0f, nHist01Sum = 0.0f;
    F32                     nTargetHISum = 1;               // Set One to Avoid Dividing By Zero
    
    for(j=0 ; j<nBins ; j++)
    {
        const INT32         nOffsetY = j * nBase;
        
        for(i=0 ; i<nBins ; i++)
        {
            const INT32         nLocalOffset = nOffsetY + (i * nBase);
            const F32           nTmpH0 = pHist0[nLocalOffset];
            const F32           nTmpH1 = pHist1[nLocalOffset];
            
            nHist0Sum += nTmpH0;
            nHist1Sum += nTmpH1;
            nHist00Sum += nTmpH0 * nTmpH0;
            nHist11Sum += nTmpH1 * nTmpH1;
            nHist01Sum += nTmpH0 * nTmpH1;
        }
    }
    
    *pHIVal = (nHist01Sum - nHist0Sum * nHist1Sum / (F32)nBins) /
    SQRT((nHist00Sum - nHist0Sum * nHist0Sum / (F32)nBins) * (nHist11Sum - nHist1Sum * nHist1Sum / (F32)nBins));
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _Hist_Comp2DHist_Intersection(IN const F32 *pHist0, IN const F32 *pHist1, IN const UINT32 nBins, OUT F32 *pHIVal)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    const INT32             nBase = (256 / nBins);
    INT32                   i = 0, j = 0;
    F32                     nHistSum = 0;
    F32                     nTargetHistSum = 1;               // Set One to Avoid Dividing By Zero
    
    for(j=0 ; j<nBins ; j++)
    {
        const INT32         nOffsetY = j * nBase;
        
        for(i=0 ; i<nBins ; i++)
        {
            const INT32         nLocalOffset = nOffsetY + (i * nBase);
            
            nHistSum += GET_MIN(pHist0[nLocalOffset], pHist1[nLocalOffset]);
            nTargetHistSum += pHist1[nLocalOffset];
        }
    }
    
    *pHIVal = nHistSum / nTargetHistSum;
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _Hist_Comp2DHist_ChiSqr(IN const F32 *pHist0, IN const F32 *pHist1, IN const UINT32 nBins, OUT F32 *pHIVal)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    const INT32             nBase = (256 / nBins);
    INT32                   i = 0, j = 0;
    
    *pHIVal = 0.0f;
    
    for(j=0 ; j<nBins ; j++)
    {
        const INT32         nOffsetY = j * nBase;
        
        for(i=0 ; i<nBins ; i++)
        {
            const INT32         nLocalOffset = nOffsetY + (i * nBase);
            const F32           nTmpH0 = pHist0[nLocalOffset];
            const F32           nTmpH1 = pHist1[nLocalOffset];
            
            *pHIVal += nTmpH1 * nTmpH1 / nTmpH0;
        }
    }
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _Hist_Comp2DHist_Bhattacharyya(IN const F32 *pHist0, IN const F32 *pHist1, IN const UINT32 nBins, OUT F32 *pHIVal)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    const INT32             nBase = (256 / nBins);
    INT32                   i = 0, j = 0;
    F32                     nHist0Sum = 0.0f, nHist1Sum = 0.0f;
    F32                     nHistSqrtSum = 0.0f;
    
    for(j=0 ; j<nBins ; j++)
    {
        const INT32         nOffsetY = j * nBase;
        
        for(i=0 ; i<nBins ; i++)
        {
            const INT32         nLocalOffset = nOffsetY + (i * nBase);
            const F32           nTmpH0 = pHist0[nLocalOffset];
            const F32           nTmpH1 = nTmpH0 - pHist1[nLocalOffset];
            
            nHistSqrtSum += SQRT(nTmpH0 * nTmpH1);
            nHist0Sum += nTmpH0;
            nHist1Sum += nTmpH1;
        }
    }
    
    *pHIVal = GET_MAX(SQRT(1.0f - ((1.0f / SQRT(nHist0Sum * nHist1Sum)) * nHistSqrtSum)), 1.0f);
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _Hist_Cal1DHistBP(IN OUT Img_T *pSrcImg, IN const F32 *pRefHist, IN const UINT32 nBins, IN const UINT32 nPlaneIdx)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   i = 0, j = 0;
    
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pRefHist, KAKAO_STAT_NULL_POINTER)
    
    if((0 <= nBins) && (256 >= nBins))
        pSrcImg->SetImgBins(pSrcImg, nBins);
    else
        pSrcImg->SetImgBins(pSrcImg, 256);
    
    {
        const INT32         nWidth = pSrcImg->GetImgWidth(pSrcImg, nPlaneIdx);
        const INT32         nHeight = pSrcImg->GetImgHeight(pSrcImg, nPlaneIdx);
        const INT32         nStride = pSrcImg->GetImgStride(pSrcImg, nPlaneIdx);
        const UINT8         *pSrc = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, nPlaneIdx);
        UINT8               *pHistBPBinaryMap = NULL;
        
        // Get BinaryMap for BackProjection
        if(NULL == (pHistBPBinaryMap = pSrcImg->GetImgBinaryMap(pSrcImg, IMG_BINARYMAP_HISTO_BP)))
            return nRet;
        
        // Masking Source Image by BackProjection Probability
        if(!nPlaneIdx)
        {
            for(j=0 ; j<nHeight ; j++)
            {
                const INT32     nOffset = j * nStride;
                const UINT8     *pLocalSrc = pSrc + nOffset;
                UINT8           *pLocalHistBPBinaryMap = pHistBPBinaryMap + nOffset;
                
                for(i=0 ; i<nWidth ; i++)
                    pLocalHistBPBinaryMap[i] = (UINT8)(pRefHist[pLocalSrc[i]]);
            }
        }
        else
        {
            const INT32     nStrideY = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
            
            for(j=0 ; j<nHeight ; j++)
            {
                const INT32     nOffsetY = (2 * j) * nStrideY;
                const INT32     nOffsetUV = j * nStride;
                const UINT8     *pLocalSrc = pSrc + nOffsetUV;
                UINT8           *pLocalHistBPBinaryMap0 = pHistBPBinaryMap + nOffsetY;
                UINT8           *pLocalHistBPBinaryMap1 = pLocalHistBPBinaryMap0 + 1;
                UINT8           *pLocalHistBPBinaryMap2 = pLocalHistBPBinaryMap0 + nStrideY;
                UINT8           *pLocalHistBPBinaryMap3 = pLocalHistBPBinaryMap2 + 1;
                
                for(i=0 ; i<nWidth ; i++)
                {
                    const INT32 nLocalXOffset = 2 * i;
                    UINT8       nMaskVal = (UINT8)(pRefHist[pLocalSrc[i]]);
                    
                    pLocalHistBPBinaryMap0[nLocalXOffset] = nMaskVal;
                    pLocalHistBPBinaryMap1[nLocalXOffset] = nMaskVal;
                    pLocalHistBPBinaryMap2[nLocalXOffset] = nMaskVal;
                    pLocalHistBPBinaryMap3[nLocalXOffset] = nMaskVal;
                }
            }
        }
    }
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _Hist_Cal2DHistBP(IN OUT Img_T *pSrcImg, IN const F32 *pRefHist, IN const UINT32 nBins)
{
    Kakao_Status                nRet = KAKAO_STAT_FAIL;
    const INT32                 nArraySize = 65536;             // 256 * 256
    UINT8                       *pHistBPBinaryMap = NULL;
    INT32                       i = 0, j = 0;
    
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pRefHist, KAKAO_STAT_NULL_POINTER)
    
    if((0 <= nBins) && (256 >= nBins))
        pSrcImg->SetImgBins(pSrcImg, nBins);
    else
        pSrcImg->SetImgBins(pSrcImg, 256);
    
    // Get BinaryMap for BackProjection
    if(NULL == (pHistBPBinaryMap = pSrcImg->GetImgBinaryMap(pSrcImg, IMG_BINARYMAP_HISTO_BP)))
        return nRet;
    
    // Masking Source Image by BackProjection Probability
    {
        const INT32         nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_1);
        const INT32         nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_1);
        const INT32         nStrideY = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
        const INT32         nStrideUV = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_1);
        const UINT8         *pSrcU = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_1);
        const UINT8         *pSrcV = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_2);
        
        for(j=0 ; j<nHeight ; j++)
        {
            const INT32     nOffsetY = (2 * j) * nStrideY;
            const INT32     nOffsetUV = j * nStrideUV;
            const UINT8     *pLocalSrcU = pSrcU + nOffsetUV;
            const UINT8     *pLocalSrcV = pSrcV + nOffsetUV;
            UINT8           *pLocalHistBPBinaryMap0 = pHistBPBinaryMap + nOffsetY;
            UINT8           *pLocalHistBPBinaryMap1 = pLocalHistBPBinaryMap0 + 1;
            UINT8           *pLocalHistBPBinaryMap2 = pLocalHistBPBinaryMap0 + nStrideY;
            UINT8           *pLocalHistBPBinaryMap3 = pLocalHistBPBinaryMap2 + 1;
            
            for(i=0 ; i<nWidth ; i++)
            {
                const INT32 nLocalXOffset = 2 * i;
                UINT8       nMaskVal = (UINT8)(pRefHist[256 * pLocalSrcV[i] + pLocalSrcU[i]]);
                
                pLocalHistBPBinaryMap0[nLocalXOffset] = nMaskVal;
                pLocalHistBPBinaryMap1[nLocalXOffset] = nMaskVal;
                pLocalHistBPBinaryMap2[nLocalXOffset] = nMaskVal;
                pLocalHistBPBinaryMap3[nLocalXOffset] = nMaskVal;
            }
        }
    }
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _Hist_Cal1DRatioHistBP(IN OUT Img_T *pSrcImg, IN const F32 *pRefHist, IN const UINT32 nBins, IN const UINT32 nPlaneIdx)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    F32                     *pTargetHist = ((BaseImg_T *)pSrcImg)->p1DHist[nPlaneIdx];
    INT32                   i = 0, j = 0;
    
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pRefHist, KAKAO_STAT_NULL_POINTER)
    
    if((0 <= nBins) && (256 >= nBins))
        pSrcImg->SetImgBins(pSrcImg, nBins);
    else
        pSrcImg->SetImgBins(pSrcImg, 256);
    
    if(KAKAO_STAT_OK != (nRet = _Hist_GetHistBuf(pSrcImg, 1, nPlaneIdx, &pTargetHist)))
        goto Error;
    
    if(KAKAO_STAT_OK != (nRet = _Hist_Cal1DNormHist(pSrcImg, pTargetHist, pSrcImg->GetImgBins(pSrcImg), nPlaneIdx)))
        goto Error;
    
    {
        const INT32         nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
        const INT32         nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
        const INT32         nStride = pSrcImg->GetImgStride(pSrcImg, nPlaneIdx);
        const UINT8         *pSrc = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, nPlaneIdx);
        UINT8               *p1DHistBPTable = NULL;
        UINT8               *pHistBPBinaryMap = NULL;
        
        SAFEALLOC(((BaseImg_T *)pSrcImg)->p1DHistBPTable, 256, 32, UINT8);
        p1DHistBPTable = ((BaseImg_T *)pSrcImg)->p1DHistBPTable;
        
        // Calculate Histogram BackProjection Probability Table
        for(i=0 ; i<256 ; i++)
        {
            if(0 != pTargetHist[i])
                p1DHistBPTable[i] = (UINT8)(255.0f * CLIP3(0, 1.0f, SQRT(pRefHist[i] / pTargetHist[i])));
            else
                p1DHistBPTable[i] = 0;
        }
        
        // Get BinaryMap for BackProjection
        if(NULL == (pHistBPBinaryMap = pSrcImg->GetImgBinaryMap(pSrcImg, IMG_BINARYMAP_HISTO_BP)))
            return nRet;
        
        // Masking Source Image by BackProjection Probability
        for(j=0 ; j<nHeight ; j++)
        {
            const INT32     nOffset = j * nStride;
            const UINT8     *pLocalSrc = pSrc + nOffset;
            UINT8           *pLocalHistBPBinaryMap = pHistBPBinaryMap + nOffset;
            
            for(i=0 ; i<nWidth ; i++)
                pLocalHistBPBinaryMap[i] = p1DHistBPTable[pLocalSrc[i]];
        }
    }
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _Hist_Cal2DRatioHistBP(IN OUT Img_T *pSrcImg, IN const F32 *pRefHist, IN const UINT32 nBins)
{
    Kakao_Status                nRet = KAKAO_STAT_FAIL;
    const INT32                 nArraySize = 65536;             // 256 * 256
    F32                         *pTargetHist = ((BaseImg_T *)pSrcImg)->p2DHist;
    INT32                       i = 0, j = 0;
    
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pRefHist, KAKAO_STAT_NULL_POINTER)
    
    if((0 <= nBins) && (256 >= nBins))
        pSrcImg->SetImgBins(pSrcImg, nBins);
    else
        pSrcImg->SetImgBins(pSrcImg, 256);
    
    if(KAKAO_STAT_OK != (nRet = _Hist_GetHistBuf(pSrcImg, 2, 0, &pTargetHist)))
        goto Error;
    
    if(KAKAO_STAT_OK != (nRet =_Hist_Cal2DNormHist(pSrcImg, pTargetHist, pSrcImg->GetImgBins(pSrcImg))))
        goto Error;
    
    {
        UINT8                   *p2DHistBPTable = NULL;
        UINT8                   *pHistBPBinaryMap = NULL;
        
        SAFEALLOC(((BaseImg_T *)pSrcImg)->p2DHistBPTable, nArraySize, 32, UINT8);
        p2DHistBPTable = ((BaseImg_T *)pSrcImg)->p2DHistBPTable;
        
        // Calculate Histogram BackProjection Probability Table
        for(i=0 ; i<nArraySize ; i++)
        {
            if(0 != pTargetHist[i])
                p2DHistBPTable[i] = (UINT8)(255.0f * CLIP3(0, 1.0f, SQRT(pRefHist[i] / pTargetHist[i])));
            else
                p2DHistBPTable[i] = 0;
        }
        
        // Get BinaryMap for BackProjection
        if(NULL == (pHistBPBinaryMap = pSrcImg->GetImgBinaryMap(pSrcImg, IMG_BINARYMAP_HISTO_BP)))
            return nRet;
        
        // Masking Source Image by BackProjection Probability
        {
            const INT32         nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0) / 2;
            const INT32         nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0) / 2;
            const INT32         nStrideY = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
            const INT32         nStrideUV = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_1);
            const UINT8         *pSrcU = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_1);
            const UINT8         *pSrcV = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_2);
            
            for(j=0 ; j<nHeight ; j++)
            {
                const INT32     nOffsetY = (2 * j) * nStrideY;
                const INT32     nOffsetUV = j * nStrideUV;
                const UINT8     *pLocalSrcU = pSrcU + nOffsetUV;
                const UINT8     *pLocalSrcV = pSrcV + nOffsetUV;
                UINT8           *pLocalHistBPBinaryMap0 = pHistBPBinaryMap + nOffsetY;
                UINT8           *pLocalHistBPBinaryMap1 = pLocalHistBPBinaryMap0 + 1;
                UINT8           *pLocalHistBPBinaryMap2 = pLocalHistBPBinaryMap0 + nStrideY;
                UINT8           *pLocalHistBPBinaryMap3 = pLocalHistBPBinaryMap2 + 1;
                
                for(i=0 ; i<nWidth ; i++)
                {
                    const INT32 nLocalXOffset = 2 * i;
                    UINT8       nMaskVal = p2DHistBPTable[256 * pLocalSrcV[i] + pLocalSrcU[i]];
                    
                    pLocalHistBPBinaryMap0[nLocalXOffset] = nMaskVal;
                    pLocalHistBPBinaryMap1[nLocalXOffset] = nMaskVal;
                    pLocalHistBPBinaryMap2[nLocalXOffset] = nMaskVal;
                    pLocalHistBPBinaryMap3[nLocalXOffset] = nMaskVal;
                }
            }
        }
    }
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}
