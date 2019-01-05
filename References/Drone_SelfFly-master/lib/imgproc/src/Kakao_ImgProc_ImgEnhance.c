


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
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_ImgEnhance_Create(IN OUT ImgEnhance_T **ppImgEnhancer, IN const INT32 nWidth, IN const INT32 nHeight)
{
    ImgEnhance_T            *pTmpImgEnhancer = NULL;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    INT32                   nAlignedWidth = GET_ALIGNED_LENGTH(nWidth, MACRO_BLOCK_SIZE);
    
    SAFEALLOC(pTmpImgEnhancer, 1, 32, ImgEnhance_T);
    
    Kakao_ImgProc_Util_UpdateSize(&pTmpImgEnhancer->nBaseSize, nWidth, nHeight, NULL, IMG_FORMAT_MAX, 4);
    
    #if 0
    {
        void    *pAdaptiveFilter_Handle = NULL;
        
        AdaptiveFilter_Create(&pAdaptiveFilter_Handle);

        ImgEnhancer->pEnhanceFilter = pAdaptiveFilter_Handle;
        
        AdaptiveFilter_Init(ImgEnhancer->pEnhanceFilter, nPaddedSizeY.nWidth, nPaddedSizeY.nHeight, 
                                        (nPaddedSizeY.nPaddedWidth - nPaddedSizeY.nWidth)/2, 4, 2, 10, 5, 0.4);
    }
    #endif
    
    
    *ppImgEnhancer = pTmpImgEnhancer;

    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_ImgEnhance_Destroy(IN OUT ImgEnhance_T **ppImgEnhancer)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    CHECK_POINTER_VALIDATION((*ppImgEnhancer), KAKAO_STAT_NULL_POINTER)
    
#if 0
    AdaptiveFilter_Destroy(ImgEnhancer->pEnhanceFilter);
#endif
 
    SAFEFREE((*ppImgEnhancer));
    
    nRet = KAKAO_STAT_OK;

    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_ImgEnhance_UpdateImgSize(IN OUT ImgEnhance_T *pImgEnhancer, IN const INT32 nWidth, IN const INT32 nHeight)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pImgEnhancer, KAKAO_STAT_NULL_POINTER)
    
    Kakao_ImgProc_Util_UpdateSize(&pImgEnhancer->nBaseSize, nWidth, nHeight, NULL, IMG_FORMAT_MAX, 4);
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_ImgEnhance_DoEnhance(IN const ImgEnhance_T *pImgEnhancer, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    //INT32                   nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    //INT32                   nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    //INT32                   nStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    //UINT32                  nImageSize;
    //UINT8                   *pSrcY = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
    //UINT8                   *pSrcU = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_1);
    //UINT8                   *pSrcV = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_2);
    //UINT8                   *pDstY = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0);
    //UINT8                   *pDstU = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_1);
    //UINT8                   *pDstV = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_2);
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    CHECK_POINTER_VALIDATION(pImgEnhancer, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER)
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER)

    #if NORMAL_IMG_ENHANCE
    {
        nImageSize = (nPaddedWidth_Y * nPaddedHeight_Y); 
        MEMCPY(pDstY, pSrcY, nImageSize);
        //Kakao_ImgProc_QuantizePix(pSrcY, pDstY, 4, pSrc->nYSize);
        //Kakao_ImgProc_Util_HistogrmaStretch(pSrcY, pDstY, &ImgEnhancer->pEnhancedImg.nYSize);
        
        nImageSize = (nPaddedWidth_UV * nPaddedHeight_UV); 
        MEMCPY(pDstU, pSrcU, nImageSize);
        MEMCPY(pDstV, pSrcV, nImageSize);

        //Kakao_ImgProc_Util_Equalization(pSrcY, pDstY, &ImgEnhancer->pEnhancedImg.nYSize);
        //Kakao_ImgProc_Util_HistogrmaStretch(pSrcY, pDstY, &ImgEnhancer->pEnhancedImg.nYSize);
        #if 0
        {
            Img_T ptmp0, ptmp1;

            ptmp0.nYSize = pSrc->nYSize;
            ptmp0.nUVSize = pSrc->nUVSize;            
            ptmp1.nYSize = pDst->nYSize;
            ptmp1.nUVSize = pDst->nUVSize;
            ptmp0.nPaddingsize = pSrc->nPaddingsize;
            ptmp1.nPaddingsize = pDst->nPaddingsize;

            SAFEALLOC(ptmp0.pImgData, (3 * nPaddedWidth_Y * nPaddedHeight_Y), 32, UINT8);
            SAFEALLOC(ptmp1.pImgData, (3 * nPaddedWidth_Y * nPaddedHeight_Y), 32, UINT8);            
            
            ColorConvert_YUV420toRGB(pSrc, &ptmp0); 
            
            Kakao_ImgProc_Util_Equalization(ptmp0.pImgData, ptmp1.pImgData, &pPreproc->ImgEnhanceInfo.pEnhancedImg.nYSize);
            Kakao_ImgProc_Util_Equalization(ptmp0.pImgData+(nWidth*nHeight), ptmp1.pImgData+(nWidth*nHeight), &pPreproc->ImgEnhanceInfo.pEnhancedImg.nYSize);
            Kakao_ImgProc_Util_Equalization(ptmp0.pImgData+(2*nWidth*nHeight), ptmp1.pImgData+(2*nWidth*nHeight), &pPreproc->ImgEnhanceInfo.pEnhancedImg.nYSize);        

            ColorConvert_RGBtoYUV420(&ptmp1, pDst);        

            SAFEFREE(ptmp0.pImgData);            
            SAFEFREE(ptmp1.pImgData);
        }
        #endif
    }
    #elif 0
    {
        Adaptive_trilateral_filtering(pPreproc, pSrcY, pPreproc->ImgEnhanceInfo.pEnhanceFilter->ptmpImg);

        nImageSize = (nPaddedWidth_Y * nPaddedHeight_Y); 
        MEMCPY(pPreproc->ImgEnhanceInfo.pEnhanceFilter->ptmpImg, pSrcY, nImageSize);
            
        nImageSize = (nPaddedWidth_UV * nPaddedHeight_UV); 
        MEMCPY(pDstU, pSrcU, nImageSize);
        MEMCPY(pDstV, pSrcV, nImageSize);
    }
    #else
    {
        //INT32           x, y;
        //INT32           i, j;
        //INT32           Temp_Dst;
        //INT32           nFilterSize;
        //INT32           nPos, nMBPos;
        //INT32           nMode;
        //D64             *nFilterCoeffi;

        #if 0
        Kakao_ImgProc_Padding_Frame(pSrc);

        Bilateral_filtering(pSrcY, pDstY, 3, 30, 10, pDst->nYSize);

        Kakao_ImgProc_QuantizePix(pSrcY, pDstY, 8, pSrc->nYSize);

        for(y=0 ; y<nHeight ; y++)
        {   
            for(x=0 ; x<nWidth ; x++)
            {
                #if 0
                    if(((y < nHeight/4) || (y > 3*nHeight/4)) || ((x < nWidth/4) || (x > 3*nWidth/4)))
                        nMode = 4;
                    else if(((y < 2*nHeight/5) || (y > 3*nHeight/5)) || ((x < 2*nWidth/5) || (x > 3*nWidth/5)))
                        nMode = 3;
                    else
                        nMode = 2;
                #else
                    nMBPos = ((y>>4) + 1)*(nStride >> 4) + ((x)>>4) + 1;
                    if(1 == pPreproc->AutoROIDetect.pAutoROIMBwiseMap[nMBPos])
                        nMode = 0;
                    else
                        nMode = 3;
                #endif
               
                nPos = 0;
                Temp_Dst = 0;
                nFilterSize = pPreproc->ImgEnhanceInfo.nFilterSize[nMode];
                nFilterCoeffi = pPreproc->ImgEnhanceInfo.nFilterCoeffi[nMode];
                for(j=-nFilterSize ; j<=nFilterSize ; j++)
                {
                    for(i=-nFilterSize ; i<=nFilterSize ; i++)
                    {
                        Temp_Dst += (INT32)(nFilterCoeffi[nPos]) * pSrcY[nPaddedWidth_Y*(nPaddingSize + y + j) + nPaddingSize + x + i];
                        nPos++;
                    }
                }

                pDstY[nPaddedWidth_Y*(nPaddingSize + y) + nPaddingSize + x] = (UINT8)(CLIP3(PIXEL_MIN, PIXEL_MAX, (Temp_Dst >> FLOATFACTOR)));
            }
        }
        
        nImageSize = (nPaddedWidth_UV * nPaddedHeight_UV); 
        MEMCPY(pDstU, pSrcU, nImageSize);
        MEMCPY(pDstV, pSrcV, nImageSize);
        #endif
    }
    #endif
    
    nRet = KAKAO_STAT_OK;
    
    return nRet;

Error:
    return nRet;
}

