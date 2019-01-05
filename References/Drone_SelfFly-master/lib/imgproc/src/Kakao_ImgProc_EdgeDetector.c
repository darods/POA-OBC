


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
__INLINE UINT32 _Edge_Sobel_Core(IN const UINT8 *pSrc, IN const UINT32 nXPos, IN const UINT32 nYPos, IN const INT32 nStride);
__INLINE UINT32 _Edge_Laplacian_Core(IN const UINT8 *pSrc, IN const UINT32 nXPos, IN const UINT32 nYPos, IN const INT32 nStride);


/*----------------------------------------------------------------------------------------
 Static Variable
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Global Variable
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Function Implementation
 ----------------------------------------------------------------------------------------*/
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Edge_Create(IN OUT EdgeDet_T **ppEdgeDetect, IN const INT32 nWidth, IN const INT32 nHeight)
{
    EdgeDet_T               *pTmpEdgeDetect = NULL;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   nAlignedWidth = GET_ALIGNED_LENGTH(nWidth, MACRO_BLOCK_SIZE);
    INT32                   nAlignedHeight = GET_ALIGNED_LENGTH(nHeight, MACRO_BLOCK_SIZE);
    
    SAFEALLOC(pTmpEdgeDetect, 1, 32, EdgeDet_T);
    
    Kakao_ImgProc_Util_UpdateSize(&pTmpEdgeDetect->nBaseSize, nWidth, nHeight, NULL, IMG_FORMAT_MAX, 4);
    
    SAFEALLOC(pTmpEdgeDetect->pEdge_CannyTmpBuf, pTmpEdgeDetect->nBaseSize.nStride[IMG_PLANE_0] * nAlignedHeight, 32, UINT8);
    
    *ppEdgeDetect = pTmpEdgeDetect;
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Edge_Destroy(IN OUT EdgeDet_T **ppEdgeDetect)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    CHECK_POINTER_VALIDATION((*ppEdgeDetect), KAKAO_STAT_NULL_POINTER)
    
    SAFEFREE((*ppEdgeDetect)->pEdge_CannyTmpBuf);
    
    SAFEFREE((*ppEdgeDetect));
    
    nRet = KAKAO_STAT_OK;
    return nRet;

Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Edge_Sobel(IN EdgeDet_T *pEdgeDetect, IN OUT Img_T *pSrcImg)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pEdgeDetect, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER)

    {
        INT32                   i = 0, j = 0;
        INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
        INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
        INT32                   nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
        INT32                   nLocalPos = 0;
        UINT32                  nSobelEdgeVal = 0;
        UINT8                   *pSrc = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
        UINT8                   *pEdgeMap = NULL;
        UINT32                  nEdgeTh = 180;

        if(NULL == (pEdgeMap = pSrcImg->GetImgBinaryMap(pSrcImg, IMG_BINARYMAP_EDGE)))
            return nRet;
        
        CHECK_POINTER_VALIDATION(pSrc, KAKAO_STAT_NULL_POINTER)
        CHECK_POINTER_VALIDATION(pEdgeMap, KAKAO_STAT_NULL_POINTER)
        
        ___START_PROFILE(Edge_Sobel)
        
        for(j=0 ; j<nHeight ; j++)
        {
            nLocalPos = j * nStride;
            for(i=0 ; i<nWidth ; i++)
            {
                nSobelEdgeVal = _Edge_Sobel_Core(pSrc, i, j, nStride);
                
                pEdgeMap[nLocalPos++] = (nSobelEdgeVal > nEdgeTh);
            }
        }

        ___STOP_PROFILE(Edge_Sobel)
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Edge_Canny(IN EdgeDet_T *pEdgeDetect, IN OUT Img_T *pSrcImg)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pEdgeDetect, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER)

    {
        INT32                   i = 0, j = 0;
        INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
        INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
        INT32                   nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
        INT32                   nLocalPos = 0;
        UINT8                   *pSrc = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
        UINT8                   *pEdgeMap = NULL;
        INT32                   nEdgeTh = 60;
        INT32                   nValue = 0;
        INT32                   cc = 0;
        UINT32                  nSmoothMask[5][5] = {2,  4,  5,  4,  2,
                                                     4,  9, 12,  9,  4,
                                                     5, 12, 15, 12,  5,
                                                     4,  9, 12,  9,  4,
                                                     2,  4,  5,  4,  2};
        UINT8                   *pTmpBuf = pEdgeDetect->pEdge_CannyTmpBuf + PADDING_SIZE;

        if(NULL == (pEdgeMap = pSrcImg->GetImgBinaryMap(pSrcImg, IMG_BINARYMAP_EDGE)))
            return nRet;
        
        CHECK_POINTER_VALIDATION(pSrc, KAKAO_STAT_NULL_POINTER)
        CHECK_POINTER_VALIDATION(pEdgeMap, KAKAO_STAT_NULL_POINTER)
        
        ___START_PROFILE(Edge_Canny)
        
        // Smoothing filtering
        for(j=0 ; j<nHeight ; j++)
        {
            nLocalPos = j * nStride;
            for(i=0 ; i<nWidth ; i++)
            {
                UINT32          m = 0;
                INT32           nInPos = 0;
                
                for(m=0 ; m<5 ; m++)
                {
                    nInPos = (j + m - 2) * nStride + (i - 2);
                    
                    nValue  = pSrc[nInPos++] * nSmoothMask[m][0];
                    nValue += pSrc[nInPos++] * nSmoothMask[m][1];
                    nValue += pSrc[nInPos++] * nSmoothMask[m][2];
                    nValue += pSrc[nInPos++] * nSmoothMask[m][3];
                    nValue += pSrc[nInPos] * nSmoothMask[m][4];
                }
                
                pTmpBuf[nLocalPos++] = CLIP3(0, 255, ABSM(nValue)/115);
            }
        }

        // Edge detection
        for(j=1 ; j<nHeight-1 ; j++)
        {
            INT32              nPrevLocalPos = 0, nNextLocalPos = 0;
            
            nLocalPos = j * nStride;
            nPrevLocalPos -= nStride;
            nNextLocalPos += nStride;

            for(i=0 ; i<nWidth ; i++)
            {
                cc = -(pTmpBuf[nPrevLocalPos - 1] + 2*pTmpBuf[nLocalPos] + pTmpBuf[nPrevLocalPos + 1]);
                cc += (pTmpBuf[nNextLocalPos - 1] + 2*pTmpBuf[nLocalPos] + pTmpBuf[nNextLocalPos + 1]);
                nValue = abs(cc);

                cc = -(pTmpBuf[nPrevLocalPos - 1] + 2*pTmpBuf[nLocalPos - 1] + pTmpBuf[nNextLocalPos - 1]);
                cc += (pTmpBuf[nPrevLocalPos + 1] + 2*pTmpBuf[nLocalPos + 1] + pTmpBuf[nNextLocalPos + 1]);
                nValue += abs(cc);
                
                if(nValue > PIXEL_MAX)
                    nValue = PIXEL_MAX;
                else if(nValue < nEdgeTh)
                    nValue = 0;
                
                pEdgeMap[nLocalPos++] = !(!nValue);
            }
        }

        ___STOP_PROFILE(Edge_Canny)
    }

    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Edge_Laplacian(IN EdgeDet_T *pEdgeDetect, IN OUT Img_T *pSrcImg)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pEdgeDetect, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER)
    
    {
        INT32                   i = 0, j = 0;
        INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
        INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
        INT32                   nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
        INT32                   nLocalPos = 0;
        UINT32                  nLaplacianEdgeVal = 0;
        UINT8                   *pSrc = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
        UINT8                   *pEdgeMap = NULL;
        UINT32                  nEdgeTh = 50;

        if(NULL == (pEdgeMap = pSrcImg->GetImgBinaryMap(pSrcImg, IMG_BINARYMAP_EDGE)))
            return nRet;
        
        CHECK_POINTER_VALIDATION(pSrc, KAKAO_STAT_NULL_POINTER)
        CHECK_POINTER_VALIDATION(pEdgeMap, KAKAO_STAT_NULL_POINTER)

        ___START_PROFILE(Edge_Laplacian)
        
        for(j=0 ; j<nHeight ; j++)
        {
            nLocalPos = j * nStride;
            for(i=0 ; i<nWidth ; i++)
            {
                nLaplacianEdgeVal = _Edge_Laplacian_Core(pSrc, i, j, nStride);
                
                if(nLaplacianEdgeVal > PIXEL_MAX)
                    nLaplacianEdgeVal = PIXEL_MAX;
                else if(nLaplacianEdgeVal < nEdgeTh)
                    nLaplacianEdgeVal = 0;
                
                pEdgeMap[nLocalPos++] = !(!nLaplacianEdgeVal);
            }    
        }

        ___STOP_PROFILE(Edge_Laplacian)
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Edge_Custom(IN EdgeDet_T *pEdgeDetect, IN OUT Img_T *pSrcImg)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pEdgeDetect, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER)
    
    {
        #if 0
        Img_T                   *pSrc = &Resizer->pResizedImg;
        Img_T                   *pDst = &pEdgeDetect->pEdgeDetectedImg;
        INT32                   i = 0, j = 0;
        INT32                   nWidth = pSrc->nYSize.nWidth;
        INT32                   nHeight = pSrc->nYSize.nHeight;
        INT32                   nPaddingSize = pSrc->nPaddingsize;
        INT32                   nPaddedWidth_Y = pSrc->nYSize.nPaddedWidth;
        INT32                   nPaddedHeight_Y = pSrc->nYSize.nPaddedHeight;
        INT32                   nPaddedWidth_UV = pSrc->nUVSize.nPaddedWidth;
        INT32                   nPaddedHeight_UV = pSrc->nUVSize.nPaddedHeight;
        INT32                   nImageSize = (nPaddedWidth_UV * nPaddedHeight_UV);
        UINT8                   *pFramewiseMap0 = EdgeDetect->pEdgeDetectFramewiseMap0 + (nPaddingSize * nPaddedWidth_Y) + nPaddingSize;
        UINT8                   *pSrcY = &(pSrc->pFrame_Y[0]) + (nPaddingSize * nPaddedWidth_Y) + nPaddingSize;
        UINT8                   *pDstY = &(pDst->pFrame_Y[0]) + (nPaddingSize * nPaddedWidth_Y) + nPaddingSize;
        UINT8                   *pDstOctave0 = &(EdgeDetect->pOctave0[0]) + (nPaddingSize * nPaddedWidth_Y) + nPaddingSize;
        UINT8                   *pDstOctave1 = &(EdgeDetect->pOctave1[0]) + (nPaddingSize * nPaddedWidth_Y) + nPaddingSize;
        UINT8                   *pDstDoG0 = &(EdgeDetect->pDoG0[0]) + (nPaddingSize * nPaddedWidth_Y) + nPaddingSize;
        UINT8                   *pDstDoG1 = &(EdgeDetect->pDoG1[0]) + (nPaddingSize * nPaddedWidth_Y) + nPaddingSize;
        UINT8                   *pDstU = &(pDst->pFrame_Cb[0]);
        UINT8                   *pDstV = &(pDst->pFrame_Cr[0]);

        // Make Octave images
        Blur_Gaussian_Filter(pSrcY, pDstOctave0, EdgeDetect->nTemp_Filter, nWidth, nHeight, nPaddingSize, EdgeDetect->nGaussianBlurMaskSize);
        Blur_Gaussian_Filter(pDstOctave0, pDstOctave1, EdgeDetect->nTemp_Filter, nWidth, nHeight, nPaddingSize, EdgeDetect->nGaussianBlurMaskSize);    

        // Make Difference of Gaussian image
        Kakao_ImgProc_Util_ABSImg(pSrcY, pDstOctave0, pDstDoG0, nWidth, nHeight, nPaddingSize);
        Kakao_ImgProc_Util_ABSImg(pDstOctave0, pDstOctave1, pDstDoG1, nWidth, nHeight, nPaddingSize);


        for(j=0 ; j<nHeight ; j++)
        {
            for(i=0 ; i<nWidth ; i++)
            {
                if(EdgeDetect->pDoG0[(j*nPaddedWidth_Y) + i])
                    pFramewiseMap0[(j*nPaddedWidth_Y) + i] = 1;
                else
                    pFramewiseMap0[(j*nPaddedWidth_Y) + i] = 0;
            }
        }

        MEMCPY(&(pDst->pFrame_Y[0]), &(pEdgeDetect->pDoG0[0]), (nPaddedWidth_Y * nPaddedHeight_Y));
        MEMSET(pDstU, PIXEL_MAX/2, nImageSize);
        MEMSET(pDstV, PIXEL_MAX/2, nImageSize);
        #endif
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Edge_Detector(IN EdgeDet_T *pEdgeDetect, IN OUT Img_T *pSrcImg)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    CHECK_POINTER_VALIDATION(pEdgeDetect, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER)

    switch (pEdgeDetect->nEdgeMode)
    {
        case EDGE_MODE_SOBEL:
            Kakao_ImgProc_Edge_Sobel(pEdgeDetect, pSrcImg);
            break;
        case EDGE_MODE_CANNY:
            Kakao_ImgProc_Edge_Canny(pEdgeDetect, pSrcImg);
            break;
        case EDGE_MODE_LAPLACIAN:
            Kakao_ImgProc_Edge_Laplacian(pEdgeDetect, pSrcImg);
            break;
        case EDGE_MODE_CUSTOM:
            Kakao_ImgProc_Edge_Custom(pEdgeDetect, pSrcImg);
            break;
        default :
            break;
    }

    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Edge_UpdateImgSize(IN OUT EdgeDet_T *pEdgeDetect, IN const INT32 nWidth, IN const INT32 nHeight)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pEdgeDetect, KAKAO_STAT_NULL_POINTER)
    
    Kakao_ImgProc_Util_UpdateSize(&pEdgeDetect->nBaseSize, nWidth, nHeight, NULL, IMG_FORMAT_MAX, 4);
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


__INLINE UINT32 _Edge_Sobel_Core(IN const UINT8 *pSrc, IN const UINT32 nXPos, IN const UINT32 nYPos, IN const INT32 nStride)
{
    INT32                   i = 0;
    INT32                   nCenterValue_X = 0, nCenterValue_Y = 0;
    INT32                   nMask0[3][3] = {-1,  0,  1,
                                            -2,  0,  2,
                                            -1,  0,  1};
    INT32                   nMask1[3][3] = { 1,  2,  1,
                                             0,  0,  0,
                                            -1, -2, -1};
    INT32                   nLocalPos = 0;
    
    for(i=2 ; i>=0 ; i--)
    {
        nLocalPos = ((nYPos + i - 1) * nStride) + nXPos;
        
        nCenterValue_X += pSrc[nLocalPos - 1] * nMask0[i][0]
                        + pSrc[nLocalPos] * nMask0[i][1]
                        + pSrc[nLocalPos + 1] * nMask0[i][2];
        
        nCenterValue_Y += pSrc[nLocalPos - 1] * nMask1[i][0]
                        + pSrc[nLocalPos] * nMask1[i][1]
                        + pSrc[nLocalPos + 1] * nMask1[i][2];
    }
    
    return  CLIP3(PIXEL_MIN, PIXEL_MAX, ABSM(nCenterValue_X) + ABSM(nCenterValue_Y));
}


__INLINE UINT32 _Edge_Laplacian_Core(IN const UINT8 *pSrc, IN const UINT32 nXPos, IN const UINT32 nYPos, IN const INT32 nStride)
{
    INT32                   i = 0;
    INT32                   nValue = 0;
    INT32                   nMask[3][3] = {1,  1,  1,
                                           1, -8,  1,
                                           1,  1,  1};
    INT32                   nLocalPos = 0;
    
    nLocalPos = ((nYPos + i - 1) * nStride) + nXPos;
    nValue += pSrc[nLocalPos - 1] * nMask[0][0] + pSrc[nLocalPos] * nMask[0][1] + pSrc[nLocalPos + 1] * nMask[0][2];

    nLocalPos += nStride;
    nValue += pSrc[nLocalPos - 1] * nMask[1][0] + pSrc[nLocalPos] * nMask[1][1] + pSrc[nLocalPos + 1] * nMask[1][2];

    nLocalPos += nStride;
    nValue += pSrc[nLocalPos - 1] * nMask[2][0] + pSrc[nLocalPos] * nMask[2][1] + pSrc[nLocalPos + 1] * nMask[2][2];

    return CLIP3(PIXEL_MIN, PIXEL_MAX, ABSM(nValue));
}












