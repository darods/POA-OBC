


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
typedef void (*fpgetNxMMean)(IN const UINT8 *pSrc, IN const INT32 nSrcStride, OUT UINT32 *pSum);
typedef void (*fpgetNxMVar)(IN const UINT8 *pSrc, IN const INT32 nSrcStride, OUT UINT32 *pSqSum, OUT UINT32 *pSum);
typedef void (*fpgetNxMDiffVar)(IN const UINT8 *pSrc0, IN const INT32 nSrcStride0, IN const UINT8 *pSrc1, IN const INT32 nSrcStride1, OUT UINT32 *pSqSum, OUT INT32 *pSum);


/*----------------------------------------------------------------------------------------
 Static Function
 ----------------------------------------------------------------------------------------*/
static void _GetMean4x4_C(IN const UINT8 *pSrc, IN const INT32 nSrcStride, OUT UINT32 *pSum);
static void _GetMean8x8_C(IN const UINT8 *pSrc, IN const INT32 nSrcStride, OUT UINT32 *pSum);
static void _GetMean16x16_C(IN const UINT8 *pSrc, IN const INT32 nSrcStride, OUT UINT32 *pSum);
static void _GetVar4x4_C(IN const UINT8 *pSrc, IN const INT32 nSrcStride, OUT UINT32 *pSqSum, OUT UINT32 *pSum);
static void _GetVar8x8_C(IN const UINT8 *pSrc, IN const INT32 nSrcStride, OUT UINT32 *pSqSum, OUT UINT32 *pSum);
static void _GetVar16x16_C(IN const UINT8 *pSrc, IN const INT32 nSrcStride, OUT UINT32 *pSqSum, OUT UINT32 *pSum);
static void _GetDiffVar4x4_C(IN const UINT8 *pSrc0, IN const INT32 nSrcStride0,
                             IN const UINT8 *pSrc1, IN const INT32 nSrcStride1, OUT UINT32 *pSqSum, OUT INT32 *pSum);
static void _GetDiffVar8x8_C(IN const UINT8 *pSrc0, IN const INT32 nSrcStride0,
                             IN const UINT8 *pSrc1, IN const INT32 nSrcStride1, OUT UINT32 *pSqSum, OUT INT32 *pSum);
static void _GetDiffVar16x16_C(IN const UINT8 *pSrc0, IN const INT32 nSrcStride0,
                             IN const UINT8 *pSrc1, IN const INT32 nSrcStride1, OUT UINT32 *pSqSum, OUT INT32 *pSum);
static void _GetMean_C(IN const UINT8 *pSrc, IN const INT32 nSrcStride, IN const INT32 nWidth, IN const INT32 nHeight,
                             OUT UINT32 *pSum, fpgetNxMMean fpMean, IN const INT32 nBlockSize);
static void _GetVariance_C(IN const UINT8 *pSrc0, IN const INT32 nSrcStride0, IN const INT32 nWidth, IN const INT32 nHeight,
                             OUT UINT32 *pSqSum, OUT UINT32 *pSum, fpgetNxMVar fpVar, IN const INT32 nBlockSize);
static void _GetDiffVariance_C(IN const UINT8 *pSrc0, IN const INT32 nSrcStride0, IN const UINT8 *pSrc1, IN const INT32 nSrcStride1,
                             IN const INT32 nWidth, IN const INT32 nHeight, OUT UINT32 *pSqSum, OUT INT32 *pSum,
                              fpgetNxMDiffVar fpVar, IN const INT32 nBlockSize);


/*----------------------------------------------------------------------------------------
 Static Variable
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Global Variable
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Function Implementation
 ----------------------------------------------------------------------------------------*/
void OneRow_Copy_C(OUT UINT8 *pDst, IN const UINT8 *pSrc, IN const INT32 nWidth)
{
    MEMCPY(pDst, pSrc, nWidth);
}


void OneRow_InterleaveUV_C(IN const UINT8 *pSrcUV, OUT UINT8 *pDstU, OUT UINT8 *pDstV, IN const INT32 nWidth)
{
    INT32               j = 0;
    const INT32         nScaledWidth = (nWidth >> 3);
    
    for(j=nScaledWidth-1 ; j>=0 ; j--)
    {
        *pDstU++ = *pSrcUV++;
        *pDstV++ = *pSrcUV++;
        *pDstU++ = *pSrcUV++;
        *pDstV++ = *pSrcUV++;
        *pDstU++ = *pSrcUV++;
        *pDstV++ = *pSrcUV++;
        *pDstU++ = *pSrcUV++;
        *pDstV++ = *pSrcUV++;
        *pDstU++ = *pSrcUV++;
        *pDstV++ = *pSrcUV++;
        *pDstU++ = *pSrcUV++;
        *pDstV++ = *pSrcUV++;
        *pDstU++ = *pSrcUV++;
        *pDstV++ = *pSrcUV++;
        *pDstU++ = *pSrcUV++;
        *pDstV++ = *pSrcUV++;
    }
}


void OneRow_Subsample_Even_C(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth)
{
    INT32               j = 0;
    const INT32         nScaledWidth = (nWidth >> 3);
    
    for(j=nScaledWidth-1 ; j>=0 ; j--)
    {
        *pDst++ = *pSrc++;  pSrc++;
        *pDst++ = *pSrc++;  pSrc++;
        *pDst++ = *pSrc++;  pSrc++;
        *pDst++ = *pSrc++;  pSrc++;
        *pDst++ = *pSrc++;  pSrc++;
        *pDst++ = *pSrc++;  pSrc++;
        *pDst++ = *pSrc++;  pSrc++;
        *pDst++ = *pSrc++;  pSrc++;
    }
}


void OneRow_Subsample_Odd_C(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth)
{
    INT32               j = 0;
    const INT32         nScaledWidth = (nWidth >> 3);
    
    for(j=nScaledWidth-1 ; j>=0 ; j--)
    {
        pSrc++; *pDst++ = *pSrc++;
        pSrc++; *pDst++ = *pSrc++;
        pSrc++; *pDst++ = *pSrc++;
        pSrc++; *pDst++ = *pSrc++;
        pSrc++; *pDst++ = *pSrc++;
        pSrc++; *pDst++ = *pSrc++;
        pSrc++; *pDst++ = *pSrc++;
        pSrc++; *pDst++ = *pSrc++;
    }
}


void OneRow_Subsample_1_4_C(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth)
{
    INT32               j = 0;
    const INT32         nScaledWidth = (nWidth >> 3);
    
    for(j=nScaledWidth-1 ; j>=0 ; j--)
    {
        *pDst++ = *pSrc;  pSrc += 4;
        *pDst++ = *pSrc;  pSrc += 4;
        *pDst++ = *pSrc;  pSrc += 4;
        *pDst++ = *pSrc;  pSrc += 4;
        *pDst++ = *pSrc;  pSrc += 4;
        *pDst++ = *pSrc;  pSrc += 4;
        *pDst++ = *pSrc;  pSrc += 4;
        *pDst++ = *pSrc;  pSrc += 4;
    }
}


void OneRow_ScaleBilinear_C(OUT UINT8* pDst,
                          IN const UINT8* pSrc, ptrdiff_t nSrcStride,
                          IN const INT32 nDstWidth, IN const INT32 nSrcYFraction)
{
    ;
}


void OneCol_ScaleBilinear_C(OUT UINT8* pDst, IN const UINT8* pSrc,
                         IN const INT32 nDstWidth, IN INT32 x, IN const INT32 dx)
{
    INT32               j = 0;
    
    for(j=0 ; j<nDstWidth-1 ; j+= 2)
    {
        INT32       xi = x >> 16;
        INT32       a = pSrc[xi];
        INT32       b = pSrc[xi + 1];
        
        pDst[0] = BLENDER(a, b, x & 0xffff);
        x += dx;
        xi = x >> 16;
        a = pSrc[xi];
        b = pSrc[xi + 1];
        pDst[1] = BLENDER(a, b, x & 0xffff);
        x += dx;
        pDst += 2;
    }
    
    if(nDstWidth & 1)
    {
        INT32       xi = x >> 16;
        INT32       a = pSrc[xi];
        INT32       b = pSrc[xi + 1];
        
        pDst[0] = BLENDER(a, b, x & 0xffff);
    }
}


void Erosion_C(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg, IN const BinaryMapType nBinaryMap, IN const INT32 nThick)
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


void Dilation_C(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg, IN const BinaryMapType nBinaryMap)
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


void OneRow_MeanBlur_C(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth, IN const INT32 nSrcStride, IN const INT32 nDstStride)
{
    INT32                   i = 0;
    UINT8                   *pSrcPrev = NULL, *pSrcNext0 = NULL, *pSrcNext1 = NULL, *pSrcNext2 = NULL;
    UINT8                   *pDst0 = NULL, *pDst1 = NULL, *pDst2 = NULL;
    UINT32                  nValue[5] = {0, };
    
    pSrcPrev = (UINT8 *)(pSrc - nSrcStride);
    pSrcNext0 = (UINT8 *)(pSrc + nSrcStride);
    pSrcNext1 = (UINT8 *)(pSrc + (2 * nSrcStride));
    pSrcNext2 = (UINT8 *)(pSrc + (3 * nSrcStride));
    
    pDst0 = pDst;
    pDst1 = pDst + nDstStride;
    pDst2 = pDst + (2 * nDstStride);
    
    for(i=0 ; i<nWidth ; i++)
    {
        nValue[0] = (UINT32)(pSrcPrev[i - 1]);
        nValue[0] += (UINT32)(pSrcPrev[i]);
        nValue[0] += (UINT32)(pSrcPrev[i + 1]);
        
        nValue[1] = (UINT32)(pSrc[i - 1]);
        nValue[1] += (UINT32)(pSrc[i]);
        nValue[1] += (UINT32)(pSrc[i + 1]);
        
        nValue[2] = (UINT32)(pSrcNext0[i - 1]);
        nValue[2] += (UINT32)(pSrcNext0[i]);
        nValue[2] += (UINT32)(pSrcNext0[i + 1]);
        
        nValue[3] = (UINT32)(pSrcNext1[i - 1]);
        nValue[3] += (UINT32)(pSrcNext1[i]);
        nValue[3] += (UINT32)(pSrcNext1[i + 1]);
        
        nValue[4] = (UINT32)(pSrcNext2[i - 1]);
        nValue[4] += (UINT32)(pSrcNext2[i]);
        nValue[4] += (UINT32)(pSrcNext2[i + 1]);
        
        *pDst0++ = (UINT8)(CLIP3(PIXEL_MIN, PIXEL_MAX, ((nValue[0] + nValue[1] + nValue[2]) / 9)));
        *pDst1++ = (UINT8)(CLIP3(PIXEL_MIN, PIXEL_MAX, ((nValue[1] + nValue[2] + nValue[3]) / 9)));
        *pDst2++ = (UINT8)(CLIP3(PIXEL_MIN, PIXEL_MAX, ((nValue[2] + nValue[3] + nValue[4]) / 9)));
    }
    
    return;
}


void OneRow_MeanBlur_Hor_C(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth)
{
    INT32                   i = 0;
    
    for(i=0 ; i<nWidth ; i++)
        pDst[i] = (pSrc[i - 1] + pSrc[i] + pSrc[i + 1]) / 3;
    
    return;
}


void OneRow_MeanBlur_Ver_C(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth, IN const INT32 nSrcStride, IN const INT32 nDstStride)
{
    INT32                   i = 0;
    UINT8                   *pSrcPrev = NULL, *pSrcNext = NULL;
    
    pSrcPrev = (UINT8 *)(pSrc - nSrcStride);
    pSrcNext = (UINT8 *)(pSrc + nSrcStride);
    
    for(i=0 ; i<nWidth ; i++)
    {
        pDst[i] = (pSrcPrev[i] + pSrc[i] + pSrcNext[i]) / 3;
    }
    
    return;
}


void OneRow_Mosaic4_C(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth, IN const INT32 nSrcStride, IN const INT32 nDstStride, IN const RectImgAddr *pBaseRectAddr)
{
    INT32                   i = 0, j = 0;
    UINT32                  nMosaicSize = 4;
    INT32                   nLocalPos = 0;
    
    for(i=0 ; i<nWidth ; i+=nMosaicSize)
    {
        UINT32          n = 0;
        UINT32          nSumVal = 0;
        
        nLocalPos = j * nSrcStride + i;
        for(n=0 ; n<nMosaicSize ; n++)
        {
            nSumVal += pSrc[nLocalPos];
            nSumVal += pSrc[nLocalPos + 1];
            nSumVal += pSrc[nLocalPos + 2];
            nSumVal += pSrc[nLocalPos + 3];
            nLocalPos += nSrcStride;
        }
        
        nSumVal >>= 4;
        
        nLocalPos = j * nDstStride + i;
        MEMSET(&(pDst[nLocalPos]), nSumVal, sizeof(UINT8) * nMosaicSize); nLocalPos += nDstStride;
        MEMSET(&(pDst[nLocalPos]), nSumVal, sizeof(UINT8) * nMosaicSize); nLocalPos += nDstStride;
        MEMSET(&(pDst[nLocalPos]), nSumVal, sizeof(UINT8) * nMosaicSize); nLocalPos += nDstStride;
        MEMSET(&(pDst[nLocalPos]), nSumVal, sizeof(UINT8) * nMosaicSize);
    }
    
    return;
}


void OneRow_Mosaic8_C(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth, IN const INT32 nSrcStride, IN const INT32 nDstStride, IN const RectImgAddr *pBaseRectAddr)
{
    INT32                   i = 0, j = 0;
    UINT32                  nMosaicSize = 8;
    INT32                   nLocalPos = 0;
    
    for(i=0 ; i<nWidth ; i+=nMosaicSize)
    {
        UINT32          n = 0;
        UINT32          nSumVal = 0;
        
        nLocalPos = j * nSrcStride + i;
        for(n=0 ; n<nMosaicSize ; n++)
        {
            nSumVal += pSrc[nLocalPos];
            nSumVal += pSrc[nLocalPos + 1];
            nSumVal += pSrc[nLocalPos + 2];
            nSumVal += pSrc[nLocalPos + 3];
            nSumVal += pSrc[nLocalPos + 4];
            nSumVal += pSrc[nLocalPos + 5];
            nSumVal += pSrc[nLocalPos + 6];
            nSumVal += pSrc[nLocalPos + 7];
            nLocalPos += nSrcStride;
        }
        
        nSumVal >>= 6;
        
        nLocalPos = j * nDstStride + i;
        MEMSET(&(pDst[nLocalPos]), nSumVal, sizeof(UINT8) * nMosaicSize); nLocalPos += nDstStride;
        MEMSET(&(pDst[nLocalPos]), nSumVal, sizeof(UINT8) * nMosaicSize); nLocalPos += nDstStride;
        MEMSET(&(pDst[nLocalPos]), nSumVal, sizeof(UINT8) * nMosaicSize); nLocalPos += nDstStride;
        MEMSET(&(pDst[nLocalPos]), nSumVal, sizeof(UINT8) * nMosaicSize); nLocalPos += nDstStride;
        MEMSET(&(pDst[nLocalPos]), nSumVal, sizeof(UINT8) * nMosaicSize); nLocalPos += nDstStride;
        MEMSET(&(pDst[nLocalPos]), nSumVal, sizeof(UINT8) * nMosaicSize); nLocalPos += nDstStride;
        MEMSET(&(pDst[nLocalPos]), nSumVal, sizeof(UINT8) * nMosaicSize); nLocalPos += nDstStride;
        MEMSET(&(pDst[nLocalPos]), nSumVal, sizeof(UINT8) * nMosaicSize);
    }
    
    return;
}


void OneRow_Mosaic16_C(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth, IN const INT32 nSrcStride, IN const INT32 nDstStride, IN const RectImgAddr *pBaseRectAddr)
{
    INT32                   i = 0, j = 0;
    UINT32                  nMosaicSize = 16;
    INT32                   nLocalPos = 0;
    
    for(i=0 ; i<nWidth ; i+=nMosaicSize)
    {
        UINT32          n = 0;
        UINT32          nSumVal = 0;
        
        nLocalPos = j * nSrcStride + i;
        for(n=0 ; n<nMosaicSize ; n++)
        {
            nSumVal += pSrc[nLocalPos];
            nSumVal += pSrc[nLocalPos + 1];
            nSumVal += pSrc[nLocalPos + 2];
            nSumVal += pSrc[nLocalPos + 3];
            nSumVal += pSrc[nLocalPos + 4];
            nSumVal += pSrc[nLocalPos + 5];
            nSumVal += pSrc[nLocalPos + 6];
            nSumVal += pSrc[nLocalPos + 7];
            nSumVal += pSrc[nLocalPos + 8];
            nSumVal += pSrc[nLocalPos + 9];
            nSumVal += pSrc[nLocalPos + 10];
            nSumVal += pSrc[nLocalPos + 11];
            nSumVal += pSrc[nLocalPos + 12];
            nSumVal += pSrc[nLocalPos + 13];
            nSumVal += pSrc[nLocalPos + 14];
            nSumVal += pSrc[nLocalPos + 15];
            
            nLocalPos += nSrcStride;
        }
        
        nSumVal >>= 8;
        
        nLocalPos = j * nDstStride + i;
        MEMSET(&(pDst[nLocalPos]), nSumVal, sizeof(UINT8) * nMosaicSize); nLocalPos += nDstStride;
        MEMSET(&(pDst[nLocalPos]), nSumVal, sizeof(UINT8) * nMosaicSize); nLocalPos += nDstStride;
        MEMSET(&(pDst[nLocalPos]), nSumVal, sizeof(UINT8) * nMosaicSize); nLocalPos += nDstStride;
        MEMSET(&(pDst[nLocalPos]), nSumVal, sizeof(UINT8) * nMosaicSize); nLocalPos += nDstStride;
        MEMSET(&(pDst[nLocalPos]), nSumVal, sizeof(UINT8) * nMosaicSize); nLocalPos += nDstStride;
        MEMSET(&(pDst[nLocalPos]), nSumVal, sizeof(UINT8) * nMosaicSize); nLocalPos += nDstStride;
        MEMSET(&(pDst[nLocalPos]), nSumVal, sizeof(UINT8) * nMosaicSize); nLocalPos += nDstStride;
        MEMSET(&(pDst[nLocalPos]), nSumVal, sizeof(UINT8) * nMosaicSize); nLocalPos += nDstStride;
        MEMSET(&(pDst[nLocalPos]), nSumVal, sizeof(UINT8) * nMosaicSize); nLocalPos += nDstStride;
        MEMSET(&(pDst[nLocalPos]), nSumVal, sizeof(UINT8) * nMosaicSize); nLocalPos += nDstStride;
        MEMSET(&(pDst[nLocalPos]), nSumVal, sizeof(UINT8) * nMosaicSize); nLocalPos += nDstStride;
        MEMSET(&(pDst[nLocalPos]), nSumVal, sizeof(UINT8) * nMosaicSize); nLocalPos += nDstStride;
        MEMSET(&(pDst[nLocalPos]), nSumVal, sizeof(UINT8) * nMosaicSize); nLocalPos += nDstStride;
        MEMSET(&(pDst[nLocalPos]), nSumVal, sizeof(UINT8) * nMosaicSize); nLocalPos += nDstStride;
        MEMSET(&(pDst[nLocalPos]), nSumVal, sizeof(UINT8) * nMosaicSize); nLocalPos += nDstStride;
        MEMSET(&(pDst[nLocalPos]), nSumVal, sizeof(UINT8) * nMosaicSize);
    }
    
    return;
}


void OneRow_Inverse_C(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth)
{
    INT32                   i = 0;
    UINT32                  nOffset = 0;
    
    for(i=0 ; i<nWidth ; i++)
        pDst[i] = (255 - pSrc[i]);
    
    return;
}


void OneRow_Clip_C(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth, IN const INT32 nMinClipVal, IN const INT32 nMaxClipVal)
{
    INT32                   i = 0;
    
    for(i=0 ; i<nWidth ; i++)
        pDst[i] = CLIP3(nMinClipVal, nMaxClipVal, pSrc[i]);
    
    return;
}


void OneRow_DiffImg_C(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, OUT UINT8 *pDst, IN const INT32 nWidth)
{
    INT32                   i = 0;
    
    for(i=0 ; i<nWidth ; i++)
        pDst[i] = (pSrc0[i] - pSrc1[i]);
    
    return;
}


void OneRow_Multiply_C(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, OUT UINT8 *pDst, IN const INT32 nWidth)
{
    INT32                   i = 0;
    
    for(i=0 ; i<nWidth ; i++)
        pDst[i] = (UINT8)(((UINT32)pSrc0[i] * (UINT32)pSrc1[i]) >> 8);
    
    return;
}


void OneRow_ContrastShift_C(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, OUT UINT8 *pDst, IN const INT32 nWidth)
{
    INT32                   i = 0;
    
    for(i=0 ; i<nWidth ; i++)
    {
        if(pSrc0[i] > 128)
            pDst[i] = 255 - (UINT8)((UINT16)((UINT16)(pSrc1[i] * (UINT16)pSrc1[i])) >> 7);
        else
            pDst[i] = (UINT8)((UINT16)((UINT16)(pSrc0[i] * (UINT16)pSrc0[i])) >> 7);
    }
    
    return;
}


void OneRow_PropMix_C(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, IN const F32 *pCoeffi, OUT UINT8 *pDst, IN const INT32 nWidth)
{
    INT32                   i = 0;
    
    for(i=0 ; i<nWidth ; i++)
        pDst[i] = (pCoeffi[i] * (F32)pSrc0[i]) + ((1.0f - pCoeffi[i]) * (F32)pSrc1[i]);
    
    return;
}


void OneRow_BlurNoiseReduction_C(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth, IN const INT32 nSrcStride, IN const INT32 nDstStride)
{
    INT32                   i = 0;
    UINT8                   *pSrcPrev = NULL, *pSrcNext0 = NULL, *pSrcNext1 = NULL, *pSrcNext2 = NULL;
    UINT8                   *pDst0 = NULL, *pDst1 = NULL, *pDst2 = NULL;
    UINT32                  nValue[5] = {0, };
    
    pSrcPrev = (UINT8 *)(pSrc - nSrcStride);
    pSrcNext0 = (UINT8 *)(pSrc + nSrcStride);
    pSrcNext1 = (UINT8 *)(pSrc + (2 * nSrcStride));
    pSrcNext2 = (UINT8 *)(pSrc + (3 * nSrcStride));
    
    pDst0 = pDst;
    pDst1 = pDst + nDstStride;
    pDst2 = pDst + (2 * nDstStride);
    
    for(i=0 ; i<nWidth ; i++)
    {
        UINT32              nTmpDst0 = 0, nTmpDst1 = 0, nTmpDst2 = 0;
        
        nValue[0] = (UINT32)(pSrcPrev[i - 1]);
        nValue[0] += (UINT32)(pSrcPrev[i]);
        nValue[0] += (UINT32)(pSrcPrev[i + 1]);
        
        nValue[1] = (UINT32)(pSrc[i - 1]);
        nValue[1] += (UINT32)(pSrc[i]);
        nValue[1] += (UINT32)(pSrc[i + 1]);
        
        nValue[2] = (UINT32)(pSrcNext0[i - 1]);
        nValue[2] += (UINT32)(pSrcNext0[i]);
        nValue[2] += (UINT32)(pSrcNext0[i + 1]);
        
        nValue[3] = (UINT32)(pSrcNext1[i - 1]);
        nValue[3] += (UINT32)(pSrcNext1[i]);
        nValue[3] += (UINT32)(pSrcNext1[i + 1]);
        
        nValue[4] = (UINT32)(pSrcNext2[i - 1]);
        nValue[4] += (UINT32)(pSrcNext2[i]);
        nValue[4] += (UINT32)(pSrcNext2[i + 1]);
        
        nTmpDst0 = ((nValue[0] + nValue[1] + nValue[2]) / 9);
        nTmpDst1 = ((nValue[1] + nValue[2] + nValue[3]) / 9);
        nTmpDst2 = ((nValue[2] + nValue[3] + nValue[4]) / 9);
        
        if(ABSM(((UINT32)pSrc[i]) - nTmpDst0) < 5)
            *pDst0++ = (UINT8)(CLIP3(PIXEL_MIN, PIXEL_MAX, nTmpDst0));
        else
            *pDst0++ = pSrc[i];
        
        if(ABSM(((UINT32)pSrcNext0[i]) - nTmpDst1) < 5)
            *pDst1++ = (UINT8)(CLIP3(PIXEL_MIN, PIXEL_MAX, nTmpDst1));
        else
            *pDst1++ = pSrcNext0[i];
        
        if(ABSM(((UINT32)pSrcNext1[i]) - nTmpDst2) < 5)
            *pDst2++ = (UINT8)(CLIP3(PIXEL_MIN, PIXEL_MAX, nTmpDst2));
        else
            *pDst2++ = pSrcNext1[i];
    }
    
    return;
}


static void _GetMean4x4_C(IN const UINT8 *pSrc, IN const INT32 nSrcStride, OUT UINT32 *pSum)
{
    UINT32                  nTmpSum = 0;
    INT32                   i = 0;
    
    // Sum
    for(i=0 ; i<4 ; i++)
    {
        nTmpSum += pSrc[0];
        nTmpSum += pSrc[1];
        nTmpSum += pSrc[2];
        nTmpSum += pSrc[3];
        
        pSrc += nSrcStride;
    }
    
    *pSum = nTmpSum;
}


static void _GetMean8x8_C(IN const UINT8 *pSrc, IN const INT32 nSrcStride, OUT UINT32 *pSum)
{
    UINT32                  nTmpSum = 0;
    INT32                   i = 0;
    
    // Sum
    for(i=0 ; i<8 ; i++)
    {
        nTmpSum += pSrc[0];
        nTmpSum += pSrc[1];
        nTmpSum += pSrc[2];
        nTmpSum += pSrc[3];
        nTmpSum += pSrc[4];
        nTmpSum += pSrc[5];
        nTmpSum += pSrc[6];
        nTmpSum += pSrc[7];
        
        pSrc += nSrcStride;
    }
    
    *pSum = nTmpSum;
}


static void _GetMean16x16_C(IN const UINT8 *pSrc, IN const INT32 nSrcStride, OUT UINT32 *pSum)
{
    UINT32                  nTmpSum = 0;
    INT32                   i = 0, j = 0;
    
    // Sum
    for(j=0 ; j<16 ; j++)
    {
        const UINT8         *pLocalSrc = pSrc;
        
        for(i=0 ; i<2 ; i++)
        {
            nTmpSum += pLocalSrc[0];
            nTmpSum += pLocalSrc[1];
            nTmpSum += pLocalSrc[2];
            nTmpSum += pLocalSrc[3];
            nTmpSum += pLocalSrc[4];
            nTmpSum += pLocalSrc[5];
            nTmpSum += pLocalSrc[6];
            nTmpSum += pLocalSrc[7];
            
            pLocalSrc += 8;
        }
        
        pSrc += nSrcStride;
    }
    
    *pSum = nTmpSum;
}


static void _GetVar4x4_C(IN const UINT8 *pSrc, IN const INT32 nSrcStride, OUT UINT32 *pSqSum, OUT UINT32 *pSum)
{
    UINT32                  nTmpSum = 0;
    UINT32                  nTmpSqSum = 0;
    INT32                   i = 0;
    
    // Sum
    for(i=0 ; i<4 ; i++)
    {
        nTmpSum += pSrc[0];
        nTmpSum += pSrc[1];
        nTmpSum += pSrc[2];
        nTmpSum += pSrc[3];
        
        nTmpSqSum += (pSrc[0] * pSrc[0]);
        nTmpSqSum += (pSrc[1] * pSrc[1]);
        nTmpSqSum += (pSrc[2] * pSrc[2]);
        nTmpSqSum += (pSrc[3] * pSrc[3]);
        
        pSrc += nSrcStride;
    }
    
    *pSum = nTmpSum;
    *pSqSum = nTmpSqSum;
}


static void _GetVar8x8_C(IN const UINT8 *pSrc, IN const INT32 nSrcStride, OUT UINT32 *pSqSum, OUT UINT32 *pSum)
{
    UINT32                  nTmpSum = 0;
    UINT32                  nTmpSqSum = 0;
    INT32                   i = 0;
    
    // Sum
    for(i=0 ; i<8 ; i++)
    {
        nTmpSum += pSrc[0];
        nTmpSum += pSrc[1];
        nTmpSum += pSrc[2];
        nTmpSum += pSrc[3];
        nTmpSum += pSrc[4];
        nTmpSum += pSrc[5];
        nTmpSum += pSrc[6];
        nTmpSum += pSrc[7];
        
        nTmpSqSum += (pSrc[0] * pSrc[0]);
        nTmpSqSum += (pSrc[1] * pSrc[1]);
        nTmpSqSum += (pSrc[2] * pSrc[2]);
        nTmpSqSum += (pSrc[3] * pSrc[3]);
        nTmpSqSum += (pSrc[4] * pSrc[4]);
        nTmpSqSum += (pSrc[5] * pSrc[5]);
        nTmpSqSum += (pSrc[6] * pSrc[6]);
        nTmpSqSum += (pSrc[7] * pSrc[7]);
        
        pSrc += nSrcStride;
    }
    
    *pSum = nTmpSum;
    *pSqSum = nTmpSqSum;
}


static void _GetVar16x16_C(IN const UINT8 *pSrc, IN const INT32 nSrcStride, OUT UINT32 *pSqSum, OUT UINT32 *pSum)
{
    UINT32                  nTmpSum = 0;
    UINT32                  nTmpSqSum = 0;
    INT32                   i = 0, j = 0;
    
    // Sum
    for(j=0 ; j<16 ; j++)
    {
        const UINT8         *pLocalSrc = pSrc;
        
        for(i=0 ; i<2 ; i++)
        {
            nTmpSum += pLocalSrc[0];
            nTmpSum += pLocalSrc[1];
            nTmpSum += pLocalSrc[2];
            nTmpSum += pLocalSrc[3];
            nTmpSum += pLocalSrc[4];
            nTmpSum += pLocalSrc[5];
            nTmpSum += pLocalSrc[6];
            nTmpSum += pLocalSrc[7];
            
            nTmpSqSum += (pLocalSrc[0] * pLocalSrc[0]);
            nTmpSqSum += (pLocalSrc[1] * pLocalSrc[1]);
            nTmpSqSum += (pLocalSrc[2] * pLocalSrc[2]);
            nTmpSqSum += (pLocalSrc[3] * pLocalSrc[3]);
            nTmpSqSum += (pLocalSrc[4] * pLocalSrc[4]);
            nTmpSqSum += (pLocalSrc[5] * pLocalSrc[5]);
            nTmpSqSum += (pLocalSrc[6] * pLocalSrc[6]);
            nTmpSqSum += (pLocalSrc[7] * pLocalSrc[7]);
            
            pLocalSrc += 8;
        }
        
        pSrc += nSrcStride;
    }
    
    *pSum = nTmpSum;
    *pSqSum = nTmpSqSum;
}


static void _GetDiffVar4x4_C(IN const UINT8 *pSrc0, IN const INT32 nSrcStride0,
                               IN const UINT8 *pSrc1, IN const INT32 nSrcStride1, OUT UINT32 *pSqSum, OUT INT32 *pSum)
{
    INT32                   nTmpSum = 0;
    UINT32                  nTmpSqSum = 0;
    INT32                   i = 0;
    
    // Sum
    for(i=0 ; i<4 ; i++)
    {
        const INT32         nDiff0 = abs(pSrc0[0] - pSrc1[0]);
        const INT32         nDiff1 = abs(pSrc0[1] - pSrc1[1]);
        const INT32         nDiff2 = abs(pSrc0[2] - pSrc1[2]);
        const INT32         nDiff3 = abs(pSrc0[3] - pSrc1[3]);
        
        nTmpSum += nDiff0;
        nTmpSum += nDiff1;
        nTmpSum += nDiff2;
        nTmpSum += nDiff3;
        
        nTmpSqSum += (nDiff0 * nDiff0);
        nTmpSqSum += (nDiff1 * nDiff1);
        nTmpSqSum += (nDiff2 * nDiff2);
        nTmpSqSum += (nDiff3 * nDiff3);
        
        pSrc0 += nSrcStride0;
        pSrc1 += nSrcStride1;
    }
    
    *pSum = nTmpSum;
    *pSqSum = nTmpSqSum;
}


static void _GetDiffVar8x8_C(IN const UINT8 *pSrc0, IN const INT32 nSrcStride0,
                               IN const UINT8 *pSrc1, IN const INT32 nSrcStride1, OUT UINT32 *pSqSum, OUT INT32 *pSum)
{
    INT32                  nTmpSum = 0;
    UINT32                  nTmpSqSum = 0;
    INT32                   i = 0;
    
    // Sum
    for(i=0 ; i<4 ; i++)
    {
        const INT32         nDiff0 = abs(pSrc0[0] - pSrc1[0]);
        const INT32         nDiff1 = abs(pSrc0[1] - pSrc1[1]);
        const INT32         nDiff2 = abs(pSrc0[2] - pSrc1[2]);
        const INT32         nDiff3 = abs(pSrc0[3] - pSrc1[3]);
        const INT32         nDiff4 = abs(pSrc0[4] - pSrc1[4]);
        const INT32         nDiff5 = abs(pSrc0[5] - pSrc1[5]);
        const INT32         nDiff6 = abs(pSrc0[6] - pSrc1[6]);
        const INT32         nDiff7 = abs(pSrc0[7] - pSrc1[7]);
        
        nTmpSum += nDiff0;
        nTmpSum += nDiff1;
        nTmpSum += nDiff2;
        nTmpSum += nDiff3;
        nTmpSum += nDiff4;
        nTmpSum += nDiff5;
        nTmpSum += nDiff6;
        nTmpSum += nDiff7;
        
        nTmpSqSum += (nDiff0 * nDiff0);
        nTmpSqSum += (nDiff1 * nDiff1);
        nTmpSqSum += (nDiff2 * nDiff2);
        nTmpSqSum += (nDiff3 * nDiff3);
        nTmpSqSum += (nDiff4 * nDiff4);
        nTmpSqSum += (nDiff5 * nDiff5);
        nTmpSqSum += (nDiff6 * nDiff6);
        nTmpSqSum += (nDiff7 * nDiff7);
        
        pSrc0 += nSrcStride0;
        pSrc1 += nSrcStride1;
    }
    
    *pSum = nTmpSum;
    *pSqSum = nTmpSqSum;
}


static void _GetDiffVar16x16_C(IN const UINT8 *pSrc0, IN const INT32 nSrcStride0,
                                 IN const UINT8 *pSrc1, IN const INT32 nSrcStride1, OUT UINT32 *pSqSum, OUT INT32 *pSum)
{
    INT32                   nTmpSum = 0;
    UINT32                  nTmpSqSum = 0;
    INT32                   i = 0, j = 0;
    
    // Sum
    for(j=0 ; j<16 ; j++)
    {
        const UINT8         *pLocalSrc0 = pSrc0;
        const UINT8         *pLocalSrc1 = pSrc1;
        
        for(i=0 ; i<2 ; i++)
        {
            const INT32     nDiff0 = abs(pLocalSrc0[0] - pLocalSrc1[0]);
            const INT32     nDiff1 = abs(pLocalSrc0[1] - pLocalSrc1[1]);
            const INT32     nDiff2 = abs(pLocalSrc0[2] - pLocalSrc1[2]);
            const INT32     nDiff3 = abs(pLocalSrc0[3] - pLocalSrc1[3]);
            const INT32     nDiff4 = abs(pLocalSrc0[4] - pLocalSrc1[4]);
            const INT32     nDiff5 = abs(pLocalSrc0[5] - pLocalSrc1[5]);
            const INT32     nDiff6 = abs(pLocalSrc0[6] - pLocalSrc1[6]);
            const INT32     nDiff7 = abs(pLocalSrc0[7] - pLocalSrc1[7]);
            
            nTmpSum += nDiff0;
            nTmpSum += nDiff1;
            nTmpSum += nDiff2;
            nTmpSum += nDiff3;
            nTmpSum += nDiff4;
            nTmpSum += nDiff5;
            nTmpSum += nDiff6;
            nTmpSum += nDiff7;
            
            nTmpSqSum += (nDiff0 * nDiff0);
            nTmpSqSum += (nDiff1 * nDiff1);
            nTmpSqSum += (nDiff2 * nDiff2);
            nTmpSqSum += (nDiff3 * nDiff3);
            nTmpSqSum += (nDiff4 * nDiff4);
            nTmpSqSum += (nDiff5 * nDiff5);
            nTmpSqSum += (nDiff6 * nDiff6);
            nTmpSqSum += (nDiff7 * nDiff7);
            
            pLocalSrc0 += 8;
            pLocalSrc1 += 8;
        }
        
        pSrc0 += nSrcStride0;
        pSrc1 += nSrcStride1;
    }
    
    *pSum = nTmpSum;
    *pSqSum = nTmpSqSum;
}


static void _GetMean_C(IN const UINT8 *pSrc, IN const INT32 nSrcStride, IN const INT32 nWidth, IN const INT32 nHeight,
                         OUT UINT32 *pSum, fpgetNxMMean fpMean, IN const INT32 nBlockSize)
{
    INT32                   i = 0, j = 0;
    
    *pSum = 0;
    
    for(i=0 ; i<nHeight ; i+=nBlockSize)
    {
        for(j=0 ; j<nWidth ; j+=nBlockSize)
        {
            UINT32          nLocalSum = 0;
            
            fpMean(pSrc + nSrcStride * i + j, nSrcStride, &nLocalSum);
            
            *pSum += nLocalSum;
        }
    }
}


static void _GetVariance_C(IN const UINT8 *pSrc, IN const INT32 nSrcStride, IN const INT32 nWidth, IN const INT32 nHeight,
                             OUT UINT32 *pSqSum, OUT UINT32 *pSum, fpgetNxMVar fpVar, IN const INT32 nBlockSize)
{
    INT32                   i = 0, j = 0;
    
    *pSum = 0;
    *pSqSum = 0;
    
    for(i=0 ; i<nHeight ; i+=nBlockSize)
    {
        for(j=0 ; j<nWidth ; j+=nBlockSize)
        {
            UINT32          nLocalSum = 0;
            UINT32          nLocalSqSum = 0;
            
            fpVar(pSrc + nSrcStride * i + j, nSrcStride, &nLocalSqSum, &nLocalSum);
            
            *pSum += nLocalSum;
            *pSqSum += nLocalSqSum;
        }
    }
}


static void _GetDiffVariance_C(IN const UINT8 *pSrc0, IN const INT32 nSrcStride0, IN const UINT8 *pSrc1, IN const INT32 nSrcStride1,
                                 IN const INT32 nWidth, IN const INT32 nHeight, OUT UINT32 *pSqSum, OUT INT32 *pSum,
                                 fpgetNxMDiffVar fpDiffVar, IN const INT32 nBlockSize)
{
    INT32                   i = 0, j = 0;
    
    *pSum = 0;
    *pSqSum = 0;
    
    for(i=0 ; i<nHeight ; i+=nBlockSize)
    {
        for(j=0 ; j<nWidth ; j+=nBlockSize)
        {
            INT32          nLocalSum = 0;
            UINT32          nLocalSqSum = 0;
            
            fpDiffVar(pSrc0 + nSrcStride0 * i + j, nSrcStride0,
                      pSrc1 + nSrcStride1 * i + j, nSrcStride1, &nLocalSqSum, &nLocalSum);
            
            *pSum += nLocalSum;
            *pSqSum += nLocalSqSum;
        }
    }
}


F32 Mean_MxN_C(IN const UINT8 *pSrc, IN const INT32 nSrcStride, IN const INT32 nSizeX, IN const INT32 nSizeY, OUT UINT32 *pSum)
{
    const INT32             nPatchSize = nSizeX * nSizeY;
    UINT32                  nLocalSum = 0;
    
    if(16 == nPatchSize)
        _GetMean4x4_C(pSrc, nSrcStride, &nLocalSum);
    else if(32 == nPatchSize)
        _GetMean_C(pSrc, nSrcStride, nSizeX, nSizeY, &nLocalSum, _GetMean4x4_C, 4);
    else if(64 == nPatchSize)
        _GetMean8x8_C(pSrc, nSrcStride, &nLocalSum);
    else if(128 == nPatchSize)
        _GetMean_C(pSrc, nSrcStride, nSizeX, nSizeY, &nLocalSum, _GetMean8x8_C, 8);
    else if(256 == nPatchSize)
        _GetMean16x16_C(pSrc, nSrcStride, &nLocalSum);
    
    *pSum = nLocalSum;
    
    return (F32)nLocalSum / (F32)nPatchSize;
}


F32 Variance_MxN_C(IN const UINT8 *pSrc, IN const INT32 nSrcStride, IN const INT32 nSizeX, IN const INT32 nSizeY, OUT UINT32 *pSqSum, OUT UINT32 *pSum)
{
    const INT32             nPatchSize = nSizeX * nSizeY;
    UINT32                  nLocalSum = 0;
    
    if(16 == nPatchSize)
        _GetVar4x4_C(pSrc, nSrcStride, pSqSum, &nLocalSum);
    else if(32 == nPatchSize)
        _GetVariance_C(pSrc, nSrcStride, nSizeX, nSizeY, pSqSum, &nLocalSum, _GetVar4x4_C, 4);
    else if(64 == nPatchSize)
        _GetVar8x8_C(pSrc, nSrcStride, pSqSum, &nLocalSum);
    else if(128 == nPatchSize)
        _GetVariance_C(pSrc, nSrcStride, nSizeX, nSizeY, pSqSum, &nLocalSum, _GetVar8x8_C, 8);
    else if(256 == nPatchSize)
        _GetVar16x16_C(pSrc, nSrcStride, pSqSum, &nLocalSum);
    
    *pSum = nLocalSum;
    
    return ((F32)(*pSqSum) / (F32)nPatchSize) - ((F32)((*pSum) * (*pSum)) / (F32)(nPatchSize * nPatchSize));
}


F32 DiffVariance_MxN_C(IN const UINT8 *pSrc0, IN const INT32 nSrcStride0, IN const UINT8 *pSrc1, IN const INT32 nSrcStride1,
                         IN const INT32 nSizeX, IN const INT32 nSizeY, OUT UINT32 *pSqSum, OUT INT32 *pSum)
{
    const INT32             nPatchSize = nSizeX * nSizeY;
    INT32                   nLocalSum = 0;
    
    if(16 == nPatchSize)
        _GetDiffVar4x4_C(pSrc0, nSrcStride0, pSrc1, nSrcStride1, pSqSum, &nLocalSum);
    else if(32 == nPatchSize)
        _GetDiffVariance_C(pSrc0, nSrcStride0, pSrc1, nSrcStride1, nSizeX, nSizeY, pSqSum, &nLocalSum, _GetDiffVar4x4_C, 4);
    else if(64 == nPatchSize)
        _GetDiffVar8x8_C(pSrc0, nSrcStride0, pSrc1, nSrcStride1, pSqSum, &nLocalSum);
    else if(128 == nPatchSize)
        _GetDiffVariance_C(pSrc0, nSrcStride0, pSrc1, nSrcStride1, nSizeX, nSizeY, pSqSum, &nLocalSum, _GetDiffVar8x8_C, 8);
    else if(256 == nPatchSize)
        _GetDiffVar16x16_C(pSrc0, nSrcStride0, pSrc1, nSrcStride1, pSqSum, &nLocalSum);
    
    *pSum = nLocalSum;
    
    return ((F32)(*pSqSum) / (F32)nPatchSize) - ((F32)((*pSum) * (*pSum)) / (F32)(nPatchSize * nPatchSize));
}
