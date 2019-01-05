


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
#if ENABLE_ISET(CPU_ARMX | ISET_NEON)
    static Kakao_Status _SkinDetector_Core_NEON(IN SkinDet_T *pSkinDetector, IN Img_T *pSrcImg, OUT Img_T *pDstImg);
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_SSE4) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_SSE4)
    static Kakao_Status _SkinDetector_Core_SSE4(IN SkinDet_T *pSkinDetector, IN Img_T *pSrcImg, OUT Img_T *pDstImg);
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_SSE3) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_SSE3)
    static Kakao_Status _SkinDetector_Core_SSE3(IN SkinDet_T *pSkinDetector, IN Img_T *pSrcImg, OUT Img_T *pDstImg);
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_SSE2) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_SSE2)
    static Kakao_Status _SkinDetector_Core_SSE2(IN SkinDet_T *pSkinDetector, IN Img_T *pSrcImg, OUT Img_T *pDstImg);
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_SSE) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_SSE)
    static Kakao_Status _SkinDetector_Core_SSE(IN SkinDet_T *pSkinDetector, IN Img_T *pSrcImg, OUT Img_T *pDstImg);
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_NONE) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_NONE)
    static Kakao_Status _SkinDetector_Core_C(IN SkinDet_T *pSkinDetector, IN Img_T *pSrcImg, OUT Img_T *pDstImg);
#else
    static Kakao_Status _SkinDetector_Core_C(IN SkinDet_T *pSkinDetector, IN Img_T *pSrcImg, OUT Img_T *pDstImg);
#endif

#define _SkinDetector_Core              MAKE_ACCEL_FUNC(_SkinDetector_Core)


/*----------------------------------------------------------------------------------------
 Static Variable
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Global Variable
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Function Implementation
 ----------------------------------------------------------------------------------------*/
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_SkinDetector_Create(IN OUT SkinDet_T **ppSkinDetector, IN INT32 nWidth, IN INT32 nHeight)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    SkinDet_T               *pTmpSkinDetector = NULL;
    INT32                   i = 0;
    INT32                   nAlignedWidth = GET_ALIGNED_LENGTH(nWidth, MACRO_BLOCK_SIZE);

    SAFEALLOC(pTmpSkinDetector, 1, 32, SkinDet_T);
    
    Kakao_ImgProc_Util_UpdateSize(&pTmpSkinDetector->nBaseSize, nWidth, nHeight, NULL, IMG_FORMAT_MAX, 4);
    
    for(i=0 ; i<2 ; i++)
    {
        //Kakao_ImgProc_Util_CreateImg(&(pTmpSkinDetector->pFramewiseMap[i]), nWidth, nHeight, IMG_FORMAT_GRAY, IMG_DEPTH_U8);
        //Kakao_ImgProc_Util_CreateImg(&(pTmpSkinDetector->pMBwiseMap[i]), nWidth>>4, nHeight>>4, IMG_FORMAT_GRAY, IMG_DEPTH_U8);
    }
    
    Kakao_ImgProc_Filter_Create(&(pTmpSkinDetector->pFilter), nWidth, nHeight);
    
    *ppSkinDetector = pTmpSkinDetector;

    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_SkinDetector_Destroy(IN OUT SkinDet_T **ppSkinDetector)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   i = 0;
    
    CHECK_POINTER_VALIDATION((*ppSkinDetector), KAKAO_STAT_NULL_POINTER);

    Kakao_ImgProc_Filter_Destroy(&((*ppSkinDetector)->pFilter));
    
    for(i=0 ; i<2 ; i++)
    {
        //Kakao_ImgProc_Util_ReleaseImg(&((*ppSkinDetector)->pFramewiseMap[i]));
        //Kakao_ImgProc_Util_ReleaseImg(&((*ppSkinDetector)->pMBwiseMap[i]));
    }

    SAFEFREE((*ppSkinDetector));
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_SkinDetector_UpdateImgSize(IN OUT SkinDet_T *pSkinDetector, IN const INT32 nWidth, IN const INT32 nHeight)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   i = 0;
    
    CHECK_POINTER_VALIDATION(pSkinDetector, KAKAO_STAT_NULL_POINTER);
    
    Kakao_ImgProc_Util_UpdateSize(&pSkinDetector->nBaseSize, nWidth, nHeight, NULL, IMG_FORMAT_MAX, 4);
    
    for(i=0 ; i<2 ; i++)
    {
        //Kakao_ImgProc_Util_UpdateImgSize(pSkinDetector->pFramewiseMap[i], nWidth, nHeight, NULL, IMG_FORMAT_GRAY);
        //Kakao_ImgProc_Util_UpdateImgSize(pSkinDetector->pMBwiseMap[i], nWidth>>4, nHeight>>4, NULL, IMG_FORMAT_GRAY);
    }
    
    Kakao_ImgProc_Filter_UpdateImgSize(pSkinDetector->pFilter, nWidth, nHeight);
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_SkinDetector_Detect(IN SkinDet_T *pSkinDetect, IN Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    CHECK_POINTER_VALIDATION(pSkinDetect, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER);
    
    ___START_PROFILE(SkinDetect)
    
    _SkinDetector_Core(pSkinDetect, pSrcImg, pDstImg);

    ___STOP_PROFILE(SkinDetect)
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


#if ENABLE_ISET(CPU_ARMX | ISET_NEON)
static Kakao_Status _SkinDetector_Core_OneRow_NEON(IN const UINT8 *pSrcU, IN const UINT8 *pSrcV, OUT const UINT8 *pDst, IN const INT32 nWidth, IN const INT32 nStride)
{
    INT32                   i = 0;
    UINT32                  nCurrLineY = 0, nNextLineY = nStride, nOffset = 0;
    const INT32             nLoopCnt = nWidth >> 4;
    const UINT32            nDoubleMBSize = (MACRO_BLOCK_SIZE << 1);
    uint8x16_t              nU_MinRef, nU_MaxRef;
    uint8x16_t              nV_MinRef, nV_MaxRef;
    uint8x16_t              nMask;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    nU_MinRef = vdupq_n_u8(85);
    nU_MaxRef = vdupq_n_u8(150);
    nV_MinRef = vdupq_n_u8(136);
    nV_MaxRef = vdupq_n_u8(169);
    nMask = vdupq_n_u8(1);
    
    for(i=nLoopCnt-1 ; i>=0 ; i--)
    {
        uint8x16_t      nUPlaneSrc, nVPlaneSrc;
        uint8x16_t      nGtELaneU, nLtELaneU, nEqLaneU;
        uint8x16_t      nGtELaneV, nLtELaneV, nEqLaneV;
        uint8x16_t      nEqLaneUV, nMaskedLane;
        uint8x16x2_t    nInterleaved;
        
        nUPlaneSrc = vld1q_u8(pSrcU + nOffset);
        nVPlaneSrc = vld1q_u8(pSrcV + nOffset);
        
        // Compare U_Plane
        nGtELaneU = vcgeq_u8(nUPlaneSrc, nU_MinRef);
        nLtELaneU = vcleq_u8(nUPlaneSrc, nU_MaxRef);
        nEqLaneU = vceqq_u8(nGtELaneU, nLtELaneU);
        
        // Compare V_Plane
        nGtELaneV = vcgeq_u8(nVPlaneSrc, nV_MinRef);
        nLtELaneV = vcleq_u8(nVPlaneSrc, nV_MaxRef);
        nEqLaneV = vceqq_u8(nGtELaneV, nLtELaneV);
        
        nEqLaneUV = vandq_u8(nEqLaneU, nEqLaneV);
        nMaskedLane = vandq_u8(nEqLaneUV, nMask);
        nInterleaved = vzipq_u8(nMaskedLane, nMaskedLane);
        
        vst1q_u8(((UINT8 *)pDst + nCurrLineY), nInterleaved.val[0]);
        vst1q_u8(((UINT8 *)pDst + nCurrLineY + MACRO_BLOCK_SIZE), nInterleaved.val[1]);
        vst1q_u8(((UINT8 *)pDst + nNextLineY), nInterleaved.val[0]);
        vst1q_u8(((UINT8 *)pDst + nNextLineY + MACRO_BLOCK_SIZE), nInterleaved.val[1]);
        
        nCurrLineY += nDoubleMBSize;
        nNextLineY += nDoubleMBSize;
        nOffset += MACRO_BLOCK_SIZE;
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


static Kakao_Status _SkinDetector_Core_NEON(IN SkinDet_T *pSkinDetector, IN Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    INT32                   j = 0;
    const INT32             nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_1);
    const INT32             nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_1);
    const INT32             nStrideY = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    const INT32             nStrideUV = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_1);
    const UINT8             *pSrcU = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_2);
    const UINT8             *pSrcV = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_1);
    UINT8                   *pSkinMap = NULL;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_PARAM_VALIDATION((!(nWidth & 0x000000FF)), KAKAO_STAT_INVALID_PARAM)
    
    if(NULL == (pSkinMap = pSrcImg->GetImgBinaryMap(pSrcImg, IMG_BINARYMAP_SKIN)))
        return nRet;
    
    for(j=nHeight-1 ; j>=0 ; j--)
    {
        _SkinDetector_Core_OneRow_NEON(pSrcU, pSrcV, pSkinMap, nWidth, nStrideY);
        
        pSrcU += nStrideUV;
        pSrcV += nStrideUV;
        pSkinMap += (nStrideY << 1);
    }

    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_SSE4) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_SSE4)
static Kakao_Status _SkinDetector_Core_SSE4(IN SkinDet_T *pSkinDetector, IN Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    INT32                   i = 0, j = 0;
    INT32                   nPixPosY, nPixPosUV;
    UINT32                  nThreshold = 70;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nStrideY = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    INT32                   nStrideUV = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_1);
    UINT8                   *pSrcImgU = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_1);
    UINT8                   *pSrcImgV = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_2);
    UINT8                   *pSkinMap = NULL;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    if(NULL == (pSkinMap = pSrcImg->GetImgBinaryMap(pSrcImg, IMG_BINARYMAP_SKIN)))
        return nRet;
    
    for(j=0 ; j<nHeight ; j++)
    {
        nPixPosY = (j * nStrideY);
        nPixPosUV = ((j/2) * nStrideUV);
        
        for(i=0 ; i<nWidth ; i++)
        {
            // Set Skin detected pixel position to SkinDetect->pSkinDetectFramewiseMap1
            if(((85 <= pSrcImgU[nPixPosUV]) && (pSrcImgU[nPixPosUV] <= 150)) &&
               ((136 <= pSrcImgV[nPixPosUV]) && (pSrcImgV[nPixPosUV] <= 169)))
                pSkinMap[nPixPosY] = INDICATE_VALUE;
            else
                pSkinMap[nPixPosY] = 0;
            
            nPixPosY++;
            if(i & 0x1)
                nPixPosUV++;
        }
    }
    
    Kakao_ImgProc_Filter_Open(pSkinDetector->pFilter, pSrcImg, pDstImg, IMG_BINARYMAP_SKIN, 1);
    
    Kakao_ImgProc_Util_FramewiseToMBwise(pDstImg, IMG_BINARYMAP_SKIN, nThreshold);
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_SSE3) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_SSE3)
static Kakao_Status _SkinDetector_Core_SSE3(IN SkinDet_T *pSkinDetector, IN Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    INT32                   i = 0, j = 0;
    INT32                   nPixPosY, nPixPosUV;
    UINT32                  nThreshold = 70;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nStrideY = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    INT32                   nStrideUV = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_1);
    UINT8                   *pSrcImgU = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_1);
    UINT8                   *pSrcImgV = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_2);
    UINT8                   *pSkinMap = NULL;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    if(NULL == (pSkinMap = pSrcImg->GetImgBinaryMap(pSrcImg, IMG_BINARYMAP_SKIN)))
        return nRet;
    
    for(j=0 ; j<nHeight ; j++)
    {
        nPixPosY = (j * nStrideY);
        nPixPosUV = ((j/2) * nStrideUV);
        
        for(i=0 ; i<nWidth ; i++)
        {
            // Set Skin detected pixel position to SkinDetect->pSkinDetectFramewiseMap1
            if(((85 <= pSrcImgU[nPixPosUV]) && (pSrcImgU[nPixPosUV] <= 150)) &&
               ((136 <= pSrcImgV[nPixPosUV]) && (pSrcImgV[nPixPosUV] <= 169)))
                pSkinMap[nPixPosY] = INDICATE_VALUE;
            else
                pSkinMap[nPixPosY] = 0;
            
            nPixPosY++;
            if(i & 0x1)
                nPixPosUV++;
        }
    }
    
    Kakao_ImgProc_Filter_Open(pSkinDetector->pFilter, pSrcImg, pDstImg, IMG_BINARYMAP_SKIN, 1);
    
    Kakao_ImgProc_Util_FramewiseToMBwise(pDstImg, IMG_BINARYMAP_SKIN, nThreshold);
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_SSE2) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_SSE2)
static Kakao_Status _SkinDetector_Core_SSE2(IN SkinDet_T *pSkinDetector, IN Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    INT32                   i = 0, j = 0;
    INT32                   nPixPosY, nPixPosUV;
    UINT32                  nThreshold = 70;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nStrideY = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    INT32                   nStrideUV = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_1);
    UINT8                   *pSrcImgU = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_1);
    UINT8                   *pSrcImgV = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_2);
    UINT8                   *pSkinMap = NULL;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    if(NULL == (pSkinMap = pSrcImg->GetImgBinaryMap(pSrcImg, IMG_BINARYMAP_SKIN)))
        return nRet;
    
    for(j=0 ; j<nHeight ; j++)
    {
        nPixPosY = (j * nStrideY);
        nPixPosUV = ((j/2) * nStrideUV);
        
        for(i=0 ; i<nWidth ; i++)
        {
            // Set Skin detected pixel position to SkinDetect->pSkinDetectFramewiseMap1
            if(((85 <= pSrcImgU[nPixPosUV]) && (pSrcImgU[nPixPosUV] <= 150)) &&
               ((136 <= pSrcImgV[nPixPosUV]) && (pSrcImgV[nPixPosUV] <= 169)))
                pSkinMap[nPixPosY] = INDICATE_VALUE;
            else
                pSkinMap[nPixPosY] = 0;
            
            nPixPosY++;
            if(i & 0x1)
                nPixPosUV++;
        }
    }
    
    Kakao_ImgProc_Filter_Open(pSkinDetector->pFilter, pSrcImg, pDstImg, IMG_BINARYMAP_SKIN, 1);
    
    Kakao_ImgProc_Util_FramewiseToMBwise(pDstImg, IMG_BINARYMAP_SKIN, nThreshold);
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_SSE) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_SSE)
static Kakao_Status _SkinDetector_Core_SSE(IN SkinDet_T *pSkinDetector, IN Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    INT32                   i = 0, j = 0;
    INT32                   nPixPosY, nPixPosUV;
    UINT32                  nThreshold = 70;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nStrideY = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    INT32                   nStrideUV = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_1);
    UINT8                   *pSrcImgU = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_1);
    UINT8                   *pSrcImgV = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_2);
    UINT8                   *pSkinMap = NULL;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    if(NULL == (pSkinMap = pSrcImg->GetImgBinaryMap(pSrcImg, IMG_BINARYMAP_SKIN)))
        return nRet;
    
    for(j=0 ; j<nHeight ; j++)
    {
        nPixPosY = (j * nStrideY);
        nPixPosUV = ((j/2) * nStrideUV);
        
        for(i=0 ; i<nWidth ; i++)
        {
            // Set Skin detected pixel position to SkinDetect->pSkinDetectFramewiseMap1
            if(((85 <= pSrcImgU[nPixPosUV]) && (pSrcImgU[nPixPosUV] <= 150)) &&
               ((136 <= pSrcImgV[nPixPosUV]) && (pSrcImgV[nPixPosUV] <= 169)))
                pSkinMap[nPixPosY] = INDICATE_VALUE;
            else
                pSkinMap[nPixPosY] = 0;
            
            nPixPosY++;
            if(i & 0x1)
                nPixPosUV++;
        }
    }
    
    Kakao_ImgProc_Filter_Open(pSkinDetector->pFilter, pSrcImg, pDstImg, IMG_BINARYMAP_SKIN, 1);
    
    Kakao_ImgProc_Util_FramewiseToMBwise(pDstImg, IMG_BINARYMAP_SKIN, nThreshold);
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}
#elif ENABLE_ISET(CPU_INTEL_IA32 | ISET_NONE) | ENABLE_ISET(CPU_INTEL_IA64 | ISET_NONE)
static Kakao_Status _SkinDetector_Core_C(IN SkinDet_T *pSkinDetector, IN Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    INT32                   i = 0, j = 0;
    INT32                   nPixPosY, nPixPosUV;
    UINT32                  nThreshold = 70;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nStrideY = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    INT32                   nStrideUV = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_1);
    UINT8                   *pSrcImgU = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_1);
    UINT8                   *pSrcImgV = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_2);
    UINT8                   *pSkinMap = NULL;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    if(NULL == (pSkinMap = pSrcImg->GetImgBinaryMap(pSrcImg, IMG_BINARYMAP_SKIN)))
        return nRet;
    
    for(j=0 ; j<nHeight ; j++)
    {
        nPixPosY = (j * nStrideY);
        nPixPosUV = ((j/2) * nStrideUV);
        
        for(i=0 ; i<nWidth ; i++)
        {
            // Set Skin detected pixel position to SkinDetect->pSkinDetectFramewiseMap1
            if(((85 <= pSrcImgU[nPixPosUV]) && (pSrcImgU[nPixPosUV] <= 150)) &&
               ((136 <= pSrcImgV[nPixPosUV]) && (pSrcImgV[nPixPosUV] <= 169)))
                pSkinMap[nPixPosY] = INDICATE_VALUE;
            else
                pSkinMap[nPixPosY] = 0;
            
            nPixPosY++;
            if(i & 0x1)
                nPixPosUV++;
        }
    }
    
    Kakao_ImgProc_Filter_Open(pSkinDetector->pFilter, pSrcImg, pDstImg, IMG_BINARYMAP_SKIN, 1);
    
    Kakao_ImgProc_Util_FramewiseToMBwise(pDstImg, IMG_BINARYMAP_SKIN, nThreshold);
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}
#else
static Kakao_Status _SkinDetector_Core_C(IN SkinDet_T *pSkinDetector, IN Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    INT32                   i = 0, j = 0;
    INT32                   nPixPosY, nPixPosUV;
    UINT32                  nThreshold = 70;
    INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32                   nStrideY = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    INT32                   nStrideUV = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_1);
    UINT8                   *pSrcImgU = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_1);
    UINT8                   *pSrcImgV = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_2);
    UINT8                   *pSkinMap = NULL;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    if(NULL == (pSkinMap = pSrcImg->GetImgBinaryMap(pSrcImg, IMG_BINARYMAP_SKIN)))
        return nRet;
    
    for(j=0 ; j<nHeight ; j++)
    {
        nPixPosY = (j * nStrideY);
        nPixPosUV = ((j/2) * nStrideUV);
        
        for(i=0 ; i<nWidth ; i++)
        {
            // Set Skin detected pixel position to SkinDetect->pSkinDetectFramewiseMap1
            if(((85 <= pSrcImgU[nPixPosUV]) && (pSrcImgU[nPixPosUV] <= 150)) &&
               ((136 <= pSrcImgV[nPixPosUV]) && (pSrcImgV[nPixPosUV] <= 169)))
                pSkinMap[nPixPosY] = INDICATE_VALUE;
            else
                pSkinMap[nPixPosY] = 0;
            
            nPixPosY++;
            if(i & 0x1)
                nPixPosUV++;
        }
    }
    
    Kakao_ImgProc_Filter_Open(pSkinDetector->pFilter, pSrcImg, pDstImg, IMG_BINARYMAP_SKIN, 1);
    
    Kakao_ImgProc_Util_FramewiseToMBwise(pDstImg, IMG_BINARYMAP_SKIN, nThreshold);
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}
#endif


