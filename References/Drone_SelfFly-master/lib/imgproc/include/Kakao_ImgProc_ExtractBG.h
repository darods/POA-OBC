//
//  Kakao_ImgProc_ExtractBG.h
//
//  Created by Maverick Park on 2014. 7. 28.
//  Copyright (c) 2015년 Kakao Corp. All rights reserved.
//

#ifndef __KAKAO_IMGPROC_EXTRACT_BACKGROUND_H__
#define __KAKAO_IMGPROC_EXTRACT_BACKGROUND_H__

#include "Kakao_ImgProc_Core.h"
#include "Kakao_ImgProc_Filter.h"

#ifdef __cplusplus
extern "C" {
#endif
    
    
typedef MEM_ALIGNED(4) struct _ExtractBG_T
{
    KISize                          nBaseSize;
    Img_T                           *pLatestBGImg;
    Img_T                           *pPrevImg;
    Img_T                           *pCurrImg;
    Img_T                           *pAccImg0;
    Img_T                           *pAccImg1;
    Img_T                           *pFirstDiffImg;
    Img_T                           *pPrevDiffImg;
    Img_T                           *pFirstDiffAccImg;
    Img_T                           *pPrevDiffAccImg;
    Img_T                           *pDiffThAccImg;
    Img_T                           *pBackGroundImg;
    Filter_T                        *pFilter;
}ExtractBG_T;


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_ExtractBG_Create(IN OUT ExtractBG_T **ppExtractBG, IN const INT32 nWidth, IN const INT32 nHeight);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_ExtractBG_Destroy(IN OUT ExtractBG_T **ppExtractBG);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_ExtractBG_GetBG(IN ExtractBG_T *pExtractBG, IN Img_T *pSrcImg);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_ExtractBG_UpdateImgSize(IN OUT ExtractBG_T *pExtractBG, IN const INT32 nWidth, IN const INT32 nHeight);
    
#ifdef __cplusplus
}
#endif


#endif
