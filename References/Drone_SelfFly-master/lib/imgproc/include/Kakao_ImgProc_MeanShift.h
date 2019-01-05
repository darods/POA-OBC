//
//  Kakao_ImgProc_MeanShift.h
//
//  Created by Maverick Park on 2015. 8. 10.
//  Copyright (c) 2015년 Kakao. All rights reserved.
//

#ifndef __KAKAO_IMGPROC_MEANSHIFT_H__
#define __KAKAO_IMGPROC_MEANSHIFT_H__

#include "Kakao_ImgProc_Core.h"

#ifdef __cplusplus
extern "C" {
#endif
    
    
typedef MEM_ALIGNED(4) struct  _MeanShift_T
{
    KISize                      nBaseSize;
    KISize                      nWindowSize;
    KISize                      nSearchRegion;
    KIPointF2                   nCenterPtr;
    F32                         nScaledRatioWidth;
    F32                         nScaledRatioHeight;
    
    ImgScaler_T                 *pScalerHndl;
    Img_T                       *pDownScaledSrcImg;
    ROI                         nTargetROI;
    
    F32                         *pEpanechnikovMask;
    F32                         *pRefHist;
    Color_RGB                   *pYUVtoRGBMapTable;
    Color_HSL                   *pYUVtoHSLMapTable;
    KIPoint2                    *pClusteringMap;
    
    F32                         *pROIHistogram;
    F32                         *pSrcImgHistogram;
    F32                         *pProbibilityHBP;
    
    INT32                       nHistoBinGap;
    INT32                       nHistoBinNum;
}MeanShift_T;


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_MeanShift_Create(IN OUT MeanShift_T **ppMeanShift, IN const INT32 nWidth, IN const INT32 nHeight);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_MeanShift_Destroy(IN OUT MeanShift_T **ppMeanShift);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_MeanShift_UpdateImgSize(IN OUT MeanShift_T *pMeanShift, IN const INT32 nWidth, IN const INT32 nHeight);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_MeanShift_Track_SetReference(IN OUT MeanShift_T *pMeanShift, IN Img_T *pSrcImg, IN const ROI nTargetROI);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_MeanShift_DoTracking(IN OUT MeanShift_T *pMeanShift, IN Img_T *pSrcImg);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_MeanShift_DoColorClustering(IN OUT MeanShift_T *pMeanShift, IN Img_T *pSrcImg, IN const INT32 nWindowSize);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_MeanShift_SetCenterPtr(IN OUT MeanShift_T *pMeanShift, IN const Img_T *pSrcImg, IN const KIPointF2 nCenterPtr);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_MeanShift_GetCenterPtr(IN OUT MeanShift_T *pMeanShift, OUT KIPointF2 *pCenterPtr);
    
    
#ifdef __cplusplus
}
#endif


#endif
