//
//  Kakao_ImgProc_Filter.h
//
//  Created by Maverick Park on 2014. 8. 14.
//  Copyright (c) 2015년 Kakao Corp. All rights reserved.
//

#ifndef __KAKAO_IMGPROC_ENHANCE_FLT_H__
#define __KAKAO_IMGPROC_ENHANCE_FLT_H__

#include "Kakao_ImgProc_Core.h"
#include "Kakao_ImgProc_Scaler.h"
#include "Kakao_ImgProc_EdgeDetector.h"


#ifdef __cplusplus
extern "C" {
#endif

    
typedef enum _QuantLevel
{
    QUANT_LEVEL_4 = 0,
    QUANT_LEVEL_8,
    QUANT_LEVEL_16,
    QUANT_LEVEL_32,
    QUANT_LEVEL_MAX,
}QuantLevel;


typedef enum _MosiacLevel
{
    MOSAIC_LEVEL_MIN = 0,
    MOSAIC_LEVEL_4 = MOSAIC_LEVEL_MIN,
    MOSAIC_LEVEL_8,
    MOSAIC_LEVEL_16,
    MOSAIC_LEVEL_32,
    MOSAIC_LEVEL_MAX,
}MosiacLevel;

    
typedef struct _RectImgAddr
{
    UINT32                          *pTL;
    UINT32                          *pTR;
    UINT32                          *pBL;
    UINT32                          *pBR;
}RectImgAddr;


typedef union _Vec2
{
    UINT8                           nDummySpace[16];
    KIPoint2                        nPoint2;
    KIPointF2                       nPointF2;
}Vec2;


typedef union _Vec3
{
    UINT8                           nDummySpace[24];
    KIPoint3                        nPoint3;
    KIPointF3                       nPointF3;
}Vec3;


typedef struct _Thread_Param    Thread_Param;
typedef MEM_ALIGNED(4) struct _Filter_T
{
    KISize              nBaseSize;

    EdgeDet_T           *pEdgeDetector;

    UINT32              nGaussianMask1DSize;
    UINT32              nGaussianMask2DSize;
    
    F32                 nSigmaD_Flat;
    F32                 nSigmaD_Edge;
    F32                 nSigmaR_Flat;
    F32                 nSigmaR_Edge;
    F32                 nAlpha;
    F32                 nSigma;
    
    F32                 *pMask_Edge;
    F32                 *pRangeMask_Edge;
    F32                 *pMask_Flat;
    F32                 *pRangeMask_Flat;
    F32                 *pPropMixCoeffi[2];

    F32                 nGaussian1DFltCoeffi[400];
    F32                 nGaussian2DFltCoeffi[400];    
    UINT8               *pQuantTable[QUANT_LEVEL_MAX];
    
    ImgScaler_T         *pScaler;
    
    Img_T               *pTmpF32Img;
    Img_T               *pTmpU8Img;
    Img_T               *pIntegralImg;
    Img_T               *pScaledImg;
    
    UINT8               *pLookUptable;          // For Kakao_ImgProc_Filter_CustomHistogramStretch
    UINT32              nPrevMinVal;            // For Kakao_ImgProc_Filter_CustomHistogramStretch
    UINT32              nPrevMaxVal;            // For Kakao_ImgProc_Filter_CustomHistogramStretch
    
    INT32               nMinClipVal;
    INT32               nMaxClipVal;
    
    // For Bulge Filter
    Img_T               *pPosTransMap;
    IMGPROC_BOOL        bIsBulgePosTransMapMade;
    F32                 nBulgeRadius;
    
    // For Proportional Mix Filter
    KISize              nPropMixFltPrevSize[2];
    
    // For ScreenBlend Filter
    Img_T               *pSBTmpBuf[4];
    
    UINT32              nNumofCPUCore;
    #if USE_MULTI_THREAD
    IMGPROC_BOOL        bIsMultiThreadEnabled;
    ThreadID            *pThreadID;
    sem_t               nStartSema[MAX_THREAD_NUM];
    sem_t               nEndSema[MAX_THREAD_NUM];
    Thread_Param        *pThreadParam;
    #endif
}Filter_T;


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_Create(IN OUT Filter_T **ppFilter, IN const INT32 nWidth, IN const INT32 nHeight);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_Destroy(IN OUT Filter_T **ppFilter);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_UpdateImgSize(IN OUT Filter_T *pFilter, IN const INT32 nWidth, IN const INT32 nHeight);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_TrilateralFlt(IN const Filter_T *pFilter, Img_T *pSrcImg0, Img_T *pSrcImg1, UINT8 *input0, UINT8 *input1);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_BilateralFlt(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg,
                                                                IN const UINT32 nMaskSize, IN const F32 nGSigma, IN const F32 nESigma);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_Gaussian1DBlur(IN Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_Gaussian2DBlur(IN Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_MeanBlur(IN Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_MeanBlur_Plane(IN Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg, IN const UINT32 nPlaneIdx);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_MeanBlur_Horizon(IN Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_MeanBlur_Vertical(IN Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_NbyNMeanBlur(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg,
                                                                IN const INT32 nMaskSize);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_SharpenFlt(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_SharpenFlt_Plane(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg, IN const UINT32 nPlaneIdx);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_NoiseReduction_Plane(IN Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg, IN const UINT32 nPlaneIdx);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_Quantize(IN Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg,
                                                                IN const QuantLevel nQuantLevel);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_Mosaic(IN Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg,
                                                                IN const MosiacLevel nMosiacLevel);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_MedianSquare(IN Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_MedianSquare_Plane(IN Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg, IN const UINT32 nPlaneIdx);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_MedianHorizon(IN Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_MedianVertical(IN Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_NormalizeByMean(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_Equalization(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_HistogrmaStretch(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_CustomHistogramStretch(IN Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_ScreenBlend(IN Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg, IN const F32 nStartRadius, IN const F32 nEndRadius);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_Logarithm(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_Inverse(IN Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_Inverse_Plane(IN Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg, IN const UINT32 nPlaneIdx);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_Binarization(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, IN const INT32 nThreshold);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_ClipImg(IN Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg,
                                                                IN const INT32 nMinClipVal, IN const INT32 nMaxClipVal);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_ClipPlane(IN Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg,
                                                                IN const INT32 nMinClipVal, IN const INT32 nMaxClipVal, IN const UINT32 nPlaneIdx);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_Erosion(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg,
                                                                IN const BinaryMapType nBinaryMap, IN const INT32 nThick);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_Dilation(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg,
                                                                IN const BinaryMapType nBinaryMap);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_Open(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg,
                                                                IN const BinaryMapType nBinaryMap, IN const INT32 nThick);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_Close(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg,
                                                                IN const BinaryMapType nBinaryMap, IN const INT32 nThick);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_Bulge(IN Filter_T *pFilter, IN Img_T *pSrcImg, OUT Img_T *pDstImg,
                                                                IN Vec2 nCenterPos, IN const F32 nAspectRatio, IN const F32 nScaleFactor, IN const F32 nRadius, IN const IMGPROC_BOOL bIsNeedUpdate);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_MeanShift(IN Filter_T *pFilter, IN Img_T *pSrcImg, OUT Img_T *pDstImg);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_DiffImg(IN Filter_T *pFilter, IN const Img_T *pSrcImg0, IN const Img_T *pSrcImg1, OUT Img_T *pDstImg);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_DiffImg_Plane(IN Filter_T *pFilter, IN const Img_T *pSrcImg0, IN const Img_T *pSrcImg1, OUT Img_T *pDstImg, IN const UINT32 nPlaneIdx);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_ABSImg(IN const Filter_T *pFilter, IN const Img_T *pSrcImg0, IN const Img_T *pSrcImg1, OUT Img_T *pDstImg);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_AccumImg(IN const Filter_T *pFilter, IN OUT Img_T *pAccumImg, IN const Img_T *pSrcImg);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_Multiply(IN Filter_T *pFilter, IN const Img_T *pSrcImg0, IN const Img_T *pSrcImg1, OUT const Img_T *pDstImg);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_Multiply_Plane(IN Filter_T *pFilter, IN const Img_T *pSrcImg0, IN const Img_T *pSrcImg1, OUT const Img_T *pDstImg, IN const UINT32 nPlaneIdx);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_MixImg(IN const Filter_T *pFilter, IN const Img_T *pSrcImg0, IN const Img_T *pSrcImg1, OUT Img_T *pDstImg, IN const F32 nMixRatio);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_MixImg_Plane(IN const Filter_T *pFilter, IN const Img_T *pSrcImg0, IN const Img_T *pSrcImg1, OUT Img_T *pDstImg,
                                                                IN const F32 nMixRatio, IN const UINT32 nPlaneIdx);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_ProportionalMixImg(IN Filter_T *pFilter, IN const Img_T *pSrcImg0, IN const Img_T *pSrcImg1, OUT Img_T *pDstImg,
                                                                IN const F32 nStartRadius, IN const F32 nEndRadius);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_ProportionalMixImg_Plane(IN Filter_T *pFilter, IN const Img_T *pSrcImg0, IN const Img_T *pSrcImg1, OUT Img_T *pDstImg,
                                                                IN const UINT32 nPlaneIdx, IN const F32 nStartRadius, IN const F32 nEndRadius);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_Generate_PropMixCoeffi(IN Filter_T *pFilter, IN const KISize nImgSize, IN const UINT32 nPlaneIdx, IN const F32 nStartRadius, IN const F32 nEndRadius);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_ContrastShift(IN Filter_T *pFilter, IN const Img_T *pSrcImg0, IN const Img_T *pSrcImg1, OUT Img_T *pDstImg, IN const UINT32 nPlaneIdx);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_ContrastShift_Plane(IN Filter_T *pFilter, IN const Img_T *pSrcImg0, IN const Img_T *pSrcImg1, OUT Img_T *pDstImg, IN const UINT32 nPlaneIdx);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_Div(IN const Filter_T *pFilter, IN OUT Img_T *pSrcImg, IN const BinaryMapType nBinaryMap, IN const UINT32 nScaleFactor);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Filter_Rotate(IN const Filter_T *pFilter, IN const Img_T *pSrcImg, OUT Img_T *pDstImg, IN const UINT32 nDegree);

    
#ifdef __cplusplus
}
#endif


#endif
