


/*----------------------------------------------------------------------------------------
 File Inclusions
 ----------------------------------------------------------------------------------------*/
#include "Kakao_ImgProc_Modules.h"
#include "Kakao_ImgProc_SIMD.h"
#include "test_sample1_640x480.h"
#include "test_sample2_640x480.h"


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
static Kakao_Status _ExtractBG_PostProcImg(IN ExtractBG_T *pExtractBG, IN Img_T *pAccumImg, IN Img_T *pSubtratcImg, IN Img_T *pOrgImg, IN Img_T *pSrcImg, UINT32 nMaxLoop);
static Kakao_Status _ExtractBG_GetBGMask(IN ExtractBG_T *pExtractBG);
static Kakao_Status _ExtractBG_Replace_BackGround(IN ExtractBG_T *pExtractBG, Img_T *pSrcImg);
static Kakao_Status _ExtractBG_Insert_SampleImg_to_ImgT(IN OUT Img_T * const pDstImg, IN const UINT8 *pSrcBuf, IN INT32 nWidth, IN INT32 nHeight);


/*----------------------------------------------------------------------------------------
 Static Variable
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Global Variable
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Function Implementation
 ----------------------------------------------------------------------------------------*/
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_ExtractBG_Create(IN OUT ExtractBG_T **ppExtractBG, IN const INT32 nWidth, IN const INT32 nHeight)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    ExtractBG_T             *pTmpExtractBG = NULL;
    INT32                   nAlignedWidth = GET_ALIGNED_LENGTH(nWidth, MACRO_BLOCK_SIZE);
    
    SAFEALLOC(pTmpExtractBG, 1, 32, ExtractBG_T);
    
    Kakao_ImgProc_Util_UpdateSize(&pTmpExtractBG->nBaseSize, nWidth, nHeight, NULL, IMG_FORMAT_MAX, 4);
    
    Kakao_ImgProc_Util_CreateImg(&(pTmpExtractBG->pLatestBGImg), nWidth, nHeight, IMG_FORMAT_GRAY, IMG_DEPTH_U8);
    Kakao_ImgProc_Util_CreateImg(&(pTmpExtractBG->pPrevImg), nWidth, nHeight, IMG_FORMAT_GRAY, IMG_DEPTH_U8);
    Kakao_ImgProc_Util_CreateImg(&(pTmpExtractBG->pAccImg0), nWidth, nHeight, IMG_FORMAT_GRAY, IMG_DEPTH_U32);
    Kakao_ImgProc_Util_CreateImg(&(pTmpExtractBG->pAccImg1), nWidth, nHeight, IMG_FORMAT_GRAY, IMG_DEPTH_U32);
    Kakao_ImgProc_Util_CreateImg(&(pTmpExtractBG->pFirstDiffImg), nWidth, nHeight, IMG_FORMAT_GRAY, IMG_DEPTH_U32);
    Kakao_ImgProc_Util_CreateImg(&(pTmpExtractBG->pPrevDiffImg), nWidth, nHeight, IMG_FORMAT_GRAY, IMG_DEPTH_U32);
    Kakao_ImgProc_Util_CreateImg(&(pTmpExtractBG->pFirstDiffAccImg), nWidth, nHeight, IMG_FORMAT_GRAY, IMG_DEPTH_U32);
    Kakao_ImgProc_Util_CreateImg(&(pTmpExtractBG->pPrevDiffAccImg), nWidth, nHeight, IMG_FORMAT_GRAY, IMG_DEPTH_U32);
    Kakao_ImgProc_Util_CreateImg(&(pTmpExtractBG->pDiffThAccImg), nWidth, nHeight, IMG_FORMAT_GRAY, IMG_DEPTH_U32);
    Kakao_ImgProc_Util_CreateImg(&(pTmpExtractBG->pBackGroundImg), nWidth, nHeight, IMG_FORMAT_I420, IMG_DEPTH_U32);
    Kakao_ImgProc_Filter_Create(&(pTmpExtractBG->pFilter), nWidth, nHeight);
    
    *ppExtractBG = pTmpExtractBG;
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    Kakao_ImgProc_ExtractBG_Destroy(&pTmpExtractBG);
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_ExtractBG_Destroy(IN OUT ExtractBG_T **ppExtractBG)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION((*ppExtractBG), KAKAO_STAT_NULL_POINTER);
    
    {
        ExtractBG_T             *pExtractBG = *ppExtractBG;
        
        Kakao_ImgProc_Util_ReleaseImg(&(pExtractBG->pLatestBGImg));
        Kakao_ImgProc_Util_ReleaseImg(&(pExtractBG->pPrevImg));
        Kakao_ImgProc_Util_ReleaseImg(&(pExtractBG->pAccImg0));
        Kakao_ImgProc_Util_ReleaseImg(&(pExtractBG->pAccImg1));
        Kakao_ImgProc_Util_ReleaseImg(&(pExtractBG->pFirstDiffImg));
        Kakao_ImgProc_Util_ReleaseImg(&(pExtractBG->pPrevDiffImg));
        Kakao_ImgProc_Util_ReleaseImg(&(pExtractBG->pFirstDiffAccImg));
        Kakao_ImgProc_Util_ReleaseImg(&(pExtractBG->pPrevDiffAccImg));
        Kakao_ImgProc_Util_ReleaseImg(&(pExtractBG->pDiffThAccImg));
        Kakao_ImgProc_Util_ReleaseImg(&(pExtractBG->pBackGroundImg));
        Kakao_ImgProc_Filter_Destroy(&(pExtractBG->pFilter));

        SAFEFREE((*ppExtractBG));
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_ExtractBG_UpdateImgSize(IN OUT ExtractBG_T *pExtractBG, IN const INT32 nWidth, IN const INT32 nHeight)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pExtractBG, KAKAO_STAT_NULL_POINTER)
    
    Kakao_ImgProc_Util_UpdateSize(&pExtractBG->nBaseSize, nWidth, nHeight, NULL, IMG_FORMAT_MAX, 4);
    Kakao_ImgProc_Util_UpdateImgSize(pExtractBG->pLatestBGImg, nWidth, nHeight, NULL, IMG_FORMAT_GRAY);
    Kakao_ImgProc_Util_UpdateImgSize(pExtractBG->pPrevImg, nWidth, nHeight, NULL, IMG_FORMAT_GRAY);
    Kakao_ImgProc_Util_UpdateImgSize(pExtractBG->pAccImg0, nWidth, nHeight, NULL, IMG_FORMAT_GRAY);
    Kakao_ImgProc_Util_UpdateImgSize(pExtractBG->pAccImg1, nWidth, nHeight, NULL, IMG_FORMAT_GRAY);
    Kakao_ImgProc_Util_UpdateImgSize(pExtractBG->pFirstDiffImg, nWidth, nHeight, NULL, IMG_FORMAT_GRAY);
    Kakao_ImgProc_Util_UpdateImgSize(pExtractBG->pPrevDiffImg, nWidth, nHeight, NULL, IMG_FORMAT_GRAY);
    Kakao_ImgProc_Util_UpdateImgSize(pExtractBG->pFirstDiffAccImg, nWidth, nHeight, NULL, IMG_FORMAT_GRAY);
    Kakao_ImgProc_Util_UpdateImgSize(pExtractBG->pPrevDiffAccImg, nWidth, nHeight, NULL, IMG_FORMAT_GRAY);
    Kakao_ImgProc_Util_UpdateImgSize(pExtractBG->pBackGroundImg, nWidth, nHeight, NULL, IMG_FORMAT_I420);
    
    Kakao_ImgProc_Filter_UpdateImgSize(pExtractBG->pFilter, nWidth, nHeight);
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_ExtractBG_GetBG(IN ExtractBG_T *pExtractBG, IN Img_T *pSrcImg)
{
    Kakao_Status            nRet = KAKAO_STAT_NOT_READY;
    
    CHECK_POINTER_VALIDATION(pExtractBG, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER)
    
    {
        static UINT32           nLoopCount = 0;
        UINT32                  nMaxLoop = 60;
        UINT32                  nCopyImgSize = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0) * pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
        
        if(nMaxLoop < nLoopCount)
            goto SkipProc;
        
        //Kakao_ImgProc_Filter_NormalizeByMean(pSrcImg);
        
        // Get FirstImg & PreviousImg
        if(0 == nLoopCount)
        {
            MEMCPY((HANDLE)((UINT8 *)pExtractBG->pLatestBGImg->GetImgPlanes(pExtractBG->pLatestBGImg, IMG_PLANE_0) - PADDING_SIZE),
                (HANDLE)((UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0) - PADDING_SIZE), nCopyImgSize);
            MEMCPY((HANDLE)((UINT8 *)pExtractBG->pPrevImg->GetImgPlanes(pExtractBG->pPrevImg, IMG_PLANE_0) - PADDING_SIZE),
                (HANDLE)((UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0) - PADDING_SIZE), nCopyImgSize);
            
            nLoopCount++;
            
            return KAKAO_STAT_NOT_READY;
        }
        else if(nMaxLoop > nLoopCount)
        {
            // Make BackGround Mask
            
            // Make Reference Img for BG Extraction
            Kakao_ImgProc_Filter_AccumImg(pExtractBG->pFilter, pExtractBG->pAccImg0, pSrcImg);
            //Kakao_ImgProc_Filter_AccumImg(pExtractBG->pFilter, pExtractBG->pAccImg1, pSrcImg);
            Kakao_ImgProc_Util_SubtractAccImg(pExtractBG->pFirstDiffImg, pExtractBG->pLatestBGImg, pSrcImg);
            //Kakao_ImgProc_Util_SubtractAccImg(pExtractBG->pPrevDiffImg, pExtractBG->pPrevImg, pSrcImg);
            //Kakao_ImgProc_Util_SubtractAccImg(pExtractBG->pFirstDiffAccImg, pExtractBG->pLatestBGImg, pSrcImg);
            //Kakao_ImgProc_Util_SubtractAccImg(pExtractBG->pPrevDiffAccImg, pExtractBG->pPrevImg, pSrcImg);
            //Kakao_ImgProc_Util_SubtractAccbyThImg(pExtractBG->pDiffThAccImg, pExtractBG->pLatestBGImg, pSrcImg, nTh);
            
            // Backup Previous Img
            MEMCPY((HANDLE)(((UINT8 *)(pExtractBG->pPrevImg->GetImgPlanes(pExtractBG->pPrevImg, IMG_PLANE_0)) - PADDING_SIZE)), 
                   (HANDLE)(((UINT8 *)(pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0)) - PADDING_SIZE)), nCopyImgSize);
        }
        else if(nMaxLoop == nLoopCount)
        {
            //Kakao_ImgProc_Util_Div(pExtractBG->pAccImg0, IMG_BINARYMAP_SKIN, 59);
            //Kakao_ImgProc_Util_Div(pExtractBG->pFirstDiffImg, IMG_BINARYMAP_SKIN, 59);
            _ExtractBG_GetBGMask(pExtractBG);
        }
        
        nLoopCount++;
        
    SkipProc:
        if(nMaxLoop <= nLoopCount)
        {
            _ExtractBG_Replace_BackGround(pExtractBG, pSrcImg);

            return KAKAO_STAT_READY;
        }
    }
    
    nRet = KAKAO_STAT_NOT_READY;
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _ExtractBG_PostProcImg(IN ExtractBG_T *pExtractBG, IN Img_T *pAccumImg, IN Img_T *pSubtratcImg, IN Img_T *pOrgImg, IN Img_T *pSrcImg, UINT32 nMaxLoop)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pSubtratcImg, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pAccumImg, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pOrgImg, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER)
    
    {
        INT32                   i = 0, j = 0;
        INT32                   nLocalPos = 0;
        const INT32             nWidth = pSubtratcImg->GetImgWidth(pSubtratcImg, IMG_PLANE_0);
        const INT32             nHeight = pSubtratcImg->GetImgHeight(pSubtratcImg, IMG_PLANE_0);
        const INT32             nStride = pSubtratcImg->GetImgStride(pSubtratcImg, IMG_PLANE_0);
        const UINT8             *pAcc = (UINT8 *)pAccumImg->GetImgPlanes(pAccumImg, IMG_PLANE_0);
        const UINT8             *pSub = (UINT8 *)pSubtratcImg->GetImgPlanes(pSubtratcImg, IMG_PLANE_0);
        const UINT8             *pOrg = (UINT8 *)pOrgImg->GetImgPlanes(pOrgImg, IMG_PLANE_0);
        UINT8                   *pDst = NULL;
        UINT8                   *pBackGroundMap = NULL;
        
        CHECK_POINTER_VALIDATION(pAcc, KAKAO_STAT_NULL_POINTER)
        CHECK_POINTER_VALIDATION(pSub, KAKAO_STAT_NULL_POINTER)
        CHECK_POINTER_VALIDATION(pOrg, KAKAO_STAT_NULL_POINTER)
        CHECK_POINTER_VALIDATION(pDst, KAKAO_STAT_NULL_POINTER)

        if(NULL == (pBackGroundMap = pOrgImg->GetImgBinaryMap(pOrgImg, IMG_BINARYMAP_BACKGND)))
            return nRet;
        
        pDst = pBackGroundMap;
        for(j=0 ; j<nHeight ; j++)
        {
            nLocalPos = j * nStride;
            
            for(i=0 ; i<nWidth ; i++)
            {
                //INT32              nAcc = (INT32)pAcc[nLocalPos];
                INT32              nSub = (INT32)pSub[nLocalPos];
                //INT32              nOrg = (INT32)pOrg[nLocalPos];
                //INT32              nAccDiv = nAcc / nMaxLoop;
                INT32              nSubDiv = nSub / nMaxLoop;
                //INT32              nLocalValAcc = (nAccDiv - nOrg) >= 0 ? (nAccDiv - nOrg) : ABSM(nAccDiv - nOrg);
                //INT32              nLocalValSub = (nSubDiv - nOrg) >= 0 ? (nSubDiv - nOrg) : ABSM(nSubDiv - nOrg);
                
                //pDst[nLocalPos] = (UINT8)((nLocalValSub < 20) || (nLocalValAcc < 20) ? 255 : 0);
                //pDst[nLocalPos] = (UINT8)((nLocalValAcc < 20) ? 255 : 0);
                //pDst[nLocalPos] = (UINT8)((nLocalValSub < 20) ? 255 : 0);
                //pDst[nLocalPos] = nAccDiv;
                pDst[nLocalPos] = nSubDiv;
                nLocalPos++;
            }
        }
        
        Kakao_ImgProc_Filter_Open(pExtractBG->pFilter, pOrgImg, pSrcImg, IMG_BINARYMAP_SKIN, 1);
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _ExtractBG_GetBGMask(IN ExtractBG_T *pExtractBG)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pExtractBG, KAKAO_STAT_NULL_POINTER)
    
    {
        INT32                   i = 0, j = 0;
        INT32                   nLocalPos = 0;
        Img_T                   *pAccImg0 = pExtractBG->pAccImg0;
        const INT32             nWidth = pAccImg0->GetImgWidth(pAccImg0, IMG_PLANE_0);
        const INT32             nHeight = pAccImg0->GetImgHeight(pAccImg0, IMG_PLANE_0);
        const INT32             nStride = pAccImg0->GetImgStride(pAccImg0, IMG_PLANE_0);
        UINT8                   *pAccImgSrc0 = pAccImg0->GetImgPlanes(pAccImg0, IMG_PLANE_0);
        UINT8                   *pFirstDiffImg = pExtractBG->pFirstDiffImg->GetImgPlanes(pExtractBG->pFirstDiffImg, IMG_PLANE_0);
        UINT8                   *pBackGroundMap = NULL;
    
        CHECK_POINTER_VALIDATION(pFirstDiffImg, KAKAO_STAT_NULL_POINTER)
        CHECK_POINTER_VALIDATION(pBackGroundMap, KAKAO_STAT_NULL_POINTER)
        
        if(NULL == (pBackGroundMap = pAccImg0->GetImgBinaryMap(pAccImg0, IMG_BINARYMAP_BACKGND)))
            return nRet;
        
        for(j=1 ; j<nHeight-1 ; j++)
        {
            nLocalPos = j * nStride;
            
            for(i=1 ; i<nWidth-1 ; i++)
            {
                if(1 != pBackGroundMap[nLocalPos])
                {
                    INT32       nTmpPos = 0;
                    INT32       nAccImgMean = 0;
                    INT32       nFirstDiffImgMean = 0;
                    
                    nTmpPos = nLocalPos - nStride - 1;
                    nAccImgMean += pAccImgSrc0[nTmpPos] + pAccImgSrc0[nTmpPos+1] + pAccImgSrc0[nTmpPos+2];      nTmpPos += nStride;
                    nAccImgMean += pAccImgSrc0[nTmpPos] + pAccImgSrc0[nTmpPos+1] + pAccImgSrc0[nTmpPos+2];      nTmpPos += nStride;
                    nAccImgMean += pAccImgSrc0[nTmpPos] + pAccImgSrc0[nTmpPos+1] + pAccImgSrc0[nTmpPos+2];      nTmpPos += nStride;
                    nAccImgMean /= 9;
                    
                    nTmpPos = nLocalPos - nStride - 1;
                    nFirstDiffImgMean += pFirstDiffImg[nTmpPos] + pFirstDiffImg[nTmpPos+1] + pFirstDiffImg[nTmpPos+2];      nTmpPos += nStride;
                    nFirstDiffImgMean += pFirstDiffImg[nTmpPos] + pFirstDiffImg[nTmpPos+1] + pFirstDiffImg[nTmpPos+2];      nTmpPos += nStride;
                    nFirstDiffImgMean += pFirstDiffImg[nTmpPos] + pFirstDiffImg[nTmpPos+1] + pFirstDiffImg[nTmpPos+2];      nTmpPos += nStride;
                    nFirstDiffImgMean /= 9;
                    
                    pBackGroundMap[nLocalPos] = ABSM(nFirstDiffImgMean - nAccImgMean) > 130 ? 1 : 0;
                }

                nLocalPos++;
            }
        }
        
        Kakao_ImgProc_Filter_Open(pExtractBG->pFilter, pExtractBG->pAccImg0, pExtractBG->pAccImg0, IMG_BINARYMAP_BACKGND, 1);
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _ExtractBG_Replace_BackGround(IN ExtractBG_T *pExtractBG, Img_T *pSrcImg)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pExtractBG, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER)
    
    {
        INT32                   i = 0, j = 0, k = 0;
        INT32                   nBGStride = 0;
        INT32                   nBGMaskPos = 0;
        INT32                   nDstPos = 0;
        UINT8                   *pBackGroundMap = NULL;
        UINT8                   *pSrc = NULL;
        UINT8                   *pDst = NULL;
        
        _ExtractBG_Insert_SampleImg_to_ImgT(pExtractBG->pBackGroundImg, nSampleImg2, nSample2_Width, nSample2_Height);
        
        if(NULL == (pBackGroundMap = pExtractBG->pAccImg0->GetImgBinaryMap(pExtractBG->pAccImg0, IMG_BINARYMAP_BACKGND)))
            return nRet;
        
        CHECK_POINTER_VALIDATION(pBackGroundMap, KAKAO_STAT_NULL_POINTER)

        for(k=0 ; k<pSrcImg->GetNumofImgPlanes(pSrcImg) ; k++)
        {
            const INT32         nWidth = pSrcImg->GetImgWidth(pSrcImg, k);
            const INT32         nHeight = pSrcImg->GetImgHeight(pSrcImg, k);
            const INT32         nStride = pSrcImg->GetImgStride(pSrcImg, k);
            const INT32         nOffset = 1 << !(!(k));
            const UINT8         *pSrc = (UINT8 *)pExtractBG->pBackGroundImg->GetImgPlanes(pExtractBG->pBackGroundImg, k);
            UINT8               *pDst = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, k);
            nBGStride = nStride;
            
            for(j=0 ; j<nHeight ; j++)
            {
                nBGMaskPos = nOffset * j * nStride;
                nDstPos = j * nStride;
                
                for(i=0 ; i<nWidth ; i++)
                {
                    if(0 != pBackGroundMap[nBGMaskPos])
                        pDst[nDstPos] = pSrc[nDstPos];
                    nDstPos++;
                    nBGMaskPos += nOffset;
                }
            }
        }
    }

    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _ExtractBG_Insert_SampleImg_to_ImgT(IN OUT Img_T * const pDstImg, IN const UINT8 *pSrcBuf, IN INT32 nWidth, IN INT32 nHeight)
{
    INT32                   y = 0;
    INT32                   nSrcStride = 0;
    INT32                   nDstStride = 0;
    const UINT8             *pSrc = NULL;
    UINT8                   *pDst = NULL;
    UINT32                  nSampleImgSize = nWidth * nHeight;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pSrcBuf, KAKAO_STAT_NULL_POINTER)

    nSrcStride = nWidth;
    nDstStride = pDstImg->GetImgStride(pDstImg, IMG_PLANE_0);
    pSrc = pSrcBuf;
    pDst = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0);
    for(y=0 ; y<nHeight ; y++)
    {
        MEMCPY(pDst, pSrc, nWidth);
        pSrc += nSrcStride;
        pDst += nDstStride;
    }
    
    nWidth = (nWidth >> 1);
    nHeight = (nHeight >> 1);
    nSrcStride = (nSample1_Width >> 1);
    nDstStride = pDstImg->GetImgStride(pDstImg, IMG_PLANE_1);
    pSrc = pSrcBuf + nSampleImgSize;
    pDst = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_1);
    for(y=0 ; y<nHeight ; y++)
    {
        MEMCPY(pDst, pSrc, nWidth);
        pSrc += nSrcStride;
        pDst += nDstStride;
    }
    
    nDstStride = pDstImg->GetImgStride(pDstImg, IMG_PLANE_2);
    pSrc = pSrcBuf + nSampleImgSize + (nSampleImgSize >> 2);
    pDst = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_2);
    for(y=0 ; y<nHeight ; y++)
    {
        MEMCPY(pDst, pSrc, nWidth);
        pSrc += nSrcStride;
        pDst += nDstStride;
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}



