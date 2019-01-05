


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
__INLINE INT32 _Pre_MBPixABS16x16(IN const UINT8 *pRef, IN const UINT8 *pSrc, IN const INT32 nStride);
__INLINE INT32 _Pre_MBPixSum16x16(IN const UINT8 *pPix, IN const INT32 nStride);
__INLINE INT32 _Pre_MBPixNormal16x16(IN const UINT8 *pPix, IN const INT32 nMean, IN const INT32 nStride);
__INLINE INT32 _FullME_Core(IN const UINT8 *pRef, IN const UINT8 *pSrc, IN INT8 *MVx, IN INT8 *MVy, IN const INT32 nStride);
__INLINE INT32 _FastME_Core(IN const UINT8 *pRef, IN const UINT8 *pSrc, IN INT8 *MVx, IN INT8 *MVy, IN const INT32 nStride);
static INT32 _Calculate_InterMAD(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, IN const INT32 nWidth, IN const INT32 nHeight, IN const INT32 nStride, OUT INT8 *pMVInfo, OUT INT32 *pInterMAD);
static INT32 _Calculate_IntraMAD(IN const UINT8 *pSrc0, IN const INT32 nWidth, IN const INT32 nHeight, IN const INT32 nStride, OUT INT32 *pIntraMAD);
static Kakao_Status _MotionSmoothing(IN SBD_T *pSBD);
static Kakao_Status _SBD_Determine_Method1(IN SBD_T *pSBD);
static Kakao_Status _SBD_ME_IntraCost_Method1(IN SBD_T *pSBD, IN const Img_T *pSrcImg);
static Kakao_Status _histogram_bin(IN const UINT8 *pImgData, OUT UINT32 *pHistogram, IN const INT32 nWidth, IN const INT32 nHeight, IN const INT32 nStride);
static Kakao_Status _histogram_intersection(IN const UINT32 *pHistogram0, IN const UINT32 *pHistogram1, OUT UINT32 *pCompResult);
static Kakao_Status _HistogramBin_ForSBD(IN SBD_T *pSBD);


/*----------------------------------------------------------------------------------------
 Static Variable
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Global Variable
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Function Implementation
 ----------------------------------------------------------------------------------------*/
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_SBD_Create(IN OUT SBD_T **ppSBD, IN const INT32 nWidth, IN const INT32 nHeight)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    UINT32                  nRealImgSize = 0;
    SBD_T                   *pTmpSBD = NULL;
    INT32                   nAlignedWidth = GET_ALIGNED_LENGTH(nWidth, MACRO_BLOCK_SIZE);
    INT32                   nAlignedHeight = GET_ALIGNED_LENGTH(nHeight, MACRO_BLOCK_SIZE);

    SAFEALLOC(pTmpSBD, 1, 32, SBD_T);
    
    Kakao_ImgProc_Util_UpdateSize(&pTmpSBD->nBaseSize, nWidth, nHeight, NULL, IMG_FORMAT_MAX, 4);
    
    nRealImgSize = nAlignedWidth * nAlignedHeight;
    
    SAFEALLOC(pTmpSBD->pMVInfoMBwise[0], (nRealImgSize >> 7), 32, INT8);
    SAFEALLOC(pTmpSBD->pMVInfoMBwise[1], (nRealImgSize >> 7), 32, INT8);

    Kakao_ImgProc_Util_CreateImg((&pTmpSBD->pRefImg[0]), nWidth, nHeight, IMG_FORMAT_I420, IMG_DEPTH_U8);
    Kakao_ImgProc_Util_CreateImg((&pTmpSBD->pRefImg[1]), nWidth, nHeight, IMG_FORMAT_I420, IMG_DEPTH_U8);
    
    SAFEALLOC(pTmpSBD->pImgAnalysis[SBD_PICTYPE_INTRA], (nRealImgSize >> 8), 32, INT32);    // ME MAD
    SAFEALLOC(pTmpSBD->pImgAnalysis[SBD_PICTYPE_INTER], (nRealImgSize >> 8), 32, INT32);    // Intra Complexity

    pTmpSBD->nFirstFrameFlag = 0;
    pTmpSBD->nCurrImgIdx = 0;
    pTmpSBD->nFrameCount = 0;
    pTmpSBD->nSBDWindowSize = 15;
    pTmpSBD->nSBDCurrWindowPos = 0;
    pTmpSBD->nNeighborIntra = 0;
    
    *ppSBD = pTmpSBD;

    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    Kakao_ImgProc_SBD_Destroy(&pTmpSBD);
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_SBD_Destroy(IN OUT SBD_T **ppSBD)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    CHECK_POINTER_VALIDATION((*ppSBD), KAKAO_STAT_NULL_POINTER)
    
    SAFEFREE((*ppSBD)->pMVInfoMBwise[0]);
    SAFEFREE((*ppSBD)->pMVInfoMBwise[1]);

    Kakao_ImgProc_Util_ReleaseImg(&((*ppSBD)->pRefImg[0]));
    Kakao_ImgProc_Util_ReleaseImg(&((*ppSBD)->pRefImg[1]));
    
    SAFEFREE((*ppSBD)->pImgAnalysis[0]);
    SAFEFREE((*ppSBD)->pImgAnalysis[1]);

    SAFEFREE((*ppSBD));
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;

Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_SBD_UpdateImgSize(IN OUT SBD_T *pSBD, IN const INT32 nWidth, IN const INT32 nHeight)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pSBD, KAKAO_STAT_NULL_POINTER);
    
    Kakao_ImgProc_Util_UpdateSize(&pSBD->nBaseSize, nWidth, nHeight, NULL, IMG_FORMAT_MAX, 4);    
    Kakao_ImgProc_Util_UpdateImgSize(pSBD->pRefImg[0], nWidth, nHeight, NULL, IMG_FORMAT_I420);
    Kakao_ImgProc_Util_UpdateImgSize(pSBD->pRefImg[1], nWidth, nHeight, NULL, IMG_FORMAT_I420);
    
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_SBD_FindSBD(IN SBD_T *pSBD, IN Img_T *pSrcImg, OUT IMGPROC_BOOL *pbForceIntraFlag)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pSBD, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pbForceIntraFlag, KAKAO_STAT_NULL_POINTER);

    ___START_PROFILE(SBDetect)
    
    // Check Shot Boundary Detection module
    _SBD_ME_IntraCost_Method1(pSBD, pSrcImg);
    
    *pbForceIntraFlag = pSBD->bForceIntraFlag;
    
    ___STOP_PROFILE(SBDetect)
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


__INLINE INT32 _Pre_MBPixABS16x16(IN const UINT8 *pRef, IN const UINT8 *pSrc, IN const INT32 nStride)
{
    INT32                  nSum = 0;
    INT32                   i = 0;

    for(i=15 ; i>=0 ;i--)
    {
        nSum += abs(pRef[0] - pSrc[0]);
        nSum += abs(pRef[1] - pSrc[1]);
        nSum += abs(pRef[2] - pSrc[2]);
        nSum += abs(pRef[3] - pSrc[3]);
        nSum += abs(pRef[4] - pSrc[4]);
        nSum += abs(pRef[5] - pSrc[5]);
        nSum += abs(pRef[6] - pSrc[6]);
        nSum += abs(pRef[7] - pSrc[7]);
        nSum += abs(pRef[8] - pSrc[8]);
        nSum += abs(pRef[9] - pSrc[9]);
        nSum += abs(pRef[10] - pSrc[10]);
        nSum += abs(pRef[11] - pSrc[11]);
        nSum += abs(pRef[12] - pSrc[12]);
        nSum += abs(pRef[13] - pSrc[13]);
        nSum += abs(pRef[14] - pSrc[14]);
        nSum += abs(pRef[15] - pSrc[15]);
        pRef += nStride;
        pSrc += nStride;
    }
    
    return nSum;
}


__INLINE INT32 _Pre_MBPixSum16x16(IN const UINT8 *pPix, IN const INT32 nStride)
{
    INT32                  nSum = 0, i = 0, j = 0;
    
    for(i=0 ; i<16 ; i++)
    {
        for(j=0 ; j<16 ; j+= 8)
        {
            nSum += pPix[0];
            nSum += pPix[1];
            nSum += pPix[2];
            nSum += pPix[3];
            nSum += pPix[4];
            nSum += pPix[5];
            nSum += pPix[6];
            nSum += pPix[7];
            
            pPix += 8;
        }
        pPix += nStride - 16;
    }
    
    return nSum;
}


__INLINE INT32 _Pre_MBPixNormal16x16(IN const UINT8 *pPix, IN const INT32 nMean, IN const INT32 nStride)
{
    INT32                  nSum = 0, i = 0, j = 0;
    
    for(i=0 ; i<16 ; i++)
    {
        for(j=0 ; j<16 ; j+= 8)
        {
            nSum += ABSM(pPix[0] - nMean);
            nSum += ABSM(pPix[1] - nMean);
            nSum += ABSM(pPix[2] - nMean);
            nSum += ABSM(pPix[3] - nMean);
            nSum += ABSM(pPix[4] - nMean);
            nSum += ABSM(pPix[5] - nMean);
            nSum += ABSM(pPix[6] - nMean);
            nSum += ABSM(pPix[7] - nMean);
            pPix += 8;
        }
        pPix += nStride - 16;
    }
    
    return nSum;
}


__INLINE INT32 _FullME_Core(IN const UINT8 *pRef, IN const UINT8 *pSrc, IN INT8 *MVx, IN INT8 *MVy, IN const INT32 nStride)
{
    INT32                   m = 0, n = 0;
    INT32                   nMinMAD = 99999999, nCurrMAD = 0;
    INT32                   nSearchRange = (MACRO_BLOCK_SIZE >> 1);
    
    *MVx = *MVy = 0;

    for(n=-nSearchRange ; n<=nSearchRange ; n++)
    {
        for(m=-nSearchRange ; m<=nSearchRange ; m++)
        {
            // MacroBlock
            nCurrMAD = _Pre_MBPixABS16x16(pRef, (const UINT8 *)(&(pSrc[(n * nStride) + m])), nStride);

            if((nCurrMAD < nMinMAD) ||
                ((nCurrMAD == nMinMAD) && (ABSM(m) + ABSM(n)) <  (ABSM(*MVx) + ABSM(*MVy))))
            {
                nMinMAD = nCurrMAD;
                *MVx = m;
                *MVy = n;                        
            }
        }    
    }   
    
    return nMinMAD;
}


__INLINE INT32 _FastME_Core(IN const UINT8 *pRef, IN const UINT8 *pSrc, IN INT8 *MVx, IN INT8 *MVy, IN const INT32 nStride)
{
    INT32                   m = 0, n = 0;
    INT32                   nMinMAD = 99999999, nCurrMAD = 0;
    INT32                   nSearchRange = (MACRO_BLOCK_SIZE);
    INT32                   nSearchRangeX0, nSearchRangeX1, nSearchRangeY0, nSearchRangeY1;
    INT32                   nSearchStep = nSearchRange;

    nSearchRangeX0 = nSearchRangeY0 = -nSearchRange;
    nSearchRangeX1 = nSearchRangeY1 = nSearchRange;    
    *MVx = *MVy = 0;

    do
    {
        for(n=nSearchRangeX0 ; n<=nSearchRangeX1 ; n+=nSearchStep)
        {
            for(m=nSearchRangeY0 ; m<=nSearchRangeY1 ; m+=nSearchStep)
            {
                // MacroBlock
                nCurrMAD = _Pre_MBPixABS16x16(pRef, (const UINT8 *)(&(pSrc[(n * nStride) + m])), nStride);

                if((nCurrMAD < nMinMAD) ||
                    ((nCurrMAD == nMinMAD) && (ABSM(m) + ABSM(n)) <  (ABSM(*MVx) + ABSM(*MVy))))
                {
                    nMinMAD = nCurrMAD;
                    *MVx = m;
                    *MVy = n;
                }
            }    
        }

        nSearchStep = nSearchStep >> 1;

        nSearchRangeX0 = *MVx - nSearchStep;
        nSearchRangeX1 = *MVx + nSearchStep;        
        nSearchRangeY0 = *MVy - nSearchStep;
        nSearchRangeY1 = *MVy + nSearchStep;        
        
    } while (nSearchStep >= 1);

    return nMinMAD;
}


static INT32 _Calculate_InterMAD(IN const UINT8 *pPrevSrc, IN const UINT8 *pCurrSrc, IN const INT32 nWidth, IN const INT32 nHeight, IN const INT32 nStride, OUT INT8 *pMVInfo, OUT INT32 *pInterMAD)
{
    INT32                   nMBPos = 0;
    INT32                   x = 0, y = 0;
    INT32                   nMinMAD;
    INT32                   nSumofMAD= 0;
    const UINT8             *pMERef = NULL;
    const UINT8             *pMECurrMB = NULL;
    INT8                    nMVx = 0, nMVy = 0;

    for(y=0 ; y<nHeight ; y+=MACRO_BLOCK_SIZE)
    {
        for(x=0 ; x<nWidth ; x+=MACRO_BLOCK_SIZE)
        {
            pMERef = &pPrevSrc[(y * nStride) + x];
            pMECurrMB = &pCurrSrc[(y * nStride) + x];
            
            //nMinMAD = _FastME_Core(pMERef, pCurrSrc, x, y, &MVx, &MVy, nWidth);
            nMinMAD = _FullME_Core((const UINT8 *)pMERef, (const UINT8 *)pMECurrMB, &nMVx, &nMVy, nStride);
            
            nSumofMAD += nMinMAD;
            
            pInterMAD[nMBPos] = nMinMAD;
            pMVInfo[2 * nMBPos] = nMVx;
            pMVInfo[2 * nMBPos+1] = nMVy;

            nMBPos++;
        }
    }

    return nSumofMAD;
}


static INT32 _Calculate_IntraMAD(IN const UINT8 *pSrc0, IN const INT32 nWidth, IN const INT32 nHeight, IN const INT32 nStride, OUT INT32 *pIntraMAD)
{
    INT32                   nMBPos = 0;
    INT32                   x = 0, y = 0;
    UINT32                  nMean = 0;
    INT32                   nSumofMAD = 0;
    const UINT8             *pTmpSrc = NULL;
    
    for(y=0 ; y<nHeight ; y+=MACRO_BLOCK_SIZE)
    {
        for(x=0 ; x<nWidth ; x+=MACRO_BLOCK_SIZE)
        {
            pTmpSrc = &(pSrc0[(y * nStride) + x]);
            
            // Avg of CurrMB
            nMean = _Pre_MBPixSum16x16(pTmpSrc, (const UINT32)nStride);
            nMean >>= 8;   // Divided by 256 (MACRO_BLOCK_SIZE * MACRO_BLOCK_SIZE)

            // Complexity of CurrMB
            pIntraMAD[nMBPos] = _Pre_MBPixNormal16x16(pTmpSrc, (const INT32)nMean, (const UINT32)nStride);

            nSumofMAD += (pIntraMAD[nMBPos++] >> 8);
        }
    }

    return nSumofMAD;
}


static Kakao_Status _MotionSmoothing(IN SBD_T *pSBD)
{
    INT32                   x = 0, y = 0;
    INT32                   nMVPos = 0;
    Img_T                   *pSrcImg = pSBD->pRefImg[pSBD->nCurrImgIdx];
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0) >> 4;
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0) >> 4;
    INT32                   nMVWidth = 2 * nWidth;
    INT8                    *pCurrMVInfo = NULL;
    INT8                    *pPrevMVInfo = NULL;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    pCurrMVInfo = pSBD->pMVInfoMBwise[pSBD->nCurrImgIdx];
    pPrevMVInfo = pSBD->pMVInfoMBwise[pSBD->nPrevImgIdx];
    
    nMVPos =  nMVWidth + 2;
    for(y=1 ; y<nHeight-1 ; y++)
    {
        for(x=2 ; x<nMVWidth-2 ; x+=2)
        {            
            // Do not change MV value of MB where there is no motion
            if((0 != pCurrMVInfo[nMVPos]) || (0 != pCurrMVInfo[nMVPos+1]))
            {
                // X
                pPrevMVInfo[nMVPos] = (INT32)((F32)(pCurrMVInfo[nMVPos - nMVWidth - 2] + pCurrMVInfo[nMVPos - nMVWidth] + pCurrMVInfo[nMVPos - nMVWidth + 2]
                                              + pCurrMVInfo[nMVPos - 2] + pCurrMVInfo[nMVPos] + pCurrMVInfo[nMVPos + 2]
                                              + pCurrMVInfo[nMVPos + nMVWidth - 2] + pCurrMVInfo[nMVPos + nMVWidth] + pCurrMVInfo[nMVPos + nMVWidth + 2]) / (F32)9);
                nMVPos++;
                
                // Y
                pPrevMVInfo[nMVPos] = (INT32)((F32)(pCurrMVInfo[nMVPos - nMVWidth - 2] + pCurrMVInfo[nMVPos - nMVWidth] + pCurrMVInfo[nMVPos - nMVWidth + 2]
                                              + pCurrMVInfo[nMVPos - 2] + pCurrMVInfo[nMVPos] + pCurrMVInfo[nMVPos + 2]
                                              + pCurrMVInfo[nMVPos + nMVWidth - 2] + pCurrMVInfo[nMVPos + nMVWidth] + pCurrMVInfo[nMVPos + nMVWidth + 2]) / (F32)9);
                nMVPos++;
            }
            else
            {
                pPrevMVInfo[nMVPos] = 0;
                pPrevMVInfo[nMVPos+1] = 0;
                nMVPos += 2;
            }
        }
    }


    nMVPos =  nMVWidth + 2;
    for(y=1 ; y<nHeight-1 ; y++)
    {
        for(x=2 ; x<nMVWidth-2 ; x+=2)
        {            
            // Do not change MV value of MB where there is no motion
            if((0 != pPrevMVInfo[nMVPos]) || (0 != pPrevMVInfo[nMVPos+1]))
            {
                // X
                pCurrMVInfo[nMVPos] = (INT32)(((F32)pPrevMVInfo[nMVPos - nMVWidth - 2] + (F32)pPrevMVInfo[nMVPos - nMVWidth + 2]
                                        + (F32)pPrevMVInfo[nMVPos] + (F32)pPrevMVInfo[nMVPos + nMVWidth - 2] + (F32)pPrevMVInfo[nMVPos + nMVWidth + 2]) / (F32)5);
                nMVPos++;
                
                // Y
                pCurrMVInfo[nMVPos] = (INT32)(((F32)pPrevMVInfo[nMVPos - nMVWidth - 2] + (F32)pPrevMVInfo[nMVPos - nMVWidth + 2]
                                        + (F32)pPrevMVInfo[nMVPos] + (F32)pPrevMVInfo[nMVPos + nMVWidth - 2] + (F32)pPrevMVInfo[nMVPos + nMVWidth + 2]) / (F32)5);
                nMVPos++;
            }
            else
            {
                pCurrMVInfo[nMVPos] = 0;
                pCurrMVInfo[nMVPos+1] = 0;
                nMVPos += 2;
            }            
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _SBD_Determine_Method1(IN SBD_T *pSBD)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    if(1 == pSBD->nFirstFrameFlag)
    {
        if(pSBD->nSBDWindowSize <= pSBD->nFrameCount)
        {
            INT32           i = 0, j = 0;
            INT32           nPrevMAD = pSBD->nSumofMAD[0][pSBD->nSBDPrevWindowPos];
            INT32           nCurrMAD = pSBD->nSumofMAD[0][pSBD->nSBDCurrWindowPos];
            F32             fAvgMAD, fMADRatio, fPrevCurrRatio;

            fAvgMAD = 0.0f;
            i = pSBD->nSBDCurrWindowPos;
            j = 0;
            do
            {
                fAvgMAD += pSBD->nSumofMAD[0][(i+1)%(pSBD->nSBDWindowSize)];
                i++;
                j++;                    
            }while(j<pSBD->nSBDWindowSize-1);
            fAvgMAD /= (pSBD->nSBDWindowSize-1);

            if(200<nCurrMAD)
            {
                fMADRatio = (F32)nCurrMAD / (F32)fAvgMAD;
                fPrevCurrRatio = (F32)nCurrMAD / (F32)nPrevMAD;
            }
            else
            {
                fMADRatio = 1.0f;
                fPrevCurrRatio = 1.0f;
            }

            if(((fMADRatio>2) || (fMADRatio<0.2)) && ((fPrevCurrRatio>2) || (fPrevCurrRatio<0.2)))
            {
                if(((F32)pSBD->nHistoDiff[0] / (F32)pSBD->nHistoDiff[1]) >= 1.05)
                    pSBD->bForceIntraFlag = IMGPROC_FALSE;
                else
                    pSBD->bForceIntraFlag = IMGPROC_TRUE;
            }
            else
            {
                if(((F32)pSBD->nHistoDiff[0] / (F32)pSBD->nHistoDiff[1]) >= 1.1)
                    pSBD->bForceIntraFlag = IMGPROC_FALSE;
                else
                    pSBD->bForceIntraFlag = IMGPROC_TRUE;
            }
        }
        else
        {
            pSBD->bForceIntraFlag = IMGPROC_TRUE;
        }
            
    }
    else
    {
        pSBD->nSumofMAD[0][pSBD->nSBDCurrWindowPos] = 0;
        pSBD->bForceIntraFlag = IMGPROC_FALSE;
    }


    if(IMGPROC_FALSE == pSBD->bForceIntraFlag)
        pSBD->nNeighborIntra = 0;
    else
        pSBD->nNeighborIntra++;

    pSBD->nSBDPrevWindowPos = pSBD->nSBDCurrWindowPos;
    pSBD->nSBDCurrWindowPos++;
    if(pSBD->nSBDCurrWindowPos >= pSBD->nSBDWindowSize)
        pSBD->nSBDCurrWindowPos = 0;
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _SBD_ME_IntraCost_Method1(IN SBD_T *pSBD, IN const Img_T *pSrcImg)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    UINT32                  nPrevImgIdx = pSBD->nPrevImgIdx;
    UINT32                  nCurrImgIdx = pSBD->nCurrImgIdx;
    Img_T                   *pPrevRefImg = pSBD->pRefImg[nPrevImgIdx];
    Img_T                   *pCurrRefImg = pSBD->pRefImg[nCurrImgIdx];

    if(1 == pSBD->nFirstFrameFlag)
    {
        // Resize input image to use input of ME
        //Resize_Bilinear(pSrc->pFrame_Y, &SBD->pMERefImg[((nCurrImgIdx+1)%2)][0], &pSrc->nYSize, &pDst->nYSize);               // For Halfpel ME
        Kakao_ImgProc_Util_CopyImg(pCurrRefImg, pSrcImg);
        
        _HistogramBin_ForSBD(pSBD);
        
        Kakao_ImgProc_Util_Padding_ImgPlane(pCurrRefImg);
        
        // Motion Estimate and get MV information
        // Calculate InterMAD        
        pSBD->nSumofMAD[SBD_PICTYPE_INTER][pSBD->nSBDCurrWindowPos] = _Calculate_InterMAD((const UINT8 *)pPrevRefImg->GetImgPlanes(pPrevRefImg, IMG_PLANE_0),
                                                                                          (const UINT8 *)pCurrRefImg->GetImgPlanes(pCurrRefImg, IMG_PLANE_0),
                                                                                          (const INT32)(pCurrRefImg->GetImgWidth(pCurrRefImg, IMG_PLANE_0)),
                                                                                          (const INT32)(pCurrRefImg->GetImgHeight(pCurrRefImg, IMG_PLANE_0)),
                                                                                          (const INT32)(pCurrRefImg->GetImgStride(pCurrRefImg, IMG_PLANE_0)),
                                                                                          pSBD->pMVInfoMBwise[nCurrImgIdx],
                                                                                          &pSBD->pImgAnalysis[SBD_PICTYPE_INTER][0]);

        // Calculate IntraMAD
        pSBD->nSumofMAD[SBD_PICTYPE_INTRA][pSBD->nSBDCurrWindowPos] = _Calculate_IntraMAD((const UINT8 *)pCurrRefImg->GetImgPlanes(pCurrRefImg, IMG_PLANE_0),
                                                                                          (const INT32)(pCurrRefImg->GetImgWidth(pCurrRefImg, IMG_PLANE_0)),
                                                                                          (const INT32)(pCurrRefImg->GetImgHeight(pCurrRefImg, IMG_PLANE_0)),
                                                                                          (const INT32)(pCurrRefImg->GetImgStride(pCurrRefImg, IMG_PLANE_0)),
                                                                                          &(pSBD->pImgAnalysis[SBD_PICTYPE_INTRA][0]));

        // Smoothing MV Information
        _MotionSmoothing(pSBD);

        // Determine SBD
        _SBD_Determine_Method1(pSBD);

        pSBD->nPrevImgIdx = nCurrImgIdx;
        pSBD->nCurrImgIdx = (nCurrImgIdx + 1) & 0x01;
        pSBD->nFrameCount++;
    }
    else
    {
        // Resize input image to use input of ME
        //Resize_Bilinear(pSrc->pFrame_Y, &SBD->pMERefImg[nCurrImgIdx][0], &pSrc->nYSize, &pDst->nYSize);          // For Halfpel ME
        Kakao_ImgProc_Util_CopyImg(pSBD->pRefImg[nCurrImgIdx], pSrcImg);

        _HistogramBin_ForSBD(pSBD);
        
        Kakao_ImgProc_Util_Padding_ImgPlane(pSBD->pRefImg[nCurrImgIdx]);
        
        // Calculate InterMAD
        pSBD->nSumofMAD[SBD_PICTYPE_INTER][pSBD->nSBDCurrWindowPos] = 0;

        // Calculate IntraMAD
        pSBD->nSumofMAD[SBD_PICTYPE_INTRA][pSBD->nSBDCurrWindowPos] = _Calculate_IntraMAD((const UINT8 *)pCurrRefImg->GetImgPlanes(pCurrRefImg, IMG_PLANE_0),
                                                                                          (const INT32)(pCurrRefImg->GetImgWidth(pCurrRefImg, IMG_PLANE_0)),
                                                                                          (const INT32)(pCurrRefImg->GetImgHeight(pCurrRefImg, IMG_PLANE_0)),
                                                                                          (const INT32)(pCurrRefImg->GetImgStride(pCurrRefImg, IMG_PLANE_0)),
                                                                                          &(pSBD->pImgAnalysis[SBD_PICTYPE_INTRA][0]));

        // Determine SBD
        _SBD_Determine_Method1(pSBD);

        pSBD->nPrevImgIdx = nCurrImgIdx;
        pSBD->nCurrImgIdx = (nCurrImgIdx + 1) & 0x01;
        pSBD->nFirstFrameFlag = 1;
        pSBD->nFrameCount++;
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _histogram_bin(IN const UINT8 *pImgData, OUT UINT32 *pHistogram, IN const INT32 nWidth, IN const INT32 nHeight, IN const INT32 nStride)
{
    INT32                   i = 0,  j = 0;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    for(j=0 ; j<nHeight ; j++)
    {
        for(i=0 ; i<nWidth ; i++)
            pHistogram[(pImgData[i] / BIN)]++;
        
        pImgData += nStride;
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _histogram_intersection(IN const UINT32 *pHistogram0, IN const UINT32 *pHistogram1, OUT UINT32 *pCompResult)
{
    INT32                   i = 0;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    *pCompResult = 0;

    for(i=BIN-1 ; i>=0 ; i--)
        (*pCompResult) += GET_MIN(pHistogram0[i], pHistogram1[i]);
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _HistogramBin_ForSBD(IN SBD_T *pSBD)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    const Img_T             *pCurrRefImg = pSBD->pRefImg[pSBD->nCurrImgIdx];

    pSBD->nHistoDiff[pSBD->nPrevImgIdx] = pSBD->nHistoDiff[pSBD->nCurrImgIdx];
    
    MEMSET((void *)(&(pSBD->nHistogram[pSBD->nCurrImgIdx][0])), 0, (BIN+1)*sizeof(UINT32));
    
    _histogram_bin((const UINT8 *)(pCurrRefImg->GetImgPlanes(pCurrRefImg, IMG_PLANE_0)),
                   &(pSBD->nHistogram[pSBD->nCurrImgIdx][0]),
                   (const INT32)(pCurrRefImg->GetImgWidth(pCurrRefImg, IMG_PLANE_0)),
                   (const INT32)(pCurrRefImg->GetImgHeight(pCurrRefImg, IMG_PLANE_0)),
                   (const INT32)(pCurrRefImg->GetImgStride(pCurrRefImg, IMG_PLANE_0)));

    _histogram_intersection((const UINT32 *)(&(pSBD->nHistogram[pSBD->nPrevImgIdx][0])),
                            (const UINT32 *)(&(pSBD->nHistogram[pSBD->nCurrImgIdx][0])),
                            &pSBD->nHistoDiff[pSBD->nCurrImgIdx]);

    nRet = KAKAO_STAT_OK;
    return nRet;
}







