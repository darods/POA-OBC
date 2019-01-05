


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
static UINT32 _Labeling(IN BlobLabel *pBlobLabel, IN OUT Img_T *pSrcImg, IN const BinaryMapType nBinaryMap, IN const UINT32 nThreshold);
static Kakao_Status _NRFInd_Neighbor(IN BlobLabel *pBlobLabel, IN OUT Img_T *pSrcImg, IN const UINT8 *pSrcMap, IN const KIPoint2 nStartPoint, OUT KIPoint2 *pTopLeft, OUT KIPoint2 *pBottomRight);
static Kakao_Status _Get_BlobInfo(BlobLabel *pBlobLabel, IN const Img_T *pSrcImg, IN const KIPoint2 KIPointnTopLeft, IN const KIPoint2 nBottomRight, IN const INT32 nStride, UINT32 *pLabelIdx, IN const UINT32 nThreshold);
static Kakao_Status _DetectBlobs_InBlob(IN const BlobLabel *pBlobLabel, IN Img_T *pSrcImg, OUT UINT8 *pSrcMap);
static Kakao_Status _Get_BlobLabelMap(IN const BlobLabel *pBlobLabel, IN const Img_T *pSrcImg, UINT8 **ppBlobLabelMap);
static Kakao_Status _Get_VisitedPtrMap(IN const BlobLabel *pBlobLabel, IN const Img_T *pSrcImg, PtVisited **ppVisitedPtrMap);


/*----------------------------------------------------------------------------------------
 Static Variable
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Global Variable
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Function Implementation
 ----------------------------------------------------------------------------------------*/
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_BlobLabeling_Create(IN OUT BlobLabel **ppBlobLabel, IN const INT32 nWidth, IN const INT32 nHeight)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    BlobLabel               *pTmpBlobLabel = NULL;
    INT32                   nAlignedWidth = GET_ALIGNED_LENGTH(nWidth, MACRO_BLOCK_SIZE);
    INT32                   nAlignedHeight = GET_ALIGNED_LENGTH(nHeight, MACRO_BLOCK_SIZE);

    SAFEALLOC(pTmpBlobLabel, 1, 32, BlobLabel);
    
    Kakao_ImgProc_Util_UpdateSize(&pTmpBlobLabel->nBaseSize, nWidth, nHeight, NULL, IMG_FORMAT_MAX, 4);

    pTmpBlobLabel->nImgRegionSize = nWidth * nHeight;
    pTmpBlobLabel->nTh = 0;
    pTmpBlobLabel->nNumofBlobs = 0;
    
    SAFEALLOC(pTmpBlobLabel->pBlobLabelMap, (MAKE_STRIDE(nAlignedWidth) * nAlignedHeight), 32, UINT8);
    SAFEALLOC(pTmpBlobLabel->pVisitedPtrMap, (MAKE_STRIDE(nAlignedWidth) * nAlignedHeight), 32, PtVisited);
    SAFEALLOC(pTmpBlobLabel->pBlobInfo, MAX_LABEL_NUM, 32, BlobInfo);

    *ppBlobLabel = pTmpBlobLabel;
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_BlobLabeling_Destroy(IN OUT BlobLabel **ppBlobLabel)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION((*ppBlobLabel), KAKAO_STAT_NULL_POINTER);

    SAFEFREE((*ppBlobLabel)->pBlobLabelMap);
    SAFEFREE((*ppBlobLabel)->pVisitedPtrMap);
    SAFEFREE((*ppBlobLabel)->pBlobInfo);
    
    SAFEFREE((*ppBlobLabel));

    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_BlobLabeling_UpdateImgSize(IN OUT BlobLabel *pBlobLabel, IN const INT32 nWidth, IN const INT32 nHeight)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pBlobLabel, KAKAO_STAT_NULL_POINTER);
    
    Kakao_ImgProc_Util_UpdateSize(&pBlobLabel->nBaseSize, nWidth, nHeight, NULL, IMG_FORMAT_MAX, 4);
    
    pBlobLabel->nImgRegionSize = nWidth * nHeight;
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_BlobLabeling_DoBlobLabeling(IN BlobLabel *pBlobLabel, IN OUT Img_T *pSrcImg, IN const BinaryMapType nBinaryMap,
                                                                        IN const UINT32 nThreshold, IN const INT32 nDegree, IN const INT32 nIndicator)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    CHECK_POINTER_VALIDATION(pBlobLabel, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER);

    ___START_PROFILE(BlobLabel)

    {
        const INT32             nHeight = pBlobLabel->nBaseSize.nHeight[IMG_PLANE_0];
        const INT32             nStride = pBlobLabel->nBaseSize.nStride[IMG_PLANE_0];
        UINT32                  nRatioTh = 0;

        MEMSET(pBlobLabel->pBlobLabelMap, 0, (sizeof(UINT8) * (nStride * nHeight)));
        MEMSET(pBlobLabel->pVisitedPtrMap, 0, (sizeof(PtVisited) * (nStride * nHeight)));
        MEMSET(pBlobLabel->pBlobInfo, 0, sizeof(BlobInfo) * MAX_LABEL_NUM);
        
        pBlobLabel->nNumofBlobs = 0;
        pBlobLabel->nDegree = nDegree;
        pBlobLabel->nIndicator = nIndicator;
        
        pBlobLabel->nImgRegionSize = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0) * pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
        nRatioTh = pBlobLabel->nImgRegionSize * nThreshold / 100;
        
        _Labeling(pBlobLabel, pSrcImg, nBinaryMap, nRatioTh);
    }
    
    ___STOP_PROFILE(BlobLabel)
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_BlobLabeling_BlobSmallSizeConstraint(IN BlobLabel *pBlobLabel, IN const INT32 nWidth, IN const INT32 nHeight,
                                                                                 OUT BlobInfo *pBlobInfo, OUT UINT32 *pNumofBlob)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pBlobLabel, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pBlobInfo, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pNumofBlob, KAKAO_STAT_NULL_POINTER)

    {
        UINT32                  i = 0;
        UINT32                  nNumofBlobs = pBlobLabel->nNumofBlobs;
        BlobInfo                *pMainBlobInfo = pBlobLabel->pBlobInfo;

        if(0 == nNumofBlobs)
            return KAKAO_STAT_OK;
        
        *pNumofBlob = 0;
        
        for(i=0; i<nNumofBlobs ; i++)
        {
            if((pMainBlobInfo[i].nRectBlobs.nWidth > nWidth) && (pMainBlobInfo[i].nRectBlobs.nHeight > nHeight))
            {
                pBlobInfo[(*pNumofBlob)] = pMainBlobInfo[i];
                (*pNumofBlob)++;
            }
        }
    }

    nRet = KAKAO_STAT_OK;

    return nRet;

Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_BlobLabeling_BlobBigSizeConstraint(IN BlobLabel *pBlobLabel, IN const INT32 nWidth, IN const INT32 nHeight,
                                                                               OUT BlobInfo *pBlobInfo, OUT UINT32 *pNumofBlob)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    CHECK_POINTER_VALIDATION(pBlobLabel, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pBlobInfo, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pNumofBlob, KAKAO_STAT_NULL_POINTER)
    
    {
        UINT32                  i = 0;
        UINT32                  nNumofBlobs = pBlobLabel->nNumofBlobs;
        BlobInfo                *pMainBlobInfo = pBlobLabel->pBlobInfo;
        
        if(0 == nNumofBlobs)
            return KAKAO_STAT_OK;
        
        *pNumofBlob = 0;
        
        for(i=0; i<nNumofBlobs ; i++)
        {
            if((pMainBlobInfo[i].nRectBlobs.nWidth < nWidth) && (pMainBlobInfo[i].nRectBlobs.nHeight < nHeight))
            {
                pBlobInfo[(*pNumofBlob)] = pMainBlobInfo[i];
                (*pNumofBlob)++;
            }
        }
    }
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


static UINT32 _Labeling(IN BlobLabel *pBlobLabel, IN OUT Img_T *pSrcImg, IN const BinaryMapType nBinaryMap, IN const UINT32 nThreshold)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    UINT32                  nLabelIdx = 0;
    INT32                   i = 0, j = 0;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    INT32                   nLocalPos = 0;
    const INT32             nIndicator = pBlobLabel->nIndicator;
    KIPoint2                nTopLeft = {0, }, nBottomRight = {0, };
    KIPoint2                nStartPoint = {0, };
    UINT8                   *pBlobLabelMap = NULL;
    UINT8                   *pSrcMap = NULL;
    
    if(NULL == (pSrcMap = pSrcImg->GetImgBinaryMap(pSrcImg, nBinaryMap)))
        return 0;
    
    CHECK_POINTER_VALIDATION(pSrcMap, KAKAO_STAT_NULL_POINTER)

    _Get_BlobLabelMap(pBlobLabel, pSrcImg, &pBlobLabelMap);
    
    // Find connected components
    for(j=0 ; j<nHeight ; j++)
    {
        nLocalPos = j * nStride;
        for(i=0 ; i<nWidth ; i++)
        {
            if((nIndicator == pSrcMap[nLocalPos]) && (0 == pBlobLabelMap[nLocalPos]))        // Is this a new component?, 255 == Object
            {
                Kakao_Status        nRet = KAKAO_STAT_FAIL;
                
                nLabelIdx++;
                
                pBlobLabelMap[nLocalPos] = nLabelIdx;
                
                nStartPoint.x = (i);
                nStartPoint.y = (j);
                nTopLeft = nBottomRight = nStartPoint;
                
                _NRFInd_Neighbor(pBlobLabel, pSrcImg, pSrcMap, nStartPoint, &nTopLeft, &nBottomRight);
                
                if((nTopLeft.x != nBottomRight.x) || (nTopLeft.y != nBottomRight.y))
                {
                    nRet = _Get_BlobInfo(pBlobLabel, pSrcImg, nTopLeft, nBottomRight, nStride, &nLabelIdx, nThreshold);

                    // Detect Blobs in the Blob
                    //if(KAKAO_STAT_OK == nRet)
                    //    _DetectBlobs_InBlob(pBlobLabel, pSrcImg, pSrcMap);
                }
                else
                    nLabelIdx--;
            }
            
            if(MAX_LABEL_NUM == pBlobLabel->nNumofBlobs)
                return pBlobLabel->nNumofBlobs;
            
            nLocalPos++;
        }
    }
    
    return pBlobLabel->nNumofBlobs;
    
Error:
    return 0;
}


static Kakao_Status _NRFInd_Neighbor(IN BlobLabel *pBlobLabel, IN OUT Img_T *pSrcImg, IN const UINT8 *pSrcMap, IN const KIPoint2 nStartPoint, OUT KIPoint2 *pTopLeft, OUT KIPoint2 *pBottomRight)
{
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    UINT8                   *pBlobLabelMap = NULL;
    PtVisited               *pVisitedPtrMap = NULL;
    KIPoint2                nCurrentPoint = {0, };
    INT32                   nLocalPos = 0;
    const INT32             nIndicator = pBlobLabel->nIndicator;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    _Get_BlobLabelMap(pBlobLabel, (const Img_T *)pSrcImg, &pBlobLabelMap);
    _Get_VisitedPtrMap(pBlobLabel, (const Img_T *)pSrcImg, &pVisitedPtrMap);
    
    nCurrentPoint.x = nStartPoint.x;
    nCurrentPoint.y = nStartPoint.y;
    
    nLocalPos = nCurrentPoint.y * nStride + nCurrentPoint.x;
    pVisitedPtrMap[nLocalPos].bVisitedFlag = IMGPROC_TRUE;
    pVisitedPtrMap[nLocalPos].nRetPoint.x = nStartPoint.x;
    pVisitedPtrMap[nLocalPos].nRetPoint.y = nStartPoint.y;
    
    while(1)
    {
        nLocalPos = nCurrentPoint.y * nStride + nCurrentPoint.x;
        
        // Position: Left
        if((nCurrentPoint.x != 0) && (nIndicator == pSrcMap[nLocalPos - 1]))
        {
            if(IMGPROC_FALSE == pVisitedPtrMap[nLocalPos - 1].bVisitedFlag)
            {
                pBlobLabelMap[nLocalPos - 1] = pBlobLabelMap[nLocalPos];
                pVisitedPtrMap[nLocalPos - 1].bVisitedFlag = IMGPROC_TRUE;
                pVisitedPtrMap[nLocalPos - 1].nRetPoint = nCurrentPoint;
                nCurrentPoint.x--;
                
                if(nCurrentPoint.x <= 0)
                    nCurrentPoint.x = 0;
                
                if(pTopLeft->x >= nCurrentPoint.x)
                    pTopLeft->x = nCurrentPoint.x;

                continue;
            }
        }
        
        // Position: Right
        if((nCurrentPoint.x != nWidth - 1) && (nIndicator == pSrcMap[nLocalPos + 1]))
        {
            if(IMGPROC_FALSE == pVisitedPtrMap[nLocalPos + 1].bVisitedFlag)
            {
                pBlobLabelMap[nLocalPos + 1] = pBlobLabelMap[nLocalPos];
                pVisitedPtrMap[nLocalPos + 1].bVisitedFlag = IMGPROC_TRUE;
                pVisitedPtrMap[nLocalPos + 1].nRetPoint = nCurrentPoint;
                nCurrentPoint.x++;
                
                if(nCurrentPoint.x >= nWidth - 1)
                    nCurrentPoint.x = nWidth - 1;
                
                if(pBottomRight->x <= nCurrentPoint.x)
                    pBottomRight->x = nCurrentPoint.x;
        
                continue;
            }
        }
        
        // Position: Down
        if((nCurrentPoint.y != 0) && (nIndicator == pSrcMap[nLocalPos - nStride]))
        {
            if(IMGPROC_FALSE == pVisitedPtrMap[nLocalPos - nStride].bVisitedFlag)
            {
                pBlobLabelMap[nLocalPos - nStride] = pBlobLabelMap[nLocalPos];
                pVisitedPtrMap[nLocalPos - nStride].bVisitedFlag = IMGPROC_TRUE;
                pVisitedPtrMap[nLocalPos - nStride].nRetPoint = nCurrentPoint;
                nCurrentPoint.y--;
                
                if(nCurrentPoint.y <= 0)
                    nCurrentPoint.y = 0;
                
                if(pTopLeft->y >= nCurrentPoint.y)
                    pTopLeft->y = nCurrentPoint.y;

                continue;
            }
        }
        
        // Position: Up
        if((nCurrentPoint.y != nHeight - 1) && (nIndicator == pSrcMap[nLocalPos + nStride]))
        {
            if(IMGPROC_FALSE == pVisitedPtrMap[nLocalPos + nStride].bVisitedFlag)
            {
                pBlobLabelMap[nLocalPos + nStride] = pBlobLabelMap[nLocalPos];
                pVisitedPtrMap[nLocalPos + nStride].bVisitedFlag = IMGPROC_TRUE;
                pVisitedPtrMap[nLocalPos + nStride].nRetPoint = nCurrentPoint;
                nCurrentPoint.y++;
                
                if(nCurrentPoint.y >= nHeight - 1)
                    nCurrentPoint.y = nHeight - 1;
                
                if(pBottomRight->y <= nCurrentPoint.y)
                    pBottomRight->y = nCurrentPoint.y;

                continue;
            }
        }

        if((nCurrentPoint.x == pVisitedPtrMap[nLocalPos].nRetPoint.x) &&
           (nCurrentPoint.y == pVisitedPtrMap[nLocalPos].nRetPoint.y))
            break;
        else
            nCurrentPoint = pVisitedPtrMap[nLocalPos].nRetPoint;
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _Get_BlobInfo(BlobLabel *pBlobLabel, IN const Img_T *pSrcImg, IN const KIPoint2 nTopLeft, IN const KIPoint2 nBottomRight, IN const INT32 nStride, UINT32 *pLabelIdx, IN const UINT32 nThreshold)
{
    INT32                   i, j;
    UINT32                  nBlobSize = 0;
    INT32                   nRestrictedVertical = 0;
    UINT8                   *pBlobLabelMap = NULL;
    KIPoint2                nCenterPoint = {0, };
    BlobInfo                *pBlobInfo = pBlobLabel->pBlobInfo;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    _Get_BlobLabelMap(pBlobLabel, pSrcImg, &pBlobLabelMap);
    
    if(((-90) == pBlobLabel->nDegree) || ((90) == pBlobLabel->nDegree) || ((270) == pBlobLabel->nDegree))
        nRestrictedVertical = CLIP3(nTopLeft.y, pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0), (100 * (nBottomRight.y - nTopLeft.y) / 100) + nTopLeft.y);
    else
        nRestrictedVertical = CLIP3(nTopLeft.y, pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0), (120 * (nBottomRight.x - nTopLeft.x) / 100) + nTopLeft.y);
    
    // Calculate BlobSize & CenterPoint
    for(j=nTopLeft.y ; j<nRestrictedVertical ; j++)
    {
        for(i=nTopLeft.x ; i<nBottomRight.x ; i++)
        {
            if(*pLabelIdx == pBlobLabelMap[j * nStride + i])
            {
                nCenterPoint.x += i;
                nCenterPoint.y += j;
                ++nBlobSize;
            }
        }
    }

    if(nBlobSize < nThreshold)
    {
        // If Blob KISize is Too Small, Then Erase Blobs
        for(j=nTopLeft.y ; j<nRestrictedVertical ; j++)
            for(i=nTopLeft.x ; i<nBottomRight.x ; i++)
                if(*pLabelIdx == pBlobLabelMap[j * nStride + i])
                    pBlobLabelMap[j * nStride + i] = 0;

        --(*pLabelIdx);
        
        return KAKAO_STAT_FAIL;
    }
    else
    {
        // Finally Find the Blob
        pBlobInfo[pBlobLabel->nNumofBlobs].nRectBlobs.x = nTopLeft.x;
        pBlobInfo[pBlobLabel->nNumofBlobs].nRectBlobs.y = nTopLeft.y;
        pBlobInfo[pBlobLabel->nNumofBlobs].nRectBlobs.nWidth = nBottomRight.x - nTopLeft.x;
        pBlobInfo[pBlobLabel->nNumofBlobs].nRectBlobs.nHeight = nRestrictedVertical - nTopLeft.y; // nBottomRight.y - nTopLeft.y;
        pBlobInfo[pBlobLabel->nNumofBlobs].nBlobCenter.x = (nCenterPoint.x / nBlobSize);
        pBlobInfo[pBlobLabel->nNumofBlobs].nBlobCenter.y = (nCenterPoint.y / nBlobSize);
        pBlobInfo[pBlobLabel->nNumofBlobs++].nBlobSize = nBlobSize;
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _DetectBlobs_InBlob(IN const BlobLabel *pBlobLabel, IN Img_T *pSrcImg, OUT UINT8 *pSrcMap)
{
    INT32                   nLocalPos = 0;
    INT32                   nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    UINT32                  nBlobIdx = pBlobLabel->nNumofBlobs - 1;
    KIRect                  *pRectBlobs = &(pBlobLabel->pBlobInfo[nBlobIdx].nRectBlobs);
    INT32                   k = 0;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    if(((-90) == pBlobLabel->nDegree) || ((90) == pBlobLabel->nDegree) || ((270) == pBlobLabel->nDegree))
    {
        UINT8       *pTmpSrcMap = NULL;
        
        nLocalPos = (((pRectBlobs->x + pRectBlobs->nWidth) * 40 + pBlobLabel->pBlobInfo[nBlobIdx].nBlobCenter.x * 60) / 100);
        
        pTmpSrcMap = &(pSrcMap[nLocalPos]);
        
        for(k=pRectBlobs->nHeight ; k>0 ; k--)
        {
            *pTmpSrcMap = 0;
            pTmpSrcMap += nStride;
        }
    }
    else
    {
        nLocalPos = pRectBlobs->x + ((pRectBlobs->y * 40 + pBlobLabel->pBlobInfo[nBlobIdx].nBlobCenter.y * 60) / 100) * nStride;
        
        MEMSET(&(pSrcMap[nLocalPos]), 0, pRectBlobs->nWidth);
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _Get_BlobLabelMap(IN const BlobLabel *pBlobLabel, IN const Img_T *pSrcImg, UINT8 **ppBlobLabelMap)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    UINT32                  nAddrOffset = 0;
    
    if(IMGPROC_TRUE == pSrcImg->GetImgROIStatus(pSrcImg))
    {
        const ROI           nROI = pSrcImg->GetImgROI(pSrcImg);
        
        nAddrOffset = nROI.nStartY * pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0) + nROI.nStartX;
    }
    
    *ppBlobLabelMap = pBlobLabel->pBlobLabelMap + nAddrOffset;
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
}


static Kakao_Status _Get_VisitedPtrMap(IN const BlobLabel *pBlobLabel, IN const Img_T *pSrcImg, PtVisited **ppVisitedPtrMap)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    UINT32                  nAddrOffset = 0;
    
    if(IMGPROC_TRUE == pSrcImg->GetImgROIStatus(pSrcImg))
    {
        const ROI           nROI = pSrcImg->GetImgROI(pSrcImg);
        
        nAddrOffset = nROI.nStartY * pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0) + nROI.nStartX;
    }
    
    *ppVisitedPtrMap = &(pBlobLabel->pVisitedPtrMap[nAddrOffset]);
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
}






