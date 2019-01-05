//
//  Kakao_ImgProc_SSE3.h
//
//  Created by Maverick Park on 2016. 9. 23.
//  Copyright (c) 2015년 Kakao Corp. All rights reserved.
//

#ifndef __KAKAO_IMGPROC_SSE3_H__
#define __KAKAO_IMGPROC_SSE3_H__

#include "Kakao_ImgProc_Core.h"

#ifdef __cplusplus
extern "C" {
#endif

    
#if ENABLE_ISET(CPU_INTEL_IA32 | ISET_SSE3) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_SSE3)
    void OneRow_Copy_SSE3(OUT UINT8 *pDst, IN const UINT8 *pSrc, IN const INT32 nWidth);
    void OneRow_InterleaveUV_SSE3(IN const UINT8 *pSrcUV, OUT UINT8 *pDstU, OUT UINT8 *pDstV, IN const INT32 nWidth);
    void OneRow_Subsample_Even_SSE3(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth);
    void OneRow_Subsample_Odd_SSE3(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth);
    void OneRow_Subsample_1_4_SSE3(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth);
    void OneRow_ScaleBilinear_SSE3(OUT UINT8* pDst,
                               IN const UINT8* pSrc, ptrdiff_t nSrcStride,
                               IN const INT32 nDstWidth, IN const INT32 nSrcYFraction);
    void OneCol_ScaleBilinear_SSE3(OUT UINT8* pDst, IN const UINT8* pSrc,
                               IN const INT32 nDstWidth, IN INT32 x, IN const INT32 dx);
    void OneRow_MeanBlur_SSE3(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth,
                               IN const INT32 nSrcStride, IN const INT32 nDstStride);
    void OneRow_MeanBlur_Hor_SSE3(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth);
    void OneRow_MeanBlur_Ver_SSE3(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth,
                               IN const INT32 nSrcStride, IN const INT32 nDstStride);
    void OneRow_Mosaic4_SSE3(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth,
                               IN const INT32 nSrcStride, IN const INT32 nDstStride, IN const RectImgAddr *pBaseRectAddr);
    void OneRow_Mosaic8_SSE3(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth,
                               IN const INT32 nSrcStride, IN const INT32 nDstStride, IN const RectImgAddr *pBaseRectAddr);
    void OneRow_Mosaic16_SSE3(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth,
                               IN const INT32 nSrcStride, IN const INT32 nDstStride, IN const RectImgAddr *pBaseRectAddr);
    void OneRow_Inverse_SSE3(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth);
    void OneRow_Clip_SSE3(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth,
                               IN const INT32 nMinClipVal, IN const INT32 nMaxClipVal);
    void OneRow_DiffImg_SSE3(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, OUT UINT8 *pDst, IN const INT32 nWidth);
    void OneRow_Multiply_SSE3(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, OUT UINT8 *pDst, IN const INT32 nWidth);
    void OneRow_ContrastShift_SSE3(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, OUT UINT8 *pDst, IN const INT32 nWidth);
    void OneRow_PropMix_SSE3(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, IN const F32 *pCoeffi, OUT UINT8 *pDst, IN const INT32 nWidth);
    void OneRow_BlurNoiseReduction_SSE3(IN const UINT8 *pSrc, OUT UINT8 *pDst,
                               IN const INT32 nWidth, IN const INT32 nSrcStride, IN const INT32 nDstStride);
    F32 Mean_MxN_SSE3(IN const UINT8 *pSrc, IN const INT32 nSrcStride, IN const INT32 nSizeX, IN const INT32 nSizeY, OUT UINT32 *pSum);
    F32 Variance_MxN_SSE3(IN const UINT8 *pSrc, IN const INT32 nSrcStride,
                               IN const INT32 nSizeX, IN const INT32 nSizeY, OUT UINT32 *pSqSum, OUT UINT32 *pSum);
    F32 DiffVariance_MxN_SSE3(IN const UINT8 *pSrc0, IN const INT32 nSrcStride0, IN const UINT8 *pSrc1, IN const INT32 nSrcStride1,
                               IN const INT32 nSizeX, IN const INT32 nSizeY, OUT UINT32 *pSqSum, OUT INT32 *pSum);
#endif
    
    
    
    
#ifdef __cplusplus
}
#endif


#endif
