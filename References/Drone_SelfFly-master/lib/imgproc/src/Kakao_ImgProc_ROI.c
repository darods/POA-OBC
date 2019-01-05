


/*----------------------------------------------------------------------------------------
 File Inclusions
 ----------------------------------------------------------------------------------------*/
#include "Kakao_ImgProc_Modules.h"


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


/*----------------------------------------------------------------------------------------
 Static Variable
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Global Variable
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Function Implementation
 ----------------------------------------------------------------------------------------*/
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_ROIDetector_Create(IN OUT ROIDetect_T **ppROIDetect, IN const INT32 nWidth, IN const INT32 nHeight)
{
    ROIDetect_T             *pTmpROIDetect = NULL;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   nAlignedWidth = GET_ALIGNED_LENGTH(nWidth, MACRO_BLOCK_SIZE);
    
    SAFEALLOC(pTmpROIDetect, 1, 32, ROIDetect_T);
    
    Kakao_ImgProc_Util_UpdateSize(&pTmpROIDetect->nBaseSize, nWidth, nHeight, NULL, IMG_FORMAT_MAX, 4);
    
    *ppROIDetect = pTmpROIDetect;

    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_ROIDetector_Destroy(IN OUT ROIDetect_T **ppROIDetect)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    CHECK_POINTER_VALIDATION((*ppROIDetect), KAKAO_STAT_NULL_POINTER)
    
    SAFEFREE((*ppROIDetect));
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_ROIDetector_UpdateImgSize(IN OUT ROIDetect_T *pROIDetect, IN const INT32 nWidth, IN const INT32 nHeight)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pROIDetect, KAKAO_STAT_NULL_POINTER)
    
    Kakao_ImgProc_Util_UpdateSize(&pROIDetect->nBaseSize, nWidth, nHeight, NULL, IMG_FORMAT_MAX, 4);
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_ROIDetector_Selection(ROIDetect_T *pROIDetect)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    CHECK_POINTER_VALIDATION(pROIDetect, KAKAO_STAT_NULL_POINTER)

    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}

