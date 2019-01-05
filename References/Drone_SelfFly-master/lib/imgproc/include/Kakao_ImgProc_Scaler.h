//
//  Kakao_ImgProc_Scaler.h
//
//  Created by Maverick Park on 2014. 7. 28.
//  Copyright (c) 2015년 Kakao Corp. All rights reserved.
//

#ifndef __KAKAO_IMGPROC_SCLAER_H__
#define __KAKAO_IMGPROC_SCLAER_H__

#include "Kakao_ImgProc_Core.h"
#include "libyuv/scale.h"

#ifdef __cplusplus
extern "C" {
#endif
    
    
typedef MEM_ALIGNED(4) struct _ImgScaler_T         ImgScaler_T;

typedef Kakao_Status (*fpScaler)(IN const ImgScaler_T *pScaler, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);


typedef enum _ScaleMode
{
    IMG_SCALEMODE_NONE          = 0,
    IMG_SCALEMODE_LINEAR,
    IMG_SCALEMODE_BILINEAR,
    IMG_SCALEMODE_BOX,
    IMG_SCALEMODE_MAX
}ScaleMode;
    
    
    
struct _ImgScaler_T
{
    KISize                  nBaseSize;
    
    fpScaler                fpScalerSubsampleFunc[IMG_FORMAT_MAX];
    fpScaler                fpScalerBilinearFunc[IMG_FORMAT_MAX];
    fpScaler                fpScalerBicubicFunc[IMG_FORMAT_MAX];
};


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Scaler_Create(IN OUT ImgScaler_T **ppScaler, IN const INT32 nWidth, IN const INT32 nHeight);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Scaler_Destroy(IN OUT ImgScaler_T **ppScaler);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Scaler_UpdateImgSize(IN OUT ImgScaler_T *pScaler, IN const INT32 nWidth, IN const INT32 nHeight);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Scaler_Subsample(IN const ImgScaler_T *pScaler, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Scaler_Bilinear(IN const ImgScaler_T *pScaler, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Scaler_Bicubic(UINT8 *pSrc, UINT8 *pDst, UINT8 *Kakao_VCmpImg, KISize *nSrcSize, KISize *nDstSize);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Scaler_External(IN const ImgScaler_T *pScaler, IN const Img_T *pSrcImg, OUT Img_T *pDstImg, const ScaleMode nScaleMode);


#ifdef __cplusplus
}
#endif


#endif
