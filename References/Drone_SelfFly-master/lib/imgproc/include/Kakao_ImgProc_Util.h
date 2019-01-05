//
//  Kakao_ImgProc_Util.h
//
//  Created by Maverick Park on 2014. 7. 28.
//  Copyright (c) 2015년 Kakao Corp. All rights reserved.
//

#ifndef __KAKAO_IMGPROC_UTIL_H__
#define __KAKAO_IMGPROC_UTIL_H__

#include "Kakao_ImgProc_Core.h"

#ifdef __cplusplus
extern "C" {
#endif
    

KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_CreateImg(IN OUT Img_T **ppTargetImg, IN const INT32 nWidth, IN const INT32 nHeight,
                                                                    IN const ImgFormat nImgFormat, IN const ImgDepth nImgDepth);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_ReleaseImg(IN OUT Img_T **ppTargetImg);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_CreateDummyImg(IN OUT Img_T **ppTargetImg, IN const INT32 nWidth, IN const INT32 nHeight,
                                                                    IN const ImgFormat nImgFormat, IN const ImgDepth nImgDepth);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_CreateBinaryMap(IN Img_T *pSrcImg, IN const BinaryMapType nBinaryMap);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_CreateMBwiseMap(IN OUT Img_T *pSrcImg);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_CreateMBwiseSADMap(IN OUT Img_T *pSrcImg);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_CopyImg(OUT Img_T *pDstImg, IN const Img_T *pSrcImg);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_CopyPlane(OUT UINT8 *pDst, IN const UINT8 *pSrc,
                                                                    IN const INT32 nWidth, IN const INT32 nHeight,
                                                                    IN const INT32 nDstStride, IN const INT32 nSrcStride);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_CopyOneRow(OUT UINT8 *pDst, IN const UINT8 *pSrc, IN const INT32 nWidth);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_Padding_Frame(IN OUT Img_T *pSrcImg);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_Padding_ImgPlane(IN OUT Img_T *pSrcImg);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_FramewiseToMBwise(IN OUT Img_T *pSrcImg, IN const BinaryMapType nBinaryMap, IN const UINT32 nThreshold);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_SubtractAccImg(OUT Img_T *pSubtratcImg, IN const Img_T *pSrcImg0, IN const Img_T *pSrcImg1);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_SubtractAccbyThImg(OUT Img_T *pSubtratcImg, IN const Img_T *pSrcImg0,
                                                                    IN const Img_T *pSrcImg1, IN const UINT32 nTh);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_GetBinaryMap(IN Img_T *pSrcImg, IN const BinaryMapType nBinaryMap, OUT UINT8 **ppBinaryMap);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_ScaleBinaryMap(IN OUT Img_T *pSrcImg, IN const BinaryMapType nBinaryMap, IN const UINT32 nScaleFactor);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_SetROI(IN OUT Img_T *pSrcImg, IN const ROI nROI);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_UpdateImgSize(IN OUT Img_T *pSrcImg, IN const INT32 nWidth, IN const INT32 nHeight,
                                                                    IN const UINT32 *pStride, IN const ImgFormat nImgFormat);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_UpdateSize(IN OUT KISize *pSize, IN const INT32 nWidth, IN const INT32 nHeight,
                                                                    IN const UINT32 *pStride, IN const ImgFormat nImgFormat, IN const INT32 nNumofPlanes);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_SetImgDefaultParam(IN OUT Img_T *pSrcImg);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_Flip(IN OUT Img_T *pSrcImg);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_SetVal(IN OUT Img_T *pSrcImg, IN const UINT8 nVal);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_SetVal_Plane(IN OUT Img_T *pSrcImg, IN const UINT8 nVal, IN const UINT32 nPlaneIdx);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_CleanImg(IN OUT Img_T *pSrcImg);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_Cal1DHistogram(IN OUT Img_T *pSrcImg, OUT F32 *pHistogram, IN const UINT32 nBins,
                                                                    IN const UINT32 nPlaneIdx, IN const IMGPROC_BOOL bNormalized);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_Cal2DHistogram(IN OUT Img_T *pSrcImg, OUT F32 *pHistogram, IN const UINT32 nBins, IN const IMGPROC_BOOL bNormalized);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_Comp1DHist(IN Img_T *pSrcImg0, IN Img_T *pSrcImg1, IN const UINT32 nBins,
                                                                    IN const ImgHistCompMode nHistCompMode, IN const UINT32 nPlaneIdx, OUT F32 *pHICompVal);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_Comp2DHist(IN Img_T *pSrcImg0, IN Img_T *pSrcImg1, IN const UINT32 nBins, IN const ImgHistCompMode nHistCompMode, OUT F32 *pHICompVal);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_Cal1DHistogramBP(IN OUT Img_T *pSrcImg, IN const F32 *pRefHistogram, IN const UINT32 nBins,
                                                                    IN const UINT32 nPlaneIdx, IN const ImgBPMode nHistBPMode);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_Cal2DHistogramBP(IN OUT Img_T *pSrcImg, IN const F32 *pRefHistogram, IN const UINT32 nBins, IN const ImgBPMode nHistBPMode);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_MakeIntegralImg(IN const Img_T *pSrcImg, OUT Img_T *pIntegralImg, OUT Img_T *pSqIntegralImg);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_MergeImgs_WithBinaryMap(IN const Img_T *pSrcImg0, IN OUT Img_T *pSrcImg1,
                                                                    IN OUT Img_T *pDstImg, UINT8 *pMaskMap);
KAKAO_EXPORT_API INT32 Kakao_ImgProc_Util_Cal_SAD16x16(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, IN const INT32 nStride);
KAKAO_EXPORT_API INT32 Kakao_ImgProc_Util_Cal_MBWiseSAD16x16_Plane(IN const Img_T *pSrcImg0, IN const Img_T *pSrcImg1, OUT Img_T *pDstImg, IN const UINT32 nPlaneIdx);
KAKAO_EXPORT_API INT32 Kakao_ImgProc_Util_Cal_Accum16x16(UINT8 *pSrc, INT32 nStride);
KAKAO_EXPORT_API INT32 Kakao_ImgProc_Util_Cal_Mean16x16(UINT8 *pSrc, INT32 nStride);
KAKAO_EXPORT_API INT32 Kakao_ImgProc_Util_Cal_Variance16x16(IN UINT8 *pSrc, IN const INT32 nMean, IN const INT32 nStride);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_FramewiseToMBwise(IN OUT Img_T *pSrcImg, IN const BinaryMapType nBinaryMap,
                                                                    IN const UINT32 nThreshold);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_ScaleBinaryMap(IN OUT Img_T *pSrcImg, IN const BinaryMapType nBinaryMap,
                                                                    IN const UINT32 nScaleFactor);
    
#if ENABLE_ISET(CPU_ARMX | ISET_NEON)
//    KAKAO_EXPORT_API INT32 Kakao_ImgProc_Util_Print_8x8(uint8x8_t Src);
//    KAKAO_EXPORT_API INT32 Kakao_ImgProc_Util_Print_8x16(uint8x16_t Src);
//    KAKAO_EXPORT_API INT32 Kakao_ImgProc_Util_Print_16x8(uint16x8_t Src);
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_SSE4) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_SSE4)
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_SSE3) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_SSE3)
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_NONE) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_NONE)
#else
#endif

KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_Get_CPUCores(OUT INT32 *pNumofCPUCore);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_Get_CPUFrequency(OUT INT32 *pCPUFrequency);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Util_Get_ImgFormatIdx(IN const ImgFormat nImgFormat, OUT INT32 *pImgFormatIdx);

#ifdef __cplusplus
}
#endif

#endif
