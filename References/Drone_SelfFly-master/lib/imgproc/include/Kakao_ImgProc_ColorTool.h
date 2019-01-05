//
//  Kakao_ImgProc_ColorTool.h
//
//  Created by Maverick Park on 2014. 7. 28.
//  Copyright (c) 2015년 Kakao Corp. All rights reserved.
//

#ifndef __KAKAO_IMGPROC_COLOR_CONVERT_H__
#define __KAKAO_IMGPROC_COLOR_CONVERT_H__

#include "Kakao_ImgProc_Core.h"
#include "Kakao_ImgProc_Rotate.h"
#include "libyuv/convert.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef MEM_ALIGNED(4) struct _ColorTool_T
{
    KISize              nBaseSize;
    Img_T               nColorCorrectorImg;     // Color Corrected image
}ColorTool_T;

    
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_ColorTool_Create(IN OUT ColorTool_T **ppColorTool, IN const INT32 nWidth, IN const INT32 nHeight);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_ColorTool_Destroy(IN OUT ColorTool_T **ppColorTool);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_ColorTool_Cvt(IN ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_ColorTool_ExtCvtToI420(IN ColorTool_T *pColorTool, IN const Img_T *pSrcImg, OUT Img_T *pDstImg,
                                                                   IN const INT32 nCropX, IN const INT32 nCropY,
                                                                   IN const INT32 nCropWidth, IN const INT32 nCropHeight,
                                                                   IN const INT32 nSrcWidth, IN const INT32 nSrcHeight,
                                                                   IN const RotateDegree nDegree, IN const ImgFormat nFourCC);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_ColorTool_UpdateImgSize(IN OUT ColorTool_T *pColorTool, IN const INT32 nWidth, IN const INT32 nHeight);
    
#ifdef __cplusplus
}
#endif


#endif
