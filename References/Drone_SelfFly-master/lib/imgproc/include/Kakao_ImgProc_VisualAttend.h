//
//  Kakao_ImgProc_VisualAttend.h
//
//  Created by Maverick Park on 2014. 7. 28.
//  Copyright (c) 2015년 Kakao Corp. All rights reserved.
//

#ifndef __KAKAO_IMGPROC_VISUAL_ATTENTION_H__
#define __KAKAO_IMGPROC_VISUAL_ATTENTION_H__

#include "Kakao_ImgProc_Core.h"



typedef MEM_ALIGNED(4) struct _VisualAttend_T
{
    UINT8                           *pVisualAttendMBwiseMap;
    Img_T                           pVisualAttendImg;
    //Kakao_VC_VisualAttention_T    pVisialAttension;
    void                            *pHandle;
    INT32                           nCheckMemAllocated;
}VisualAttend_T;

#endif
