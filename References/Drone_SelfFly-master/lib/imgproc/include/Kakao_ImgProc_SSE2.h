//
//  Kakao_ImgProc_SSE2.h
//
//  Created by Maverick Park on 2016. 9. 23.
//  Copyright (c) 2015년 Kakao Corp. All rights reserved.
//

#ifndef __KAKAO_IMGPROC_SSE2_H__
#define __KAKAO_IMGPROC_SSE2_H__

#include "Kakao_ImgProc_Core.h"

#ifdef __cplusplus
extern "C" {
#endif

    
#if ENABLE_ISET(CPU_INTEL_IA32 | ISET_SSE2) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_SSE2)
    void OneRow_Copy_SSE2(OUT UINT8 *pDst, IN const UINT8 *pSrc, IN const INT32 nWidth);
    void OneRow_InterleaveUV_SSE2(IN const UINT8 *pSrcUV, OUT UINT8 *pDstU, OUT UINT8 *pDstV, IN const INT32 nWidth);
    void OneRow_Subsample_Even_SSE2(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth);
    void OneRow_Subsample_Odd_SSE2(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth);
    void OneRow_Subsample_1_4_SSE2(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth);
    void OneRow_ScaleBilinear_SSE2(OUT UINT8* pDst,
                               IN const UINT8* pSrc, ptrdiff_t nSrcStride,
                               IN const INT32 nDstWidth, IN const INT32 nSrcYFraction);
    void OneCol_ScaleBilinear_SSE2(OUT UINT8* pDst, IN const UINT8* pSrc,
                               IN const INT32 nDstWidth, IN INT32 x, IN const INT32 dx);
    void OneRow_MeanBlur_SSE2(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth,
                               IN const INT32 nSrcStride, IN const INT32 nDstStride);
    void OneRow_MeanBlur_Hor_SSE2(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth);
    void OneRow_MeanBlur_Ver_SSE2(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth,
                               IN const INT32 nSrcStride, IN const INT32 nDstStride);
    void OneRow_Mosaic4_SSE2(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth,
                               IN const INT32 nSrcStride, IN const INT32 nDstStride, IN const RectImgAddr *pBaseRectAddr);
    void OneRow_Mosaic8_SSE2(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth,
                               IN const INT32 nSrcStride, IN const INT32 nDstStride, IN const RectImgAddr *pBaseRectAddr);
    void OneRow_Mosaic16_SSE2(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth,
                               IN const INT32 nSrcStride, IN const INT32 nDstStride, IN const RectImgAddr *pBaseRectAddr);
    void OneRow_Inverse_SSE2(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth);
    void OneRow_Clip_SSE2(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth,
                               IN const INT32 nMinClipVal, IN const INT32 nMaxClipVal);
    void OneRow_DiffImg_SSE2(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, OUT UINT8 *pDst, IN const INT32 nWidth);
    void OneRow_Multiply_SSE2(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, OUT UINT8 *pDst, IN const INT32 nWidth);
    void OneRow_ContrastShift_SSE2(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, OUT UINT8 *pDst, IN const INT32 nWidth);
    void OneRow_PropMix_SSE2(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, IN const F32 *pCoeffi, OUT UINT8 *pDst, IN const INT32 nWidth);
    void OneRow_BlurNoiseReduction_SSE2(IN const UINT8 *pSrc, OUT UINT8 *pDst,
                               IN const INT32 nWidth, IN const INT32 nSrcStride, IN const INT32 nDstStride);
    F32 Mean_MxN_SSE2(IN const UINT8 *pSrc, IN const INT32 nSrcStride, IN const INT32 nSizeX, IN const INT32 nSizeY, OUT UINT32 *pSum);
    F32 Variance_MxN_SSE2(IN const UINT8 *pSrc, IN const INT32 nSrcStride,
                               IN const INT32 nSizeX, IN const INT32 nSizeY, OUT UINT32 *pSqSum, OUT UINT32 *pSum);
    F32 DiffVariance_MxN_SSE2(IN const UINT8 *pSrc0, IN const INT32 nSrcStride0, IN const UINT8 *pSrc1, IN const INT32 nSrcStride1,
                               IN const INT32 nSizeX, IN const INT32 nSizeY, OUT UINT32 *pSqSum, OUT INT32 *pSum);
#endif
    
    
    
    
#ifdef __cplusplus
}
#endif


#endif
