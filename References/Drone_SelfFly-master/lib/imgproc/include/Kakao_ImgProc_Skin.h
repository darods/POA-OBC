//
//  Kakao_ImgProc_Skin.h
//
//  Created by Maverick Park on 2014. 7. 28.
//  Copyright (c) 2015년 Kakao Corp. All rights reserved.
//

#ifndef __KAKAO_IMGPROC_SKIN_DETECTOR_H__
#define __KAKAO_IMGPROC_SKIN_DETECTOR_H__

#include "Kakao_ImgProc_Core.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef MEM_ALIGNED(4) struct _SkinDet_T
{
    KISize              nBaseSize;
    Img_T               *pFramewiseMap[2];
    Img_T               *pMBwiseMap[2];

    Filter_T            *pFilter;
}SkinDet_T;




KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_SkinDetector_Create(IN OUT SkinDet_T **ppSkinDetector, IN INT32 nWidth, IN INT32 nHeight);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_SkinDetector_Destroy(IN OUT SkinDet_T **ppSkinDetector);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_SkinDetector_UpdateImgSize(IN OUT SkinDet_T *pSkinDetector, IN const INT32 nWidth, IN const INT32 nHeight);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_SkinDetector_Detect(IN SkinDet_T *pSkinDetect, IN Img_T *pSrcImg, OUT Img_T *pDstImg);
    
#ifdef __cplusplus
}
#endif

#endif
