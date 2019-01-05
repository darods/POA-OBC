


/*----------------------------------------------------------------------------------------
 File Inclusions
 ----------------------------------------------------------------------------------------*/
#include "Kakao_ImgProc_Modules.h"
#include "libyuv/rotate.h"

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
static Kakao_Status _Rorate_090Degree(IN const Rotate_T *pRotate, IN const Img_T *pSrcImg, OUT Img_T *pDstImg, const INT32 nPlaneIdx);
static Kakao_Status _Rorate_180Degree(IN const Rotate_T *pRotate, IN const Img_T *pSrcImg, OUT Img_T *pDstImg, const INT32 nPlaneIdx);
static Kakao_Status _Rorate_270Degree(IN const Rotate_T *pRotate, IN const Img_T *pSrcImg, OUT Img_T *pDstImg, const INT32 nPlaneIdx);


/*----------------------------------------------------------------------------------------
 Static Variable
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Global Variable
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Function Implementation
 ----------------------------------------------------------------------------------------*/
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Rotate_Create(IN OUT Rotate_T **ppRotate, IN const INT32 nWidth, IN const INT32 nHeight)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    UINT32                  nRealImgSize = 0;
    Rotate_T                *pTmpRotate = NULL;
    INT32                   nAlignedWidth = GET_ALIGNED_LENGTH(nWidth, MACRO_BLOCK_SIZE);
    INT32                   nAlignedHeight = GET_ALIGNED_LENGTH(nHeight, MACRO_BLOCK_SIZE);
    
    SAFEALLOC(pTmpRotate, 1, 32, Rotate_T);
    
    Kakao_ImgProc_Util_UpdateSize(&pTmpRotate->nBaseSize, nWidth, nHeight, NULL, IMG_FORMAT_MAX, 4);
    
    nRealImgSize = nAlignedWidth * nAlignedHeight;
    
    *ppRotate = pTmpRotate;
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    Kakao_ImgProc_Rotate_Destroy(&pTmpRotate);
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Rotate_Destroy(IN OUT Rotate_T **ppRotate)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION((*ppRotate), KAKAO_STAT_NULL_POINTER)

    SAFEFREE((*ppRotate));
    
    nRet = KAKAO_STAT_OK;

    return nRet;

Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Rotate_UpdateImgSize(IN OUT Rotate_T *pRotate, IN const INT32 nWidth, IN const INT32 nHeight)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pRotate, KAKAO_STAT_NULL_POINTER);
    
    Kakao_ImgProc_Util_UpdateSize(&pRotate->nBaseSize, nWidth, nHeight, NULL, IMG_FORMAT_MAX, 4);
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Rotate_DoRotate(IN const Rotate_T *pRotate, IN const Img_T *pSrcImg, OUT Img_T *pDstImg, const RotateDegree nDegree)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pRotate, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER)
    
    if((ROTATE_CW_000 == nDegree) || (ROTATE_CCW_000 == nDegree))
    {
        Kakao_ImgProc_Util_CopyImg(pDstImg, pSrcImg);
    }
    else
    {
        const INT32         nNumofPlanes = pSrcImg->GetNumofImgPlanes(pSrcImg);
        INT32               i = 0;
        
        for(i=0 ; i<nNumofPlanes ; i++)
        {
            if((ROTATE_CW_090 == nDegree) || (ROTATE_CCW_270 == nDegree))
                _Rorate_090Degree(pRotate, pSrcImg, pDstImg, i);
            else if((ROTATE_CW_180 == nDegree) || (ROTATE_CCW_180 == nDegree))
                _Rorate_180Degree(pRotate, pSrcImg, pDstImg, i);
            else if((ROTATE_CW_270 == nDegree) || (ROTATE_CCW_090 == nDegree))
                _Rorate_270Degree(pRotate, pSrcImg, pDstImg, i);
        }
    }
    
    nRet = KAKAO_STAT_OK;

    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _Rorate_090Degree(IN const Rotate_T *pRotate, IN const Img_T *pSrcImg, OUT Img_T *pDstImg, const INT32 nPlaneIdx)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, nPlaneIdx);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, nPlaneIdx);
    INT32                   nSrcStride = pSrcImg->GetImgStride(pSrcImg, nPlaneIdx);
    INT32                   nDstStride = pDstImg->GetImgStride(pDstImg, nPlaneIdx);
    UINT8                   *pSrc = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, nPlaneIdx);
    UINT8                   *pDst = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, nPlaneIdx);

    RotatePlane90(pSrc, nSrcStride, pDst, nDstStride, nWidth, nHeight);

    nRet = KAKAO_STAT_OK;

    return nRet;
}


static Kakao_Status _Rorate_180Degree(IN const Rotate_T *pRotate, IN const Img_T *pSrcImg, OUT Img_T *pDstImg, const INT32 nPlaneIdx)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, nPlaneIdx);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, nPlaneIdx);
    INT32                   nSrcStride = pSrcImg->GetImgStride(pSrcImg, nPlaneIdx);
    INT32                   nDstStride = pDstImg->GetImgStride(pDstImg, nPlaneIdx);
    UINT8                   *pSrc = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, nPlaneIdx);
    UINT8                   *pDst = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, nPlaneIdx);
    
    RotatePlane180(pSrc, nSrcStride, pDst, nDstStride, nWidth, nHeight);
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
}


static Kakao_Status _Rorate_270Degree(IN const Rotate_T *pRotate, IN const Img_T *pSrcImg, OUT Img_T *pDstImg, const INT32 nPlaneIdx)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, nPlaneIdx);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, nPlaneIdx);
    INT32                   nSrcStride = pSrcImg->GetImgStride(pSrcImg, nPlaneIdx);
    INT32                   nDstStride = pDstImg->GetImgStride(pDstImg, nPlaneIdx);
    UINT8                   *pSrc = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, nPlaneIdx);
    UINT8                   *pDst = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, nPlaneIdx);
    
    RotatePlane270(pSrc, nSrcStride, pDst, nDstStride, nWidth, nHeight);
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
}

