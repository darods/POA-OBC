//
//  Kakao_ImgProc_ROI.h
//
//  Created by Maverick Park on 2014. 7. 28.
//  Copyright (c) 2015년 Kakao Corp. All rights reserved.
//

#ifndef __KAKAO_IMGPROC_ROI_DETECTOR_H__
#define __KAKAO_IMGPROC_ROI_DETECTOR_H__

#include "Kakao_ImgProc_Core.h"

#ifdef __cplusplus
extern "C" {
#endif
    

typedef enum _ROIPriority{
    Low,
    Normal,
    High,
}ROIPriority;



typedef MEM_ALIGNED(4) struct _ROIDetect_T
{
    KISize                  nBaseSize;
    UINT8                   *pAutoROIMBwiseMap;
    Img_T                   *pROIImg;                   // Visual Attention image∞° ¿˙¿Âµ«¥¬ ∞¯∞£
    INT32                   nCheckMemAllocated;
}ROIDetect_T;

    
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_ROIDetector_Create(IN OUT ROIDetect_T **ppROIDetect, IN const INT32 nWidth, IN const INT32 nHeight);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_ROIDetector_Destroy(IN OUT ROIDetect_T **ppROIDetect);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_ROIDetector_UpdateImgSize(IN OUT ROIDetect_T *pROIDetect, IN const INT32 nWidth, IN const INT32 nHeight);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_ROIDetector_Selection(ROIDetect_T *pROIDetect);
#ifdef __cplusplus
}
#endif


    
#endif
