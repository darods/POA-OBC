//
//  Kakao_ImgProc_Rotate.h
//
//  Created by Maverick Park on 2015. 8. 27.
//  Copyright (c) 2015년 Kakao. All rights reserved.
//

#ifndef __KAKAO_IMGPROC_ROTATE_H__
#define __KAKAO_IMGPROC_ROTATE_H__

#include "Kakao_ImgProc_Core.h"

#ifdef __cplusplus
extern "C" {
#endif
    
typedef enum _RotateDegree{
    ROTATE_CW_000 = 0,
    ROTATE_CW_090,
    ROTATE_CW_180,
    ROTATE_CW_270,

    ROTATE_CCW_000 = ROTATE_CW_000,
    ROTATE_CCW_090 = ROTATE_CW_270,
    ROTATE_CCW_180 = ROTATE_CW_180,
    ROTATE_CCW_270 = ROTATE_CW_090,
}RotateDegree;


typedef struct _Rotate_T
{
    KISize                  nBaseSize;
}Rotate_T;


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Rotate_Create(IN OUT Rotate_T **ppRotate, IN const INT32 nWidth, IN const INT32 nHeight);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Rotate_Destroy(IN OUT Rotate_T **ppRotate);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Rotate_UpdateImgSize(IN OUT Rotate_T *pRotate, IN const INT32 nWidth, IN const INT32 nHeight);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Rotate_DoRotate(IN const Rotate_T *pRotate, IN const Img_T *pSrcImg, OUT Img_T *pDstImg, const RotateDegree nDegree);

    
#ifdef __cplusplus
}
#endif


#endif
