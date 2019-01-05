//
//  Kakao_ImgProc_C_Opt.h
//
//  Created by Maverick Park on 2014. 8. 2.
//  Copyright (c) 2015년 Kakao Corp. All rights reserved.
//

#ifndef __KAKAO_IMGPROC_C_OPT_H__
#define __KAKAO_IMGPROC_C_OPT_H__

#include "Kakao_ImgProc_Core.h"

#ifdef __cplusplus
extern "C" {
#endif

void OneRow_Copy_C(OUT UINT8 *pDst, IN const UINT8 *pSrc, IN const INT32 nWidth);
void OneRow_InterleaveUV_C(IN const UINT8 *pSrcUV, OUT UINT8 *pDstU, OUT UINT8 *pDstV, IN const INT32 nWidth);
void OneRow_Subsample_Even_C(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN INT32 nWidth);
void OneRow_Subsample_Odd_C(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN INT32 nWidth);
void OneRow_Subsample_1_4_C(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN INT32 nWidth);
void OneRow_ScaleBilinear_C(OUT UINT8* pDst,
                               IN const UINT8* pSrc, ptrdiff_t nSrcStride,
                               IN const INT32 nDstWidth, IN const INT32 nSrcYFraction);
void OneCol_ScaleBilinear_C(OUT UINT8* pDst, IN const UINT8* pSrc,
                               IN const INT32 nDstWidth, IN INT32 x, IN const INT32 dx);
void OneRow_MeanBlur_C(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth,
                               IN const INT32 nSrcStride, IN const INT32 nDstStride);
void OneRow_MeanBlur_Hor_C(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth);
void OneRow_MeanBlur_Ver_C(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth,
                               IN const INT32 nSrcStride, IN const INT32 nDstStride);
void OneRow_Mosaic4_C(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth,
                               IN const INT32 nSrcStride, IN const INT32 nDstStride, IN const RectImgAddr *pBaseRectAddr);
void OneRow_Mosaic8_C(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth,
                               IN const INT32 nSrcStride, IN const INT32 nDstStride, IN const RectImgAddr *pBaseRectAddr);
void OneRow_Mosaic16_C(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth,
                               IN const INT32 nSrcStride, IN const INT32 nDstStride, IN const RectImgAddr *pBaseRectAddr);
void OneRow_Inverse_C(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth);
void OneRow_Clip_C(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth,
                               IN const INT32 nMinClipVal, IN const INT32 nMaxClipVal);
void CustomHistogramStretch_C(IN const Filter_T *pFilter, IN const UINT8 *pSrc,
                               OUT UINT8 *pDst, IN const INT32 nWidth, IN const INT32 nHeight,
                               IN const INT32 nSrcStride, IN const INT32 nDstStride, IN const UINT8 *pLookupTbl, IN const UINT32 nThreadIdx);
void OneRow_DiffImg_C(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, OUT UINT8 *pDst, IN const INT32 nWidth);
void OneRow_Multiply_C(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, OUT UINT8 *pDst, IN const INT32 nWidth);
void OneRow_ContrastShift_C(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, OUT UINT8 *pDst, IN const INT32 nWidth);
void OneRow_PropMix_C(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, IN const F32 *pCoeffi, OUT UINT8 *pDst, IN const INT32 nWidth);
void OneRow_BlurNoiseReduction_C(IN const UINT8 *pSrc, OUT UINT8 *pDst,
                               IN const INT32 nWidth, IN const INT32 nSrcStride, IN const INT32 nDstStride);
F32 Mean_MxN_C(IN const UINT8 *pSrc, IN const INT32 nSrcStride, IN const INT32 nSizeX, IN const INT32 nSizeY, OUT UINT32 *pSum);
F32 Variance_MxN_C(IN const UINT8 *pSrc, IN const INT32 nSrcStride,
                               IN const INT32 nSizeX, IN const INT32 nSizeY, OUT UINT32 *pSqSum, OUT UINT32 *pSum);
F32 DiffVariance_MxN_C(IN const UINT8 *pSrc0, IN const INT32 nSrcStride0, IN const UINT8 *pSrc1, IN const INT32 nSrcStride1,
                               IN const INT32 nSizeX, IN const INT32 nSizeY, OUT UINT32 *pSqSum, OUT INT32 *pSum);

#ifdef __cplusplus
}
#endif


#endif
