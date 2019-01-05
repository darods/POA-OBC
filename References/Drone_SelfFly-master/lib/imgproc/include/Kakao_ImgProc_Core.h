//
//  Kakao_ImgProc_Core.h
//
//  Created by Maverick Park on 2014. 8. 7.
//  Copyright (c) 2015년 Kakao Corp. All rights reserved.
//

#ifndef __KAKAO_IMGPROC_CORE_H__
#define __KAKAO_IMGPROC_CORE_H__

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <math.h>
#include <memory.h>
#include "Kakao_ImgProc_Platform.h"
#include "Kakao_ImgProc_Typedefs.h"
#include "Kakao_ImgProc_Define.h"
#include "Kakao_ImgProc_Misc.h"
#include "Kakao_ImgProc_Structure.h"
#include "Kakao_ImgProc_Processor.h"
#include "Kakao_ImgProc_Profile.h"

#if USE_MULTI_THREAD
    #include "Kakao_ImgProc_Thread.h"
#endif

#endif
