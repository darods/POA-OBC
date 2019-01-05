


/*----------------------------------------------------------------------------------------
 File Inclusions
 ----------------------------------------------------------------------------------------*/
#include "Kakao_ImgProc_Modules.h"
#include "Kakao_ImgProc_SIMD.h"
#include "Kakao_Haarfeatures_Eyes.h"
//#include "Kakao_Haarfeatures_Face_front.h"
#include "Kakao_Haarfeatures_Face_front_alt.h"
//#include "Kakao_Haarfeatures_Nose.h"
//#include "Kakao_Haarfeatures_Smile.h"


/*----------------------------------------------------------------------------------------
 Constant Definitions
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Macro Definitions
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Type Definitions
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Static Function
 ----------------------------------------------------------------------------------------*/
static Kakao_Status _Scan_OneScale(IN ObjDetect_T *pObjDetect, IN const ObjModel nObjModel, IN const Img_T *pSrcImg, IN const UINT32 nMaxNumofDetectedObj);
static Kakao_Status _ObjDetector_ScanFeature(IN ObjDetect_T *pObjDetect, HaarObjModel_T *pHaarObjModel, INT32 nOffset);
#if ENABLE_ISET(CPU_ARMX | ISET_NEON)
    static Kakao_Status _ObjDetector_Set_Classifier_ForScale_NEON(IN const ObjDetect_T *pObjDetect, IN HaarObjModel_T *pHaarObjModel,
                                                                  IN const F32 nScale, IN const INT32 nScaleIdx, IN const ObjModel nObjModel, IN const INT32 nDegree);
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_SSE4) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_SSE4)
    static Kakao_Status _ObjDetector_Set_Classifier_ForScale_SSE4(IN const ObjDetect_T *pObjDetect, IN HaarObjModel_T *pHaarObjModel,
                                                                  IN const F32 nScale, IN const INT32 nScaleIdx, IN const ObjModel nObjModel, IN const INT32 nDegree);
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_SSE3) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_SSE3)
    static Kakao_Status _ObjDetector_Set_Classifier_ForScale_SSE3(IN const ObjDetect_T *pObjDetect, IN HaarObjModel_T *pHaarObjModel,
                                                                  IN const F32 nScale, IN const INT32 nScaleIdx, IN const ObjModel nObjModel, IN const INT32 nDegree);
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_SSE2) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_SSE2)
    static Kakao_Status _ObjDetector_Set_Classifier_ForScale_SSE2(IN const ObjDetect_T *pObjDetect, IN HaarObjModel_T *pHaarObjModel,
                                                                  IN const F32 nScale, IN const INT32 nScaleIdx, IN const ObjModel nObjModel, IN const INT32 nDegree);
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_SSE) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_SSE)
    static Kakao_Status _ObjDetector_Set_Classifier_ForScale_SSE(IN const ObjDetect_T *pObjDetect, IN HaarObjModel_T *pHaarObjModel,
                                                                 IN const F32 nScale, IN const INT32 nScaleIdx, IN const ObjModel nObjModel, IN const INT32 nDegree);
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_NONE) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_NONE)
    static Kakao_Status _ObjDetector_Set_Classifier_ForScale_C(IN const ObjDetect_T *pObjDetect, IN HaarObjModel_T *pHaarObjModel,
                                                               IN const F32 nScale, IN const INT32 nScaleIdx, IN const ObjModel nObjModel, IN const INT32 nDegree);
#else
    static Kakao_Status _ObjDetector_Set_Classifier_ForScale_C(IN const ObjDetect_T *pObjDetect, IN HaarObjModel_T *pHaarObjModel,
                                                               IN const F32 nScale, IN const INT32 nScaleIdx, IN const ObjModel nObjModel, IN const INT32 nDegree);
#endif
static Kakao_Status _ImgProc_ObjDetector_Update_RotateBasedFeature(IN ObjDetect_T *pObjDetect, IN const ObjModel nObjModel, IN const INT32 nDegree);
static Kakao_Status _ImgProc_ObjDetector_Rotate_Rect(KIRect *pSrcRect, KIRect *pDstRect, const INT32 nDegree, const INT32 nCenterX, const INT32 nCenterY);
static Kakao_Status _ImgProc_ObjDetector_RefineDetectedObj(UnRefinedDetectedInfo_T *pUnrefinedDetecedInfo, IN const INT32 nGroupTh, IN const D64 nEps);
static INT32 _ImgProc_ObjDetector_RectPartition(IN UnRefinedDetectedInfo_T *pUnrefinedDetecedInfo, IN const D64 nWeight);
static IMGPROC_BOOL _ImgProc_ObjDetector_Predicate(IN const KIRect *pRect0, IN const KIRect *pRect1, IN const D64 nWeight);


#define _ObjDetector_Set_Classifier_ForScale              MAKE_ACCEL_FUNC(_ObjDetector_Set_Classifier_ForScale)


/*----------------------------------------------------------------------------------------
 Static Variable
 ----------------------------------------------------------------------------------------*/
static const HaarObjData_T  *_gpHaarObjData[OBJMODEL_MAX] =
{
    /*NULL, &Model_FaceFront, &Model_FaceFront_Alt, &Model_Eye, &Model_Nose, &Model_Smile*/
    NULL, NULL, &Model_FaceFront_Alt, &Model_Eye, NULL, NULL
};


/*----------------------------------------------------------------------------------------
 Global Variable
 ----------------------------------------------------------------------------------------*/
//UnRefinedDetectedInfo_T _gnUnrefinedDetecedInfo;                  // For Debug


/*----------------------------------------------------------------------------------------
 Function Implementation
 ----------------------------------------------------------------------------------------*/
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_ObjDetector_Create(IN OUT ObjDetect_T **ppObjDetector, IN const INT32 nWidth, IN const INT32 nHeight)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   i = 0;
    ObjDetect_T             *pTmpObjDetector = NULL;
    INT32                   nAlignedWidth = GET_ALIGNED_LENGTH(nWidth, MACRO_BLOCK_SIZE);
    
    SAFEALLOC(pTmpObjDetector, 1, 32, ObjDetect_T);
    
    Kakao_ImgProc_Util_UpdateSize(&pTmpObjDetector->nBaseSize, nWidth, nHeight, NULL, IMG_FORMAT_MAX, 4);
    
    for(i=OBJMODEL_FACE_FRONT ; i<OBJMODEL_MAX ; i++)
    {
        HaarObjModel_T          *pLocalHaarObjModel = NULL;
        
        SAFEALLOC(pTmpObjDetector->nObjectModel.pHaarObjModel[i], 1, 32, HaarObjModel_T);
        SAFEALLOC(pTmpObjDetector->nObjectModel.pHaarObjModel[i]->pRefScaledObjModelFeature, MAX_NUM_OF_SCALEDLEVEL, 32, HaarObjModel_ScaledFeature);
        
        pLocalHaarObjModel = pTmpObjDetector->nObjectModel.pHaarObjModel[i];
        
        pLocalHaarObjModel->nCurrDegree = 0xFFFFFFFF;
        pLocalHaarObjModel->nCurrLoopCnt = 0;
        pLocalHaarObjModel->nCurrObjModel = OBJMODEL_MAX;
        
        if(NULL != _gpHaarObjData[i])
            MEMCPY(&(pLocalHaarObjModel->nHaarObjData), _gpHaarObjData[i], sizeof(HaarObjData_T));
    }
    
    Kakao_ImgProc_Util_CreateImg(&(pTmpObjDetector->pIntegralImg), nWidth, nHeight, IMG_FORMAT_GRAY, IMG_DEPTH_U32);
    Kakao_ImgProc_Util_CreateImg(&(pTmpObjDetector->pSqIntegralImg), nWidth, nHeight, IMG_FORMAT_GRAY, IMG_DEPTH_U32);
    
    SAFEALLOC(pTmpObjDetector->pDetectedObjDataSet, 1, 32, DetectedObjDataSet_T);
    
    pTmpObjDetector->bIsUpdateIntegralImg = IMGPROC_FALSE;
    
    *ppObjDetector = pTmpObjDetector;
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    Kakao_ImgProc_ObjDetector_Destroy(&pTmpObjDetector);
    return nRet;
}



KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_ObjDetector_Destroy(IN OUT ObjDetect_T **ppObjDetector)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   i = 0, j = 0;
    
    CHECK_POINTER_VALIDATION((*ppObjDetector), KAKAO_STAT_NULL_POINTER);

    SAFEFREE((*ppObjDetector)->pDetectedObjDataSet);
    
    Kakao_ImgProc_Util_ReleaseImg(&((*ppObjDetector)->pIntegralImg));
    Kakao_ImgProc_Util_ReleaseImg(&((*ppObjDetector)->pSqIntegralImg));
    
    for(i=OBJMODEL_FACE_FRONT ; i<OBJMODEL_MAX ; i++)
    {
        HaarObjModel_T          *pLocalHaarObjModel = NULL;
        
        pLocalHaarObjModel = (*ppObjDetector)->nObjectModel.pHaarObjModel[i];

        for(j=0 ; j<MAX_NUM_OF_SCALEDLEVEL ; j++)
            SAFEFREE(pLocalHaarObjModel->pRefScaledObjModelFeature[j].pScaledFeatureTable);

        if(NULL != pLocalHaarObjModel->pDegHaarObjData)
        {
            SAFEFREE(pLocalHaarObjModel->pDegHaarObjData->pNumofClassifiers);
            SAFEFREE(pLocalHaarObjModel->pDegHaarObjData->pThresofStages);
            SAFEFREE(pLocalHaarObjModel->pDegHaarObjData->pFeatures);
            SAFEFREE(pLocalHaarObjModel->pDegHaarObjData);
        }
        
        SAFEFREE((*ppObjDetector)->nObjectModel.pHaarObjModel[i]->pRefScaledObjModelFeature);
        SAFEFREE((*ppObjDetector)->nObjectModel.pHaarObjModel[i]);
    }
    
    SAFEFREE((*ppObjDetector));
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_ObjDetector_UpdateImgSize(IN OUT ObjDetect_T *pObjDetector, IN const INT32 nWidth, IN const INT32 nHeight)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pObjDetector, KAKAO_STAT_NULL_POINTER);
    
    Kakao_ImgProc_Util_UpdateSize(&pObjDetector->nBaseSize, nWidth, nHeight, NULL, IMG_FORMAT_MAX, 4);
    
    Kakao_ImgProc_Util_UpdateImgSize(pObjDetector->pIntegralImg, nWidth, nHeight, NULL, IMG_FORMAT_GRAY);
    Kakao_ImgProc_Util_UpdateImgSize(pObjDetector->pSqIntegralImg, nWidth, nHeight, NULL, IMG_FORMAT_GRAY);
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_ObjDetector_Update_RotateBasedFeature(IN ObjDetect_T *pObjDetector, IN const ObjModel nObjModel, IN const INT32 nDegree)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    CHECK_POINTER_VALIDATION(pObjDetector, KAKAO_STAT_NULL_POINTER);

    if(KAKAO_STAT_OK != (nRet = _ImgProc_ObjDetector_Update_RotateBasedFeature(pObjDetector, nObjModel, nDegree)))
        return nRet;

    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_ObjDetector_FindObject(IN ObjDetect_T *pObjDetector, IN const Img_T *pSrcImg,
                                                               IN const ObjModel nObjModel, IN const F32 nScaleFactor,
                                                               IN const F32 nStartScale, IN const F32 nMaxScaleUp,
                                                               IN const KIRect nMinRegion, IN const KIRect nMaxRegion,
                                                               IN const INT32 nDegree, IN const UINT32 nMinNeighbor, IN const UINT32 nMaxNumofDetectedObj)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    ObjectModel_T           *pObjectModel = NULL;
    HaarObjModel_T          *pHaarObjModel = NULL;
    HaarObjData_T           *pHaarObjData = NULL;
    UINT32                  i = 0, nLoopCnt = 0;
    INT32                   nWidth = 0;
    INT32                   nHeight = 0;
    INT32                   nStride = 0;
    UINT32                  nScaleLoop = 0;
    IMGPROC_BOOL            bDoneMakingIntegralImg = IMGPROC_FALSE;
    F32                     nScale = 0.0f;
    //IMGPROC_BOOL            bIsSkinMap = IMGPROC_FALSE;
    //UINT8                   *pSkinMap = NULL;
    
    CHECK_POINTER_VALIDATION(pObjDetector, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER);
    
    pObjectModel = &(pObjDetector->nObjectModel);
    pHaarObjModel = pObjectModel->pHaarObjModel[nObjModel];
    pHaarObjData = &(pHaarObjModel->nHaarObjData);
    
    nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    
    // Calculate Total Loop for ScaleSpace
    for(nScaleLoop = 0, nScale = nStartScale;
        ((nScale < nMaxScaleUp) && ((pHaarObjData->nBaseWidth * nScale) < (nWidth-10)) && ((pHaarObjData->nBaseHeight * nScale) < (nHeight-10)));
        nScaleLoop++, nScale *= nScaleFactor)
        ;
    
    // Restrict the Loop
    if(MAX_NUM_OF_SCALEDLEVEL < nScaleLoop)
        nScaleLoop = MAX_NUM_OF_SCALEDLEVEL;

    if((pHaarObjModel->nCurrDegree != nDegree) || (pHaarObjModel->nCurrObjModel != nObjModel) || (pHaarObjModel->nCurrLoopCnt != nScaleLoop))
    {
        // Update Features by Degree
        if((pHaarObjModel->nCurrDegree != nDegree) || (pHaarObjModel->nCurrObjModel != nObjModel))
        {
            _ImgProc_ObjDetector_Update_RotateBasedFeature(pObjDetector, nObjModel, nDegree);
            pHaarObjModel->nCurrLoopCnt = 0;
        }
    
        // Make Scaled Feature At Once for Each Scale
        nScale = nStartScale;
        for(nLoopCnt=0 ; nLoopCnt<nScaleLoop ; nLoopCnt++)
        {
            nScale *= nScaleFactor;
            
            _ObjDetector_Set_Classifier_ForScale(pObjDetector, pHaarObjModel, nScale, nLoopCnt, nObjModel, nDegree);
        }

        pHaarObjModel->nCurrDegree = nDegree;
        pHaarObjModel->nCurrObjModel = nObjModel;
        pHaarObjModel->nCurrLoopCnt = nScaleLoop;
        
        pObjDetector->pDetectedObjDataSet->nNumofDetectedObj = 0;
        
        // Making Rotated- and Scaled- FeatureMap is Very Heavy Job, Thus Skip This Frame for Performance
        return KAKAO_STAT_OK;
    }

    ___START_PROFILE(ObjDetect)

    // Do Finding Object
    nScale = nStartScale;
    pObjDetector->nUnrefinedDetecedInfo.nNumofDetectedObj = 0;
    pObjDetector->nUnrefinedDetecedInfo.nObjModel = nObjModel;
    MEMSET(&(pObjDetector->nUnrefinedDetecedInfo.nDetectedObj[0]), 0, MAX_NUM_OF_DETECTED_OBJ * sizeof(KIRect));
    
    //if(NULL != pSrcImg->pBinaryMap[IMG_BINARYMAP_SKIN])
    //{
    //    if(NULL == (pSkinMap = pSrcImg->GetImgBinaryMap(pSrcImg, IMG_BINARYMAP_SKIN)))
    //        return KAKAO_STAT_FAIL;
    //    bIsSkinMap = IMGPROC_TRUE;
    //}
    
    for(nLoopCnt=0 ; nLoopCnt<nScaleLoop ; nLoopCnt++)
    {
        KIPoint2            nEndPtr = {0, };
        
        nScale *= nScaleFactor;
        pHaarObjModel->nCurrScale = nScale;
        
        pHaarObjModel->nScaledWidth = ROUND(pHaarObjData->nBaseWidth * nScale);
        pHaarObjModel->nScaledHeight = ROUND(pHaarObjData->nBaseHeight * nScale);
        
        if((pHaarObjModel->nScaledWidth < nMinRegion.nWidth) || (pHaarObjModel->nScaledHeight < nMinRegion.nHeight))
            continue;
        
        if((pHaarObjModel->nScaledWidth > nMaxRegion.nWidth) || (pHaarObjModel->nScaledHeight > nMaxRegion.nHeight))
            break;

        nEndPtr.x = ((BaseImg_T *)pSrcImg)->nROI.nEndX - pHaarObjModel->nScaledWidth;
        nEndPtr.y = ((BaseImg_T *)pSrcImg)->nROI.nEndY - pHaarObjModel->nScaledHeight;
        
        if((nEndPtr.x <= 0) || (nEndPtr.y <= 0))
           break;

        // Make Integral Image
        if((IMGPROC_TRUE == pObjDetector->bIsUpdateIntegralImg) && (IMGPROC_FALSE == bDoneMakingIntegralImg))
        {
            Kakao_ImgProc_Util_MakeIntegralImg(pSrcImg, pObjDetector->pIntegralImg, pObjDetector->pSqIntegralImg);
            
            bDoneMakingIntegralImg = IMGPROC_TRUE;
            pObjDetector->bIsUpdateIntegralImg = IMGPROC_FALSE;
        }
        
        // Set Proper Param for Each Scale using Pre-Calculated Param
        pHaarObjModel->nTargetScaledObjModelFeature = pHaarObjModel->pRefScaledObjModelFeature[nLoopCnt];
        
        _Scan_OneScale(pObjDetector, nObjModel, pSrcImg, nMaxNumofDetectedObj);
    }

    // For Debug
    //MEMCPY(&_gnUnrefinedDetecedInfo, &(pObjDetect->nUnrefinedDetecedInfo), sizeof(UnRefinedDetectedInfo_T));
    
//SkipFinding:
    // Refine Detected ObjectInfo
    _ImgProc_ObjDetector_RefineDetectedObj(&(pObjDetector->nUnrefinedDetecedInfo), GET_MAX(nMinNeighbor, 1), 0.2);
    
    // Copy Refined Info of Detected Object
    for(i=0 ; i<pObjDetector->nUnrefinedDetecedInfo.nNumofDetectedObj ; i++)
    {
        DetectedObjInfo_T       *pDstObjInfo = &(pObjDetector->pDetectedObjDataSet->nDetectedObjInfo[i]);
        
        pDstObjInfo->nLabel = i;
        pDstObjInfo->nObjModel = pObjDetector->nUnrefinedDetecedInfo.nObjModel;
        pDstObjInfo->nDetectedObjRect = pObjDetector->nUnrefinedDetecedInfo.nDetectedObj[i];
    }
    pObjDetector->pDetectedObjDataSet->nNumofDetectedObj = pObjDetector->nUnrefinedDetecedInfo.nNumofDetectedObj;
    
    ___STOP_PROFILE(ObjDetect)
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_ObjDetector_GetDetectedObjDataSet(IN const ObjDetect_T *pObjDetector, OUT DetectedObjDataSet_T *pDetectedObjDataSet)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pObjDetector, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pDetectedObjDataSet, KAKAO_STAT_NULL_POINTER);
    
    #if 0
    pDetectedObjDataSet->nNumofDetectedObj = pObjDetector->pDetectedObjDataSet->nNumofDetectedObj;
    if(0 != pObjDetector->pDetectedObjDataSet->nNumofDetectedObj)
        MEMCPY(&(pDetectedObjDataSet->nDetectedObjInfo[0]), &(pObjDetector->pDetectedObjDataSet->nDetectedObjInfo[0]),
               pObjDetector->pDetectedObjDataSet->nNumofDetectedObj * sizeof(DetectedObjInfo_T));
    #else
    {
        INT32               i = 0, j = 0, k = 0;
        INT32               nNumofFace = 0;
        INT32               nAllObjIdx = 0;
        UINT32              nDistArray[MAX_NUM_OF_DETECTED_OBJ] = {0, };

        pDetectedObjDataSet->nNumofDetectedObj = pObjDetector->pDetectedObjDataSet->nNumofDetectedObj;

        // Calculate Distance From Origin of Image If Object is Face
        for(i=0 ; i<pDetectedObjDataSet->nNumofDetectedObj ; i++)
        {
            const DetectedObjInfo_T     *pDetectedObjInfo = &(pObjDetector->pDetectedObjDataSet->nDetectedObjInfo[i]);
            
            if((OBJMODEL_FACE_FRONT == pDetectedObjInfo->nObjModel)
               || (OBJMODEL_FACE_FRONT_ALT == pDetectedObjInfo->nObjModel))
            {
                const KIRect            *pObjRec = &(pDetectedObjInfo->nDetectedObjRect);
                const KIPoint2          nObjCenter = {(pObjRec->x + (pObjRec->nWidth / 2)), (pObjRec->y + (pObjRec->nHeight / 2))};
                
                nDistArray[i] = SQRT((nObjCenter.x * nObjCenter.x) + (nObjCenter.y * nObjCenter.y));
                nNumofFace++;
            }
        }

        // Sort Objects by Distance From Origin
        for(i=0 ; i<nNumofFace ; i++)
        {
            UINT32                      nMinVal = 0xFFFFFFFF;
            INT32                       nMinIdx = 0;
            const DetectedObjInfo_T     *pDetectedObjInfo = pObjDetector->pDetectedObjDataSet->nDetectedObjInfo;
            
            for(j=0 ; j<nNumofFace ; j++)
            {
                if(nDistArray[j] < nMinVal)
                {
                    nMinVal = nDistArray[j];
                    nMinIdx = j;
                }
            }

            // Save Face Info & Loop to Check Whether Belonged Object is or Not
            pDetectedObjDataSet->nDetectedObjInfo[nAllObjIdx++] = pDetectedObjInfo[nMinIdx];
            for(k=0 ; k<pDetectedObjDataSet->nNumofDetectedObj ; k++)
                if((pDetectedObjInfo[nMinIdx].nLabel == pDetectedObjInfo[k].nLabel) && (nMinIdx != k))
                    pDetectedObjDataSet->nDetectedObjInfo[nAllObjIdx++] = pDetectedObjInfo[k];
        
            nDistArray[nMinIdx] = 0xFFFFFFFF;
        }
    }
    #endif
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_ObjDetector_UpdateIntegralImg(IN OUT ObjDetect_T *pObjDetector, IN const IMGPROC_BOOL bIsUpdateIntegralImg)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pObjDetector, KAKAO_STAT_NULL_POINTER);

    pObjDetector->bIsUpdateIntegralImg = bIsUpdateIntegralImg;
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _Scan_OneScale(IN ObjDetect_T *pObjDetect, IN const ObjModel nObjModel, IN const Img_T *pSrcImg, IN const UINT32 nMaxNumofDetectedObj)
{
    INT32                   i = 0, j = 0;
    KIPoint2                nStartPtr = {0, }, nEndPtr = {0, };
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    ObjectModel_T           *pObjectModel = &(pObjDetect->nObjectModel);
    HaarObjModel_T          *pHaarObjModel = pObjectModel->pHaarObjModel[nObjModel];
    const INT32             nDivOffset = (OBJMODEL_FACE_FRONT_ALT == nObjModel) ? 8 : 6;
    const INT32             nXSearchOffset = pHaarObjModel->nScaledHeight / nDivOffset;
    const INT32             nYSearchOffset = pHaarObjModel->nScaledHeight / nDivOffset;
    
    if(IMGPROC_TRUE != pSrcImg->GetImgROIStatus(pSrcImg))
    {
        nStartPtr.x = 0;
        nStartPtr.y = 0;
        nEndPtr.x = nWidth - pHaarObjModel->nScaledWidth;
        nEndPtr.y = nHeight - pHaarObjModel->nScaledHeight;
    }
    else
    {
        const ROI           nROI = pSrcImg->GetImgROI(pSrcImg);
        
        nStartPtr.x = nROI.nStartX;
        nStartPtr.y = nROI.nStartY;
        nEndPtr.x = nROI.nEndX - pHaarObjModel->nScaledWidth;
        nEndPtr.y = nROI.nEndY - pHaarObjModel->nScaledHeight;
    }
    
    for(j=nStartPtr.y ; j<nEndPtr.y ; j+=nYSearchOffset)
    {
        for(i=nStartPtr.x ; i<nEndPtr.x ; i+=nXSearchOffset)
        {
            const INT32     nOffset = j * nStride + i;
            //const INT32     nSkinMapOffset = (j + nROIStartY) * nStride + (i + nROIStartX);;
            
            //if((IMGPROC_TRUE == bIsSkinMap) && (0 == pSkinMap[nSkinMapOffset]))
            //    continue;
            
            if(KAKAO_STAT_OK == _ObjDetector_ScanFeature(pObjDetect, pHaarObjModel, nOffset))
            {
                const UINT32            nNumofDetectedObj = pObjDetect->nUnrefinedDetecedInfo.nNumofDetectedObj;
                KIRect                  *pDstRect = &(pObjDetect->nUnrefinedDetecedInfo.nDetectedObj[nNumofDetectedObj]);
            
                if((MAX_NUM_OF_DETECTED_OBJ <= pObjDetect->nUnrefinedDetecedInfo.nNumofDetectedObj) ||
                   (nMaxNumofDetectedObj <= pObjDetect->nUnrefinedDetecedInfo.nNumofDetectedObj))
                    return KAKAO_STAT_OK;

                // Put Detected Result
                pDstRect->x = i;
                pDstRect->y = j;
                pDstRect->nWidth = pHaarObjModel->nScaledWidth;
                pDstRect->nHeight = pHaarObjModel->nScaledHeight;
                pObjDetect->nUnrefinedDetecedInfo.nNumofDetectedObj++;
            }
        }
    }
    
    return KAKAO_STAT_OK;
}


static Kakao_Status _ObjDetector_ScanFeature(IN ObjDetect_T *pObjDetect, HaarObjModel_T *pHaarObjModel, INT32 nOffset)
{
    Kakao_Status                        nRet = KAKAO_STAT_FAIL;
    HaarObjData_T                       *pHaarObjData = &(pHaarObjModel->nHaarObjData);
    UINT32                              nCurrHaarFeature = 0;
    UINT32                              i = 0, j = 0;
    F32                                 nSumofStage = 0.0f;
    F32                                 nNormalizedVar = 0.0f;
    F32                                 nMean = 0.0f;
    const HaarObjModel_ScaledFeature    *pLocalScaledHaarObjModelFeature = &(pHaarObjModel->nTargetScaledObjModelFeature);
    
    nMean = (F32)(CALCULATE_INTEGRAL_SUM(pLocalScaledHaarObjModelFeature->nIntgralFeature, nOffset)) * pLocalScaledHaarObjModelFeature->nInvRectSize;
    nNormalizedVar = (F32)(CALCULATE_INTEGRAL_SUM(pLocalScaledHaarObjModelFeature->nSqIntgralFeature, nOffset));
    nNormalizedVar = nNormalizedVar * pLocalScaledHaarObjModelFeature->nInvRectSize;
    nNormalizedVar -= (nMean * nMean);
    if(nNormalizedVar >= 0.0f)
        nNormalizedVar = (F32)(SQRT(nNormalizedVar));
    else
        nNormalizedVar = 1.0f;
    
    for(i=0 ; i<pHaarObjData->nTotalNumofStages ; i++)
    {
        const Feature_T                 *pHaarFeature = &(pHaarObjData->pFeatures[nCurrHaarFeature]);
        const ScaledFeatureTable_T      *pScaledFeatureTable = &(pLocalScaledHaarObjModelFeature->pScaledFeatureTable[nCurrHaarFeature]);
        const UINT32                    nNumofClassifiers = pHaarObjData->pNumofClassifiers[i];
     
        nSumofStage = 0.0f;

        for(j=0 ; j<nNumofClassifiers ; j++)
        {
            const ScaledFeature_T       *pScaleFeature0 = &(pScaledFeatureTable->nScaledFeature[0]);
            const ScaledFeature_T       *pScaleFeature1 = &(pScaledFeatureTable->nScaledFeature[1]);
            const ScaledFeature_T       *pScaleFeature2 = &(pScaledFeatureTable->nScaledFeature[2]);
            const F32                   nTh = pHaarFeature->nThreshold * nNormalizedVar;
            F32                         nIntegralSum = 0.0f;
            
            nIntegralSum += (F32)(CALCULATE_INTEGRAL_SUM((*pScaleFeature0), nOffset)) * pScaleFeature0->nWeight;
            nIntegralSum += (F32)(CALCULATE_INTEGRAL_SUM((*pScaleFeature1), nOffset)) * pScaleFeature1->nWeight;
            
            if(3 == pHaarFeature->nNumofRect)
                if(pScaledFeatureTable->nScaledFeature[2].pTL)
                    nIntegralSum += (F32)(CALCULATE_INTEGRAL_SUM((*pScaleFeature2), nOffset)) * pScaleFeature2->nWeight;
            
            nSumofStage += pHaarFeature->nBTreeVal[nIntegralSum >= nTh];

            pHaarFeature++;
            pScaledFeatureTable++;
            nCurrHaarFeature++;
        }
        
        if(nSumofStage < pHaarObjData->pThresofStages[i])
            return KAKAO_STAT_FAIL;
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}


#if ENABLE_ISET(CPU_ARMX | ISET_NEON)
#ifndef __arm64__
static Kakao_Status _ObjDetector_Set_Classifier_ForScale_NEON(IN const ObjDetect_T *pObjDetect, IN HaarObjModel_T *pHaarObjModel, IN const F32 nScale, IN const INT32 nScaleIdx, IN const ObjModel nObjModel, IN const INT32 nDegree)
{
    Kakao_Status                        nRet = KAKAO_STAT_FAIL;
    const Img_T                         *pIntegralImg = pObjDetect->pIntegralImg;
    const HaarObjData_T                 *pHaarObjData = pHaarObjModel->pDegHaarObjData;
    const INT32                         nStride = pIntegralImg->GetImgStride(pIntegralImg, IMG_PLANE_0);
    UINT32                              nCurrHaarFeature = 0;
    INT32                               i = 0, j = 0;
    F32                                 nWeightScale = 0.0f;
    INT32                               nOrgCenterX = 0;
    INT32                               nOrgCenterY = 0;
    int32x4_t                           nX, nY, nW, nH, nS;
    int32x4_t                           nXWAdd, nYHAdd;
    int32x4x2_t                         nXWComp;
    int32x4_t                           nYHComp;
    int32x4_t                           nStartPos, nLocalPos, nScaledPos, nDstPos;
    HaarObjModel_ScaledFeature          *pLocalScaledHaarObjModelFeature = &(pHaarObjModel->pRefScaledObjModelFeature[nScaleIdx]);
    
    if(NULL == pLocalScaledHaarObjModelFeature->pScaledFeatureTable)
        SAFEALLOC(pLocalScaledHaarObjModelFeature->pScaledFeatureTable, pHaarObjData->nTotalNumofClassifier, 32, ScaledFeatureTable_T);

    // Load Stride
    nS = vdupq_n_s32(nStride);

    {
        KIRect              nTmpRect = {0, };
        UINT32              *pIntegralSrc = (UINT32 *)(pObjDetect->pIntegralImg->GetImgPlanes(pObjDetect->pIntegralImg, IMG_PLANE_0));
        UINT32              *pSqIntegralSrc = (UINT32 *)(pObjDetect->pSqIntegralImg->GetImgPlanes(pObjDetect->pSqIntegralImg, IMG_PLANE_0));
        int32x4_t           nTargetRect;
        
        nTmpRect.x = nTmpRect.y = ROUND(nScale);
        nTmpRect.nWidth = ROUND((pHaarObjData->nBaseWidth - 2) * nScale);
        nTmpRect.nHeight = ROUND((pHaarObjData->nBaseHeight - 2) * nScale);
        nOrgCenterX = ROUND(pHaarObjData->nBaseWidth * nScale) / 2 + 1;
        nOrgCenterY = ROUND(pHaarObjData->nBaseHeight * nScale) / 2 + 1;
        nWeightScale = 1.0f / (nTmpRect.nWidth * nTmpRect.nHeight);
        pLocalScaledHaarObjModelFeature->nInvRectSize = nWeightScale;
        
        nTargetRect = vld1q_s32((INT32 *)(&nTmpRect));
        
        nX = vdupq_n_s32(vgetq_lane_s32(nTargetRect, 0));
        nY = vdupq_n_s32(vgetq_lane_s32(nTargetRect, 1));
        nW = vdupq_n_s32(vgetq_lane_s32(nTargetRect, 2));
        nH = vdupq_n_s32(vgetq_lane_s32(nTargetRect, 3));
        nXWAdd = vaddq_s32(nX, nW);
        nYHAdd = vaddq_s32(nY, nH);
        nXWComp = vtrnq_s32(nX, nXWAdd);
        nYHComp = vextq_s32(nY, nYHAdd, 2);
        nStartPos = vdupq_n_s32(((INT32)(pIntegralSrc)));
        nLocalPos = vmlaq_s32(nXWComp.val[0], nYHComp, nS);
        nScaledPos = vshlq_n_s32(nLocalPos, 2);
        nDstPos = vaddq_s32(nStartPos, nScaledPos);
        MEMCPY(((INT32 *)((&(pLocalScaledHaarObjModelFeature->nIntgralFeature)))), (INT32 *)(&nDstPos), 4 * sizeof(INT32));

        nStartPos = vdupq_n_s32(((INT32)(pSqIntegralSrc)));
        nDstPos = vaddq_s32(nStartPos, nScaledPos);
        MEMCPY(((INT32 *)((&(pLocalScaledHaarObjModelFeature->nSqIntgralFeature)))), (INT32 *)(&nDstPos), 4 * sizeof(INT32));
    }
    
    {
        float32x4_t             nScale_F32 = vdupq_n_f32(0.5);
        UINT32                  *pIntegralSrc = (UINT32 *)(pObjDetect->pIntegralImg->GetImgPlanes(pObjDetect->pIntegralImg, IMG_PLANE_0));

        // Load StartPtr of Image
        nStartPos = vdupq_n_s32(((INT32)(pIntegralSrc)));

        for(i=0 ; i<pHaarObjData->nTotalNumofStages ; i++)
        {
            Feature_T               *pHaarFeature = &(pHaarObjData->pFeatures[nCurrHaarFeature]);
            ScaledFeatureTable_T    *pPreScaledFeatureTable = &(pLocalScaledHaarObjModelFeature->pScaledFeatureTable[nCurrHaarFeature]);
            const UINT32            nNumofClassifiers = pHaarObjData->pNumofClassifiers[i];
            
            for(j=0 ; j<nNumofClassifiers ; j++)
            {
                F32                 nRectSize = 0.0f;
                F32                 nWegithedRectSize = 0.0f;
                INT32               *pRect0 = (INT32 *)(&(pHaarFeature->nFeature[0].nRect));
                INT32               *pRect1 = (INT32 *)(&(pHaarFeature->nFeature[1].nRect));
                int32x4_t           nRectInfo_S32x4_0 = vld1q_s32(pRect0);
                int32x4_t           nRectInfo_S32x4_1 = vld1q_s32(pRect1);
                ScaledFeature_T     *pScaledFeature0 = &(pPreScaledFeatureTable->nScaledFeature[0]);
                ScaledFeature_T     *pScaledFeature1 = &(pPreScaledFeatureTable->nScaledFeature[1]);
                float32x4_t         nRectInfo_F32x4_0 = vcvtq_f32_s32(nRectInfo_S32x4_0);
                float32x4_t         nRectInfo_F32x4_1 = vcvtq_f32_s32(nRectInfo_S32x4_1);
                int32x4_t           nDst0, nDst1;
                KIRect              *pTargetRect0 = NULL, *pTargetRect1 = NULL;
                
                nDst0 = vcvtq_s32_f32(vmlaq_n_f32(nScale_F32, nRectInfo_F32x4_0, nScale));
                nDst1 = vcvtq_s32_f32(vmlaq_n_f32(nScale_F32, nRectInfo_F32x4_1, nScale));

                pTargetRect0 = (KIRect *)(&nDst0);
                pScaledFeature0->nWeight = pHaarFeature->nFeature[0].nWeight * nWeightScale;
                nX = vdupq_n_s32(vgetq_lane_s32(nDst0, 0));
                nY = vdupq_n_s32(vgetq_lane_s32(nDst0, 1));
                nW = vdupq_n_s32(vgetq_lane_s32(nDst0, 2));
                nH = vdupq_n_s32(vgetq_lane_s32(nDst0, 3));
                nXWAdd = vaddq_s32(nX, nW);
                nYHAdd = vaddq_s32(nY, nH);
                nXWComp = vtrnq_s32(nX, nXWAdd);
                nYHComp = vextq_s32(nY, nYHAdd, 2);
                nLocalPos = vmlaq_s32(nXWComp.val[0], nYHComp, nS);
                nScaledPos = vshlq_n_s32(nLocalPos, 2);
                nDstPos = vaddq_s32(nStartPos, nScaledPos);
                MEMCPY((INT32 *)(pScaledFeature0), (INT32 *)(&nDstPos), 4 * sizeof(INT32));
                
                pTargetRect1 = (KIRect *)(&nDst1);
                pScaledFeature1->nWeight = pHaarFeature->nFeature[1].nWeight * nWeightScale;
                nX = vdupq_n_s32(vgetq_lane_s32(nDst1, 0));
                nY = vdupq_n_s32(vgetq_lane_s32(nDst1, 1));
                nW = vdupq_n_s32(vgetq_lane_s32(nDst1, 2));
                nH = vdupq_n_s32(vgetq_lane_s32(nDst1, 3));
                nXWAdd = vaddq_s32(nX, nW);
                nYHAdd = vaddq_s32(nY, nH);
                nXWComp = vtrnq_s32(nX, nXWAdd);
                nYHComp = vextq_s32(nY, nYHAdd, 2);
                nLocalPos = vmlaq_s32(nXWComp.val[0], nYHComp, nS);
                nScaledPos = vshlq_n_s32(nLocalPos, 2);
                nDstPos = vaddq_s32(nStartPos, nScaledPos);
                MEMCPY((INT32 *)(pScaledFeature1), (INT32 *)(&nDstPos), 4 * sizeof(INT32));

                nRectSize = pTargetRect0->nWidth * pTargetRect0->nHeight;
                nWegithedRectSize += pScaledFeature1->nWeight * pTargetRect1->nWidth * pTargetRect1->nHeight;
                
                if(3 == pHaarFeature->nNumofRect)
                {
                    INT32                   *pRect2 = (INT32 *)(&(pHaarFeature->nFeature[3].nRect));
                    KIRect                  *pTargetRect2 = NULL;
                    ScaledFeature_T         *pScaledFeature2 = &(pPreScaledFeatureTable->nScaledFeature[3]);
                    int32x4_t               nRectInfo_S32x4 = vld1q_s32(((INT32 *)pRect2));
                    float32x4_t             nRectInfo_F32x4 = vcvtq_f32_s32(nRectInfo_S32x4);
                    int32x4_t               nDst2;
                    
                    nDst2 = vcvtq_s32_f32(vmlaq_n_f32(nScale_F32, nRectInfo_F32x4, nScale));
                    pTargetRect2 = (KIRect *)(&nDst2);
                    pScaledFeature2->nWeight = pHaarFeature->nFeature[3].nWeight * nWeightScale;
                    nX = vdupq_n_s32(vgetq_lane_s32(nDst2, 0));
                    nY = vdupq_n_s32(vgetq_lane_s32(nDst2, 1));
                    nW = vdupq_n_s32(vgetq_lane_s32(nDst2, 2));
                    nH = vdupq_n_s32(vgetq_lane_s32(nDst2, 3));
                    nXWAdd = vaddq_s32(nX, nW);
                    nYHAdd = vaddq_s32(nY, nH);
                    nXWComp = vtrnq_s32(nX, nXWAdd);
                    nYHComp = vextq_s32(nY, nYHAdd, 2);
                    nLocalPos = vmlaq_s32(nXWComp.val[0], nYHComp, nS);
                    nScaledPos = vshlq_n_s32(nLocalPos, 2);
                    nDstPos = vaddq_s32(nStartPos, nScaledPos);
                    MEMCPY((INT32 *)(pScaledFeature2), (INT32 *)(&nDstPos), 4 * sizeof(INT32));
                    
                    nWegithedRectSize += pScaledFeature2->nWeight * pTargetRect2->nWidth * pTargetRect2->nHeight;
                }
                
                pPreScaledFeatureTable->nScaledFeature[0].nWeight = -(nWegithedRectSize / nRectSize);
                
                pHaarFeature++;
                pPreScaledFeatureTable++;
                nCurrHaarFeature++;
            }
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}
#else
static Kakao_Status _ObjDetector_Set_Classifier_ForScale_NEON(IN const ObjDetect_T *pObjDetect, IN HaarObjModel_T *pHaarObjModel, IN const F32 nScale, IN const INT32 nScaleIdx, IN const ObjModel nObjModel, IN const INT32 nDegree)
{
    Kakao_Status                        nRet = KAKAO_STAT_FAIL;
    Img_T                               *pIntegralImg = pObjDetect->pIntegralImg;
    HaarObjData_T                       *pHaarObjData = pHaarObjModel->pDegHaarObjData;
    INT32                               nStride = pIntegralImg->GetImgStride(pIntegralImg, IMG_PLANE_0);
    UINT32                              nCurrHaarFeature = 0;
    INT32                               i = 0, j = 0, k = 0;
    F32                                 nWeightScale = 0.0f;
    INT32                               nOrgCenterX = 0;
    INT32                               nOrgCenterY = 0;
    HaarObjModel_ScaledFeature          *pLocalScaledHaarObjModelFeature = &(pHaarObjModel->pRefScaledObjModelFeature[nScaleIdx]);
    
    if(NULL == pLocalScaledHaarObjModelFeature->pScaledFeatureTable)
        SAFEALLOC(pLocalScaledHaarObjModelFeature->pScaledFeatureTable, pHaarObjData->nTotalNumofClassifier, 32, ScaledFeatureTable_T);
    
    {
        KIRect              nTmpRect = {0, };
        UINT32              *pIntegralSrc = (UINT32 *)(pObjDetect->pIntegralImg->GetImgPlanes(pObjDetect->pIntegralImg, IMG_PLANE_0));
        UINT32              *pSqIntegralSrc = (UINT32 *)(pObjDetect->pSqIntegralImg->GetImgPlanes(pObjDetect->pSqIntegralImg, IMG_PLANE_0));
        
        nTmpRect.x = nTmpRect.y = ROUND(nScale);
        nTmpRect.nWidth = ROUND((pHaarObjData->nBaseWidth - 2) * nScale);
        nTmpRect.nHeight = ROUND((pHaarObjData->nBaseHeight - 2) * nScale);
        nOrgCenterX = ROUND(pHaarObjData->nBaseWidth * nScale) / 2 + 1;
        nOrgCenterY = ROUND(pHaarObjData->nBaseHeight * nScale) / 2 + 1;
        nWeightScale = 1.0f / (nTmpRect.nWidth * nTmpRect.nHeight);
        pLocalScaledHaarObjModelFeature->nInvRectSize = nWeightScale;
        
        pLocalScaledHaarObjModelFeature->nIntgralFeature.pTL = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTmpRect.x, nTmpRect.y, nStride);
        pLocalScaledHaarObjModelFeature->nIntgralFeature.pTR = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTmpRect.x + nTmpRect.nWidth, nTmpRect.y, nStride);
        pLocalScaledHaarObjModelFeature->nIntgralFeature.pBL = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTmpRect.x, nTmpRect.y+ nTmpRect.nHeight, nStride);
        pLocalScaledHaarObjModelFeature->nIntgralFeature.pBR = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTmpRect.x + nTmpRect.nWidth, nTmpRect.y + nTmpRect.nHeight, nStride);
        
        pLocalScaledHaarObjModelFeature->nSqIntgralFeature.pTL = GET_INTEGRAL_PIX_POS(pSqIntegralSrc, nTmpRect.x, nTmpRect.y, nStride);
        pLocalScaledHaarObjModelFeature->nSqIntgralFeature.pTR = GET_INTEGRAL_PIX_POS(pSqIntegralSrc, nTmpRect.x + nTmpRect.nWidth, nTmpRect.y, nStride);
        pLocalScaledHaarObjModelFeature->nSqIntgralFeature.pBL = GET_INTEGRAL_PIX_POS(pSqIntegralSrc, nTmpRect.x, nTmpRect.y+ nTmpRect.nHeight, nStride);
        pLocalScaledHaarObjModelFeature->nSqIntgralFeature.pBR = GET_INTEGRAL_PIX_POS(pSqIntegralSrc, nTmpRect.x + nTmpRect.nWidth, nTmpRect.y + nTmpRect.nHeight, nStride);
    }
    
    for(i=0 ; i<pHaarObjData->nTotalNumofStages ; i++)
    {
        Feature_T               *pHaarFeature = &(pHaarObjData->pFeatures[nCurrHaarFeature]);
        ScaledFeatureTable_T    *pPreScaledFeatureTable = &(pLocalScaledHaarObjModelFeature->pScaledFeatureTable[nCurrHaarFeature]);
        UINT32                  *pIntegralSrc = (UINT32 *)(pObjDetect->pIntegralImg->GetImgPlanes(pObjDetect->pIntegralImg, IMG_PLANE_0));
        
        for(j=0 ; j<pHaarObjData->pNumofClassifiers[i] ; j++)
        {
            F32             nRectSize = 0.0f;
            F32             nWegithedRectSize = 0.0f;
            
            for(k=0 ; k<pHaarFeature->nNumofRect ; k++)
            {
                KIRect                  *pRect = &(pHaarFeature->nFeature[k].nRect);
                KIRect                  nTargetRect = {0, };
                ScaledFeature_T         *pScaledFeature = &(pPreScaledFeatureTable->nScaledFeature[k]);
                
                nTargetRect.x = ROUND(pRect->x * nScale);
                nTargetRect.y = ROUND(pRect->y * nScale);
                nTargetRect.nWidth = ROUND(pRect->nWidth * nScale);
                nTargetRect.nHeight = ROUND(pRect->nHeight * nScale);
                
                pScaledFeature->pTL = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTargetRect.x, nTargetRect.y, nStride);
                pScaledFeature->pTR = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTargetRect.x + nTargetRect.nWidth, nTargetRect.y, nStride);
                pScaledFeature->pBL = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTargetRect.x, nTargetRect.y + nTargetRect.nHeight, nStride);
                pScaledFeature->pBR = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTargetRect.x + nTargetRect.nWidth, nTargetRect.y + nTargetRect.nHeight, nStride);
                pScaledFeature->nWeight = pHaarFeature->nFeature[k].nWeight * nWeightScale;
                
                if(0 == k)
                    nRectSize = nTargetRect.nWidth * nTargetRect.nHeight;
                else
                    nWegithedRectSize += pScaledFeature->nWeight * nTargetRect.nWidth * nTargetRect.nHeight;
            }
            
            pPreScaledFeatureTable->nScaledFeature[0].nWeight = -(nWegithedRectSize / nRectSize);
            
            pHaarFeature++;
            pPreScaledFeatureTable++;
            nCurrHaarFeature++;
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}
#endif
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_SSE4) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_SSE4)
static Kakao_Status _ObjDetector_Set_Classifier_ForScale_SSE4(IN const ObjDetect_T *pObjDetect, IN HaarObjModel_T *pHaarObjModel, IN const F32 nScale, IN const INT32 nScaleIdx, IN const ObjModel nObjModel, IN const INT32 nDegree)
{
    Kakao_Status                        nRet = KAKAO_STAT_FAIL;
    Img_T                               *pIntegralImg = pObjDetect->pIntegralImg;
    HaarObjData_T                       *pHaarObjData = pHaarObjModel->pDegHaarObjData;
    INT32                               nStride = pIntegralImg->GetImgStride(pIntegralImg, IMG_PLANE_0);
    UINT32                              nCurrHaarFeature = 0;
    INT32                               i = 0, j = 0, k = 0;
    F32                                 nWeightScale = 0.0f;
    INT32                               nOrgCenterX = 0;
    INT32                               nOrgCenterY = 0;
    HaarObjModel_ScaledFeature          *pLocalScaledHaarObjModelFeature = &(pHaarObjModel->pRefScaledObjModelFeature[nScaleIdx]);
    
    if(NULL == pLocalScaledHaarObjModelFeature->pScaledFeatureTable)
        SAFEALLOC(pLocalScaledHaarObjModelFeature->pScaledFeatureTable, pHaarObjData->nTotalNumofClassifier, 32, ScaledFeatureTable_T);
    
    {
        KIRect              nTmpRect = {0, };
        UINT32              *pIntegralSrc = (UINT32 *)(pObjDetect->pIntegralImg->GetImgPlanes(pObjDetect->pIntegralImg, IMG_PLANE_0));
        UINT32              *pSqIntegralSrc = (UINT32 *)(pObjDetect->pSqIntegralImg->GetImgPlanes(pObjDetect->pSqIntegralImg, IMG_PLANE_0));
        
        nTmpRect.x = nTmpRect.y = ROUND(nScale);
        nTmpRect.nWidth = ROUND((pHaarObjData->nBaseWidth - 2) * nScale);
        nTmpRect.nHeight = ROUND((pHaarObjData->nBaseHeight - 2) * nScale);
        nOrgCenterX = ROUND(pHaarObjData->nBaseWidth * nScale) / 2 + 1;
        nOrgCenterY = ROUND(pHaarObjData->nBaseHeight * nScale) / 2 + 1;
        nWeightScale = 1.0f / (nTmpRect.nWidth * nTmpRect.nHeight);
        pLocalScaledHaarObjModelFeature->nInvRectSize = nWeightScale;
        
        pLocalScaledHaarObjModelFeature->nIntgralFeature.pTL = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTmpRect.x, nTmpRect.y, nStride);
        pLocalScaledHaarObjModelFeature->nIntgralFeature.pTR = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTmpRect.x + nTmpRect.nWidth, nTmpRect.y, nStride);
        pLocalScaledHaarObjModelFeature->nIntgralFeature.pBL = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTmpRect.x, nTmpRect.y+ nTmpRect.nHeight, nStride);
        pLocalScaledHaarObjModelFeature->nIntgralFeature.pBR = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTmpRect.x + nTmpRect.nWidth, nTmpRect.y + nTmpRect.nHeight, nStride);
        
        pLocalScaledHaarObjModelFeature->nSqIntgralFeature.pTL = GET_INTEGRAL_PIX_POS(pSqIntegralSrc, nTmpRect.x, nTmpRect.y, nStride);
        pLocalScaledHaarObjModelFeature->nSqIntgralFeature.pTR = GET_INTEGRAL_PIX_POS(pSqIntegralSrc, nTmpRect.x + nTmpRect.nWidth, nTmpRect.y, nStride);
        pLocalScaledHaarObjModelFeature->nSqIntgralFeature.pBL = GET_INTEGRAL_PIX_POS(pSqIntegralSrc, nTmpRect.x, nTmpRect.y+ nTmpRect.nHeight, nStride);
        pLocalScaledHaarObjModelFeature->nSqIntgralFeature.pBR = GET_INTEGRAL_PIX_POS(pSqIntegralSrc, nTmpRect.x + nTmpRect.nWidth, nTmpRect.y + nTmpRect.nHeight, nStride);
    }
    
    for(i=0 ; i<pHaarObjData->nTotalNumofStages ; i++)
    {
        Feature_T               *pHaarFeature = &(pHaarObjData->pFeatures[nCurrHaarFeature]);
        ScaledFeatureTable_T    *pPreScaledFeatureTable = &(pLocalScaledHaarObjModelFeature->pScaledFeatureTable[nCurrHaarFeature]);
        UINT32                  *pIntegralSrc = (UINT32 *)(pObjDetect->pIntegralImg->GetImgPlanes(pObjDetect->pIntegralImg, IMG_PLANE_0));
        
        for(j=0 ; j<pHaarObjData->pNumofClassifiers[i] ; j++)
        {
            F32             nRectSize = 0.0f;
            F32             nWegithedRectSize = 0.0f;
            
            for(k=0 ; k<pHaarFeature->nNumofRect ; k++)
            {
                KIRect                  *pRect = &(pHaarFeature->nFeature[k].nRect);
                KIRect                  nTargetRect = {0, };
                ScaledFeature_T         *pScaledFeature = &(pPreScaledFeatureTable->nScaledFeature[k]);
                
                nTargetRect.x = ROUND(pRect->x * nScale);
                nTargetRect.y = ROUND(pRect->y * nScale);
                nTargetRect.nWidth = ROUND(pRect->nWidth * nScale);
                nTargetRect.nHeight = ROUND(pRect->nHeight * nScale);
                
                pScaledFeature->pTL = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTargetRect.x, nTargetRect.y, nStride);
                pScaledFeature->pTR = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTargetRect.x + nTargetRect.nWidth, nTargetRect.y, nStride);
                pScaledFeature->pBL = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTargetRect.x, nTargetRect.y + nTargetRect.nHeight, nStride);
                pScaledFeature->pBR = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTargetRect.x + nTargetRect.nWidth, nTargetRect.y + nTargetRect.nHeight, nStride);
                pScaledFeature->nWeight = pHaarFeature->nFeature[k].nWeight * nWeightScale;
                
                if(0 == k)
                    nRectSize = nTargetRect.nWidth * nTargetRect.nHeight;
                else
                    nWegithedRectSize += pScaledFeature->nWeight * nTargetRect.nWidth * nTargetRect.nHeight;
            }
            
            pPreScaledFeatureTable->nScaledFeature[0].nWeight = -(nWegithedRectSize / nRectSize);
            
            pHaarFeature++;
            pPreScaledFeatureTable++;
            nCurrHaarFeature++;
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_SSE3) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_SSE3)
static Kakao_Status _ObjDetector_Set_Classifier_ForScale_SSE3(IN const ObjDetect_T *pObjDetect, IN HaarObjModel_T *pHaarObjModel, IN const F32 nScale, IN const INT32 nScaleIdx, IN const ObjModel nObjModel, IN const INT32 nDegree)
{
    Kakao_Status                        nRet = KAKAO_STAT_FAIL;
    Img_T                               *pIntegralImg = pObjDetect->pIntegralImg;
    HaarObjData_T                       *pHaarObjData = pHaarObjModel->pDegHaarObjData;
    INT32                               nStride = pIntegralImg->GetImgStride(pIntegralImg, IMG_PLANE_0);
    UINT32                              nCurrHaarFeature = 0;
    INT32                               i = 0, j = 0, k = 0;
    F32                                 nWeightScale = 0.0f;
    INT32                               nOrgCenterX = 0;
    INT32                               nOrgCenterY = 0;
    HaarObjModel_ScaledFeature          *pLocalScaledHaarObjModelFeature = &(pHaarObjModel->pRefScaledObjModelFeature[nScaleIdx]);
    
    if(NULL == pLocalScaledHaarObjModelFeature->pScaledFeatureTable)
        SAFEALLOC(pLocalScaledHaarObjModelFeature->pScaledFeatureTable, pHaarObjData->nTotalNumofClassifier, 32, ScaledFeatureTable_T);
    
    {
        KIRect              nTmpRect = {0, };
        UINT32              *pIntegralSrc = (UINT32 *)(pObjDetect->pIntegralImg->GetImgPlanes(pObjDetect->pIntegralImg, IMG_PLANE_0));
        UINT32              *pSqIntegralSrc = (UINT32 *)(pObjDetect->pSqIntegralImg->GetImgPlanes(pObjDetect->pSqIntegralImg, IMG_PLANE_0));
        
        nTmpRect.x = nTmpRect.y = ROUND(nScale);
        nTmpRect.nWidth = ROUND((pHaarObjData->nBaseWidth - 2) * nScale);
        nTmpRect.nHeight = ROUND((pHaarObjData->nBaseHeight - 2) * nScale);
        nOrgCenterX = ROUND(pHaarObjData->nBaseWidth * nScale) / 2 + 1;
        nOrgCenterY = ROUND(pHaarObjData->nBaseHeight * nScale) / 2 + 1;
        nWeightScale = 1.0f / (nTmpRect.nWidth * nTmpRect.nHeight);
        pLocalScaledHaarObjModelFeature->nInvRectSize = nWeightScale;
        
        pLocalScaledHaarObjModelFeature->nIntgralFeature.pTL = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTmpRect.x, nTmpRect.y, nStride);
        pLocalScaledHaarObjModelFeature->nIntgralFeature.pTR = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTmpRect.x + nTmpRect.nWidth, nTmpRect.y, nStride);
        pLocalScaledHaarObjModelFeature->nIntgralFeature.pBL = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTmpRect.x, nTmpRect.y+ nTmpRect.nHeight, nStride);
        pLocalScaledHaarObjModelFeature->nIntgralFeature.pBR = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTmpRect.x + nTmpRect.nWidth, nTmpRect.y + nTmpRect.nHeight, nStride);
        
        pLocalScaledHaarObjModelFeature->nSqIntgralFeature.pTL = GET_INTEGRAL_PIX_POS(pSqIntegralSrc, nTmpRect.x, nTmpRect.y, nStride);
        pLocalScaledHaarObjModelFeature->nSqIntgralFeature.pTR = GET_INTEGRAL_PIX_POS(pSqIntegralSrc, nTmpRect.x + nTmpRect.nWidth, nTmpRect.y, nStride);
        pLocalScaledHaarObjModelFeature->nSqIntgralFeature.pBL = GET_INTEGRAL_PIX_POS(pSqIntegralSrc, nTmpRect.x, nTmpRect.y+ nTmpRect.nHeight, nStride);
        pLocalScaledHaarObjModelFeature->nSqIntgralFeature.pBR = GET_INTEGRAL_PIX_POS(pSqIntegralSrc, nTmpRect.x + nTmpRect.nWidth, nTmpRect.y + nTmpRect.nHeight, nStride);
    }
    
    for(i=0 ; i<pHaarObjData->nTotalNumofStages ; i++)
    {
        Feature_T               *pHaarFeature = &(pHaarObjData->pFeatures[nCurrHaarFeature]);
        ScaledFeatureTable_T    *pPreScaledFeatureTable = &(pLocalScaledHaarObjModelFeature->pScaledFeatureTable[nCurrHaarFeature]);
        UINT32                  *pIntegralSrc = (UINT32 *)(pObjDetect->pIntegralImg->GetImgPlanes(pObjDetect->pIntegralImg, IMG_PLANE_0));
        
        for(j=0 ; j<pHaarObjData->pNumofClassifiers[i] ; j++)
        {
            F32             nRectSize = 0.0f;
            F32             nWegithedRectSize = 0.0f;
            
            for(k=0 ; k<pHaarFeature->nNumofRect ; k++)
            {
                KIRect                  *pRect = &(pHaarFeature->nFeature[k].nRect);
                KIRect                  nTargetRect = {0, };
                ScaledFeature_T         *pScaledFeature = &(pPreScaledFeatureTable->nScaledFeature[k]);
                
                nTargetRect.x = ROUND(pRect->x * nScale);
                nTargetRect.y = ROUND(pRect->y * nScale);
                nTargetRect.nWidth = ROUND(pRect->nWidth * nScale);
                nTargetRect.nHeight = ROUND(pRect->nHeight * nScale);
                
                pScaledFeature->pTL = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTargetRect.x, nTargetRect.y, nStride);
                pScaledFeature->pTR = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTargetRect.x + nTargetRect.nWidth, nTargetRect.y, nStride);
                pScaledFeature->pBL = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTargetRect.x, nTargetRect.y + nTargetRect.nHeight, nStride);
                pScaledFeature->pBR = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTargetRect.x + nTargetRect.nWidth, nTargetRect.y + nTargetRect.nHeight, nStride);
                pScaledFeature->nWeight = pHaarFeature->nFeature[k].nWeight * nWeightScale;
                
                if(0 == k)
                    nRectSize = nTargetRect.nWidth * nTargetRect.nHeight;
                else
                    nWegithedRectSize += pScaledFeature->nWeight * nTargetRect.nWidth * nTargetRect.nHeight;
            }
            
            pPreScaledFeatureTable->nScaledFeature[0].nWeight = -(nWegithedRectSize / nRectSize);
            
            pHaarFeature++;
            pPreScaledFeatureTable++;
            nCurrHaarFeature++;
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_SSE2) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_SSE2)
static Kakao_Status _ObjDetector_Set_Classifier_ForScale_SSE2(IN const ObjDetect_T *pObjDetect, IN HaarObjModel_T *pHaarObjModel, IN const F32 nScale, IN const INT32 nScaleIdx, IN const ObjModel nObjModel, IN const INT32 nDegree)
{
    Kakao_Status                        nRet = KAKAO_STAT_FAIL;
    Img_T                               *pIntegralImg = pObjDetect->pIntegralImg;
    HaarObjData_T                       *pHaarObjData = pHaarObjModel->pDegHaarObjData;
    INT32                               nStride = pIntegralImg->GetImgStride(pIntegralImg, IMG_PLANE_0);
    UINT32                              nCurrHaarFeature = 0;
    INT32                               i = 0, j = 0, k = 0;
    F32                                 nWeightScale = 0.0f;
    INT32                               nOrgCenterX = 0;
    INT32                               nOrgCenterY = 0;
    HaarObjModel_ScaledFeature          *pLocalScaledHaarObjModelFeature = &(pHaarObjModel->pRefScaledObjModelFeature[nScaleIdx]);
    
    if(NULL == pLocalScaledHaarObjModelFeature->pScaledFeatureTable)
        SAFEALLOC(pLocalScaledHaarObjModelFeature->pScaledFeatureTable, pHaarObjData->nTotalNumofClassifier, 32, ScaledFeatureTable_T);
    
    {
        KIRect              nTmpRect = {0, };
        UINT32              *pIntegralSrc = (UINT32 *)(pObjDetect->pIntegralImg->GetImgPlanes(pObjDetect->pIntegralImg, IMG_PLANE_0));
        UINT32              *pSqIntegralSrc = (UINT32 *)(pObjDetect->pSqIntegralImg->GetImgPlanes(pObjDetect->pSqIntegralImg, IMG_PLANE_0));
        
        nTmpRect.x = nTmpRect.y = ROUND(nScale);
        nTmpRect.nWidth = ROUND((pHaarObjData->nBaseWidth - 2) * nScale);
        nTmpRect.nHeight = ROUND((pHaarObjData->nBaseHeight - 2) * nScale);
        nOrgCenterX = ROUND(pHaarObjData->nBaseWidth * nScale) / 2 + 1;
        nOrgCenterY = ROUND(pHaarObjData->nBaseHeight * nScale) / 2 + 1;
        nWeightScale = 1.0f / (nTmpRect.nWidth * nTmpRect.nHeight);
        pLocalScaledHaarObjModelFeature->nInvRectSize = nWeightScale;
        
        pLocalScaledHaarObjModelFeature->nIntgralFeature.pTL = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTmpRect.x, nTmpRect.y, nStride);
        pLocalScaledHaarObjModelFeature->nIntgralFeature.pTR = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTmpRect.x + nTmpRect.nWidth, nTmpRect.y, nStride);
        pLocalScaledHaarObjModelFeature->nIntgralFeature.pBL = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTmpRect.x, nTmpRect.y+ nTmpRect.nHeight, nStride);
        pLocalScaledHaarObjModelFeature->nIntgralFeature.pBR = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTmpRect.x + nTmpRect.nWidth, nTmpRect.y + nTmpRect.nHeight, nStride);
        
        pLocalScaledHaarObjModelFeature->nSqIntgralFeature.pTL = GET_INTEGRAL_PIX_POS(pSqIntegralSrc, nTmpRect.x, nTmpRect.y, nStride);
        pLocalScaledHaarObjModelFeature->nSqIntgralFeature.pTR = GET_INTEGRAL_PIX_POS(pSqIntegralSrc, nTmpRect.x + nTmpRect.nWidth, nTmpRect.y, nStride);
        pLocalScaledHaarObjModelFeature->nSqIntgralFeature.pBL = GET_INTEGRAL_PIX_POS(pSqIntegralSrc, nTmpRect.x, nTmpRect.y+ nTmpRect.nHeight, nStride);
        pLocalScaledHaarObjModelFeature->nSqIntgralFeature.pBR = GET_INTEGRAL_PIX_POS(pSqIntegralSrc, nTmpRect.x + nTmpRect.nWidth, nTmpRect.y + nTmpRect.nHeight, nStride);
    }
    
    for(i=0 ; i<pHaarObjData->nTotalNumofStages ; i++)
    {
        Feature_T               *pHaarFeature = &(pHaarObjData->pFeatures[nCurrHaarFeature]);
        ScaledFeatureTable_T    *pPreScaledFeatureTable = &(pLocalScaledHaarObjModelFeature->pScaledFeatureTable[nCurrHaarFeature]);
        UINT32                  *pIntegralSrc = (UINT32 *)(pObjDetect->pIntegralImg->GetImgPlanes(pObjDetect->pIntegralImg, IMG_PLANE_0));
        
        for(j=0 ; j<pHaarObjData->pNumofClassifiers[i] ; j++)
        {
            F32             nRectSize = 0.0f;
            F32             nWegithedRectSize = 0.0f;
            
            for(k=0 ; k<pHaarFeature->nNumofRect ; k++)
            {
                KIRect                  *pRect = &(pHaarFeature->nFeature[k].nRect);
                KIRect                  nTargetRect = {0, };
                ScaledFeature_T         *pScaledFeature = &(pPreScaledFeatureTable->nScaledFeature[k]);
                
                nTargetRect.x = ROUND(pRect->x * nScale);
                nTargetRect.y = ROUND(pRect->y * nScale);
                nTargetRect.nWidth = ROUND(pRect->nWidth * nScale);
                nTargetRect.nHeight = ROUND(pRect->nHeight * nScale);
                
                pScaledFeature->pTL = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTargetRect.x, nTargetRect.y, nStride);
                pScaledFeature->pTR = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTargetRect.x + nTargetRect.nWidth, nTargetRect.y, nStride);
                pScaledFeature->pBL = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTargetRect.x, nTargetRect.y + nTargetRect.nHeight, nStride);
                pScaledFeature->pBR = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTargetRect.x + nTargetRect.nWidth, nTargetRect.y + nTargetRect.nHeight, nStride);
                pScaledFeature->nWeight = pHaarFeature->nFeature[k].nWeight * nWeightScale;
                
                if(0 == k)
                    nRectSize = nTargetRect.nWidth * nTargetRect.nHeight;
                else
                    nWegithedRectSize += pScaledFeature->nWeight * nTargetRect.nWidth * nTargetRect.nHeight;
            }
            
            pPreScaledFeatureTable->nScaledFeature[0].nWeight = -(nWegithedRectSize / nRectSize);
            
            pHaarFeature++;
            pPreScaledFeatureTable++;
            nCurrHaarFeature++;
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_SSE) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_SSE)
static Kakao_Status _ObjDetector_Set_Classifier_ForScale_SSE(IN const ObjDetect_T *pObjDetect, IN HaarObjModel_T *pHaarObjModel, IN const F32 nScale, IN const INT32 nScaleIdx, IN const ObjModel nObjModel, IN const INT32 nDegree)
{
    Kakao_Status                        nRet = KAKAO_STAT_FAIL;
    Img_T                               *pIntegralImg = pObjDetect->pIntegralImg;
    HaarObjData_T                       *pHaarObjData = pHaarObjModel->pDegHaarObjData;
    INT32                               nStride = pIntegralImg->GetImgStride(pIntegralImg, IMG_PLANE_0);
    UINT32                              nCurrHaarFeature = 0;
    INT32                               i = 0, j = 0, k = 0;
    F32                                 nWeightScale = 0.0f;
    INT32                               nOrgCenterX = 0;
    INT32                               nOrgCenterY = 0;
    HaarObjModel_ScaledFeature          *pLocalScaledHaarObjModelFeature = &(pHaarObjModel->pRefScaledObjModelFeature[nScaleIdx]);
    
    if(NULL == pLocalScaledHaarObjModelFeature->pScaledFeatureTable)
        SAFEALLOC(pLocalScaledHaarObjModelFeature->pScaledFeatureTable, pHaarObjData->nTotalNumofClassifier, 32, ScaledFeatureTable_T);
    
    {
        KIRect              nTmpRect = {0, };
        UINT32              *pIntegralSrc = (UINT32 *)(pObjDetect->pIntegralImg->GetImgPlanes(pObjDetect->pIntegralImg, IMG_PLANE_0));
        UINT32              *pSqIntegralSrc = (UINT32 *)(pObjDetect->pSqIntegralImg->GetImgPlanes(pObjDetect->pSqIntegralImg, IMG_PLANE_0));
        
        nTmpRect.x = nTmpRect.y = ROUND(nScale);
        nTmpRect.nWidth = ROUND((pHaarObjData->nBaseWidth - 2) * nScale);
        nTmpRect.nHeight = ROUND((pHaarObjData->nBaseHeight - 2) * nScale);
        nOrgCenterX = ROUND(pHaarObjData->nBaseWidth * nScale) / 2 + 1;
        nOrgCenterY = ROUND(pHaarObjData->nBaseHeight * nScale) / 2 + 1;
        nWeightScale = 1.0f / (nTmpRect.nWidth * nTmpRect.nHeight);
        pLocalScaledHaarObjModelFeature->nInvRectSize = nWeightScale;
        
        pLocalScaledHaarObjModelFeature->nIntgralFeature.pTL = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTmpRect.x, nTmpRect.y, nStride);
        pLocalScaledHaarObjModelFeature->nIntgralFeature.pTR = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTmpRect.x + nTmpRect.nWidth, nTmpRect.y, nStride);
        pLocalScaledHaarObjModelFeature->nIntgralFeature.pBL = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTmpRect.x, nTmpRect.y+ nTmpRect.nHeight, nStride);
        pLocalScaledHaarObjModelFeature->nIntgralFeature.pBR = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTmpRect.x + nTmpRect.nWidth, nTmpRect.y + nTmpRect.nHeight, nStride);
        
        pLocalScaledHaarObjModelFeature->nSqIntgralFeature.pTL = GET_INTEGRAL_PIX_POS(pSqIntegralSrc, nTmpRect.x, nTmpRect.y, nStride);
        pLocalScaledHaarObjModelFeature->nSqIntgralFeature.pTR = GET_INTEGRAL_PIX_POS(pSqIntegralSrc, nTmpRect.x + nTmpRect.nWidth, nTmpRect.y, nStride);
        pLocalScaledHaarObjModelFeature->nSqIntgralFeature.pBL = GET_INTEGRAL_PIX_POS(pSqIntegralSrc, nTmpRect.x, nTmpRect.y+ nTmpRect.nHeight, nStride);
        pLocalScaledHaarObjModelFeature->nSqIntgralFeature.pBR = GET_INTEGRAL_PIX_POS(pSqIntegralSrc, nTmpRect.x + nTmpRect.nWidth, nTmpRect.y + nTmpRect.nHeight, nStride);
    }
    
    for(i=0 ; i<pHaarObjData->nTotalNumofStages ; i++)
    {
        Feature_T               *pHaarFeature = &(pHaarObjData->pFeatures[nCurrHaarFeature]);
        ScaledFeatureTable_T    *pPreScaledFeatureTable = &(pLocalScaledHaarObjModelFeature->pScaledFeatureTable[nCurrHaarFeature]);
        UINT32                  *pIntegralSrc = (UINT32 *)(pObjDetect->pIntegralImg->GetImgPlanes(pObjDetect->pIntegralImg, IMG_PLANE_0));
        
        for(j=0 ; j<pHaarObjData->pNumofClassifiers[i] ; j++)
        {
            F32             nRectSize = 0.0f;
            F32             nWegithedRectSize = 0.0f;
            
            for(k=0 ; k<pHaarFeature->nNumofRect ; k++)
            {
                KIRect                  *pRect = &(pHaarFeature->nFeature[k].nRect);
                KIRect                  nTargetRect = {0, };
                ScaledFeature_T         *pScaledFeature = &(pPreScaledFeatureTable->nScaledFeature[k]);
                
                nTargetRect.x = ROUND(pRect->x * nScale);
                nTargetRect.y = ROUND(pRect->y * nScale);
                nTargetRect.nWidth = ROUND(pRect->nWidth * nScale);
                nTargetRect.nHeight = ROUND(pRect->nHeight * nScale);
                
                pScaledFeature->pTL = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTargetRect.x, nTargetRect.y, nStride);
                pScaledFeature->pTR = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTargetRect.x + nTargetRect.nWidth, nTargetRect.y, nStride);
                pScaledFeature->pBL = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTargetRect.x, nTargetRect.y + nTargetRect.nHeight, nStride);
                pScaledFeature->pBR = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTargetRect.x + nTargetRect.nWidth, nTargetRect.y + nTargetRect.nHeight, nStride);
                pScaledFeature->nWeight = pHaarFeature->nFeature[k].nWeight * nWeightScale;
                
                if(0 == k)
                    nRectSize = nTargetRect.nWidth * nTargetRect.nHeight;
                else
                    nWegithedRectSize += pScaledFeature->nWeight * nTargetRect.nWidth * nTargetRect.nHeight;
            }
            
            pPreScaledFeatureTable->nScaledFeature[0].nWeight = -(nWegithedRectSize / nRectSize);
            
            pHaarFeature++;
            pPreScaledFeatureTable++;
            nCurrHaarFeature++;
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_NONE) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_NONE)
static Kakao_Status _ObjDetector_Set_Classifier_ForScale_C(IN const ObjDetect_T *pObjDetect, IN HaarObjModel_T *pHaarObjModel, IN const F32 nScale, IN const INT32 nScaleIdx, IN const ObjModel nObjModel, IN const INT32 nDegree)
{
    Kakao_Status                        nRet = KAKAO_STAT_FAIL;
    Img_T                               *pIntegralImg = pObjDetect->pIntegralImg;
    HaarObjData_T                       *pHaarObjData = pHaarObjModel->pDegHaarObjData;
    INT32                               nStride = pIntegralImg->GetImgStride(pIntegralImg, IMG_PLANE_0);
    UINT32                              nCurrHaarFeature = 0;
    UINT32                              i = 0, j = 0, k = 0;
    F32                                 nWeightScale = 0.0f;
    INT32                               nOrgCenterX = 0;
    INT32                               nOrgCenterY = 0;
    HaarObjModel_ScaledFeature          *pLocalScaledHaarObjModelFeature = &(pHaarObjModel->pRefScaledObjModelFeature[nScaleIdx]);
    
    if(NULL == pLocalScaledHaarObjModelFeature->pScaledFeatureTable)
        SAFEALLOC(pLocalScaledHaarObjModelFeature->pScaledFeatureTable, pHaarObjData->nTotalNumofClassifier, 32, ScaledFeatureTable_T);
    
    {
        KIRect              nTmpRect = {0, };
        UINT32              *pIntegralSrc = (UINT32 *)(pObjDetect->pIntegralImg->GetImgPlanes(pObjDetect->pIntegralImg, IMG_PLANE_0));
        UINT32              *pSqIntegralSrc = (UINT32 *)(pObjDetect->pSqIntegralImg->GetImgPlanes(pObjDetect->pSqIntegralImg, IMG_PLANE_0));
        
        nTmpRect.x = nTmpRect.y = ROUND(nScale);
        nTmpRect.nWidth = ROUND((pHaarObjData->nBaseWidth - 2) * nScale);
        nTmpRect.nHeight = ROUND((pHaarObjData->nBaseHeight - 2) * nScale);
        nOrgCenterX = ROUND(pHaarObjData->nBaseWidth * nScale) / 2 + 1;
        nOrgCenterY = ROUND(pHaarObjData->nBaseHeight * nScale) / 2 + 1;
        nWeightScale = 1.0f / (nTmpRect.nWidth * nTmpRect.nHeight);
        pLocalScaledHaarObjModelFeature->nInvRectSize = nWeightScale;
        
        pLocalScaledHaarObjModelFeature->nIntgralFeature.pTL = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTmpRect.x, nTmpRect.y, nStride);
        pLocalScaledHaarObjModelFeature->nIntgralFeature.pTR = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTmpRect.x + nTmpRect.nWidth, nTmpRect.y, nStride);
        pLocalScaledHaarObjModelFeature->nIntgralFeature.pBL = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTmpRect.x, nTmpRect.y+ nTmpRect.nHeight, nStride);
        pLocalScaledHaarObjModelFeature->nIntgralFeature.pBR = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTmpRect.x + nTmpRect.nWidth, nTmpRect.y + nTmpRect.nHeight, nStride);
        
        pLocalScaledHaarObjModelFeature->nSqIntgralFeature.pTL = GET_INTEGRAL_PIX_POS(pSqIntegralSrc, nTmpRect.x, nTmpRect.y, nStride);
        pLocalScaledHaarObjModelFeature->nSqIntgralFeature.pTR = GET_INTEGRAL_PIX_POS(pSqIntegralSrc, nTmpRect.x + nTmpRect.nWidth, nTmpRect.y, nStride);
        pLocalScaledHaarObjModelFeature->nSqIntgralFeature.pBL = GET_INTEGRAL_PIX_POS(pSqIntegralSrc, nTmpRect.x, nTmpRect.y+ nTmpRect.nHeight, nStride);
        pLocalScaledHaarObjModelFeature->nSqIntgralFeature.pBR = GET_INTEGRAL_PIX_POS(pSqIntegralSrc, nTmpRect.x + nTmpRect.nWidth, nTmpRect.y + nTmpRect.nHeight, nStride);
    }
    
    for(i=0 ; i<pHaarObjData->nTotalNumofStages ; i++)
    {
        Feature_T               *pHaarFeature = &(pHaarObjData->pFeatures[nCurrHaarFeature]);
        ScaledFeatureTable_T    *pPreScaledFeatureTable = &(pLocalScaledHaarObjModelFeature->pScaledFeatureTable[nCurrHaarFeature]);
        UINT32                  *pIntegralSrc = (UINT32 *)(pObjDetect->pIntegralImg->GetImgPlanes(pObjDetect->pIntegralImg, IMG_PLANE_0));
        
        for(j=0 ; j<pHaarObjData->pNumofClassifiers[i] ; j++)
        {
            F32             nRectSize = 0.0f;
            F32             nWegithedRectSize = 0.0f;
            
            for(k=0 ; k<pHaarFeature->nNumofRect ; k++)
            {
                KIRect                  *pRect = &(pHaarFeature->nFeature[k].nRect);
                KIRect                  nTargetRect = {0, };
                ScaledFeature_T         *pScaledFeature = &(pPreScaledFeatureTable->nScaledFeature[k]);
                
                nTargetRect.x = ROUND(pRect->x * nScale);
                nTargetRect.y = ROUND(pRect->y * nScale);
                nTargetRect.nWidth = ROUND(pRect->nWidth * nScale);
                nTargetRect.nHeight = ROUND(pRect->nHeight * nScale);
                
                pScaledFeature->pTL = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTargetRect.x, nTargetRect.y, nStride);
                pScaledFeature->pTR = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTargetRect.x + nTargetRect.nWidth, nTargetRect.y, nStride);
                pScaledFeature->pBL = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTargetRect.x, nTargetRect.y + nTargetRect.nHeight, nStride);
                pScaledFeature->pBR = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTargetRect.x + nTargetRect.nWidth, nTargetRect.y + nTargetRect.nHeight, nStride);
                pScaledFeature->nWeight = pHaarFeature->nFeature[k].nWeight * nWeightScale;
                
                if(0 == k)
                    nRectSize = (F32)(nTargetRect.nWidth * nTargetRect.nHeight);
                else
                    nWegithedRectSize += pScaledFeature->nWeight * (F32)(nTargetRect.nWidth * nTargetRect.nHeight);
            }
            
            pPreScaledFeatureTable->nScaledFeature[0].nWeight = -(nWegithedRectSize / nRectSize);
            
            pHaarFeature++;
            pPreScaledFeatureTable++;
            nCurrHaarFeature++;
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}
#else
static Kakao_Status _ObjDetector_Set_Classifier_ForScale_C(IN const ObjDetect_T *pObjDetect, IN HaarObjModel_T *pHaarObjModel, IN const F32 nScale, IN const INT32 nScaleIdx, IN const ObjModel nObjModel, IN const INT32 nDegree)
{
    Kakao_Status                        nRet = KAKAO_STAT_FAIL;
    Img_T                               *pIntegralImg = pObjDetect->pIntegralImg;
    HaarObjData_T                       *pHaarObjData = pHaarObjModel->pDegHaarObjData;
    INT32                               nStride = pIntegralImg->GetImgStride(pIntegralImg, IMG_PLANE_0);
    UINT32                              nCurrHaarFeature = 0;
    INT32                               i = 0, j = 0, k = 0;
    F32                                 nWeightScale = 0.0f;
    INT32                               nOrgCenterX = 0;
    INT32                               nOrgCenterY = 0;
    HaarObjModel_ScaledFeature          *pLocalScaledHaarObjModelFeature = &(pHaarObjModel->pRefScaledObjModelFeature[nScaleIdx]);
    
    if(NULL == pLocalScaledHaarObjModelFeature->pScaledFeatureTable)
        SAFEALLOC(pLocalScaledHaarObjModelFeature->pScaledFeatureTable, pHaarObjData->nTotalNumofClassifier, 32, ScaledFeatureTable_T);
    
    {
        KIRect              nTmpRect = {0, };
        UINT32              *pIntegralSrc = (UINT32 *)(pObjDetect->pIntegralImg->GetImgPlanes(pObjDetect->pIntegralImg, IMG_PLANE_0));
        UINT32              *pSqIntegralSrc = (UINT32 *)(pObjDetect->pSqIntegralImg->GetImgPlanes(pObjDetect->pSqIntegralImg, IMG_PLANE_0));
        
        nTmpRect.x = nTmpRect.y = ROUND(nScale);
        nTmpRect.nWidth = ROUND((pHaarObjData->nBaseWidth - 2) * nScale);
        nTmpRect.nHeight = ROUND((pHaarObjData->nBaseHeight - 2) * nScale);
        nOrgCenterX = ROUND(pHaarObjData->nBaseWidth * nScale) / 2 + 1;
        nOrgCenterY = ROUND(pHaarObjData->nBaseHeight * nScale) / 2 + 1;
        nWeightScale = 1.0f / (nTmpRect.nWidth * nTmpRect.nHeight);
        pLocalScaledHaarObjModelFeature->nInvRectSize = nWeightScale;
        
        pLocalScaledHaarObjModelFeature->nIntgralFeature.pTL = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTmpRect.x, nTmpRect.y, nStride);
        pLocalScaledHaarObjModelFeature->nIntgralFeature.pTR = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTmpRect.x + nTmpRect.nWidth, nTmpRect.y, nStride);
        pLocalScaledHaarObjModelFeature->nIntgralFeature.pBL = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTmpRect.x, nTmpRect.y+ nTmpRect.nHeight, nStride);
        pLocalScaledHaarObjModelFeature->nIntgralFeature.pBR = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTmpRect.x + nTmpRect.nWidth, nTmpRect.y + nTmpRect.nHeight, nStride);
        
        pLocalScaledHaarObjModelFeature->nSqIntgralFeature.pTL = GET_INTEGRAL_PIX_POS(pSqIntegralSrc, nTmpRect.x, nTmpRect.y, nStride);
        pLocalScaledHaarObjModelFeature->nSqIntgralFeature.pTR = GET_INTEGRAL_PIX_POS(pSqIntegralSrc, nTmpRect.x + nTmpRect.nWidth, nTmpRect.y, nStride);
        pLocalScaledHaarObjModelFeature->nSqIntgralFeature.pBL = GET_INTEGRAL_PIX_POS(pSqIntegralSrc, nTmpRect.x, nTmpRect.y+ nTmpRect.nHeight, nStride);
        pLocalScaledHaarObjModelFeature->nSqIntgralFeature.pBR = GET_INTEGRAL_PIX_POS(pSqIntegralSrc, nTmpRect.x + nTmpRect.nWidth, nTmpRect.y + nTmpRect.nHeight, nStride);
    }
    
    for(i=0 ; i<pHaarObjData->nTotalNumofStages ; i++)
    {
        Feature_T               *pHaarFeature = &(pHaarObjData->pFeatures[nCurrHaarFeature]);
        ScaledFeatureTable_T    *pPreScaledFeatureTable = &(pLocalScaledHaarObjModelFeature->pScaledFeatureTable[nCurrHaarFeature]);
        UINT32                  *pIntegralSrc = (UINT32 *)(pObjDetect->pIntegralImg->GetImgPlanes(pObjDetect->pIntegralImg, IMG_PLANE_0));
        
        for(j=0 ; j<pHaarObjData->pNumofClassifiers[i] ; j++)
        {
            F32             nRectSize = 0.0f;
            F32             nWegithedRectSize = 0.0f;
            
            for(k=0 ; k<pHaarFeature->nNumofRect ; k++)
            {
                KIRect                  *pRect = &(pHaarFeature->nFeature[k].nRect);
                KIRect                  nTargetRect = {0, };
                ScaledFeature_T         *pScaledFeature = &(pPreScaledFeatureTable->nScaledFeature[k]);
                
                nTargetRect.x = ROUND(pRect->x * nScale);
                nTargetRect.y = ROUND(pRect->y * nScale);
                nTargetRect.nWidth = ROUND(pRect->nWidth * nScale);
                nTargetRect.nHeight = ROUND(pRect->nHeight * nScale);
                
                pScaledFeature->pTL = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTargetRect.x, nTargetRect.y, nStride);
                pScaledFeature->pTR = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTargetRect.x + nTargetRect.nWidth, nTargetRect.y, nStride);
                pScaledFeature->pBL = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTargetRect.x, nTargetRect.y + nTargetRect.nHeight, nStride);
                pScaledFeature->pBR = GET_INTEGRAL_PIX_POS(pIntegralSrc, nTargetRect.x + nTargetRect.nWidth, nTargetRect.y + nTargetRect.nHeight, nStride);
                pScaledFeature->nWeight = pHaarFeature->nFeature[k].nWeight * nWeightScale;
                
                if(0 == k)
                    nRectSize = nTargetRect.nWidth * nTargetRect.nHeight;
                else
                    nWegithedRectSize += pScaledFeature->nWeight * nTargetRect.nWidth * nTargetRect.nHeight;
            }
            
            pPreScaledFeatureTable->nScaledFeature[0].nWeight = -(nWegithedRectSize / nRectSize);
            
            pHaarFeature++;
            pPreScaledFeatureTable++;
            nCurrHaarFeature++;
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}
#endif


static Kakao_Status _ImgProc_ObjDetector_Update_RotateBasedFeature(IN ObjDetect_T *pObjDetect, IN const ObjModel nObjModel, IN const INT32 nDegree)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    UINT32                  nCurrHaarFeature = 0;
    UINT32                  i = 0, j = 0, k = 0;
    INT32                   nOrgCenterX = 0;
    INT32                   nOrgCenterY = 0;
    ObjectModel_T           *pObjectModel = NULL;
    HaarObjModel_T          *pHaarObjModel = NULL;
    HaarObjData_T           *pHaarObjData = NULL;
    HaarObjData_T           *pSrcHaarObjData = NULL;
    HaarObjData_T           *pDstHaarObjData = NULL;
    
    CHECK_POINTER_VALIDATION(pObjDetect, KAKAO_STAT_NULL_POINTER);
    
    pObjectModel = &(pObjDetect->nObjectModel);
    pHaarObjModel = pObjectModel->pHaarObjModel[nObjModel];
    pHaarObjData = &(pHaarObjModel->nHaarObjData);

    CHECK_POINTER_VALIDATION(pObjectModel, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pHaarObjModel, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pHaarObjData, KAKAO_STAT_NULL_POINTER);

    if(NULL == pHaarObjModel->pDegHaarObjData)
    {
        SAFEALLOC(pHaarObjModel->pDegHaarObjData, 1, 32, HaarObjData_T);
        SAFEALLOC(pHaarObjModel->pDegHaarObjData->pNumofClassifiers, pHaarObjData->nTotalNumofStages, 32, UINT32);
        SAFEALLOC(pHaarObjModel->pDegHaarObjData->pThresofStages, pHaarObjData->nTotalNumofStages, 32, F32);
        SAFEALLOC(pHaarObjModel->pDegHaarObjData->pFeatures, pHaarObjData->nTotalNumofClassifier, 32, Feature_T);
    }
    
    pSrcHaarObjData = pHaarObjData;
    pDstHaarObjData = pHaarObjModel->pDegHaarObjData;
    
    pDstHaarObjData->nBaseWidth = pSrcHaarObjData->nBaseWidth;
    pDstHaarObjData->nBaseHeight = pSrcHaarObjData->nBaseHeight;
    pDstHaarObjData->nTotalNumofStages = pSrcHaarObjData->nTotalNumofStages;
    pDstHaarObjData->nTotalNumofClassifier = pSrcHaarObjData->nTotalNumofClassifier;
    
    MEMCPY(pDstHaarObjData->pNumofClassifiers, pSrcHaarObjData->pNumofClassifiers, pSrcHaarObjData->nTotalNumofStages * sizeof(UINT32));
    MEMCPY(pDstHaarObjData->pThresofStages, pSrcHaarObjData->pThresofStages, pSrcHaarObjData->nTotalNumofStages * sizeof(F32));
    MEMCPY(pDstHaarObjData->pFeatures, pSrcHaarObjData->pFeatures, pSrcHaarObjData->nTotalNumofClassifier * sizeof(Feature_T));
    
    nOrgCenterX = (pSrcHaarObjData->nBaseWidth) / 2;
    nOrgCenterY = (pSrcHaarObjData->nBaseHeight) / 2;
    
    for(i=0 ; i<pSrcHaarObjData->nTotalNumofStages ; i++)
    {
        Feature_T               *pSrcHaarFeature = &(pSrcHaarObjData->pFeatures[nCurrHaarFeature]);
        Feature_T               *pDstHaarFeature = &(pDstHaarObjData->pFeatures[nCurrHaarFeature]);
        
        for(j=0 ; j<pSrcHaarObjData->pNumofClassifiers[i] ; j++)
        {
            for(k=0 ; k<pSrcHaarFeature->nNumofRect ; k++)
            {
                KIRect                  *pSrcRect = &(pSrcHaarFeature->nFeature[k].nRect);
                KIRect                  *pDstRect = &(pDstHaarFeature->nFeature[k].nRect);
                
                _ImgProc_ObjDetector_Rotate_Rect(pSrcRect, pDstRect, nDegree, nOrgCenterX, nOrgCenterY);
            }
            
            pSrcHaarFeature++;
            pDstHaarFeature++;
            nCurrHaarFeature++;
        }
    }
    
    pHaarObjModel->nCurrDegree = nDegree;
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _ImgProc_ObjDetector_Rotate_Rect(KIRect *pSrcRect, KIRect *pDstRect, const INT32 nDegree, const INT32 nCenterX, const INT32 nCenterY)
{
    switch(nDegree)
    {
        case 0:
        case -360:
        {
            MEMCPY(pDstRect, pSrcRect, sizeof(KIRect));
            break;
        }
        case 90:
        case -270:
        {
            // Rotate CountClockwise
            pDstRect->x = nCenterX + nCenterX - (pSrcRect->y + pSrcRect->nHeight);
            pDstRect->y = pSrcRect->x + nCenterY - nCenterY;
            pDstRect->nWidth = pSrcRect->nHeight;
            pDstRect->nHeight = pSrcRect->nWidth;
            break;
        }
        case 180:
        case -180:
        {
            pDstRect->x = nCenterX + nCenterX - (pSrcRect->x + pSrcRect->nWidth);
            pDstRect->y = nCenterY + nCenterY - (pSrcRect->y + pSrcRect->nHeight);
            pDstRect->nWidth = pSrcRect->nWidth;
            pDstRect->nHeight = pSrcRect->nHeight;
            break;
        }
        case -90:
        case 270:
        {
            // Rotate Clockwise
            pDstRect->x = pSrcRect->y - nCenterX + nCenterX;
            pDstRect->y = nCenterY + nCenterY - (pSrcRect->x + pSrcRect->nWidth);
            pDstRect->nWidth = pSrcRect->nHeight;
            pDstRect->nHeight = pSrcRect->nWidth;
            break;
        }
        default:
        {
            MEMCPY(pDstRect, pSrcRect, sizeof(KIRect));
            break;
        }
    }
    
    return KAKAO_STAT_OK;
}


static Kakao_Status _ImgProc_ObjDetector_RefineDetectedObj(UnRefinedDetectedInfo_T *pUnrefinedDetecedInfo, IN const INT32 nGroupTh, IN const D64 nEps)
{
    INT32               i = 0, j = 0;
    INT32               nNumofLabels = pUnrefinedDetecedInfo->nNumofDetectedObj;
    INT32               nClasses = 0;
    KIRect              nLocalRect[MAX_NUM_OF_DETECTED_OBJ] = {0, };
    INT32               nLocalWeights[MAX_NUM_OF_DETECTED_OBJ] = {0, };
    Kakao_Status        nRet = KAKAO_STAT_FAIL;

    if((nGroupTh <= 0) || (0 == pUnrefinedDetecedInfo->nNumofDetectedObj))
    {
        INT32           *pWeight = &(pUnrefinedDetecedInfo->nWeight[0]);
        UINT32          nNumofDetectedObj = pUnrefinedDetecedInfo->nNumofDetectedObj;
        UINT32          k = 0;

        for(k=0 ; k<nNumofDetectedObj ; k++)
            pWeight[k] = 1;

        return nRet;
    }
   
    // Reset Labels
    MEMSET(&(pUnrefinedDetecedInfo->nLabel[0]), 0, MAX_NUM_OF_DETECTED_OBJ * sizeof(INT32));

    // Get Num of Classes
    nClasses = _ImgProc_ObjDetector_RectPartition(pUnrefinedDetecedInfo, nEps);
    
    // Calculating Average x, y Position, Width and Height
    {
        for(i=0 ; i<nNumofLabels ; i++)
        {
            INT32           nLabel = pUnrefinedDetecedInfo->nLabel[i];
            KIRect          *pRefRect = &(pUnrefinedDetecedInfo->nDetectedObj[i]);
            
            nLocalRect[nLabel].x += pRefRect->x;
            nLocalRect[nLabel].y += pRefRect->y;
            nLocalRect[nLabel].nWidth += pRefRect->nWidth;
            nLocalRect[nLabel].nHeight += pRefRect->nHeight;
            nLocalWeights[nLabel]++;
        }
        
        for(i=0 ; i<nClasses ; i++)
        {
            KIRect          nRect = nLocalRect[i];
            F32             nWeight = 1.0f / (F32)(nLocalWeights[i]);
            
            nLocalRect[i].x = (INT32)((F32)(nRect.x) * nWeight + 0.5f);
            nLocalRect[i].y = (INT32)((F32)(nRect.y) * nWeight + 0.5f);
            nLocalRect[i].nWidth = (INT32)((F32)(nRect.nWidth) * nWeight + 0.5f);
            nLocalRect[i].nHeight = (INT32)((F32)(nRect.nHeight) * nWeight + 0.5f);
        }
    }
    
    pUnrefinedDetecedInfo->nNumofDetectedObj = 0;
    MEMSET(&(pUnrefinedDetecedInfo->nDetectedObj[0]), 0, MAX_NUM_OF_DETECTED_OBJ * sizeof(KIRect));
    MEMSET(&(pUnrefinedDetecedInfo->nWeight[0]), 0, MAX_NUM_OF_DETECTED_OBJ * sizeof(INT32));
    
    for(i=0 ; i<nClasses ; i++)
    {
        KIRect          nRect0 = nLocalRect[i];
        INT32           nWeight0 = nLocalWeights[i];
        
        if(nWeight0 <= nGroupTh)
            continue;
        
        // filter out small face rectangles inside large rectangles
        for(j=0 ; j<nClasses ; j++)
        {
            INT32       nWeight1 = nLocalWeights[j];
            KIRect      nRect1 = {0, };
            INT32       nDx = 0;
            INT32       nDy = 0;
            
            if((j == i) || (nWeight1 <= nGroupTh))
                continue;
            
            nRect1 = nLocalRect[j];
            nDx = (INT32)((F32)(nRect1.nWidth) * nEps + 0.5f);
            nDy = (INT32)((F32)(nRect1.nHeight) * nEps + 0.5f);
            
            if((i != j) &&
               (nRect0.x >= (nRect1.x - nDx)) && (nRect0.y >= (nRect1.y - nDy)) &&
               ((nRect0.x + nRect0.nWidth) <= (nRect1.x + nRect1.nWidth + nDx)) &&
               ((nRect0.y + nRect0.nHeight) <= (nRect1.y + nRect1.nHeight + nDy)) &&
                ((nWeight1 > GET_MAX(3, nWeight0)) || (nWeight0 < 3)))
                break;
        }
        
        if(j == nClasses)
        {
            pUnrefinedDetecedInfo->nDetectedObj[pUnrefinedDetecedInfo->nNumofDetectedObj] = nRect0;
            pUnrefinedDetecedInfo->nWeight[pUnrefinedDetecedInfo->nNumofDetectedObj] = nWeight0;
            pUnrefinedDetecedInfo->nNumofDetectedObj++;
        }
    }
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
}


static INT32 _ImgProc_ObjDetector_RectPartition(UnRefinedDetectedInfo_T *pUnrefinedDetecedInfo, IN const D64 nEps)
{
    INT32                   i, j, nNumofSample = pUnrefinedDetecedInfo->nNumofDetectedObj;
    INT32                   nRectNode[MAX_NUM_OF_DETECTED_OBJ][2] = {0, };
    const KIRect            *pRefRect = &(pUnrefinedDetecedInfo->nDetectedObj[0]);
    INT32                   *pLabel = &(pUnrefinedDetecedInfo->nLabel[0]);
    INT32                   nClasses = 0;
    
    #define PARENT          (0)
    #define RANK            (1)
    
    // The first O(N) pass: create N single-vertex trees
    for(i=0 ; i<nNumofSample ; i++)
    {
        nRectNode[i][PARENT] = -1;
        nRectNode[i][RANK] = 0;
    }
    
    // The main O(N^2) pass: merge connected components
    for(i=0 ; i<nNumofSample ; i++)
    {
        INT32               nRoot0 = i;
        
        // find root
        while(nRectNode[nRoot0][PARENT] >= 0)
            nRoot0 = nRectNode[nRoot0][PARENT];
        
        for(j=0 ; j<nNumofSample ; j++)
        {
            INT32               nRoot1 = 0;

            if((i == j) || (!_ImgProc_ObjDetector_Predicate(&(pRefRect[i]), &(pRefRect[j]), nEps)))
                continue;
            
            nRoot1 = j;
            
            while(0 <= nRectNode[nRoot1][PARENT])
                nRoot1 = nRectNode[nRoot1][PARENT];
            
            if(nRoot1 != nRoot0)
            {
                INT32           nRank0 = 0, nRank1 = 0;
                INT32           k = 0, nParent = 0;
                
                // unite both trees
                nRank0 = nRectNode[nRoot0][RANK],
                nRank1 = nRectNode[nRoot1][RANK];
                if(nRank0 > nRank1)
                    nRectNode[nRoot1][PARENT] = nRoot0;
                else
                {
                    nRectNode[nRoot0][PARENT] = nRoot1;
                    nRectNode[nRoot1][RANK] += (nRank0 == nRank1);
                    nRoot0 = nRoot1;
                }
                //assert(nRectNode[nRoot0][PARENT] < 0);
                
                k = j;
                // compress the path from node2 to root
                while(0 <= (nParent = nRectNode[k][PARENT]))
                {
                    nRectNode[k][PARENT] = nRoot0;
                    k = nParent;
                }
                
                // compress the path from node to root
                k = i;
                while(0 <= (nParent = nRectNode[k][PARENT]))
                {
                    nRectNode[k][PARENT] = nRoot0;
                    k = nParent;
                }
            }
        }
    }
    
    // Final O(N) pass: enumerate classes
    for(i=0 ; i<nNumofSample ; i++)
    {
        INT32       nRoot0 = i;
        
        while(0 <= nRectNode[nRoot0][PARENT])
            nRoot0 = nRectNode[nRoot0][PARENT];
        
        // re-use the rank as the class label
        if(nRectNode[nRoot0][RANK] >= 0)
            nRectNode[nRoot0][RANK] = ~nClasses++;
        
        pLabel[i] = ~nRectNode[nRoot0][RANK];
    }
    
    return nClasses;
}


static IMGPROC_BOOL _ImgProc_ObjDetector_Predicate(IN const KIRect *pRect0, IN const KIRect *pRect1, IN const D64 nEps)
{
    D64                     nDelta = 0.0f;
    IMGPROC_BOOL            bPredication = IMGPROC_FALSE;
    
    nDelta = nEps * (GET_MIN(pRect0->nWidth, pRect1->nWidth) + GET_MIN(pRect0->nHeight, pRect1->nHeight)) * 0.5;
    
    bPredication = ABSM(pRect0->x - pRect1->x) <= nDelta &&
                    ABSM(pRect0->y - pRect1->y) <= nDelta &&
                    ABSM((pRect0->x + pRect0->nWidth) - (pRect1->x + pRect1->nWidth)) <= nDelta &&
                    ABSM((pRect0->y + pRect0->nHeight) - (pRect1->y + pRect1->nHeight)) <= nDelta;
    
    return bPredication;
}


