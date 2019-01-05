//
//  Kakao_ImgProc_SBD.h
//
//  Created by Maverick Park on 2014. 7. 28.
//  Copyright (c) 2015년 Kakao Corp. All rights reserved.
//

#ifndef __KAKAO_IMGPROC_SHOT_BOUNDARY_DETECTOR_H__
#define __KAKAO_IMGPROC_SHOT_BOUNDARY_DETECTOR_H__

#include "Kakao_ImgProc_Core.h"

#ifdef __cplusplus
extern "C" {
#endif
    

#define SBD_NUM_REF_IMG             (2)

typedef enum _SBD_PicType
{
    SBD_PICTYPE_INTRA,
    SBD_PICTYPE_INTER,
    SBD_PICTYPE_MAX,
}SBD_PicType;



typedef MEM_ALIGNED(4) struct _SBD_T
{
    KISize                  nBaseSize;
    Img_T                   *pRefImg[SBD_NUM_REF_IMG];
    INT8                    *pMVInfoMBwise[SBD_NUM_REF_IMG];
    INT8                    *pTmpMVInfoMBwise;
    INT32                   *pImgAnalysis[SBD_NUM_REF_IMG];
    INT32                   nFirstFrameFlag;
    UINT32                  nCurrImgIdx;
    UINT32                  nPrevImgIdx;
    INT32                   nFrameCount;
    INT32                   nSBDWindowSize;
    INT32                   nSBDPrevWindowPos;
    INT32                   nSBDCurrWindowPos;
    INT32                   nNeighborIntra;
    INT32                   nSumofMAD[SBD_PICTYPE_MAX][20];
    INT32                   nCheckMemAllocated;
    UINT32                  nHistogram[SBD_NUM_REF_IMG][BIN+1];
    UINT32                  nHistoDiff[SBD_NUM_REF_IMG];
    IMGPROC_BOOL            bForceIntraFlag;
}SBD_T;

    
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_SBD_Create(IN OUT SBD_T **ppSBD, IN const INT32 nWidth, IN const INT32 nHeight);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_SBD_Destroy(IN OUT SBD_T **ppSBD);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_SBD_UpdateImgSize(IN OUT SBD_T *pSBD, IN const INT32 nWidth, IN const INT32 nHeight);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_SBD_FindSBD(IN SBD_T *pSBD, IN Img_T *pSrcImg, OUT IMGPROC_BOOL *pbForceIntraFlag);

    
#ifdef __cplusplus
}
#endif


#endif
