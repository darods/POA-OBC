//
//  Kakao_ImgProc_IQA.h
//
//  Created by Maverick Park on 2014. 8. 28.
//  Copyright (c) 2015년 Kakao Corp. All rights reserved.
//

#ifndef __KAKAO_IMGPROC_IMAGE_QUALITY_ASSESSMENT_H__
#define __KAKAO_IMGPROC_IMAGE_QUALITY_ASSESSMENT_H__

#include "Kakao_ImgProc_Core.h"

#ifdef __cplusplus
extern "C" {
#endif
    

typedef struct _DSSIM_T
{
    Img_T                   *pMu;
    Img_T                   *pSigma_Sq;
    Img_T                   *pTmpSquare;
}DSSIM_T;


typedef MEM_ALIGNED(4) struct _IQA_T
{
    KISize                  nBaseSize;
    
    // For SSIM
    MEM_ALIGNED(4) DSSIM_T  DSSIM[2];
    Img_T                   *pTmpBuf;
    Img_T                   *pMultiply12;
    Img_T                   *pSigma12;
    UINT32                  nFrameBufIdx;
    UINT32                  *pSquareTable;
}IQA_T;



KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_IQA_Create(IN OUT IQA_T **ppIQA, IN const INT32 nWidth, IN const INT32 nHeight);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_IQA_Destroy(IN OUT IQA_T **ppIQA);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_IQA_UpdateImgSize(IN OUT IQA_T *pIQA, IN const INT32 nWidth, IN const INT32 nHeight);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_IQA_CalculatePSNR(IN const IQA_T *pIQA, IN const Img_T *pSrcImg0, IN const Img_T *pSrcImg1, IN const INT32 nPlaneIdx, OUT F32 *pPSNR);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_IQA_DSSIM_Calculate(IN const IQA_T *pIQA, IN const Img_T *pSrcImg0, IN const Img_T *pSrcImg1, OUT D64 *pDSSIM);

    
#ifdef __cplusplus
}
#endif


#endif
