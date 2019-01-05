


/*----------------------------------------------------------------------------------------
 File Inclusions
 ----------------------------------------------------------------------------------------*/
#include "Kakao_ImgProc_Modules.h"
#include "Kakao_ImgProc_SIMD.h"


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
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Feature_Mean(IN const Img_T *pSrcImg, IN const INT32 nPlaneIdx, IN const INT32 nPosX, IN const INT32 nPosY,
                                                         IN const BlockType nBlockType, OUT UINT32 *pSum, OUT F32 *pMean)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSum, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pMean, KAKAO_STAT_NULL_POINTER);
    
    {
        const INT32         nSrcStride = pSrcImg->GetImgStride(pSrcImg, nPlaneIdx);
        const UINT8         *pSrc = pSrcImg->GetImgPlanes(pSrcImg, nPlaneIdx) + (nPosY * nSrcStride + nPosX);
        
        *pSum = 0;
        *pMean = 0.0;
        
        switch (nBlockType)
        {
            case BLOCKTYPE_4x4:
                *pMean = Mean_MxN(pSrc, nSrcStride, 4, 4, pSum);
                break;
            case BLOCKTYPE_8x4:
                *pMean = Mean_MxN(pSrc, nSrcStride, 8, 4, pSum);
                break;
            case BLOCKTYPE_4x8:
                *pMean = Mean_MxN(pSrc, nSrcStride, 4, 8, pSum);
                break;
            case BLOCKTYPE_8x8:
                *pMean = Mean_MxN(pSrc, nSrcStride, 8, 8, pSum);
                break;
            case BLOCKTYPE_16x8:
                *pMean = Mean_MxN(pSrc, nSrcStride, 16, 8, pSum);
                break;
            case BLOCKTYPE_8x16:
                *pMean = Mean_MxN(pSrc, nSrcStride, 8, 16, pSum);
                break;
            case BLOCKTYPE_16x16:
                *pMean = Mean_MxN(pSrc, nSrcStride, 16, 16, pSum);
                break;
            default:
                break;
        }
    }
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Feature_Variance(IN const Img_T *pSrcImg, IN const INT32 nPlaneIdx, IN const INT32 nPosX, IN const INT32 nPosY,
                                                             IN const BlockType nBlockType, OUT F32 *pMean, OUT F32 *pVar, OUT UINT32 *pSqSum)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pMean, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pVar, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSqSum, KAKAO_STAT_NULL_POINTER);
    
    {
        const INT32         nSrcStride = pSrcImg->GetImgStride(pSrcImg, nPlaneIdx);
        const UINT8         *pSrc = pSrcImg->GetImgPlanes(pSrcImg, nPlaneIdx) + (nPosY * nSrcStride + nPosX);
        UINT32              nSum = 0.0;

        switch (nBlockType)
        {
            case BLOCKTYPE_4x4:
                *pVar = Variance_MxN(pSrc, nSrcStride, 4, 4, pSqSum, &nSum);
                *pMean = nSum > 8 ? (F32)(nSum - 8) / 16.0 : (F32)(nSum) / 16.0;
                break;
            case BLOCKTYPE_8x4:
                *pVar = Variance_MxN(pSrc, nSrcStride, 8, 4, pSqSum, &nSum);
                *pMean = nSum > 16 ? (F32)(nSum - 16) / 32.0 : (F32)(nSum) / 32.0;
                break;
            case BLOCKTYPE_4x8:
                *pVar = Variance_MxN(pSrc, nSrcStride, 4, 8, pSqSum, &nSum);
                *pMean = nSum > 16 ? (F32)(nSum - 16) / 32.0 : (F32)(nSum) / 32.0;
                break;
            case BLOCKTYPE_8x8:
                *pVar = Variance_MxN(pSrc, nSrcStride, 8, 8, pSqSum, &nSum);
                *pMean = nSum > 32 ? (F32)(nSum - 32) / 64.0 : (F32)(nSum) / 64.0;
                break;
            case BLOCKTYPE_16x8:
                *pVar = Variance_MxN(pSrc, nSrcStride, 16, 8, pSqSum, &nSum);
                *pMean = nSum > 64 ? (F32)(nSum - 64) / 128.0 : (F32)(nSum) / 128.0;
                break;
            case BLOCKTYPE_8x16:
                *pVar = Variance_MxN(pSrc, nSrcStride, 8, 16, pSqSum, &nSum);
                *pMean = nSum > 64 ? (F32)(nSum - 64) / 128.0 : (F32)(nSum) / 128.0;
                break;
            case BLOCKTYPE_16x16:
                *pVar = Variance_MxN(pSrc, nSrcStride, 16, 16, pSqSum, &nSum);
                *pMean = nSum > 128 ? (F32)(nSum - 128) / 256.0 : (F32)(nSum) / 256.0;
                break;
            default:
                *pMean = 0.0;
                *pVar = 0.0;
                break;
        }
    }
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Feature_DiffVariance(IN const Img_T *pSrcImg0, IN const Img_T *pSrcImg1, IN const INT32 nPlaneIdx,
                                                                 IN const INT32 nPosX, IN const INT32 nPosY, IN const BlockType nBlockType,
                                                                 OUT F32 *pMean, OUT F32 *pVar, OUT UINT32 *pSqSum)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pSrcImg0, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg1, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pMean, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pVar, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSqSum, KAKAO_STAT_NULL_POINTER);
    
    {
        const INT32         nSrcStride0 = pSrcImg0->GetImgStride(pSrcImg0, nPlaneIdx);
        const UINT8         *pSrc0      = pSrcImg0->GetImgPlanes(pSrcImg0, nPlaneIdx) + (nPosY * nSrcStride0 + nPosX);
        const INT32         nSrcStride1 = pSrcImg1->GetImgStride(pSrcImg1, nPlaneIdx);
        const UINT8         *pSrc1      = pSrcImg1->GetImgPlanes(pSrcImg1, nPlaneIdx) + (nPosY * nSrcStride1 + nPosX);
        INT32               nSum = 0.0;
        
        switch (nBlockType)
        {
            case BLOCKTYPE_4x4:
                *pVar = DiffVariance_MxN(pSrc0, nSrcStride0, pSrc1, nSrcStride1, 4, 4, pSqSum, &nSum);
                *pMean = (F32)nSum / 16.0;
                break;
            case BLOCKTYPE_8x4:
                *pVar = DiffVariance_MxN(pSrc0, nSrcStride0, pSrc1, nSrcStride1, 8, 4, pSqSum, &nSum);
                *pMean = (F32)nSum / 32.0;
                break;
            case BLOCKTYPE_4x8:
                *pVar = DiffVariance_MxN(pSrc0, nSrcStride0, pSrc1, nSrcStride1, 4, 8, pSqSum, &nSum);
                *pMean = (F32)nSum / 32.0;
                break;
            case BLOCKTYPE_8x8:
                *pVar = DiffVariance_MxN(pSrc0, nSrcStride0, pSrc1, nSrcStride1, 8, 8, pSqSum, &nSum);
                *pMean = (F32)nSum / 64.0;
                break;
            case BLOCKTYPE_16x8:
                *pVar = DiffVariance_MxN(pSrc0, nSrcStride0, pSrc1, nSrcStride1, 16, 8, pSqSum, &nSum);
                *pMean = (F32)nSum / 128.0;
                break;
            case BLOCKTYPE_8x16:
                *pVar = DiffVariance_MxN(pSrc0, nSrcStride0, pSrc1, nSrcStride1, 8, 4, pSqSum, &nSum);
                *pMean = (F32)nSum / 128.0;
                break;
            case BLOCKTYPE_16x16:
                *pVar = DiffVariance_MxN(pSrc0, nSrcStride0, pSrc1, nSrcStride1, 16, 16, pSqSum, &nSum);
                *pMean = (F32)nSum / 256.0;
                break;
            default:
                *pMean = 0.0;
                *pVar = 0.0;
                break;
        }
    }
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}

