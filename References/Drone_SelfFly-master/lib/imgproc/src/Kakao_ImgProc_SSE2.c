


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
#define READ64(p, stride, i) \
                _mm_unpacklo_epi8(_mm_cvtsi32_si128(*(const UINT32 *)(p + i * stride)), \
                _mm_cvtsi32_si128(*(const UINT32 *)(p + (i + 1) * stride)))


/*----------------------------------------------------------------------------------------
 Type Definitions
 ----------------------------------------------------------------------------------------*/
typedef void (*fpgetNxMMean)(IN const UINT8 *pSrc, IN const INT32 nSrcStride, OUT UINT32 *pSum);
typedef void (*fpgetNxMVar)(IN const UINT8 *pSrc, IN const INT32 nSrcStride, OUT UINT32 *pSqSum, OUT UINT32 *pSum);
typedef void (*fpgetNxMDiffVar)(IN const UINT8 *pSrc0, IN const INT32 nSrcStride0, IN const UINT8 *pSrc1, IN const INT32 nSrcStride1,
                              OUT UINT32 *pSqSum, OUT INT32 *pSum);


/*----------------------------------------------------------------------------------------
 Static Function
 ----------------------------------------------------------------------------------------*/
static void _GetMean4x4_SSE2(IN const UINT8 *pSrc, IN const INT32 nSrcStride, OUT UINT32 *pSum);
static void _GetMean8x8_SSE2(IN const UINT8 *pSrc, IN const INT32 nSrcStride, OUT UINT32 *pSum);
static void _GetMean16x16_SSE2(IN const UINT8 *pSrc, IN const INT32 nSrcStride, OUT UINT32 *pSum);
static void _GetVar4x4_SSE2(IN const UINT8 *pSrc, IN const INT32 nSrcStride, OUT UINT32 *pSqSum, OUT UINT32 *pSum);
static void _GetVar8x8_SSE2(IN const UINT8 *pSrc, IN const INT32 nSrcStride, OUT UINT32 *pSqSum, OUT UINT32 *pSum);
static void _GetVar16x16_SSE2(IN const UINT8 *pSrc, IN const INT32 nSrcStride, OUT UINT32 *pSqSum, OUT UINT32 *pSum);
static void _GetDiffVar4x4_SSE2(IN const UINT8 *pSrc0, IN const INT32 nSrcStride0,
                             IN const UINT8 *pSrc1, IN const INT32 nSrcStride1, OUT UINT32 *pSqSum, OUT INT32 *pSum);
static void _GetDiffVar8x8_SSE2(IN const UINT8 *pSrc0, IN const INT32 nSrcStride0,
                             IN const UINT8 *pSrc1, IN const INT32 nSrcStride1, OUT UINT32 *pSqSum, OUT INT32 *pSum);
static void _GetDiffVar16x16_SSE2(IN const UINT8 *pSrc0, IN const INT32 nSrcStride0,
                             IN const UINT8 *pSrc1, IN const INT32 nSrcStride1, OUT UINT32 *pSqSum, OUT INT32 *pSum);
static void _GetMean_SSE2(IN const UINT8 *pSrc, IN const INT32 nSrcStride, IN const INT32 nWidth, IN const INT32 nHeight,
                             OUT UINT32 *pSum, fpgetNxMMean fpMean, IN const INT32 nBlockSize);
static void _GetVariance_SSE2(IN const UINT8 *pSrc0, IN const INT32 nSrcStride0, IN const INT32 nWidth, IN const INT32 nHeight,
                             OUT UINT32 *pSqSum, OUT UINT32 *pSum, fpgetNxMVar fpVar, IN const INT32 nBlockSize);
static void _GetDiffVariance_SSE2(IN const UINT8 *pSrc0, IN const INT32 nSrcStride0, IN const UINT8 *pSrc1, IN const INT32 nSrcStride1,
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
#if ENABLE_ISET(CPU_INTEL_IA32 | ISET_SSE2) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_SSE2)
void OneRow_Copy_SSE2(OUT UINT8 *pDst, IN const UINT8 *pSrc, IN const INT32 nWidth)
{
    MEMCPY(pDst, pSrc, nWidth);
}


void OneRow_InterleaveUV_SSE2(IN const UINT8 *pSrcUV, OUT UINT8 *pDstU, OUT UINT8 *pDstV, IN const INT32 nWidth)
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


void OneRow_Subsample_Even_SSE2(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth)
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


void OneRow_Subsample_Odd_SSE2(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth)
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


void OneRow_Subsample_1_4_SSE2(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth)
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


void OneRow_ScaleBilinear_SSE2(OUT UINT8* pDst,
                            IN const UINT8* pSrc, ptrdiff_t nSrcStride,
                            IN const INT32 nDstWidth, IN const INT32 nSrcYFraction)
{
    ;
}


void OneCol_ScaleBilinear_SSE2(OUT UINT8* pDst, IN const UINT8* pSrc,
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


void OneRow_MeanBlur_SSE2(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth, IN const INT32 nSrcStride, IN const INT32 nDstStride)
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


void OneRow_MeanBlur_Hor_SSE2(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth)
{
    INT32                   i = 0;
    const UINT8             *pSrcPrev = pSrc - 1;
    const UINT8             *pSrcNext = pSrc + 1;
    
    for(i=0 ; i<nWidth ; i++)
        pDst[i] = (pSrcPrev[i] + pSrc[i] + pSrcNext[i]) / 3;
    
    return;
}


void OneRow_MeanBlur_Ver_SSE2(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth, IN const INT32 nSrcStride, IN const INT32 nDstStride)
{
    INT32                   i = 0;
    const UINT8             *pSrcPrev = pSrc - nSrcStride;
    const UINT8             *pSrcNext = pSrc + nSrcStride;
    
    for(i=0 ; i<nWidth ; i++)
        pDst[i] = (pSrcPrev[i] + pSrc[i] + pSrcNext[i]) / 3;
    
    return;
}


void OneRow_Mosaic4_SSE2(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth, IN const INT32 nSrcStride, IN const INT32 nDstStride, IN const RectImgAddr *pBaseRectAddr)
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


void OneRow_Mosaic8_SSE2(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth, IN const INT32 nSrcStride, IN const INT32 nDstStride, IN const RectImgAddr *pBaseRectAddr)
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


void OneRow_Mosaic16_SSE2(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth, IN const INT32 nSrcStride, IN const INT32 nDstStride, IN const RectImgAddr *pBaseRectAddr)
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


void OneRow_Inverse_SSE2(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth)
{
    INT32                   i = 0;
    const INT32             nLoopNum = nWidth >> 4;
    const INT32             nOffset = 16;
    const __m128i           nPixVal255 = _mm_set1_epi8(255);
    
    for(i=0 ; i<nLoopNum ; i++)
    {
        const __m128i       nSrc = _mm_loadu_si128(pSrc);
        
        *(__m128i *)pDst = _mm_subs_epu8(nPixVal255, nSrc);
        
        pSrc += nOffset;
        pDst += nOffset;
    }
    
    return;
}


void OneRow_Clip_SSE2(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth, IN const INT32 nMinClipVal, IN const INT32 nMaxClipVal)
{
    INT32                   i = 0;
    
    for(i=0 ; i<nWidth ; i++)
        pDst[i] = CLIP3(nMinClipVal, nMaxClipVal, pSrc[i]);
    
    return;
}


void OneRow_DiffImg_SSE2(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, OUT UINT8 *pDst, IN const INT32 nWidth)
{
    INT32                   i = 0;
    const INT32             nLoopNum = nWidth >> 4;
    const INT32             nOffset = 16;
    
    for(i=0 ; i<nLoopNum ; i++)
    {
        *(__m128i *)pDst = _mm_subs_epu8(*(__m128i *)pSrc0, *(__m128i *)pSrc1);
        
        pSrc0 += nOffset;
        pSrc1 += nOffset;
        pDst += nOffset;
    }
    
    return;
}


void OneRow_Multiply_SSE2(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, OUT UINT8 *pDst, IN const INT32 nWidth)
{
    INT32                   i = 0;
    const INT32             nLoopNum = nWidth >> 4;
    const INT32             nOffset = 16;
    const __m128i           nZero  = _mm_set1_epi8(0);
    
    for(i=0 ; i<nLoopNum ; i++)
    {
        __m128i             nSrcL16b_0, nSrcL16b_1;
        __m128i             nSrcH16b_0, nSrcH16b_1;
        
        nSrcL16b_0 = _mm_unpacklo_epi8(*(__m128i *)pSrc0, nZero);
        nSrcH16b_0 = _mm_unpackhi_epi8(*(__m128i *)pSrc0, nZero);
        nSrcL16b_1 = _mm_unpacklo_epi8(*(__m128i *)pSrc1, nZero);
        nSrcH16b_1 = _mm_unpackhi_epi8(*(__m128i *)pSrc1, nZero);
        
        *(__m128i *)pDst = _mm_packus_epi16(_mm_srli_epi16(_mm_mullo_epi16(nSrcL16b_0, nSrcL16b_1), 8),
                                            _mm_srli_epi16(_mm_mullo_epi16(nSrcH16b_0, nSrcH16b_1), 8));
        
        pSrc0 += nOffset;
        pSrc1 += nOffset;
        pDst += nOffset;
    }
    
    return;
}


void OneRow_ContrastShift_SSE2(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, OUT UINT8 *pDst, IN const INT32 nWidth)
{
    INT32                   i = 0;
    const INT32             nLoopNum = nWidth >> 4;
    const INT32             nOffset = 16;
    const __m128i           nZero  = _mm_set1_epi8(0);
    const __m128i           nPixVal128 = _mm_set1_epi8(128);
    const __m128i           nPixVal255 = _mm_set1_epi8(255);
    
    /*
     for(i=0 ; i<nLoopNum ; i++)
     {
     __m128i             nSrc0L16b, nSrc0H16b;
     __m128i             nSrc1L16b, nSrc1H16b;
     __m128i             nMul0L, nMul0H;
     __m128i             nMul1L, nMul1H;
     __m128i             nMul0L_Shift, nMul0H_Shift;
     __m128i             nMul1L_Shift, nMul1H_Shift;
     __m128i             nTmpDst0, nTmpDst1, nInvTmpDst1;
     const __m128i       nCmpTable = _mm_cmpgt_epi8(*(__m128i *)pSrc0, nPixVal128);
     
     nSrc0L16b = _mm_unpacklo_epi8(*(__m128i *)pSrc0, nZero);
     nSrc0H16b = _mm_unpackhi_epi8(*(__m128i *)pSrc0, nZero);
     nSrc1L16b = _mm_unpacklo_epi8(*(__m128i *)pSrc1, nZero);
     nSrc1H16b = _mm_unpackhi_epi8(*(__m128i *)pSrc1, nZero);
     
     nMul0L = _mm_mullo_epi16(nSrc0L16b, nSrc0L16b);
     nMul0H = _mm_mullo_epi16(nSrc0H16b, nSrc0H16b);
     nMul1L = _mm_mullo_epi16(nSrc1L16b, nSrc1L16b);
     nMul1H = _mm_mullo_epi16(nSrc1H16b, nSrc1H16b);
     
     nMul0L_Shift = _mm_srli_epi16(nMul0L, 7);
     nMul0H_Shift = _mm_srli_epi16(nMul0H, 7);
     nMul1L_Shift = _mm_srli_epi16(nMul1L, 7);
     nMul1H_Shift = _mm_srli_epi16(nMul1H, 7);
     
     nTmpDst0 = _mm_packus_epi16(nMul0L_Shift, nMul0H_Shift);
     nTmpDst1 = _mm_packus_epi16(nMul1L_Shift, nMul1H_Shift);
     nInvTmpDst1 = _mm_subs_epu8(nPixVal255, nTmpDst1);
     
     *(__m128i *)pDst = _mm_or_si128(_mm_and_si128(nCmpTable, nInvTmpDst1), _mm_andnot_si128(nCmpTable, nTmpDst0));
     
     pSrc0 += nOffset;
     pSrc1 += nOffset;
     pDst += nOffset;
     }
     */
    
    for(i=0 ; i<nWidth ; i++)
    {
        if(pSrc0[i] > 128)
            pDst[i] = 255 - (UINT8)((UINT16)((UINT16)pSrc1[i] * (UINT16)pSrc1[i]) / 154);
        else
            pDst[i] = (UINT8)((UINT16)((UINT16)pSrc0[i] * (UINT16)pSrc0[i]) / 100);
    }
    
    return;
}


void OneRow_PropMix_SSE2(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, IN const F32 *pCoeffi, OUT UINT8 *pDst, IN const INT32 nWidth)
{
    INT32                   i = 0;
    const INT32             nLoopNum = nWidth >> 4;
    const INT32             nOffset = 16;
    const __m128i           nZero  = _mm_set1_epi8(0);
    const __m128            nFloatOne = _mm_set1_ps(1);
    
    for(i=0 ; i<nLoopNum ; i++)
    {
        __m128i             nSrc0iL16b, nSrc0iH16b;
        __m128i             nSrc1iL16b, nSrc1iH16b;
        __m128i             nSrc0iLL32b, nSrc0iLH32b, nSrc0iHL32b, nSrc0iHH32b;
        __m128i             nSrc1iLL32b, nSrc1iLH32b, nSrc1iHL32b, nSrc1iHH32b;
        __m128              nSrc0fLL32b, nSrc0fLH32b, nSrc0fHL32b, nSrc0fHH32b;
        __m128              nSrc1fLL32b, nSrc1fLH32b, nSrc1fHL32b, nSrc1fHH32b;
        __m128              nCoeffLL32b, nCoeffLH32b, nCoeffHL32b, nCoeffHH32b;
        __m128              nInvCoeffLL32b, nInvCoeffLH32b, nInvCoeffHL32b, nInvCoeffHH32b;
        __m128              nMul00, nMul01, nMul02, nMul03;
        __m128              nMul10, nMul11, nMul12, nMul13;
        __m128              nAddf0, nAddf1, nAddf2, nAddf3;
        __m128i             nAddi0, nAddi1, nAddi2, nAddi3;
        const __m128i       nSrc0 = _mm_loadu_si128(pSrc0);
        const __m128i       nSrc1 = _mm_loadu_si128(pSrc1);
        
        // Load Coefficient
        nCoeffLL32b = _mm_load_ps(pCoeffi);
        nCoeffLH32b = _mm_load_ps(pCoeffi + 4);
        nCoeffHL32b = _mm_load_ps(pCoeffi + 8);
        nCoeffHH32b = _mm_load_ps(pCoeffi + 12);
        
        // Load 8bit Integer & Unpack to 16bit Integer
        nSrc0iL16b = _mm_unpacklo_epi8(nSrc0, nZero);
        nSrc0iH16b = _mm_unpackhi_epi8(nSrc0, nZero);
        nSrc1iL16b = _mm_unpacklo_epi8(nSrc1, nZero);
        nSrc1iH16b = _mm_unpackhi_epi8(nSrc1, nZero);
        
        // Unpack to 32bit Integer
        nSrc0iLL32b = _mm_unpacklo_epi16(nSrc0iL16b, nZero);
        nSrc0iLH32b = _mm_unpackhi_epi16(nSrc0iL16b, nZero);
        nSrc0iHL32b = _mm_unpacklo_epi16(nSrc0iH16b, nZero);
        nSrc0iHH32b = _mm_unpackhi_epi16(nSrc0iH16b, nZero);
        nSrc1iLL32b = _mm_unpacklo_epi16(nSrc1iL16b, nZero);
        nSrc1iLH32b = _mm_unpackhi_epi16(nSrc1iL16b, nZero);
        nSrc1iHL32b = _mm_unpacklo_epi16(nSrc1iH16b, nZero);
        nSrc1iHH32b = _mm_unpackhi_epi16(nSrc1iH16b, nZero);
        
        // Convert Integer to Float
        nSrc0fLL32b = _mm_cvtepi32_ps(nSrc0iLL32b);
        nSrc0fLH32b = _mm_cvtepi32_ps(nSrc0iLH32b);
        nSrc0fHL32b = _mm_cvtepi32_ps(nSrc0iHL32b);
        nSrc0fHH32b = _mm_cvtepi32_ps(nSrc0iHH32b);
        nSrc1fLL32b = _mm_cvtepi32_ps(nSrc1iLL32b);
        nSrc1fLH32b = _mm_cvtepi32_ps(nSrc1iLH32b);
        nSrc1fHL32b = _mm_cvtepi32_ps(nSrc1iHL32b);
        nSrc1fHH32b = _mm_cvtepi32_ps(nSrc1iHH32b);
        
        // Make Inverse Coefficient
        nInvCoeffLL32b = _mm_sub_ps(nFloatOne, nCoeffLL32b);
        nInvCoeffLH32b = _mm_sub_ps(nFloatOne, nCoeffLH32b);
        nInvCoeffHL32b = _mm_sub_ps(nFloatOne, nCoeffHL32b);
        nInvCoeffHH32b = _mm_sub_ps(nFloatOne, nCoeffHH32b);
        
        // Multiply Src0 & Coefficient
        nMul00 = _mm_mul_ps(nSrc0fLL32b, nCoeffLL32b);
        nMul01 = _mm_mul_ps(nSrc0fLH32b, nCoeffLH32b);
        nMul02 = _mm_mul_ps(nSrc0fHL32b, nCoeffHL32b);
        nMul03 = _mm_mul_ps(nSrc0fHH32b, nCoeffHH32b);
        
        // Multiply Src0 & Coefficient
        nMul10 = _mm_mul_ps(nSrc1fLL32b, nInvCoeffLL32b);
        nMul11 = _mm_mul_ps(nSrc1fLH32b, nInvCoeffLH32b);
        nMul12 = _mm_mul_ps(nSrc1fHL32b, nInvCoeffHL32b);
        nMul13 = _mm_mul_ps(nSrc1fHH32b, nInvCoeffHH32b);
        
        // Add Multipied Result
        nAddf0 = _mm_add_ps(nMul00, nMul10);
        nAddf1 = _mm_add_ps(nMul01, nMul11);
        nAddf2 = _mm_add_ps(nMul02, nMul12);
        nAddf3 = _mm_add_ps(nMul03, nMul13);
        
        // Convert Float to Integer
        nAddi0 = _mm_cvtps_epi32(nAddf0);
        nAddi1 = _mm_cvtps_epi32(nAddf1);
        nAddi2 = _mm_cvtps_epi32(nAddf2);
        nAddi3 = _mm_cvtps_epi32(nAddf3);
        
        *(__m128i *)pDst = _mm_packus_epi16(_mm_packs_epi32(nAddi0, nAddi1), _mm_packs_epi32(nAddi2, nAddi3));
        
        pSrc0 += nOffset;
        pSrc1 += nOffset;
        pCoeffi += nOffset;
        pDst += nOffset;
    }
    
    return;
}


void OneRow_BlurNoiseReduction_SSE2(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth, IN const INT32 nSrcStride, IN const INT32 nDstStride)
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


static void _GetMean4x4_SSE2(IN const UINT8 *pSrc, IN const INT32 nSrcStride, OUT UINT32 *pSum)
{
    const __m128i           nZero = _mm_setzero_si128();
    const __m128i           nSrc0 = _mm_unpacklo_epi8(READ64(pSrc, nSrcStride, 0), nZero);
    const __m128i           nSrc1 = _mm_unpacklo_epi8(READ64(pSrc, nSrcStride, 2), nZero);
    __m128i                 nTmpSum;
    
    // Sum
    nTmpSum = _mm_add_epi16(nSrc0, nSrc1);
    nTmpSum = _mm_add_epi16(nTmpSum, _mm_srli_si128(nTmpSum, 8));
    nTmpSum = _mm_add_epi16(nTmpSum, _mm_srli_si128(nTmpSum, 4));
    nTmpSum = _mm_add_epi16(nTmpSum, _mm_srli_si128(nTmpSum, 2));
    *pSum = (UINT32)_mm_extract_epi16(nTmpSum, 0);
}


static void _GetMean8x8_SSE2(IN const UINT8 *pSrc, IN const INT32 nSrcStride, OUT UINT32 *pSum)
{
    const __m128i           nZero = _mm_setzero_si128();
    __m128i                 nTmpSum = _mm_setzero_si128();
    __m128i                 nTmpSqSum = _mm_setzero_si128();
    INT32                   i = 0;
    
    for(i=0 ; i<8 ; i+=2)
    {
        const __m128i nSrc0 = _mm_unpacklo_epi8(_mm_loadl_epi64((const __m128i *)pSrc), nZero);
        const __m128i nSrc1 = _mm_unpacklo_epi8(_mm_loadl_epi64((const __m128i *)(pSrc + nSrcStride)), nZero);
        
        nTmpSum = _mm_add_epi16(nTmpSum, nSrc0);
        nTmpSum = _mm_add_epi16(nTmpSum, nSrc1);
        
        pSrc += (2 * nSrcStride);
    }
    
    // Sum
    nTmpSum = _mm_add_epi16(nTmpSum, _mm_srli_si128(nTmpSum, 8));
    nTmpSum = _mm_add_epi16(nTmpSum, _mm_srli_si128(nTmpSum, 4));
    nTmpSum = _mm_add_epi16(nTmpSum, _mm_srli_si128(nTmpSum, 2));
    *pSum = (UINT32)_mm_extract_epi16(nTmpSum, 0);
}


static void _GetMean16x16_SSE2(IN const UINT8 *pSrc, IN const INT32 nSrcStride, OUT UINT32 *pSum)
{
    const __m128i           nZero = _mm_setzero_si128();
    __m128i                 nTmpSum = _mm_setzero_si128();
    INT32                   i = 0;
    
    for(i=0 ; i<16 ; i++)
    {
        const __m128i       nSrc = _mm_loadu_si128((const __m128i *)pSrc);
        const __m128i       nSrc0 = _mm_unpacklo_epi8(nSrc, nZero);
        const __m128i       nSrc1 = _mm_unpackhi_epi8(nSrc, nZero);
        
        nTmpSum = _mm_add_epi16(nTmpSum, nSrc0);
        nTmpSum = _mm_add_epi16(nTmpSum, nSrc1);
        
        pSrc += nSrcStride;
    }
    
    // sum
    nTmpSum = _mm_add_epi16(nTmpSum, _mm_srli_si128(nTmpSum, 8));
    nTmpSum = _mm_add_epi16(nTmpSum, _mm_srli_si128(nTmpSum, 4));
    *pSum = (UINT32)_mm_extract_epi16(nTmpSum, 0) + (UINT32)_mm_extract_epi16(nTmpSum, 1);
}


static void _GetVar4x4_SSE2(IN const UINT8 *pSrc, IN const INT32 nSrcStride, OUT UINT32 *pSqSum, OUT UINT32 *pSum)
{
    const __m128i           nZero = _mm_setzero_si128();
    const __m128i           nSrc0 = _mm_unpacklo_epi8(READ64(pSrc, nSrcStride, 0), nZero);
    const __m128i           nSrc1 = _mm_unpacklo_epi8(READ64(pSrc, nSrcStride, 2), nZero);
    __m128i                 nTmpSum = _mm_setzero_si128();
    __m128i                 nTmpSqSum = _mm_setzero_si128();
    
    // Sum
    nTmpSum = _mm_add_epi16(nSrc0, nSrc1);
    nTmpSum = _mm_add_epi16(nTmpSum, _mm_srli_si128(nTmpSum, 8));
    nTmpSum = _mm_add_epi16(nTmpSum, _mm_srli_si128(nTmpSum, 4));
    nTmpSum = _mm_add_epi16(nTmpSum, _mm_srli_si128(nTmpSum, 2));
    *pSum = (UINT32)_mm_extract_epi16(nTmpSum, 0);
    
    // Squared Sum
    nTmpSqSum = _mm_add_epi32(_mm_madd_epi16(nSrc0, nSrc0), _mm_madd_epi16(nSrc1, nSrc1));
    nTmpSqSum = _mm_add_epi32(nTmpSqSum, _mm_srli_si128(nTmpSqSum, 8));
    nTmpSqSum = _mm_add_epi32(nTmpSqSum, _mm_srli_si128(nTmpSqSum, 4));
    *pSqSum = (UINT32)_mm_cvtsi128_si32(nTmpSqSum);
}


static void _GetVar8x8_SSE2(IN const UINT8 *pSrc, IN const INT32 nSrcStride, OUT UINT32 *pSqSum, OUT UINT32 *pSum)
{
    const __m128i           nZero = _mm_setzero_si128();
    __m128i                 nTmpSum = _mm_setzero_si128();
    __m128i                 nTmpSqSum = _mm_setzero_si128();
    INT32                   i = 0;
    
    for(i=0 ; i<8 ; i+=2)
    {
        const __m128i nSrc0 = _mm_unpacklo_epi8(_mm_loadl_epi64((const __m128i *)(pSrc + i * nSrcStride)), nZero);
        const __m128i nSrc1 = _mm_unpacklo_epi8(_mm_loadl_epi64((const __m128i *)(pSrc + (i + 1) * nSrcStride)), nZero);
        
        nTmpSum = _mm_add_epi16(nTmpSum, nSrc0);
        nTmpSum = _mm_add_epi16(nTmpSum, nSrc1);
        nTmpSqSum = _mm_add_epi32(nTmpSqSum, _mm_madd_epi16(nSrc0, nSrc0));
        nTmpSqSum = _mm_add_epi32(nTmpSqSum, _mm_madd_epi16(nSrc1, nSrc1));
    }
    
    // Sum
    nTmpSum = _mm_add_epi16(nTmpSum, _mm_srli_si128(nTmpSum, 8));
    nTmpSum = _mm_add_epi16(nTmpSum, _mm_srli_si128(nTmpSum, 4));
    nTmpSum = _mm_add_epi16(nTmpSum, _mm_srli_si128(nTmpSum, 2));
    *pSum = (UINT32)_mm_extract_epi16(nTmpSum, 0);
    
    // Squared Sum
    nTmpSqSum = _mm_add_epi32(nTmpSqSum, _mm_srli_si128(nTmpSqSum, 8));
    nTmpSqSum = _mm_add_epi32(nTmpSqSum, _mm_srli_si128(nTmpSqSum, 4));
    *pSqSum = (UINT32)_mm_cvtsi128_si32(nTmpSqSum);
}


static void _GetVar16x16_SSE2(IN const UINT8 *pSrc, IN const INT32 nSrcStride, OUT UINT32 *pSqSum, OUT UINT32 *pSum)
{
    const __m128i           nZero = _mm_setzero_si128();
    __m128i                 nTmpSum = _mm_setzero_si128();
    __m128i                 nTmpSqSum = _mm_setzero_si128();
    INT32                   i = 0;
    
    for(i=0 ; i<16 ; i++)
    {
        const __m128i       s = _mm_loadu_si128((const __m128i *)pSrc);
        
        const __m128i       nSrc0 = _mm_unpacklo_epi8(s, nZero);
        const __m128i       nSrc1 = _mm_unpackhi_epi8(s, nZero);
        
        nTmpSum = _mm_add_epi16(nTmpSum, nSrc0);
        nTmpSum = _mm_add_epi16(nTmpSum, nSrc1);
        nTmpSqSum = _mm_add_epi32(nTmpSqSum, _mm_madd_epi16(nSrc0, nSrc0));
        nTmpSqSum = _mm_add_epi32(nTmpSqSum, _mm_madd_epi16(nSrc1, nSrc1));
        
        pSrc += nSrcStride;
    }
    
    // sum
    nTmpSum = _mm_add_epi16(nTmpSum, _mm_srli_si128(nTmpSum, 8));
    nTmpSum = _mm_add_epi16(nTmpSum, _mm_srli_si128(nTmpSum, 4));
    *pSum = (UINT32)_mm_extract_epi16(nTmpSum, 0) + (UINT32)_mm_extract_epi16(nTmpSum, 1);
    
    // Squared Sum
    nTmpSqSum = _mm_add_epi32(nTmpSqSum, _mm_srli_si128(nTmpSqSum, 8));
    nTmpSqSum = _mm_add_epi32(nTmpSqSum, _mm_srli_si128(nTmpSqSum, 4));
    *pSqSum = (UINT32)_mm_cvtsi128_si32(nTmpSqSum);
}


static void _GetDiffVar4x4_SSE2(IN const UINT8 *pSrc0, IN const INT32 nSrcStride0,
                            IN const UINT8 *pSrc1, IN const INT32 nSrcStride1, OUT UINT32 *pSqSum, OUT INT32 *pSum)
{
    const __m128i           nZero = _mm_setzero_si128();
    const __m128i           nSrc0 = _mm_unpacklo_epi8(READ64(pSrc0, nSrcStride0, 0), nZero);
    const __m128i           nSrc1 = _mm_unpacklo_epi8(READ64(pSrc0, nSrcStride0, 2), nZero);
    const __m128i           nRef0 = _mm_unpacklo_epi8(READ64(pSrc1, nSrcStride1, 0), nZero);
    const __m128i           nRef1 = _mm_unpacklo_epi8(READ64(pSrc1, nSrcStride1, 2), nZero);
    const __m128i           nDiff0 = _mm_sub_epi16(nSrc0, nRef0);
    const __m128i           nDiff1 = _mm_sub_epi16(nSrc1, nRef1);
    __m128i                 nTmpSum = _mm_setzero_si128();
    __m128i                 nTmpSqSum = _mm_setzero_si128();
    
    // Sum
    nTmpSum = _mm_add_epi16(nDiff0, nDiff1);
    nTmpSum = _mm_add_epi16(nTmpSum, _mm_srli_si128(nTmpSum, 8));
    nTmpSum = _mm_add_epi16(nTmpSum, _mm_srli_si128(nTmpSum, 4));
    nTmpSum = _mm_add_epi16(nTmpSum, _mm_srli_si128(nTmpSum, 2));
    *pSum = (INT16)_mm_extract_epi16(nTmpSum, 0);
    
    // Squared Sum
    nTmpSqSum = _mm_add_epi32(_mm_madd_epi16(nDiff0, nDiff0), _mm_madd_epi16(nDiff1, nDiff1));
    nTmpSqSum = _mm_add_epi32(nTmpSqSum, _mm_srli_si128(nTmpSqSum, 8));
    nTmpSqSum = _mm_add_epi32(nTmpSqSum, _mm_srli_si128(nTmpSqSum, 4));
    *pSqSum = (UINT32)_mm_cvtsi128_si32(nTmpSqSum);
}


static void _GetDiffVar8x8_SSE2(IN const UINT8 *pSrc0, IN const INT32 nSrcStride0,
                            IN const UINT8 *pSrc1, IN const INT32 nSrcStride1, OUT UINT32 *pSqSum, OUT INT32 *pSum)
{
    const __m128i           nZero = _mm_setzero_si128();
    __m128i                 nTmpSum = _mm_setzero_si128();
    __m128i                 nTmpSqSum = _mm_setzero_si128();
    INT32                   i = 0;
    
    for(i=0 ; i<8 ; i+=2)
    {
        const __m128i nSrc0 = _mm_unpacklo_epi8(_mm_loadl_epi64((const __m128i *)(pSrc0 + i * nSrcStride0)), nZero);
        const __m128i nRef0 = _mm_unpacklo_epi8(_mm_loadl_epi64((const __m128i *)(pSrc1 + i * nSrcStride1)), nZero);
        const __m128i nDiff0 = _mm_sub_epi16(nSrc0, nRef0);
        
        const __m128i nSrc1 = _mm_unpacklo_epi8(_mm_loadl_epi64((const __m128i *)(pSrc0 + (i + 1) * nSrcStride0)), nZero);
        const __m128i nRef1 = _mm_unpacklo_epi8(_mm_loadl_epi64((const __m128i *)(pSrc1 + (i + 1) * nSrcStride1)), nZero);
        const __m128i nDiff1 = _mm_sub_epi16(nSrc1, nRef1);
        
        nTmpSum = _mm_add_epi16(nTmpSum, nDiff0);
        nTmpSum = _mm_add_epi16(nTmpSum, nDiff1);
        nTmpSqSum = _mm_add_epi32(nTmpSqSum, _mm_madd_epi16(nDiff0, nDiff0));
        nTmpSqSum = _mm_add_epi32(nTmpSqSum, _mm_madd_epi16(nDiff1, nDiff1));
    }
    
    // Sum
    nTmpSum = _mm_add_epi16(nTmpSum, _mm_srli_si128(nTmpSum, 8));
    nTmpSum = _mm_add_epi16(nTmpSum, _mm_srli_si128(nTmpSum, 4));
    nTmpSum = _mm_add_epi16(nTmpSum, _mm_srli_si128(nTmpSum, 2));
    *pSum = (INT16)_mm_extract_epi16(nTmpSum, 0);
    
    // Squared Sum
    nTmpSqSum = _mm_add_epi32(nTmpSqSum, _mm_srli_si128(nTmpSqSum, 8));
    nTmpSqSum = _mm_add_epi32(nTmpSqSum, _mm_srli_si128(nTmpSqSum, 4));
    *pSqSum = (UINT32)_mm_cvtsi128_si32(nTmpSqSum);
}


static void _GetDiffVar16x16_SSE2(IN const UINT8 *pSrc0, IN const INT32 nSrcStride0,
                              IN const UINT8 *pSrc1, IN const INT32 nSrcStride1, OUT UINT32 *pSqSum, OUT INT32 *pSum)
{
    const __m128i           nZero = _mm_setzero_si128();
    __m128i                 nTmpSum = _mm_setzero_si128();
    __m128i                 nTmpSqSum = _mm_setzero_si128();
    INT32                   i = 0;
    
    for(i=0 ; i<16 ; i++)
    {
        const __m128i       s = _mm_loadu_si128((const __m128i *)pSrc0);
        const __m128i       r = _mm_loadu_si128((const __m128i *)pSrc1);
        
        const __m128i       nSrc0 = _mm_unpacklo_epi8(s, nZero);
        const __m128i       nRef0 = _mm_unpacklo_epi8(r, nZero);
        const __m128i       nDiff0 = _mm_sub_epi16(nSrc0, nRef0);
        
        const __m128i       nSrc1 = _mm_unpackhi_epi8(s, nZero);
        const __m128i       nRef1 = _mm_unpackhi_epi8(r, nZero);
        const __m128i       nDiff1 = _mm_sub_epi16(nSrc1, nRef1);
        
        nTmpSum = _mm_add_epi16(nTmpSum, nDiff0);
        nTmpSum = _mm_add_epi16(nTmpSum, nDiff1);
        nTmpSqSum = _mm_add_epi32(nTmpSqSum, _mm_madd_epi16(nDiff0, nDiff0));
        nTmpSqSum = _mm_add_epi32(nTmpSqSum, _mm_madd_epi16(nDiff1, nDiff1));
        
        pSrc0 += nSrcStride0;
        pSrc1 += nSrcStride1;
    }
    
    // sum
    nTmpSum = _mm_add_epi16(nTmpSum, _mm_srli_si128(nTmpSum, 8));
    nTmpSum = _mm_add_epi16(nTmpSum, _mm_srli_si128(nTmpSum, 4));
    *pSum = (INT16)_mm_extract_epi16(nTmpSum, 0) + (INT16)_mm_extract_epi16(nTmpSum, 1);
    
    // Squared Sum
    nTmpSqSum = _mm_add_epi32(nTmpSqSum, _mm_srli_si128(nTmpSqSum, 8));
    nTmpSqSum = _mm_add_epi32(nTmpSqSum, _mm_srli_si128(nTmpSqSum, 4));
    *pSqSum = (UINT32)_mm_cvtsi128_si32(nTmpSqSum);
}

static void _GetMean_SSE2(IN const UINT8 *pSrc, IN const INT32 nSrcStride, IN const INT32 nWidth, IN const INT32 nHeight,
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


static void _GetVariance_SSE2(IN const UINT8 *pSrc, IN const INT32 nSrcStride, IN const INT32 nWidth, IN const INT32 nHeight,
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


static void _GetDiffVariance_SSE2(IN const UINT8 *pSrc0, IN const INT32 nSrcStride0, IN const UINT8 *pSrc1, IN const INT32 nSrcStride1,
                                  IN const INT32 nWidth, IN const INT32 nHeight,
                                  OUT UINT32 *pSqSum, OUT INT32 *pSum, fpgetNxMDiffVar fpDiffVar, IN const INT32 nBlockSize)
{
    INT32                   i = 0, j = 0;
    
    *pSum = 0;
    *pSqSum = 0;
    
    for(i=0 ; i<nHeight ; i+=nBlockSize)
    {
        for(j=0 ; j<nWidth ; j+=nBlockSize)
        {
            INT32           nLocalSum = 0;
            UINT32          nLocalSqSum = 0;
            
            fpDiffVar(pSrc0 + nSrcStride0 * i + j, nSrcStride0,
                      pSrc1 + nSrcStride1 * i + j, nSrcStride1,
                      &nLocalSqSum, &nLocalSum);
            
            *pSum += nLocalSum;
            *pSqSum += nLocalSqSum;
        }
    }
}


F32 Mean_MxN_SSE2(IN const UINT8 *pSrc, IN const INT32 nSrcStride, IN const INT32 nSizeX, IN const INT32 nSizeY, OUT UINT32 *pSum)
{
    const INT32             nPatchSize = nSizeX * nSizeY;
    UINT32                  nLocalSum = 0;
    
    if(16 == nPatchSize)
        _GetMean4x4_SSE2(pSrc, nSrcStride, &nLocalSum);
    else if(32 == nPatchSize)
        _GetMean_SSE2(pSrc, nSrcStride, nSizeX, nSizeY, &nLocalSum, _GetMean4x4_SSE2, 4);
    else if(64 == nPatchSize)
        _GetMean8x8_SSE2(pSrc, nSrcStride, &nLocalSum);
    else if(128 == nPatchSize)
        _GetMean_SSE2(pSrc, nSrcStride, nSizeX, nSizeY, &nLocalSum, _GetMean8x8_SSE2, 8);
    else if(256 == nPatchSize)
        _GetMean16x16_SSE2(pSrc, nSrcStride, &nLocalSum);
    
    *pSum = nLocalSum;
    
    return (F32)nLocalSum / (F32)nPatchSize;
}


F32 Variance_MxN_SSE2(IN const UINT8 *pSrc, IN const INT32 nSrcStride, IN const INT32 nSizeX, IN const INT32 nSizeY, OUT UINT32 *pSqSum, OUT UINT32 *pSum)
{
    const INT32             nPatchSize = nSizeX * nSizeY;
    UINT32                  nLocalSum = 0;
    
    if(16 == nPatchSize)
        _GetVar4x4_SSE2(pSrc, nSrcStride, pSqSum, &nLocalSum);
    else if(32 == nPatchSize)
        _GetVariance_SSE2(pSrc, nSrcStride, nSizeX, nSizeY, pSqSum, &nLocalSum, _GetVar4x4_SSE2, 4);
    else if(64 == nPatchSize)
        _GetVar8x8_SSE2(pSrc, nSrcStride, pSqSum, &nLocalSum);
    else if(128 == nPatchSize)
        _GetVariance_SSE2(pSrc, nSrcStride, nSizeX, nSizeY, pSqSum, &nLocalSum, _GetVar8x8_SSE2, 8);
    else if(256 == nPatchSize)
        _GetVar16x16_SSE2(pSrc, nSrcStride, pSqSum, &nLocalSum);
    
    *pSum = nLocalSum;
    
    return ((F32)(*pSqSum) / (F32)nPatchSize) - ((F32)((*pSum) * (*pSum)) / (F32)(nPatchSize * nPatchSize));
}


F32 DiffVariance_MxN_SSE2(IN const UINT8 *pSrc0, IN const INT32 nSrcStride0, IN const UINT8 *pSrc1, IN const INT32 nSrcStride1,
                          IN const INT32 nSizeX, IN const INT32 nSizeY, OUT UINT32 *pSqSum, OUT INT32 *pSum)
{
    const INT32             nPatchSize = nSizeX * nSizeY;
    INT32                   nLocalSum = 0;
    
    if(16 == nPatchSize)
        _GetDiffVar4x4_SSE2(pSrc0, nSrcStride0, pSrc1, nSrcStride1, pSqSum, &nLocalSum);
    else if(32 == nPatchSize)
        _GetDiffVariance_SSE2(pSrc0, nSrcStride0, pSrc1, nSrcStride1, nSizeX, nSizeY, pSqSum, &nLocalSum, _GetDiffVar4x4_SSE2, 4);
    else if(64 == nPatchSize)
        _GetDiffVar8x8_SSE2(pSrc0, nSrcStride0, pSrc1, nSrcStride1, pSqSum, &nLocalSum);
    else if(128 == nPatchSize)
        _GetDiffVariance_SSE2(pSrc0, nSrcStride0, pSrc1, nSrcStride1, nSizeX, nSizeY, pSqSum, &nLocalSum, _GetDiffVar8x8_SSE2, 8);
    else if(256 == nPatchSize)
        _GetDiffVar16x16_SSE2(pSrc0, nSrcStride0, pSrc1, nSrcStride1, pSqSum, &nLocalSum);
    
    *pSum = nLocalSum;
    
    return ((F32)(*pSqSum) / (F32)nPatchSize) - ((F32)((*pSum) * (*pSum)) / (F32)(nPatchSize * nPatchSize));
}
#endif






