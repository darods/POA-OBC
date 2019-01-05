//
//  Kakao_ImgProc_ImgFeature.h
//
//  Created by Maverick Park on 2016. 9. 27.
//  Copyright (c) 2015년 Kakao Corp. All rights reserved.
//

#ifndef __KAKAO_IMGPROC_IMAGE_FEATURE_H__
#define __KAKAO_IMGPROC_IMAGE_FEATURE_H__

#include "Kakao_ImgProc_Core.h"

#ifdef __cplusplus
extern "C" {
#endif


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Feature_Mean(IN const Img_T *pSrcImg, IN const INT32 nPlaneIdx, IN const INT32 nPosX, IN const INT32 nPosY,
                                                         IN const BlockType nBlockType, OUT UINT32 *pSum, OUT F32 *pMean);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Feature_Variance(IN const Img_T *pSrcImg, IN const INT32 nPlaneIdx, IN const INT32 nPosX, IN const INT32 nPosY,
                                                         IN const BlockType nBlockType, OUT F32 *pMean, OUT F32 *pVar, OUT UINT32 *pSqSum);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Feature_DiffVariance(IN const Img_T *pSrcImg0, IN const Img_T *pSrcImg1, IN const INT32 nPlaneIdx,
                                                         IN const INT32 nPosX, IN const INT32 nPosY, IN const BlockType nBlockType,
                                                         OUT F32 *pMean, OUT F32 *pVar, OUT UINT32 *pSqSum);


#ifdef __cplusplus
}
#endif


#endif
