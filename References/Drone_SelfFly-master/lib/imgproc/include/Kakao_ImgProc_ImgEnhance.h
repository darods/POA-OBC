//
//  Kakao_ImgProc_ImgEnhance.h
//
//  Created by Maverick Park on 2014. 7. 28.
//  Copyright (c) 2015년 Kakao Corp. All rights reserved.
//

#ifndef __KAKAO_IMGPROC_IMAGE_ENHANCE_H__
#define __KAKAO_IMGPROC_IMAGE_ENHANCE_H__

#include "Kakao_ImgProc_Core.h"
#include "Kakao_ImgProc_Filter.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef MEM_ALIGNED(4) struct _ImgEnhance_T
{
    KISize                      nBaseSize;
    Img_T                       *pEnhancedImg;              // Enhanced image∞° ¿˙¿Âµ«¥¬ ∞¯∞£
    KISize                      nImgSize;
    D64                         nSigma;                                   // Blurring Strength
    D64                         *nFilterCoeffi[5];
    INT32                       nFilterSize[5];
    
    Filter_T                    *pTrilateralFilter;
}ImgEnhance_T;

    
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_ImgEnhance_Create(IN OUT ImgEnhance_T **ppImgEnhancer, IN const INT32 nWidth, IN const INT32 nHeight);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_ImgEnhance_Destroy(IN OUT ImgEnhance_T **ppImgEnhancer);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_ImgEnhance_UpdateImgSize(IN OUT ImgEnhance_T *pImgEnhancer, IN const INT32 nWidth, IN const INT32 nHeight);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_ImgEnhance_DoEnhance(IN const ImgEnhance_T *pImgEnhancer, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);

    
#ifdef __cplusplus
}
#endif


#endif
