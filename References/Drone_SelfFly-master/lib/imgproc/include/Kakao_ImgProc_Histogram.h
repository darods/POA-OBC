//
//  Kakao_ImgProc_Histogram.h
//
//  Created by Maverick Park on 2016. 5. 17.
//  Copyright (c) 2015년 Kakao Corp. All rights reserved.
//

#ifndef __KAKAO_IMGPROC_HIST_H__
#define __KAKAO_IMGPROC_HIST_H__

#include "Kakao_ImgProc_Core.h"

#ifdef __cplusplus
extern "C" {
#endif
    
    
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Hist_Cal1DHist(IN OUT Img_T *pSrcImg, OUT F32 *pHist, IN const UINT32 nBins,
                                                                IN const UINT32 nPlaneIdx, IN const IMGPROC_BOOL bNormalized);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Hist_Cal2DHist(IN OUT Img_T *pSrcImg, OUT F32 *pHist, IN const UINT32 nBins, IN const IMGPROC_BOOL bNormalized);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Hist_Comp1DHist(IN Img_T *pSrcImg0, IN Img_T *pSrcImg1, IN const UINT32 nBins,
                                                                IN const ImgHistCompMode nHistCompMode, IN const UINT32 nPlaneIdx, OUT F32 *pHICompVal);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Hist_Comp2DHist(IN Img_T *pSrcImg0, IN Img_T *pSrcImg1, IN const UINT32 nBins, IN const ImgHistCompMode nHistCompMode, OUT F32 *pHICompVal);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Hist_Cal1DHistBP(IN OUT Img_T *pSrcImg, IN const F32 *pRefHistogram, IN const UINT32 nBins,
                                                                IN const UINT32 nPlaneIdx, IN const ImgBPMode nHistBPMode);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Hist_Cal2DHistBP(IN OUT Img_T *pSrcImg, IN const F32 *pRefHistogram, IN const UINT32 nBins, IN const ImgBPMode nHistBPMode);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Hist_GetHistMap(IN Img_T *pSrcImg, IN const INT32 nDimension, IN const INT32 nPlaneIdx, OUT F32 **ppHistMapMap);
    
#ifdef __cplusplus
}
#endif

#endif
