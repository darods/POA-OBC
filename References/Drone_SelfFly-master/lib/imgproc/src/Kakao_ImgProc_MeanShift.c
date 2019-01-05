
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


/*----------------------------------------------------------------------------------------
 Static Function
 ----------------------------------------------------------------------------------------*/
static Kakao_Status _MeanShift_Track_CalNewCenter(IN const MeanShift_T *pMeanShift, IN const UINT8 *pTargetData, IN const INT32 nTargetStride,
                                                    IN const INT32 nWindowWidth, IN const INT32 nWindowHeight, IN const KIPointF2 nLocalCenter, OUT KIPointF2 *pDeltaCenter);
static Kakao_Status _MeanShift_Cluster_CalNewCenter(IN const MeanShift_T *pMeanShift, IN const F32 *pTargetData, IN const INT32 nTargetStride,
                                                    IN const INT32 nWindowWidth, IN const INT32 nWindowHeight, IN const KIPointF2 nLocalCenter, OUT KIPointF2 *pDeltaCenter);
static Kakao_Status _MeanShift_MakeEpanechnikovMask(IN MeanShift_T *pMeanShift);
static Kakao_Status _MeanShift_MakeColorLookUpTable(IN MeanShift_T *pMeanShift);

/*----------------------------------------------------------------------------------------
 Static Variable
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Global Variable
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Function Implementation
 ----------------------------------------------------------------------------------------*/
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_MeanShift_Create(IN OUT MeanShift_T **ppMeanShift, IN const INT32 nWidth, IN const INT32 nHeight)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    MeanShift_T             *pTmpMeanShift = NULL;
    const INT32             nScaledWidth = nWidth / 2;
    const INT32             nScaledHeight = nHeight / 2;
    INT32                   nAlignedWidth = GET_ALIGNED_LENGTH(nScaledWidth, MACRO_BLOCK_SIZE);
    INT32                   i = 0;
    
    SAFEALLOC(pTmpMeanShift, 1, 32, MeanShift_T);
    
    Kakao_ImgProc_Util_UpdateSize(&pTmpMeanShift->nBaseSize, nWidth, nHeight, NULL, IMG_FORMAT_MAX, 4);

    pTmpMeanShift->nWindowSize.nWidth[IMG_PLANE_0] = -1;
    pTmpMeanShift->nWindowSize.nHeight[IMG_PLANE_0] = -1;
    pTmpMeanShift->nSearchRegion.nWidth[IMG_PLANE_0] = -1;
    pTmpMeanShift->nSearchRegion.nHeight[IMG_PLANE_0] = -1;
    pTmpMeanShift->nCenterPtr.x = -1.0f;
    pTmpMeanShift->nCenterPtr.y = -1.0f;
    pTmpMeanShift->nHistoBinNum = 64;
    pTmpMeanShift->nScaledRatioWidth = 1.0f;
    pTmpMeanShift->nScaledRatioHeight = 1.0f;

    // Create Scaler Handler
    if(KAKAO_STAT_OK != (nRet = Kakao_ImgProc_Scaler_Create(&(pTmpMeanShift->pScalerHndl), nScaledWidth, nScaledHeight)))
        goto Error;
    
    // Create BackProjection Image
    if(KAKAO_STAT_OK != (nRet = Kakao_ImgProc_Util_CreateImg(&pTmpMeanShift->pDownScaledSrcImg, nScaledWidth, nScaledHeight, IMG_FORMAT_I420, IMG_DEPTH_U8)))
        goto Error;
    
    SAFEALLOC(pTmpMeanShift->pRefHist, (256 * 256), 32, F32);
    SAFEALLOC(pTmpMeanShift->pClusteringMap, (256 * 256), 32, KIPoint2);
    SAFEALLOC(pTmpMeanShift->pYUVtoRGBMapTable, (64 * 64 * 64), 32, Color_RGB);        // YUV to RGB Lookup Table Quantized by 4
    SAFEALLOC(pTmpMeanShift->pYUVtoHSLMapTable, (64 * 64 * 64), 32, Color_HSL);        // YUV to HSI Lookup Table Quantized by 4
    
    // Create YUV to (RGB & HSL) Converter LookUp Table
    _MeanShift_MakeColorLookUpTable(pTmpMeanShift);
    
    *ppMeanShift = pTmpMeanShift;
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_MeanShift_Destroy(IN OUT MeanShift_T **ppMeanShift)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   i = 0;
    
    // Release Patch Info
    SAFEFREE((*ppMeanShift)->pRefHist);
    SAFEFREE((*ppMeanShift)->pClusteringMap);
    SAFEFREE((*ppMeanShift)->pEpanechnikovMask);
    SAFEFREE((*ppMeanShift)->pYUVtoRGBMapTable);
    SAFEFREE((*ppMeanShift)->pYUVtoHSLMapTable);
    
    // Release DownScaled Image Buffer
    Kakao_ImgProc_Util_ReleaseImg(&((*ppMeanShift)->pDownScaledSrcImg));
    
    // Destroy Scaler Handler
    Kakao_ImgProc_Scaler_Destroy(&((*ppMeanShift)->pScalerHndl));
    
    // Release Mean Shift Main Handler
    SAFEFREE((*ppMeanShift));
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_MeanShift_UpdateImgSize(IN OUT MeanShift_T *pMeanShift, IN const INT32 nWidth, IN const INT32 nHeight)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pMeanShift, KAKAO_STAT_NULL_POINTER);
    
    Kakao_ImgProc_Util_UpdateSize(&pMeanShift->nBaseSize, nWidth, nHeight, NULL, IMG_FORMAT_MAX, 4);
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_MeanShift_Track_SetReference(IN OUT MeanShift_T *pMeanShift, IN Img_T *pSrcImg, IN const ROI nTargetROI)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   i = 0;
    Img_T                   *pDownScaledSrcImg = NULL;
    ROI                     nScaledROI = {0, };
    const ROI				nDefaultROI = {0, };
    const KIPointF2         nCenterPtr = {(F32)(nTargetROI.nStartX + nTargetROI.nEndX) / 2.0f,
                                          (F32)(nTargetROI.nStartY + nTargetROI.nEndY) / 2.0f};
    
    CHECK_POINTER_VALIDATION(pMeanShift, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER);
    
    pDownScaledSrcImg = pMeanShift->pDownScaledSrcImg;
    
    // Check Input Image Size
    if(pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0) < WIDTH_QCIF)
        goto Error;
    
    pMeanShift->nHistoBinGap = 256 / pMeanShift->nHistoBinNum;

    // Calculate Epanechnikov Mask
    if(NULL == pMeanShift->pEpanechnikovMask)
    {
        const INT32         nWindowSize = GET_MIN((nTargetROI.nEndX - nTargetROI.nStartX), (nTargetROI.nEndY - nTargetROI.nStartY));
        
        SAFEALLOC(pMeanShift->pEpanechnikovMask, (nWindowSize * nWindowSize), 32, F32);
        
        pMeanShift->nWindowSize.nWidth[IMG_PLANE_0] = nWindowSize;
        pMeanShift->nWindowSize.nHeight[IMG_PLANE_0] = nWindowSize;
        pMeanShift->nSearchRegion.nWidth[IMG_PLANE_0] = 3 * nWindowSize + 1;
        pMeanShift->nSearchRegion.nHeight[IMG_PLANE_0] = 3 * nWindowSize + 1;

        _MeanShift_MakeEpanechnikovMask(pMeanShift);
    }
    
    // Get Downscaled Ratio
    pMeanShift->nScaledRatioWidth = (F32)(pDownScaledSrcImg->GetImgWidth(pDownScaledSrcImg, IMG_PLANE_0)) / (F32)(pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0));
    pMeanShift->nScaledRatioHeight = (F32)(pDownScaledSrcImg->GetImgHeight(pDownScaledSrcImg, IMG_PLANE_0)) / (F32)(pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0));
    
    // Downscaling Input Image
    Kakao_ImgProc_Scaler_External(pMeanShift->pScalerHndl, pSrcImg, pMeanShift->pDownScaledSrcImg, IMG_SCALEMODE_NONE);
    
    // Update Center Point If Needed
    if((-1.0f != nCenterPtr.x) && (-1.0f != nCenterPtr.y))
    {
        pMeanShift->nCenterPtr.x = nCenterPtr.x * pMeanShift->nScaledRatioWidth;
        pMeanShift->nCenterPtr.y = nCenterPtr.y * pMeanShift->nScaledRatioHeight;
    }
    
    {
        const INT32             nWidth = pDownScaledSrcImg->GetImgWidth(pDownScaledSrcImg, IMG_PLANE_0);
        const INT32             nHeight = pDownScaledSrcImg->GetImgHeight(pDownScaledSrcImg, IMG_PLANE_0);
        const INT32             nHalfROIWidth = (INT32)((F32)(nTargetROI.nEndX - nTargetROI.nStartX) * pMeanShift->nScaledRatioWidth / 2.0f);
        const INT32             nHalfROIHeight = (INT32)((F32)(nTargetROI.nEndY - nTargetROI.nStartY) * pMeanShift->nScaledRatioHeight / 2.0f);
        
        pMeanShift->nTargetROI = nTargetROI;
        
        // Restrict Center Position
        pMeanShift->nCenterPtr.x = GET_MIN(GET_MAX(nHalfROIWidth, pMeanShift->nCenterPtr.x), (nWidth - nHalfROIWidth));
        pMeanShift->nCenterPtr.y = GET_MIN(GET_MAX(nHalfROIHeight, pMeanShift->nCenterPtr.y), (nHeight - nHalfROIHeight));
        
        // Set ROI
        nScaledROI.nStartX = pMeanShift->nCenterPtr.x - nHalfROIWidth;
        nScaledROI.nEndX = pMeanShift->nCenterPtr.x + nHalfROIWidth;
        nScaledROI.nStartY = pMeanShift->nCenterPtr.y - nHalfROIHeight;
        nScaledROI.nEndY = pMeanShift->nCenterPtr.y + nHalfROIHeight;
        Kakao_ImgProc_Util_SetROI(pMeanShift->pDownScaledSrcImg, nScaledROI);
    
        // Get Histogram and PDF of Input Image
        if(KAKAO_STAT_OK != (nRet = Kakao_ImgProc_Hist_Cal2DHist(pMeanShift->pDownScaledSrcImg, pMeanShift->pRefHist, pMeanShift->nHistoBinNum, IMGPROC_TRUE)))
            goto Error;
        
        // Reset ROI
        Kakao_ImgProc_Util_SetROI(pMeanShift->pDownScaledSrcImg, nDefaultROI);
    }
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_MeanShift_DoTracking(IN OUT MeanShift_T *pMeanShift, IN Img_T *pSrcImg)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   nLoopCnt = 0;
    Img_T                   *pDownScaledSrcImg = NULL;
    ROI                     nROI = {0, };
    const ROI				nDefaultROI = {0, };
    F32                     nErrorRate = 0.0f;
    
    CHECK_POINTER_VALIDATION(pMeanShift, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER);
    
    pDownScaledSrcImg = pMeanShift->pDownScaledSrcImg;
    
    // Check Input Image Size
    if(pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0) < WIDTH_QCIF)
        goto Error;
    
    // Downscaling Input Image
    Kakao_ImgProc_Scaler_External(pMeanShift->pScalerHndl, pSrcImg, pMeanShift->pDownScaledSrcImg, IMG_SCALEMODE_NONE);
    
    // Get Downscaled Ratio
    pMeanShift->nScaledRatioWidth = (F32)(pDownScaledSrcImg->GetImgWidth(pDownScaledSrcImg, IMG_PLANE_0)) / (F32)(pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0));
    pMeanShift->nScaledRatioHeight = (F32)(pDownScaledSrcImg->GetImgHeight(pDownScaledSrcImg, IMG_PLANE_0)) / (F32)(pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0));
    
    {
        const INT32             nWidth = pDownScaledSrcImg->GetImgWidth(pDownScaledSrcImg, IMG_PLANE_0);
        const INT32             nHeight = pDownScaledSrcImg->GetImgHeight(pDownScaledSrcImg, IMG_PLANE_0);
        const INT32             nStride = pDownScaledSrcImg->GetImgStride(pDownScaledSrcImg, IMG_PLANE_0);
        const INT32             nWindowWidth = pMeanShift->nWindowSize.nWidth[IMG_PLANE_0];
        const INT32             nWindowHeight = pMeanShift->nWindowSize.nHeight[IMG_PLANE_0];
        const INT32             nHalfWindowWidth = nWindowWidth >> 1;
        const INT32             nHalfWindowHeight = nWindowHeight >> 1;
        const ROI               nTargetROI = pMeanShift->nTargetROI;
        const INT32             nHalfROIWidth = (INT32)((F32)(nTargetROI.nEndX - nTargetROI.nStartX) * pMeanShift->nScaledRatioWidth / 2.0f);
        const INT32             nHalfROIHeight = (INT32)((F32)(nTargetROI.nEndY - nTargetROI.nStartY) * pMeanShift->nScaledRatioHeight / 2.0f);
        UINT8                   *pTargetData = NULL;
        KIPointF2               nLocalCenter = pMeanShift->nCenterPtr;

        // Get Histogram and PDF of Input Image
        if(KAKAO_STAT_OK != (nRet = Kakao_ImgProc_Hist_Cal2DHist(pMeanShift->pDownScaledSrcImg, NULL, pMeanShift->nHistoBinNum, IMGPROC_TRUE)))
            goto Error;

        // Make BackProjection Image
        if(KAKAO_STAT_OK != (nRet = Kakao_ImgProc_Hist_Cal2DHistBP(pMeanShift->pDownScaledSrcImg, pMeanShift->pRefHist, pMeanShift->nHistoBinNum, IMG_BP_RATIO)))
            goto Error;
        
        if(NULL == (pTargetData = pMeanShift->pDownScaledSrcImg->GetImgBinaryMap(pMeanShift->pDownScaledSrcImg, IMG_BINARYMAP_HISTO_BP)))
            goto Error;
        
        do
        {
            KIPointF2               nDeltaCenter = {0.0f, 0.0f};
            UINT8                   *pLocalTargetData = NULL;
            
            // Restrict Center Position
            nLocalCenter.x = GET_MIN(GET_MAX(nHalfROIWidth, nLocalCenter.x), (nWidth - nHalfROIWidth));
            nLocalCenter.y = GET_MIN(GET_MAX(nHalfROIHeight, nLocalCenter.y), (nHeight - nHalfROIHeight));
        
            if(KAKAO_STAT_OK != (nRet = _MeanShift_Track_CalNewCenter(pMeanShift, pTargetData, nStride,
                                                                      nWindowWidth, nWindowHeight, nLocalCenter, &nDeltaCenter)))
                goto Error;
            
            nErrorRate = SQRT((nDeltaCenter.x * nDeltaCenter.x) + (nDeltaCenter.y * nDeltaCenter.y));
            
            nLocalCenter.x += (nDeltaCenter.x + 0.5f);
            nLocalCenter.y += (nDeltaCenter.y + 0.5f);
            nLoopCnt++;
        }while((3.0f <= nErrorRate) && (5 >= nLoopCnt));
        
        pMeanShift->nCenterPtr.x = GET_MIN(GET_MAX(nHalfROIWidth, nLocalCenter.x), (nWidth - nHalfROIWidth));
        pMeanShift->nCenterPtr.y = GET_MIN(GET_MAX(nHalfROIHeight, nLocalCenter.y), (nHeight - nHalfROIHeight));
    }
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}

//extern UINT8   *_gpHist1Buf;
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_MeanShift_DoColorClustering(IN OUT MeanShift_T *pMeanShift, IN Img_T *pSrcImg, IN const INT32 nWindowSize)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    UINT8                   *pMeanShiftClusterBinaryMap = NULL;
    static F32              *pTempBuf = NULL;
    
    CHECK_POINTER_VALIDATION(pMeanShift, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER);
    
    //SAFEALLOC(pTempBuf, 65536, 32, F32);
    
    // Check Input Image Size
    if(pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0) < WIDTH_QCIF)
        goto Error;

    // Calculate Epanechnikov Mask
    if(NULL == pMeanShift->pEpanechnikovMask)
    {
        SAFEALLOC(pMeanShift->pEpanechnikovMask, (nWindowSize * nWindowSize), 32, F32);
        
        pMeanShift->nWindowSize.nWidth[IMG_PLANE_0] = nWindowSize;
        pMeanShift->nWindowSize.nHeight[IMG_PLANE_0] = nWindowSize;
        pMeanShift->nSearchRegion.nWidth[IMG_PLANE_0] = 3 * nWindowSize + 1;
        pMeanShift->nSearchRegion.nHeight[IMG_PLANE_0] = 3 * nWindowSize + 1;

        _MeanShift_MakeEpanechnikovMask(pMeanShift);
    }
    
    // Downscaling Input Image
    Kakao_ImgProc_Scaler_External(pMeanShift->pScalerHndl, pSrcImg, pMeanShift->pDownScaledSrcImg, IMG_SCALEMODE_NONE);
    
    // Get Histogram and PDF of Input Image
    if(KAKAO_STAT_OK != (nRet = Kakao_ImgProc_Hist_Cal2DHist(pMeanShift->pDownScaledSrcImg, NULL, pMeanShift->nHistoBinNum, IMGPROC_FALSE)))
        goto Error;
    
    {
        const INT32         nHalfWindowSize = nWindowSize / 2;
        const INT32         nTargetStride = 256;
        KIPoint2            *pClusteringMap = pMeanShift->pClusteringMap;
        INT32               i = 0, j = 0;
        F32                 *pTargetData = NULL;
        F32                 nErrorRate = 0.0f;

        Kakao_ImgProc_Hist_GetHistMap(pMeanShift->pDownScaledSrcImg, 2, 0, &pTargetData);
        if(NULL == pTargetData)
            goto Error;

        for(j=nHalfWindowSize ; j<256-nHalfWindowSize ; j++)
        {
            const INT32     nOffset = 256 * j;
            const F32       *pLocalTargetData = pTargetData + nOffset;
            KIPoint2        *pLocalClusteringMap = pClusteringMap + nOffset;
            
            for(i=nHalfWindowSize ; i<256-nHalfWindowSize ; i++)
            {
                KIPointF2   nDeltaCenter = {0.0f, 0.0f};
                KIPointF2   nLocalCenter = {i, j};
                INT32       nLoopCnt = 0;
                
                if(0 != pLocalTargetData[i])
                {
                    do
                    {
                        if(KAKAO_STAT_OK != (nRet = _MeanShift_Cluster_CalNewCenter(pMeanShift, pTargetData, nTargetStride,
                                                                                    nWindowSize, nWindowSize, nLocalCenter, &nDeltaCenter)))
                            goto Error;
                        
                        nErrorRate = SQRT((nDeltaCenter.x * nDeltaCenter.x) + (nDeltaCenter.y * nDeltaCenter.y));
                        
                        nLocalCenter.x += (nDeltaCenter.x + 0.5f);
                        nLocalCenter.y += (nDeltaCenter.y + 0.5f);
                        nLoopCnt++;
                    }while((1.0f <= nErrorRate) && (20 >= nLoopCnt));
                }
                
                pLocalClusteringMap[i].x = (INT32)nLocalCenter.x;
                pLocalClusteringMap[i].y = (INT32)nLocalCenter.y;
                
                //if((0 != pLocalClusteringMap[i].x - i) && (0 != pLocalClusteringMap[i].y - j))
                //    pTempBuf[256*pLocalClusteringMap[i].y + pLocalClusteringMap[i].x] = 255;
                //else
                //    pTempBuf[256*pLocalClusteringMap[i].y + pLocalClusteringMap[i].x] = 0;
            }
        }
        
        //for(i=0 ; i<65536 ; i++)
        //    _gpHist1Buf[i] = (UINT8)pTempBuf[i];
    }
    
    {
        const INT32         nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_1);
        const INT32         nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_1);
        const INT32         nStrideY = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
        const INT32         nStrideUV = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_1);
        const KIPoint2      *pClusteringMap = pMeanShift->pClusteringMap;
        UINT8               *pSrcY = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
        UINT8               *pSrcU = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_1);
        UINT8               *pSrcV = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_2);
        INT32               i = 0, j = 0;

        for(j=0 ; j<nHeight ; j++)
        {
            UINT8           *pLocalSrcY = pSrcY + (2 * j * nStrideY);
            UINT8           *pLocalSrcU = pSrcU + (j * nStrideUV);
            UINT8           *pLocalSrcV = pSrcV + (j * nStrideUV);
            
            for(i=0 ; i<nWidth ; i++)
            {
                const INT32 nCurrU = (INT32)(pLocalSrcU[i]);
                const INT32 nCurrV = (INT32)(pLocalSrcV[i]);
                const INT32 nOffset = 256 * nCurrV + nCurrU;
                
                {
                    pLocalSrcU[i] = (UINT8)(pClusteringMap[nOffset].x);
                    pLocalSrcV[i] = (UINT8)(pClusteringMap[nOffset].y);
                }
                
                pLocalSrcY[2*i] = pLocalSrcY[2*i+1] =
                pLocalSrcY[nStrideY + 2*i] = pLocalSrcY[nStrideY + 2*i+1] = 128;//(UINT8)((pClusteringMap[nOffset].x + pClusteringMap[nOffset].y)/2);
            }
        }
    }
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_MeanShift_SetCenterPtr(IN OUT MeanShift_T *pMeanShift, IN const Img_T *pSrcImg, IN const KIPointF2 nCenterPtr)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    Img_T                   *pDownScaledSrcImg = NULL;
    
    CHECK_POINTER_VALIDATION(pMeanShift, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER);
    
    pDownScaledSrcImg = pMeanShift->pDownScaledSrcImg;
    
    {
        const INT32             nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
        const INT32             nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
        const INT32             nHalfSearchWidth = (pMeanShift->nSearchRegion.nWidth[IMG_PLANE_0] >> 1);
        const INT32             nHalfSearchHeight = (pMeanShift->nSearchRegion.nHeight[IMG_PLANE_0] >> 1);
        const F32               nInvScaledRatioWidth = (F32)(pDownScaledSrcImg->GetImgWidth(pDownScaledSrcImg, IMG_PLANE_0)) / (F32)(pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0));
        const F32               nInvScaledRatioHeight = (F32)(pDownScaledSrcImg->GetImgHeight(pDownScaledSrcImg, IMG_PLANE_0)) / (F32)(pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0));
        
        // Restrict Center Position
        pMeanShift->nCenterPtr.x = GET_MIN(GET_MAX(nHalfSearchWidth, (nInvScaledRatioWidth * nCenterPtr.x)), (nWidth - nHalfSearchWidth));
        pMeanShift->nCenterPtr.y = GET_MIN(GET_MAX(nHalfSearchHeight, (nInvScaledRatioHeight * nCenterPtr.y)), (nHeight - nHalfSearchHeight));
    }
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_MeanShift_GetCenterPtr(IN OUT MeanShift_T *pMeanShift, OUT KIPointF2 *pCenterPtr)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pMeanShift, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pCenterPtr, KAKAO_STAT_NULL_POINTER);
    
    pCenterPtr->x = pMeanShift->nCenterPtr.x / pMeanShift->nScaledRatioWidth;
    pCenterPtr->y = pMeanShift->nCenterPtr.y / pMeanShift->nScaledRatioHeight;
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _MeanShift_Track_CalNewCenter(IN const MeanShift_T *pMeanShift, IN const UINT8 *pTargetData, IN const INT32 nTargetStride,
                                                    IN const INT32 nWindowWidth, IN const INT32 nWindowHeight, IN const KIPointF2 nLocalCenter, OUT KIPointF2 *pDeltaCenter)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    const INT32             nHalfWindowWidth = nWindowWidth >> 1;
    const INT32             nHalfWindowHeight = nWindowHeight >> 1;
    const F32               *pEpanechnikovMask = pMeanShift->pEpanechnikovMask;
    F32                     nCurrPDSumX = 0.0f, nCurrPDSumY = 0.0f, nCurrPDSumAll = 0.0f;
    INT32                   i = 0, j = 0;
    
    pTargetData += ((INT32)(nLocalCenter.y - (F32)nHalfWindowHeight) * nTargetStride) + (INT32)(nLocalCenter.x - (F32)nHalfWindowWidth);
    
    for(j=0 ; j<nWindowHeight ; j++)
    {
        const UINT8         *pLocalTargetData = pTargetData + (j * nTargetStride);
        const F32           *pTmpEpanechnikovMask = pEpanechnikovMask + (j * nWindowWidth);
        
        for(i=0 ; i<nWindowWidth ; i++)
        {
            const F32   nWeightedBP = (F32)(pLocalTargetData[i]) * pTmpEpanechnikovMask[i];
            
            nCurrPDSumAll += nWeightedBP;
            nCurrPDSumX += (nWeightedBP * (i - nHalfWindowWidth));
            nCurrPDSumY += (nWeightedBP * (j - nHalfWindowHeight));
        }
    }

    if(0 == nCurrPDSumAll)
    {
        pDeltaCenter->x = 0.0f;
        pDeltaCenter->y = 0.0f;
    }
    else
    {
        pDeltaCenter->x = nCurrPDSumX / nCurrPDSumAll;
        pDeltaCenter->y = nCurrPDSumY / nCurrPDSumAll;
    }
    
    nRet = KAKAO_STAT_OK;

    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _MeanShift_Cluster_CalNewCenter(IN const MeanShift_T *pMeanShift, IN const F32 *pTargetData, IN const INT32 nTargetStride,
                                                    IN const INT32 nWindowWidth, IN const INT32 nWindowHeight, IN const KIPointF2 nLocalCenter, OUT KIPointF2 *pDeltaCenter)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    const INT32             nHalfWindowWidth = nWindowWidth >> 1;
    const INT32             nHalfWindowHeight = nWindowHeight >> 1;
    const F32               *pEpanechnikovMask = pMeanShift->pEpanechnikovMask;
    F32                     nCurrPDSumX = 0.0f, nCurrPDSumY = 0.0f, nCurrPDSumAll = 0.0f;
    INT32                   i = 0, j = 0;
    
    pTargetData += ((INT32)(nLocalCenter.y - (F32)nHalfWindowHeight) * nTargetStride) + (INT32)(nLocalCenter.x - (F32)nHalfWindowWidth);
    
    for(j=0 ; j<nWindowHeight ; j++)
    {
        const F32           *pLocalTargetData = pTargetData + (j * nTargetStride);
        const F32           *pTmpEpanechnikovMask = pEpanechnikovMask + (j * nWindowWidth);
        
        for(i=0 ; i<nWindowWidth ; i++)
        {
            const F32   nWeightedBP = pLocalTargetData[i] * pTmpEpanechnikovMask[i];
            
            nCurrPDSumAll += nWeightedBP;
            nCurrPDSumX += (nWeightedBP * (i - nHalfWindowWidth));
            nCurrPDSumY += (nWeightedBP * (j - nHalfWindowHeight));
        }
    }
    
    if(0 == nCurrPDSumAll)
    {
        pDeltaCenter->x = 0.0f;
        pDeltaCenter->y = 0.0f;
    }
    else
    {
        pDeltaCenter->x = nCurrPDSumX / nCurrPDSumAll;
        pDeltaCenter->y = nCurrPDSumY / nCurrPDSumAll;
    }
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _MeanShift_MakeEpanechnikovMask(IN MeanShift_T *pMeanShift)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    const F32               nStep = (1.0f / ((F32)(pMeanShift->nWindowSize.nWidth[IMG_PLANE_0]) * 0.5));
    const INT32             nWindowWidth = pMeanShift->nWindowSize.nWidth[IMG_PLANE_0];
    const INT32             nWindowHeight = pMeanShift->nWindowSize.nHeight[IMG_PLANE_0];
    F32                     nVec[256] = {0.0f, };
    F32                     *pEpanechnikovMask = pMeanShift->pEpanechnikovMask;
    INT32                   i = 0, j = 0;
    F32                     k = -1.0f + nStep/2;
    
    for(j=0 ; j<nWindowWidth ; k+=nStep, j++)
        nVec[j] = (1.0f - (F32)(k * k)) * 3.0f / 4.0f;
    
    for(j=0 ; j<nWindowHeight ; j++)
    {
        F32                 *pTmpEpanechnikovMask = pEpanechnikovMask + (j * nWindowWidth);
        
        for(i=0 ; i<nWindowWidth ; i++)
            pTmpEpanechnikovMask[i] = nVec[j] * nVec[i];
    }
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _MeanShift_MakeColorLookUpTable(IN MeanShift_T *pMeanShift)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    Color_RGB               *pYUVtoRGBMapTable = pMeanShift->pYUVtoRGBMapTable;
    Color_HSL               *pYUVtoHSLMapTable = pMeanShift->pYUVtoHSLMapTable;
    INT32                   i = 0, j = 0, k = 0;
    
    for(k=0 ; k<64 ; k++)
    {
        // For Y
        INT32               nScaledK = k << 2;
        
        for(j=-32 ; j<32 ; j++)
        {
            // For U
            INT32               nScaledJ = j << 2;
            
            for(i=-32 ; i<32 ; i++)
            {
                // For V
                INT32               nScaledI = i << 2;
                
                const D64   nRGB_R = CLIP3(1, PIXEL_MAX, (D64)nScaledK +   1.402 * (D64)nScaledI);
                const D64   nRGB_G = CLIP3(1, PIXEL_MAX, (D64)nScaledK - 0.34414 * (D64)nScaledJ - 0.71414 * (D64)nScaledI);
                const D64   nRGB_B = CLIP3(1, PIXEL_MAX, (D64)nScaledK +   1.772 * (D64)nScaledJ);
                
                const D64   nHSL_L = (nRGB_R + nRGB_G + nRGB_B) / 3.0;
                const D64   nHSL_S = 1.0 - (1.0 / nHSL_L) * GET_MIN(GET_MIN(nRGB_R, nRGB_G), nRGB_B);
                const D64   nHSL_H = acos((2.0 * nRGB_R - nRGB_G - nRGB_B) / (2.0 * SQRT((nRGB_R - nRGB_G) * (nRGB_R - nRGB_G) + (nRGB_R - nRGB_B) * (nRGB_G - nRGB_B))));
                
                pYUVtoRGBMapTable->r = (UINT8)nRGB_R;
                pYUVtoRGBMapTable->g = (UINT8)nRGB_G;
                pYUVtoRGBMapTable->b = (UINT8)nRGB_B;
                pYUVtoRGBMapTable++;
                
                pYUVtoHSLMapTable->h = nHSL_H;
                pYUVtoHSLMapTable->s = nHSL_S;
                pYUVtoHSLMapTable->l = (nRGB_G - nRGB_B) > 0.0 ? nHSL_L : 360.0 - nHSL_L;
                pYUVtoHSLMapTable++;
            }
        }
    }
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}

