/*
 * Copyright (c) 2011 porneL. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials
 * provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERDSSIMTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */



/*----------------------------------------------------------------------------------------
 File Inclusions
 ----------------------------------------------------------------------------------------*/
#include "Kakao_ImgProc_Modules.h"
#include <math.h>


/*----------------------------------------------------------------------------------------
 Constant Definitions
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Macro Definitions
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Type Definitions
 ----------------------------------------------------------------------------------------*/
typedef void rowcallback(UINT8 *, const INT32 nWidth);


/*----------------------------------------------------------------------------------------
 Static Function
 ----------------------------------------------------------------------------------------*/
static void _DSSIM_Calculate(IN const IQA_T *pIQA, OUT D64 *pAvgSSIM);
static void _Transposing_1DBlur(IN const IQA_T *pIQA, IN const Img_T *pSrcImg, IN const Img_T *pDstImg);
static void _Regular_1DBlur(IN const IQA_T *pIQA, IN const Img_T *pSrcImg, IN const Img_T *pDstImg, rowcallback *const pCallback);
static void _Blur(IN const IQA_T *pIQA, IN const Img_T *pSrcImg, IN const Img_T *pDstImg, rowcallback *const pCallback);

/*----------------------------------------------------------------------------------------
 Static Variable
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Global Variable
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Function Implementation
 ----------------------------------------------------------------------------------------*/
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_IQA_Create(IN OUT IQA_T **ppIQA, IN const INT32 nWidth, IN const INT32 nHeight)
{
    IQA_T                   *pTmpIQA = NULL;
    INT32                   i = 0;
    INT32                   nAlignedWidth = GET_ALIGNED_LENGTH(nWidth, MACRO_BLOCK_SIZE);
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    SAFEALLOC(pTmpIQA, 1, 32, IQA_T);
    
    Kakao_ImgProc_Util_UpdateSize(&pTmpIQA->nBaseSize, nWidth, nHeight, NULL, IMG_FORMAT_MAX, 4);

    pTmpIQA->nFrameBufIdx = 0;
    
    for(i=0 ; i<2 ; i++)
    {
        Kakao_ImgProc_Util_CreateImg(&(pTmpIQA->DSSIM[i].pMu), nWidth, nHeight, IMG_FORMAT_GRAY, IMG_DEPTH_F32);
        CHECK_POINTER_VALIDATION(pTmpIQA->DSSIM[i].pMu, KAKAO_STAT_NULL_POINTER)
        Kakao_ImgProc_Util_CreateImg(&(pTmpIQA->DSSIM[i].pSigma_Sq), nWidth, nHeight, IMG_FORMAT_GRAY, IMG_DEPTH_F32);
        CHECK_POINTER_VALIDATION(pTmpIQA->DSSIM[i].pSigma_Sq, KAKAO_STAT_NULL_POINTER)
        Kakao_ImgProc_Util_CreateImg(&(pTmpIQA->DSSIM[i].pTmpSquare), nWidth, nHeight, IMG_FORMAT_GRAY, IMG_DEPTH_F32);
        CHECK_POINTER_VALIDATION(pTmpIQA->DSSIM[i].pTmpSquare, KAKAO_STAT_NULL_POINTER)
    }
    
    Kakao_ImgProc_Util_CreateImg(&(pTmpIQA->pSigma12), nWidth, nHeight, IMG_FORMAT_GRAY, IMG_DEPTH_F32);
    CHECK_POINTER_VALIDATION(pTmpIQA->pSigma12, KAKAO_STAT_NULL_POINTER)
    Kakao_ImgProc_Util_CreateImg(&(pTmpIQA->pMultiply12), nWidth, nHeight, IMG_FORMAT_GRAY, IMG_DEPTH_F32);
    CHECK_POINTER_VALIDATION(pTmpIQA->pMultiply12, KAKAO_STAT_NULL_POINTER)
    Kakao_ImgProc_Util_CreateImg(&(pTmpIQA->pTmpBuf), nWidth, nHeight, IMG_FORMAT_GRAY, IMG_DEPTH_F32);
    CHECK_POINTER_VALIDATION(pTmpIQA->pTmpBuf, KAKAO_STAT_NULL_POINTER)
    
    SAFEALLOC(pTmpIQA->pSquareTable, 256, 32, UINT32);
    for(i=1 ; i<256 ; i++)
        pTmpIQA->pSquareTable[i] = i * i;
    
    *ppIQA = pTmpIQA;
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    Kakao_ImgProc_IQA_Destroy(&pTmpIQA);
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_IQA_Destroy(IN OUT IQA_T **ppIQA)
{
    INT32                   i = 0;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION((*ppIQA), KAKAO_STAT_NULL_POINTER);

    for(i=0 ; i<2 ; i++)
    {
        Kakao_ImgProc_Util_ReleaseImg(&((*ppIQA)->DSSIM[i].pMu));
        Kakao_ImgProc_Util_ReleaseImg(&((*ppIQA)->DSSIM[i].pSigma_Sq));
        Kakao_ImgProc_Util_ReleaseImg(&((*ppIQA)->DSSIM[i].pTmpSquare));
    }
    
    Kakao_ImgProc_Util_ReleaseImg(&((*ppIQA)->pSigma12));
    Kakao_ImgProc_Util_ReleaseImg(&((*ppIQA)->pMultiply12));
    Kakao_ImgProc_Util_ReleaseImg(&((*ppIQA)->pTmpBuf));
    
    SAFEFREE(((*ppIQA)->pSquareTable));
    
    SAFEFREE((*ppIQA));
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_IQA_UpdateImgSize(IN OUT IQA_T *pIQA, IN const INT32 nWidth, IN const INT32 nHeight)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   i = 0;
    
    CHECK_POINTER_VALIDATION(pIQA, KAKAO_STAT_NULL_POINTER)
    
    Kakao_ImgProc_Util_UpdateSize(&pIQA->nBaseSize, nWidth, nHeight, NULL, IMG_FORMAT_MAX, 4);
    
    for(i=0 ; i<2 ; i++)
    {
        Kakao_ImgProc_Util_UpdateImgSize(pIQA->DSSIM[i].pMu, nWidth, nHeight, NULL, IMG_FORMAT_GRAY);
        Kakao_ImgProc_Util_UpdateImgSize(pIQA->DSSIM[i].pSigma_Sq, nWidth, nHeight, NULL, IMG_FORMAT_GRAY);
        Kakao_ImgProc_Util_UpdateImgSize(pIQA->DSSIM[i].pTmpSquare, nWidth, nHeight, NULL, IMG_FORMAT_GRAY);
    }
    
    Kakao_ImgProc_Util_UpdateImgSize(pIQA->pSigma12, nWidth, nHeight, NULL, IMG_FORMAT_GRAY);
    Kakao_ImgProc_Util_UpdateImgSize(pIQA->pMultiply12, nWidth, nHeight, NULL, IMG_FORMAT_GRAY);
    Kakao_ImgProc_Util_UpdateImgSize(pIQA->pTmpBuf, nWidth, nHeight, NULL, IMG_FORMAT_GRAY);
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_IQA_CalculatePSNR(IN const IQA_T *pIQA, IN const Img_T *pSrcImg0, IN const Img_T *pSrcImg1, IN const INT32 nPlaneIdx, OUT F32 *pPSNR)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    CHECK_POINTER_VALIDATION(pIQA, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg0, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg1, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pPSNR, KAKAO_STAT_NULL_POINTER);
    
    {
        INT32               i = 0, j = 0;
        const INT32         nWidth = pSrcImg0->GetImgWidth(pSrcImg0, nPlaneIdx);
        const INT32         nHeight = pSrcImg0->GetImgHeight(pSrcImg0, nPlaneIdx);
        INT32               nLocalPos0 = 0, nLocalPos1 = 0;
        const UINT32        *pSquareTable = pIQA->pSquareTable;
        INT32               nPixDiff = 0;
        unsigned long long  nSquareSum = 0;
        UINT8               *pSrc0 = NULL;
        UINT8               *pSrc1 = NULL;
        
        pSrc0 = (UINT8 *)pSrcImg0->GetImgPlanes(pSrcImg0, nPlaneIdx);
        pSrc1 = (UINT8 *)pSrcImg1->GetImgPlanes(pSrcImg1, nPlaneIdx);
        
        CHECK_POINTER_VALIDATION(pSrc0, KAKAO_STAT_NULL_POINTER)
        CHECK_POINTER_VALIDATION(pSrc1, KAKAO_STAT_NULL_POINTER)
        CHECK_POINTER_VALIDATION(pSquareTable, KAKAO_STAT_NULL_POINTER)
        
        for(j=0 ; j<nHeight ; j++)
        {
            nLocalPos0 = j * pSrcImg0->GetImgStride(pSrcImg0, nPlaneIdx);
            nLocalPos1 = j * pSrcImg1->GetImgStride(pSrcImg1, nPlaneIdx);
            for(i=0 ; i<nWidth ; i++, nLocalPos0++, nLocalPos1++)
            {
                nPixDiff = (INT32)pSrc1[nLocalPos1] - (INT32)pSrc0[nLocalPos0];
                nSquareSum += pSquareTable[abs(nPixDiff)];
            }
        }
        
        if(0 != nSquareSum)
            *pPSNR = (F32)(10 * log10(65025.0f * (D64)((D64)(nWidth * nHeight) / nSquareSum)));
        else
            *pPSNR = 99.99;
    }

    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


/*
 Algorithm based on Rabah Mehdi's C++ implementation
 
 Returns dssim.
 Saves dissimilarity visualisation as ssimfilename (pass NULL if not needed)
 
 You must call dssim_set_original and dssim_set_modified first.
 */
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_IQA_DSSIM_Calculate(IN const IQA_T *pIQA, IN const Img_T *pSrcImg0, IN const Img_T *pSrcImg1, OUT D64 *pDSSIM)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    CHECK_POINTER_VALIDATION(pIQA, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg0, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg1, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pDSSIM, KAKAO_STAT_NULL_POINTER);
    
    {
        INT32               i = 0, j = 0, k = 0;
        const Img_T         *pSrcImg[2] = {pSrcImg0, pSrcImg1};
        const INT32         nWidth = pSrcImg0->GetImgWidth(pSrcImg0, IMG_PLANE_0);
        const INT32         nHeight = pSrcImg0->GetImgHeight(pSrcImg0, IMG_PLANE_0);
        
        // Make Square Img
        for(k=0 ; i<2 ; k++)
        {
            Img_T           *pDstImg = pIQA->DSSIM[k].pTmpSquare;
            const INT32     nSrcStride = pSrcImg[k]->GetImgStride(pSrcImg[k], IMG_PLANE_0);
            const INT32     nDstStride = pDstImg->GetImgStride(pDstImg, IMG_PLANE_0);
            const UINT8     *pSrc = (UINT8 *)pSrcImg[k]->GetImgPlanes(pSrcImg[k], IMG_PLANE_0);
            F32             *pDst = (F32 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0);

            CHECK_POINTER_VALIDATION(pSrc, KAKAO_STAT_NULL_POINTER)
            CHECK_POINTER_VALIDATION(pDst, KAKAO_STAT_NULL_POINTER)

            for(j=0 ; j<nHeight ; j++)
            {
                INT32       nSrcLocalPos = j * nSrcStride;
                INT32       nDstLocalPos = j * nDstStride;
                
                for(i=0 ; i<nWidth ; i++, nSrcLocalPos++, nDstLocalPos++)
                    pDst[nDstLocalPos] = (F32)(pSrc[nSrcLocalPos] * pSrc[nSrcLocalPos]);
            }
            _Blur(pIQA, pSrcImg0, pIQA->DSSIM[k].pMu, NULL);
            _Blur(pIQA, pDstImg, pIQA->DSSIM[k].pSigma_Sq, NULL);
        }
        
        // Make Bi-Square Img
        {
            const INT32     nSrcStride0 = pSrcImg0->GetImgStride(pSrcImg0, IMG_PLANE_0);
            const INT32     nSrcStride1 = pSrcImg1->GetImgStride(pSrcImg1, IMG_PLANE_0);
            const INT32     nDstStride = pIQA->pMultiply12->GetImgStride(pIQA->pMultiply12, IMG_PLANE_0);
            UINT8           *pSrc0 = (UINT8 *)pSrcImg0->GetImgPlanes(pSrcImg0, IMG_PLANE_0);
            UINT8           *pSrc1 = (UINT8 *)pSrcImg1->GetImgPlanes(pSrcImg1, IMG_PLANE_0);
            F32             *pDst = (F32 *)pIQA->pMultiply12->GetImgPlanes(pIQA->pMultiply12, IMG_PLANE_0);
            
            CHECK_POINTER_VALIDATION(pSrc0, KAKAO_STAT_NULL_POINTER)
            CHECK_POINTER_VALIDATION(pSrc1, KAKAO_STAT_NULL_POINTER)
            CHECK_POINTER_VALIDATION(pDst, KAKAO_STAT_NULL_POINTER)

            for(j=0 ; j<nHeight ; j++)
            {
                UINT32      nSrcLocalPos0 = j * nSrcStride0;
                UINT32      nSrcLocalPos1 = j * nSrcStride1;
                UINT32      nDstLocalPos = j * nDstStride;
                for(i=0 ; i<nWidth ; i++, nSrcLocalPos0++, nSrcLocalPos1++, nDstLocalPos++)
                    pDst[nDstLocalPos] = (F32)(pSrc0[nSrcLocalPos0] * pSrc1[nSrcLocalPos1]);
            }
            
            _Blur(pIQA, pIQA->pMultiply12, pIQA->pSigma12, NULL);
        }
    }

    _DSSIM_Calculate(pIQA, pDSSIM);
    
    *pDSSIM = 1.0 / (*pDSSIM) - 1.0;
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


static void _DSSIM_Calculate(IN const IQA_T *pIQA, D64 *pAvgSSIM)
{
    INT32                   i = 0, j = 0;
    Img_T                   *pSrcImg = pIQA->DSSIM[0].pSigma_Sq;
    const INT32             nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    const INT32             nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    const INT32             nStride = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    const F32               *pMu0 = (F32 *)pIQA->DSSIM[0].pMu->GetImgPlanes(pIQA->DSSIM[0].pMu, IMG_PLANE_0);
    const F32               *pMu1 = (F32 *)pIQA->DSSIM[1].pMu->GetImgPlanes(pIQA->DSSIM[1].pMu, IMG_PLANE_0);
    const F32               *pSigma_Sq0 = (F32 *)pIQA->DSSIM[0].pSigma_Sq->GetImgPlanes(pIQA->DSSIM[0].pSigma_Sq, IMG_PLANE_0);
    const F32               *pSigma_Sq1 = (F32 *)pIQA->DSSIM[1].pSigma_Sq->GetImgPlanes(pIQA->DSSIM[1].pSigma_Sq, IMG_PLANE_0);
    const F32               *pSigma12 = (F32 *)pIQA->pSigma12->GetImgPlanes(pIQA->pSigma12, IMG_PLANE_0);
    const D64               c1 = (0.01 * 0.01), c2 = (0.03 * 0.03);
    D64                     nAvgSSIM = 0;
    
    for(j=0 ; j<nHeight ; j++)
    {
        INT32           nLocalPos = j * nStride;
        
        for(i=0 ; i<nWidth ; i++, nLocalPos++)
        {
            const D64 nMu0_Sq = pMu0[nLocalPos] * pMu0[nLocalPos];
            const D64 nMu1_Sq = pMu1[nLocalPos] * pMu1[nLocalPos];
            const D64 nMu0Mu1 = pMu0[nLocalPos] * pMu1[nLocalPos];
            
            const D64 nSSIM = (c1 + 2.0 * nMu0Mu1) * (c2 + 2.0 * (pSigma12[nLocalPos] - nMu0Mu1))
            / ((c1 + nMu0_Sq + nMu1_Sq) * (c2 + pSigma_Sq0[nLocalPos] - nMu0_Sq + pSigma_Sq1[nLocalPos] - nMu1_Sq));
            
            nAvgSSIM += nSSIM;
        }
    }
    
    *pAvgSSIM = nAvgSSIM / ((D64)nWidth * nHeight);
    
    return;
}


static void _Transposing_1DBlur(IN const IQA_T *pIQA, IN const Img_T *pSrcImg, IN const Img_T *pDstImg)
{
    INT32                   i = 0, j = 0;
    const INT32             nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    const INT32             nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    const INT32             nSize = 4;
    const F32               nSizeof = (F32)nSize;
    F32                     *pSrc = (F32 *)(pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0));
    F32                     *pDst = (F32 *)(pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0));
    
    for(j=0 ; j<nHeight ; j++)
    {
        F32                 *pSrcRow = pSrc + j * nWidth;
        
        // accumulate sum for pixels outside line
        F32 nSum = pSrcRow[0] * nSizeof;
        for(i=0; i<GET_MIN(nWidth, nSize) ; i++)
            nSum += pSrcRow[i];
        
        // blur with left side outside line
        for(i=0 ; i<GET_MIN(nWidth, nSize) ; i++)
        {
            nSum -= pSrcRow[0];
            if((i + nSize) < nWidth){
                nSum += pSrcRow[i+nSize];
            }
            
            pDst[i*nHeight + j] = nSum / (nSizeof * 2.0f);
        }
        
        for(i=nSize ; i<nWidth-nSize ; i++)
        {
            nSum -= pSrcRow[i-nSize];
            nSum += pSrcRow[i+nSize];
            
            pDst[i*nHeight + j] = nSum / (nSizeof * 2.0f);
        }
        
        // blur with right side outside line
        for(i=nWidth-nSize ; i<nWidth ; i++)
        {
            if(i-nSize >= 0)
                nSum -= pSrcRow[i-nSize];
            
            nSum += pSrcRow[nWidth-1];
            
            pDst[i*nHeight + j] = nSum / (nSizeof * 2.0f);
        }
    }
    
    return;
}


static void _Regular_1DBlur(IN const IQA_T *pIQA, IN const Img_T *pSrcImg, IN const Img_T *pDstImg, rowcallback *const pCallback)
{
    INT32                   i = 0, j = 0;
    const INT32             nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    const INT32             nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    const INT32             nSize = 1;
    const F32               nSizeof = (F32)nSize;
    UINT8                   *pSrc = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
    F32                     *pDst = (F32 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0);
    
    for(j=0 ; j<nHeight ; j++)
    {
        UINT8               *pSrcRow = pSrc + j * nWidth;
        F32                 *pDstRow = pDst + j * nWidth;
        F32                 nSum = 0.0f;
        
        // preprocess line
        if(NULL != pCallback)
            pCallback(pSrcRow, nWidth);
        
        // accumulate sum for pixels outside line
        nSum = (F32)pSrcRow[0] * nSizeof;
        for(i=0 ; i<GET_MIN(nWidth, nSize) ; i++)
            nSum += pSrcRow[i];
        
        // blur with left side outside line
        for(i=0 ; i<GET_MIN(nWidth, nSize) ; i++)
        {
            nSum -= (F32)pSrcRow[0];
            if((i + nSize) < nWidth)
                nSum += (F32)pSrcRow[i + nSize];
            
            pDstRow[i] = nSum / (nSizeof * 2.0f);
        }
        
        for(i=nSize ; i<nWidth - nSize ; i++)
        {
            nSum -= (F32)pSrcRow[i - nSize];
            nSum += (F32)pSrcRow[i + nSize];
            
            pDstRow[i] = nSum / (nSizeof * 2.0f);
        }
        
        // blur with right side outside line
        for(i=nWidth - nSize ; i<nWidth ; i++)
        {
            if (i - nSize >= 0) {
                nSum -= (F32)pSrcRow[i - nSize];
            }
            nSum += (F32)pSrcRow[nWidth - 1];
            
            pDstRow[i] = nSum / (nSizeof * 2.0f);
        }
    }
    
    return;
}


/*
 * Filters image with callback and blurs (lousy approximate of gaussian)
 */
static void _Blur(IN const IQA_T *pIQA, IN const Img_T *pSrcImg, IN const Img_T *pDstImg, rowcallback *const pCallback)
{
    Img_T                   *pTmpImg = pIQA->pTmpBuf;
    
    _Regular_1DBlur(pIQA, pSrcImg, pTmpImg, pCallback);
    _Regular_1DBlur(pIQA, pTmpImg, pDstImg, NULL);
    
    _Transposing_1DBlur(pIQA, pDstImg, pTmpImg);
    
    _Regular_1DBlur(pIQA, pTmpImg, pDstImg, NULL);
    _Regular_1DBlur(pIQA, pDstImg, pTmpImg, NULL);
    _Transposing_1DBlur(pIQA, pTmpImg, pDstImg);
    
    return;
}











