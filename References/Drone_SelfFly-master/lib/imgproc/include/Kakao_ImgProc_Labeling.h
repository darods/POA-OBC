//
//  Kakao_ImgProc_Labeling.h
//
//  Created by Maverick Park on 2014. 8. 2.
//  Copyright (c) 2015년 Kakao Corp. All rights reserved.
//

#ifndef __KAKAO_IMGPROC_LABELING_H__
#define __KAKAO_IMGPROC_LABELING_H__

#include "Kakao_ImgProc_Core.h"

#ifdef __cplusplus
extern "C" {
#endif
    

#define MAX_LABEL_NUM           (50)

    
typedef MEM_ALIGNED(4) struct _PtVisited
{
	IMGPROC_BOOL            bVisitedFlag;
	KIPoint2                nRetPoint;
}PtVisited;


typedef MEM_ALIGNED(4) struct _BlobInfo
{
    KIRect                  nRectBlobs;
    KIPoint2                nBlobCenter;
    UINT32                  nBlobSize;
    IMGPROC_BOOL            bObjDetected;
}BlobInfo;

    
typedef MEM_ALIGNED(4) struct _BlobLabel
{
    KISize                  nBaseSize;

	UINT8                   *pBlobLabelMap;
    PtVisited               *pVisitedPtrMap;

    UINT32                  nImgRegionSize;
	UINT32                  nTh;
    INT32                   nIndicator;
	
    UINT32                  nNumofBlobs;
    INT32                   nDegree;
	KIRect                  *pRectBlobs;
    BlobInfo                *pBlobInfo;
}BlobLabel;



KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_BlobLabeling_Create(IN OUT BlobLabel **ppBlobLabel, IN const INT32 nWidth, IN const INT32 nHeight);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_BlobLabeling_Destroy(IN OUT BlobLabel **ppBlobLabel);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_BlobLabeling_UpdateImgSize(IN OUT BlobLabel *pBlobLabel, IN const INT32 nWidth, IN const INT32 nHeight);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_BlobLabeling_DoBlobLabeling(IN BlobLabel *pBlobLabel, IN OUT Img_T *pSrcImg, IN const BinaryMapType nBinaryMap,
                                                                IN const UINT32 nThreshold, IN const INT32 nDegree, IN const INT32 nIndicator);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_BlobLabeling_BlobSmallSizeConstraint(IN BlobLabel *pBlobLabel, IN const INT32 nWidth, IN const INT32 nHeight,
                                                                OUT BlobInfo *pBlobInfo, OUT UINT32 *pNumofBlob);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_BlobLabeling_BlobBigSizeConstraint(IN BlobLabel *pBlobLabel, IN INT32 const nWidth, IN const INT32 nHeight,
                                                                OUT BlobInfo *pBlobInfo, OUT UINT32 *pNumofBlob);
    
#ifdef __cplusplus
}
#endif


#endif
