//
//  Kakao_ImgProc_ObjDetector.h
//
//  Created by Maverick Park on 2014. 11. 11.
//  Copyright (c) 2015년 Kakao Corp. All rights reserved.
//

#ifndef __KAKAO_IMGPROC_OBJECT_DETECTOR_H__
#define __KAKAO_IMGPROC_OBJECT_DETECTOR_H__

#include "Kakao_ImgProc_Core.h"

#ifdef __cplusplus
extern "C" {
#endif
    
    
#define MAX_NUM_OF_FEATURES                 (3)
#define MAX_NUM_OF_DETECTED_OBJ             (15)
#define MAX_NUM_OF_SCALEDLEVEL              (50)
    
    
typedef enum _ObjModel
{
    OBJMODEL_MIN = 0,
    OBJMODEL_FACE_FRONT,
    OBJMODEL_FACE_FRONT_ALT,
    OBJMODEL_EYE,
    OBJMODEL_NOSE,
    OBJMODEL_SMILE,
    OBJMODEL_MAX,
}ObjModel;


typedef enum _Eye_Site
{
    EYE_SITE_LEFT = 0,
    EYE_SITE_RIGHT,
}Eye_Site;


typedef MEM_ALIGNED(4) struct _ScaledFeature_T
{
    UINT32                          *pTL;
    UINT32                          *pTR;
    UINT32                          *pBL;
    UINT32                          *pBR;
    F32                             nWeight;
}ScaledFeature_T;

    
typedef MEM_ALIGNED(4) struct _ScaledFeatureTable_T
{
    ScaledFeature_T                 nScaledFeature[MAX_NUM_OF_FEATURES];
}ScaledFeatureTable_T;
   

typedef MEM_ALIGNED(4) struct _Feature_T
{
    UINT32                          nNumofRect;
     MEM_ALIGNED(4) struct
    {
        KIRect                      nRect;
        F32                         nWeight;
    }nFeature[MAX_NUM_OF_FEATURES];
    F32                             nThreshold;
    F32                             nBTreeVal[2];
}Feature_T;


typedef MEM_ALIGNED(4) struct _HaarObjData_T
{
    INT32                           nBaseWidth;
    INT32                           nBaseHeight;
    UINT32                          nTotalNumofStages;                                  // Total Num of Stages
    UINT32                          nTotalNumofClassifier;                              // Total Num of Classifiers
    UINT32                          *pNumofClassifiers;                                 // Num of Classifiers of Each Stage
    F32                             *pThresofStages;                                    // Threshold of Each Stage
    Feature_T                       *pFeatures;                                         // Haar Features of Each Classifier of Each Stage
}HaarObjData_T;
    
    
typedef MEM_ALIGNED(4) struct _HaarObjModel_ScaledFeature
{
    ScaledFeature_T                 nIntgralFeature;
    ScaledFeature_T                 nSqIntgralFeature;
    ScaledFeatureTable_T            *pScaledFeatureTable;
    F32                             nInvRectSize;
}HaarObjModel_ScaledFeature;
    
    
typedef MEM_ALIGNED(4) struct _HaarObjModel_T
{
    INT32                           nCurrDegree;
    UINT32                          nCurrLoopCnt;
    ObjModel                        nCurrObjModel;

    HaarObjData_T                   nHaarObjData;
    HaarObjData_T                   *pDegHaarObjData;
    INT32                           nScaledWidth;
    INT32                           nScaledHeight;
    F32                             nCurrScale;

    HaarObjModel_ScaledFeature      nTargetScaledObjModelFeature;
    HaarObjModel_ScaledFeature      *pRefScaledObjModelFeature;
}HaarObjModel_T;


typedef MEM_ALIGNED(4) struct _ObjectModel_T
{
    HaarObjModel_T                  *pHaarObjModel[OBJMODEL_MAX];
}ObjectModel_T;

    
typedef struct _DetectedObjInfo_T
{
    INT32                           nLabel;
    ObjModel                        nObjModel;
    KIRect                          nDetectedObjRect;
}DetectedObjInfo_T;
    
    
typedef MEM_ALIGNED(4) struct _DetectedObjDataSet_T
{
    UINT32                          nNumofDetectedObj;
    DetectedObjInfo_T               nDetectedObjInfo[MAX_NUM_OF_DETECTED_OBJ];
}DetectedObjDataSet_T;

    
typedef MEM_ALIGNED(4) struct _UnRefinedDetectedInfo_T
{
    ObjModel                        nObjModel;
    UINT32                          nNumofDetectedObj;
    INT32                           nLabel[MAX_NUM_OF_DETECTED_OBJ];
    INT32                           nWeight[MAX_NUM_OF_DETECTED_OBJ];
    KIRect                          nDetectedObj[MAX_NUM_OF_DETECTED_OBJ];
}UnRefinedDetectedInfo_T;
    

typedef MEM_ALIGNED(4) struct _ObjDetect_T
{
    KISize                          nBaseSize;
 
    Img_T                           *pIntegralImg;
    Img_T                           *pSqIntegralImg;

    ObjectModel_T                   nObjectModel;
    UnRefinedDetectedInfo_T         nUnrefinedDetecedInfo;
    DetectedObjDataSet_T            *pDetectedObjDataSet;
    
    IMGPROC_BOOL                    bIsUpdateIntegralImg;
}ObjDetect_T;

    
    
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_ObjDetector_Create(IN OUT ObjDetect_T **ppObjDetector, IN const INT32 nWidth, IN const INT32 nHeight);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_ObjDetector_Destroy(IN OUT ObjDetect_T **ppObjDetector);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_ObjDetector_UpdateImgSize(IN OUT ObjDetect_T *pObjDetector, IN const INT32 nWidth, IN const INT32 nHeight);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_ObjDetector_Update_RotateBasedFeature(IN ObjDetect_T *pObjDetector, IN const ObjModel nObjModel, IN const INT32 nDegree);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_ObjDetector_FindObject(IN ObjDetect_T *pObjDetector, IN const Img_T *pSrcImg,
                                                                   IN const ObjModel nObjModel, IN const F32 nScaleFactor,
                                                                   IN const F32 nStartScale, IN const F32 nMaxScaleUp,
                                                                   IN const KIRect nMinRegion, IN const KIRect nMaxRegion,
                                                                   IN const INT32 nDegree, IN const UINT32 nMinNeighbor, IN const UINT32 nMaxNumofDetectedObj);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_ObjDetector_GetDetectedObjDataSet(IN const ObjDetect_T *pObjDetector, OUT DetectedObjDataSet_T *pDetectedObjDataSet);
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_ObjDetector_UpdateIntegralImg(IN OUT ObjDetect_T *pObjDetector, IN const IMGPROC_BOOL bIsUpdateIntegralImg);

#ifdef __cplusplus
}
#endif


#endif
