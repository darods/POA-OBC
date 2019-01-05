//
//  Kakao_ImgProc_SSE.h
//
//  Created by Maverick Park on 2014. 8. 2.
//  Copyright (c) 2015년 Kakao Corp. All rights reserved.
//

#ifndef __KAKAO_IMGPROC_SSE_H__
#define __KAKAO_IMGPROC_SSE_H__

#include "Kakao_ImgProc_Core.h"

#ifdef __cplusplus
extern "C" {
#endif

    
#if ENABLE_ISET(CPU_INTEL_IA32 | ISET_SSE) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_SSE)
    void OneRow_Copy_SSE(OUT UINT8 *pDst, IN const UINT8 *pSrc, IN const INT32 nWidth);
    void OneRow_InterleaveUV_SSE(IN const UINT8 *pSrcUV, OUT UINT8 *pDstU, OUT UINT8 *pDstV, IN const INT32 nWidth);
    void OneRow_Subsample_Even_SSE(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth);
    void OneRow_Subsample_Odd_SSE(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth);
    void OneRow_Subsample_1_4_SSE(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth);
    void OneRow_ScaleBilinear_SSE(OUT UINT8* pDst,
                               IN const UINT8* pSrc, ptrdiff_t nSrcStride,
                               IN const INT32 nDstWidth, IN const INT32 nSrcYFraction);
    void OneCol_ScaleBilinear_SSE(OUT UINT8* pDst, IN const UINT8* pSrc,
                               IN const INT32 nDstWidth, IN INT32 x, IN const INT32 dx);
    void OneRow_MeanBlur_SSE(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth,
                               IN const INT32 nSrcStride, IN const INT32 nDstStride);
    void OneRow_MeanBlur_Hor_SSE(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth);
    void OneRow_MeanBlur_Ver_SSE(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth,
                               IN const INT32 nSrcStride, IN const INT32 nDstStride);
    void OneRow_Mosaic4_SSE(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth,
                               IN const INT32 nSrcStride, IN const INT32 nDstStride, IN const RectImgAddr *pBaseRectAddr);
    void OneRow_Mosaic8_SSE(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth,
                               IN const INT32 nSrcStride, IN const INT32 nDstStride, IN const RectImgAddr *pBaseRectAddr);
    void OneRow_Mosaic16_SSE(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth,
                               IN const INT32 nSrcStride, IN const INT32 nDstStride, IN const RectImgAddr *pBaseRectAddr);
    void OneRow_Inverse_SSE(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth);
    void OneRow_Clip_SSE(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth,
                               IN const INT32 nMinClipVal, IN const INT32 nMaxClipVal);
    void OneRow_DiffImg_SSE(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, OUT UINT8 *pDst, IN const INT32 nWidth);
    void OneRow_Multiply_SSE(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, OUT UINT8 *pDst, IN const INT32 nWidth);
    void OneRow_ContrastShift_SSE(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, OUT UINT8 *pDst, IN const INT32 nWidth);
    void OneRow_PropMix_SSE(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, IN const F32 *pCoeffi, OUT UINT8 *pDst, IN const INT32 nWidth);
    void OneRow_BlurNoiseReduction_SSE(IN const UINT8 *pSrc, OUT UINT8 *pDst,
                               IN const INT32 nWidth, IN const INT32 nSrcStride, IN const INT32 nDstStride);
    F32 Mean_MxN_SSE(IN const UINT8 *pSrc, IN const INT32 nSrcStride, IN const INT32 nSizeX, IN const INT32 nSizeY, OUT UINT32 *pSum);
    F32 Variance_MxN_SSE(IN const UINT8 *pSrc, IN const INT32 nSrcStride,
                               IN const INT32 nSizeX, IN const INT32 nSizeY, OUT UINT32 *pSqSum, OUT UINT32 *pSum);
    F32 DiffVariance_MxN_SSE(IN const UINT8 *pSrc0, IN const INT32 nSrcStride0, IN const UINT8 *pSrc1, IN const INT32 nSrcStride1,
                               IN const INT32 nSizeX, IN const INT32 nSizeY, OUT UINT32 *pSqSum, OUT INT32 *pSum);
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_MMX) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_MMX)
    void OneRow_Copy_MMX(OUT UINT8 *pDst, IN const UINT8 *pSrc, IN const INT32 nWidth);
    void OneRow_InterleaveUV_MMX(IN const UINT8 *pSrcUV, OUT UINT8 *pDstU, OUT UINT8 *pDstV, IN const INT32 nWidth);
    void OneRow_Subsample_Even_MMX(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth);
    void OneRow_Subsample_Odd_MMX(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth);
    void OneRow_Subsample_1_4_MMX(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth);
    void OneRow_ScaleBilinear_MMX(OUT UINT8* pDst,
                               IN const UINT8* pSrc, ptrdiff_t nSrcStride,
                               IN const INT32 nDstWidth, IN const INT32 nSrcYFraction);
    void OneCol_ScaleBilinear_MMX(OUT UINT8* pDst, IN const UINT8* pSrc,
                               IN const INT32 nDstWidth, IN INT32 x, IN const INT32 dx);
    void OneRow_MeanBlur_MMX(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth,
                               IN const INT32 nSrcStride, IN const INT32 nDstStride);
    void OneRow_MeanBlur_Hor_MMX(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth);
    void OneRow_MeanBlur_Ver_MMX(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth,
                               IN const INT32 nSrcStride, IN const INT32 nDstStride);
    void OneRow_Mosaic4_MMX(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth,
                               IN const INT32 nSrcStride, IN const INT32 nDstStride, IN const RectImgAddr *pBaseRectAddr);
    void OneRow_Mosaic8_MMX(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth,
                               IN const INT32 nSrcStride, IN const INT32 nDstStride, IN const RectImgAddr *pBaseRectAddr);
    void OneRow_Mosaic16_MMX(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth,
                               IN const INT32 nSrcStride, IN const INT32 nDstStride, IN const RectImgAddr *pBaseRectAddr);
    void OneRow_Inverse_MMX(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth);
    void OneRow_Clip_MMX(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth,
                               IN const INT32 nMinClipVal, IN const INT32 nMaxClipVal);
    void OneRow_DiffImg_MMX(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, OUT UINT8 *pDst, IN const INT32 nWidth);
    void OneRow_Multiply_MMX(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, OUT UINT8 *pDst, IN const INT32 nWidth);
    void OneRow_ContrastShift_MMX(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, OUT UINT8 *pDst, IN const INT32 nWidth);
    void OneRow_PropMix_MMX(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, IN const F32 *pCoeffi, OUT UINT8 *pDst, IN const INT32 nWidth);
    void OneRow_BlurNoiseReduction_MMX(IN const UINT8 *pSrc, OUT UINT8 *pDst,
                               IN const INT32 nWidth, IN const INT32 nSrcStride, IN const INT32 nDstStride);
    F32 Mean_MxN_MMX(IN const UINT8 *pSrc, IN const INT32 nSrcStride, IN const INT32 nSizeX, IN const INT32 nSizeY, OUT UINT32 *pSum);
    F32 Variance_MxN_MMX(IN const UINT8 *pSrc, IN const INT32 nSrcStride,
                               IN const INT32 nSizeX, IN const INT32 nSizeY, OUT UINT32 *pSqSum, OUT UINT32 *pSum);
    F32 DiffVariance_MxN_MMX(IN const UINT8 *pSrc0, IN const INT32 nSrcStride0, IN const UINT8 *pSrc1, IN const INT32 nSrcStride1,
                               IN const INT32 nSizeX, IN const INT32 nSizeY, OUT UINT32 *pSqSum, OUT INT32 *pSum);
#endif
    
    
    
    
#ifdef __cplusplus
}
#endif


#endif
