//
//  Kakao_ImgProc_EdgeDetector.h
//
//  Created by Maverick Park on 2014. 7. 28.
//  Copyright (c) 2015년 Kakao Corp. All rights reserved.
//

#ifndef __KAKAO_IMGPROC_EDGE_DETECTOR_H__
#define __KAKAO_IMGPROC_EDGE_DETECTOR_H__

#include "Kakao_ImgProc_Core.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef enum _Edge_Mode{
    EDGE_MODE_SOBEL = 0,
    EDGE_MODE_CANNY,
    EDGE_MODE_LAPLACIAN,
    EDGE_MODE_CUSTOM,
}EdgeMode;


typedef MEM_ALIGNED(4) struct _EdgeDet_T
{
    KISize              nBaseSize;
    EdgeMode            nEdgeMode;                              // Edge Mode:
                                                                //        -1 -> Disable
                                                                //         0 -> Soble
                                                                //         1 -> Canny
                                                                //         2 -> Laplacian

    UINT8               *pEdge_CannyTmpBuf;
    
    INT32               nGaussianBlurMaskSize;
}EdgeDet_T;


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Edge_Create(IN OUT EdgeDet_T **ppEdgeDetect, IN const INT32 nWidth, IN const INT32 nHeight);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Edge_Destroy(IN OUT EdgeDet_T **ppEdgeDetect);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Edge_Sobel(IN EdgeDet_T *pEdgeDetect, IN OUT Img_T *pSrcImg);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Edge_Canny(IN EdgeDet_T *pEdgeDetect, IN OUT Img_T *pSrcImg);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Edge_Laplacian(IN EdgeDet_T *pEdgeDetect, IN OUT Img_T *pSrcImg);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Edge_Custom(IN EdgeDet_T *pEdgeDetect, IN OUT Img_T *pSrcImg);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Edge_Detector(IN EdgeDet_T *pEdgeDetect, IN OUT Img_T *pSrcImg);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Edge_UpdateImgSize(IN OUT EdgeDet_T *pEdgeDetect, IN const INT32 nWidth, IN const INT32 nHeight);
    
#ifdef __cplusplus
}
#endif


#endif
