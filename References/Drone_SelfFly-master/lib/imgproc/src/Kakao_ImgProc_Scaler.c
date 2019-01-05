


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
static void _Scale_Subsample_1_2(IN const UINT8 *pSrc, IN const INT32 nSrcWidth, IN const INT32 nSrcHeight, IN const INT32 nSrcStride,
                                                OUT UINT8 *pDst, IN const INT32 nDstWidth, IN const INT32 nDstHeight, IN const INT32 nDstStride);
static void _Scale_Subsample_1_4(IN const UINT8 *pSrc, IN const INT32 nSrcWidth, IN const INT32 nSrcHeight, IN const INT32 nSrcStride,
                                                OUT UINT8 *pDst, IN const INT32 nDstWidth, IN const INT32 nDstHeight, IN const INT32 nDstStride);
static void _Scale_Subsample_Arbitrary(IN const UINT8 *pSrc, IN const INT32 nSrcWidth, IN const INT32 nSrcHeight, IN const INT32 nSrcStride,
                                                OUT UINT8 *pDst, IN const INT32 nDstWidth, IN const INT32 nDstHeight, IN const INT32 nDstStride);
static Kakao_Status _Scaler_SubsampleCoreByte(IN const UINT8 *pSrc, IN const INT32 nSrcWidth, IN const INT32 nSrcHeight, IN const INT32 nSrcStride,
                                                OUT UINT8 *pDst, IN const INT32 nDstWidth, INT32 nDstHeight, IN const INT32 nDstStride);
static Kakao_Status _Scaler_BilinearCoreByte(IN const UINT8 *pSrc, IN const INT32 nSrcWidth, IN const INT32 nSrcHeight, IN const INT32 nSrcStride,
                                                OUT UINT8 *pDst, IN const INT32 nDstWidth, IN const INT32 nDstHeight, IN const INT32 nDstStride);
static Kakao_Status _Scaler_BilinearCoreDouble(IN const D64 *pSrc, D64 *pDst, IN const INT32 nWidth, IN const INT32 nHeight,
                                                IN const INT32 nSrcStride, IN const INT32 nDstStride, IN const F32 nRatioX, IN const F32 nRatioY);
static Kakao_Status _Scaler_Subsample_Gray(IN const ImgScaler_T *pScaler, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
static Kakao_Status _Scaler_Subsample_I420(IN const ImgScaler_T *pScaler, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
static Kakao_Status _Scaler_Bilinear_Gray(IN const ImgScaler_T *pScaler, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
static Kakao_Status _Scaler_Bilinear_I420(IN const ImgScaler_T *pScaler, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
static Kakao_Status _Scaler_Bilinear_HSV(IN const ImgScaler_T *pScaler, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);
static Kakao_Status _Scaler_Bilinear_HSL(IN const ImgScaler_T *pScaler, IN const Img_T *pSrcImg, OUT Img_T *pDstImg);


/*----------------------------------------------------------------------------------------
 Static Variable
 ----------------------------------------------------------------------------------------*/
static fpScaler     fpScaler_SubSample[IMG_FORMAT_MAX] = {_Scaler_Subsample_Gray, NULL, NULL, NULL, NULL, _Scaler_Subsample_I420,
                                                        NULL, NULL, NULL, NULL, NULL, NULL};
static fpScaler     fpScaler_Bilinear[IMG_FORMAT_MAX] = {_Scaler_Bilinear_Gray, NULL, NULL, NULL, NULL, _Scaler_Bilinear_I420,
                                                        NULL, _Scaler_Bilinear_HSV, _Scaler_Bilinear_HSL, NULL, NULL, NULL};
static fpScaler     fpScaler_Bicubic[IMG_FORMAT_MAX] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};


/*----------------------------------------------------------------------------------------
 Global Variable
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Function Implementation
 ----------------------------------------------------------------------------------------*/
KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Scaler_Create(IN OUT ImgScaler_T **ppScaler, IN const INT32 nWidth, IN const INT32 nHeight)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    ImgScaler_T             *pTmpScaler = NULL;
    INT32                   nAlignedWidth = GET_ALIGNED_LENGTH(nWidth, MACRO_BLOCK_SIZE);
    
    SAFEALLOC(pTmpScaler, 1, 32, ImgScaler_T);
    
    Kakao_ImgProc_Util_UpdateSize(&pTmpScaler->nBaseSize, nWidth, nHeight, NULL, IMG_FORMAT_MAX, 4);

    MEMCPY(pTmpScaler->fpScalerSubsampleFunc, fpScaler_SubSample, (IMG_FORMAT_MAX * sizeof(fpScaler)));
    MEMCPY(pTmpScaler->fpScalerBilinearFunc, fpScaler_Bilinear, (IMG_FORMAT_MAX * sizeof(fpScaler)));
    MEMCPY(pTmpScaler->fpScalerBicubicFunc, fpScaler_Bicubic, (IMG_FORMAT_MAX * sizeof(fpScaler)));
    
    *ppScaler = pTmpScaler;
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Scaler_Destroy(IN OUT ImgScaler_T **ppScaler)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    CHECK_POINTER_VALIDATION((*ppScaler), KAKAO_STAT_NULL_POINTER);
    
    SAFEFREE((*ppScaler));
    
    nRet = KAKAO_STAT_OK;
    return nRet;

Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Scaler_UpdateImgSize(IN OUT ImgScaler_T *pScaler, IN const INT32 nWidth, IN const INT32 nHeight)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pScaler, KAKAO_STAT_NULL_POINTER);
    
    Kakao_ImgProc_Util_UpdateSize(&pScaler->nBaseSize, nWidth, nHeight, NULL, IMG_FORMAT_MAX, 4);
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Scaler_Subsample(IN const ImgScaler_T *pScaler, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    CHECK_POINTER_VALIDATION(pScaler, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER);
    
    ___START_PROFILE(Scaler_Sub)

    if((pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0) == pDstImg->GetImgWidth(pDstImg, IMG_PLANE_0)) &&
       (pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0) == pDstImg->GetImgHeight(pDstImg, IMG_PLANE_0)))
    {
        nRet = Kakao_ImgProc_Util_CopyImg(pDstImg, pSrcImg);
    }
    else
    {
        INT32                   nImgFormatIdx = 0;

        Kakao_ImgProc_Util_Get_ImgFormatIdx(pSrcImg->GetImgFormat(pSrcImg), &nImgFormatIdx);
        
        if(NULL != pScaler->fpScalerSubsampleFunc[nImgFormatIdx])
            nRet = pScaler->fpScalerSubsampleFunc[nImgFormatIdx](pScaler, pSrcImg, pDstImg);
    }
    
    ___STOP_PROFILE(Scaler_Sub)
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
    
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Scaler_Bilinear(IN const ImgScaler_T *pScaler, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    CHECK_POINTER_VALIDATION(pScaler, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER);
    
    ___START_PROFILE(Scaler_Bil)
    
    if((pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0) == pDstImg->GetImgWidth(pDstImg, IMG_PLANE_0)) &&
        (pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0) == pDstImg->GetImgHeight(pDstImg, IMG_PLANE_0)))
    {
        nRet = Kakao_ImgProc_Util_CopyImg(pDstImg, pSrcImg);
    }
    else
    {
        INT32                   nImgFormatIdx = 0;
        
        Kakao_ImgProc_Util_Get_ImgFormatIdx(pSrcImg->GetImgFormat(pSrcImg), &nImgFormatIdx);
        
        if(NULL != pScaler->fpScalerBilinearFunc[nImgFormatIdx])
            nRet = pScaler->fpScalerBilinearFunc[nImgFormatIdx](pScaler, pSrcImg, pDstImg);
    }

    ___STOP_PROFILE(Scaler_Bil)

    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;

}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Scaler_Bicubic(UINT8 *pSrc, UINT8 *pDst, UINT8 *Kakao_VCmpImg, KISize *nSrcSize, KISize *nDstSize)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

#if 0
    INT32                   nSrcWidth = nSrcSize->nWidth;
    INT32                   nSrcHeight = nSrcSize->nHeight;
    INT32                   nDstWidth = nDstSize->nWidth;
    INT32                   nDstHeight = nDstSize->nHeight;
    INT32                   nSrcPaddedWidth = nSrcSize->nPaddedWidth;
    INT32                   nDstPaddedWidth = nDstSize->nPaddedWidth;
    INT32                   nSrcPaddingSize = (nSrcPaddedWidth - nSrcWidth) / 2;
    INT32                   nDstPaddingSize = (nDstPaddedWidth - nDstWidth) / 2;
    F32                     hratio = (F32) nSrcWidth /nDstWidth ;
    F32                     vratio = (F32) nSrcHeight / nDstHeight;
    D64                     CV0,CV1,CV2,CV3;
    D64                     CH0,CH1,CH2,CH3;
    F32                     hscale_ratio;
    F32                     vscale_ratio;
    F32                     sr_v, sr_h;
    INT32                   ii;
    INT32                   jj;
    D64                     temp_v, temp_h;
    INT32                   i,j;

    UINT8   *pDst_temp = Kakao_VCmpImg + (nSrcPaddedWidth * nSrcPaddingSize) + nSrcPaddingSize;
    UINT8   *pIn = pSrc + (nSrcPaddedWidth * nSrcPaddingSize) + nSrcPaddingSize;
    UINT8   *pOut = pDst + (nDstPaddedWidth * nDstPaddingSize) + nDstPaddingSize;    

    for(i=0 ; i<nDstHeight ; i++)
    {
        vscale_ratio = (F32)(vratio * (i+1));
        ii = (INT32)(vscale_ratio);

        if(ii>nSrcHeight-1) 
        ii = nSrcHeight-3;

        sr_v = vscale_ratio - ii;
        CV0 = -(1.0/6.0)*sr_v*sr_v*sr_v + (1.0/2.0)*sr_v*sr_v - (1.0/3.0)*sr_v;
        CV1 = (1.0/2.0)*sr_v*sr_v*sr_v - sr_v*sr_v - (1.0/2.0)*sr_v + 1;
        CV2 = -(1.0/2.0)*sr_v*sr_v*sr_v + (1.0/2.0)*sr_v*sr_v + sr_v;
        CV3 = (1.0/6.0)*sr_v*sr_v*sr_v - (1.0/6.0)*sr_v;

        for(j=0 ; j<nSrcWidth ; j++)
        {
            temp_v = pIn[(nSrcPaddedWidth * (ii-1))+ j]*CV0 + pIn[(nSrcPaddedWidth * (ii))+ j]*CV1 + pIn[(nSrcPaddedWidth * (ii+1))+ j]*CV2 + pIn[(nSrcPaddedWidth * (ii+2))+ j]*CV3;
            if(temp_v < 0) temp_v = 0; 
            else if(temp_v > 255) temp_v = 255;

            pDst_temp[(nSrcPaddedWidth * (i)) + j] = (UINT8)(temp_v+.5);
        }
    }

    for(j=0 ; j<nDstWidth ; j++)
    {
        hscale_ratio = (F32)(hratio * (j+1));
        jj = (INT32)(hscale_ratio);
        if(jj>nSrcWidth-1) 
        jj = nSrcWidth-3;

        sr_h = hscale_ratio - jj;

        CH0 = -(1.0/6.0)*sr_h*sr_h*sr_h + (1.0/2.0)*sr_h*sr_h - (1.0/3.0)*sr_h;
        CH1 = (1.0/2.0)*sr_h*sr_h*sr_h - sr_h*sr_h - (1.0/2.0)*sr_h + 1;
        CH2 = -(1.0/2.0)*sr_h*sr_h*sr_h + (1.0/2.0)*sr_h*sr_h + sr_h;
        CH3 = (1.0/6.0)*sr_h*sr_h*sr_h - (1.0/6.0)*sr_h;

        for(i=0 ; i<nDstHeight ; i++)
        {
            temp_h = pDst_temp[(nSrcPaddedWidth * i) + (jj-1)]*CH0 + pDst_temp[(nSrcPaddedWidth * i) + (jj)]*CH1 + pDst_temp[(nSrcPaddedWidth * i) + (jj+1)]*CH2 + pDst_temp[(nSrcPaddedWidth * i) + (jj+2)]*CH3;
            if(temp_h < 0) temp_h = 0;
            else if(temp_h > 255) temp_h = 255;

            pOut[(nDstPaddedWidth * (i)) + j] = (UINT8)(temp_h+.5);
        }
    }
#endif

    nRet = KAKAO_STAT_OK;
    return nRet;
}


KAKAO_EXPORT_API Kakao_Status Kakao_ImgProc_Scaler_External(IN const ImgScaler_T *pScaler, IN const Img_T *pSrcImg, OUT Img_T *pDstImg, const ScaleMode nScaleMode)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    UINT32                  i = 0;
    
    CHECK_POINTER_VALIDATION(pScaler, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pSrcImg, KAKAO_STAT_NULL_POINTER);
    CHECK_POINTER_VALIDATION(pDstImg, KAKAO_STAT_NULL_POINTER);
    
    ___START_PROFILE(Scaler_Ext)

    for(i=0 ; i<3 ; i++)
    {
        INT32           nSrcWidth = pSrcImg->GetImgWidth(pSrcImg, i);
        INT32           nSrcHeight = pSrcImg->GetImgHeight(pSrcImg, i);
        INT32           nSrcStride = pSrcImg->GetImgStride(pSrcImg, i);
        UINT8           *pSrcBuf = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, i);
        INT32           nDstWidth = pDstImg->GetImgWidth(pDstImg, i);
        INT32           nDstHeight = pDstImg->GetImgHeight(pDstImg, i);
        INT32           nDstStride = pDstImg->GetImgStride(pDstImg, i);
        UINT8           *pDstBuf = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, i);
        
        ScalePlane(pSrcBuf, nSrcStride, nSrcWidth, nSrcHeight,
                   pDstBuf, nDstStride, nDstWidth, nDstHeight,
                   (enum FilterMode)nScaleMode);
    }
    
    ___STOP_PROFILE(Scaler_Ext)
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
    
}


static void _Scale_Subsample_1_2(IN const UINT8 *pSrc, IN const INT32 nSrcWidth, IN const INT32 nSrcHeight, IN const INT32 nSrcStride,
                                 OUT UINT8 *pDst, IN const INT32 nDstWidth, IN const INT32 nDstHeight, IN const INT32 nDstStride)
{
    INT32                   j = 0;
    INT32                   nSrcStride2 = (nSrcStride << 1);
    
    for(j=0 ; j<nDstHeight ; j++)
    {
        OneRow_Subsample_Even(pSrc, pDst, nDstWidth);
        pSrc += nSrcStride2;
        pDst += nDstStride;
    }
}


static void _Scale_Subsample_1_4(IN const UINT8 *pSrc, IN const INT32 nSrcWidth, IN const INT32 nSrcHeight, IN const INT32 nSrcStride,
                                 OUT UINT8 *pDst, IN const INT32 nDstWidth, IN const INT32 nDstHeight, IN const INT32 nDstStride)
{
    INT32                   j = 0;
    INT32                   nSrcStride4 = (nSrcStride << 2);
    
    for(j=0 ; j<nDstHeight ; j++)
    {
        OneRow_Subsample_1_4(pSrc, pDst, nDstWidth);
        pSrc += nSrcStride4;
        pDst += nDstStride;
    }
}


static void _Scale_Subsample_Arbitrary(IN const UINT8 *pSrc, IN const INT32 nSrcWidth, IN const INT32 nSrcHeight, IN const INT32 nSrcStride,
                                       OUT UINT8 *pDst, IN const INT32 nDstWidth, IN const INT32 nDstHeight, IN const INT32 nDstStride)
{
    INT32                   i = 0, j = 0;
    INT32                   dx = (nSrcWidth << 16) / nDstWidth;
    INT32                   dy = (nSrcHeight << 16) / nDstHeight;
    INT32                   y = (dy >= 65536) ? ((dy >> 1) - 32768) : (dy >> 1);
    
    for(j=0 ; j<nDstHeight ; j++)
    {
        INT32               x = (dx >= 65536) ? ((dx >> 1) - 32768) : (dx >> 1);
        INT32               yi = y >> 16;
        const UINT8         *pLocalSrc = pSrc + yi * nSrcStride;
        UINT8               *pLocalDst = pDst;
        
        for(i=0 ; i<nDstWidth ; i++)
        {
            *pLocalDst++ = pLocalSrc[x >> 16];
            x += dx;
        }
        pDst += nDstStride;
        y += dy;
    }
}


static Kakao_Status _Scaler_SubsampleCoreByte(IN const UINT8 *pSrc, IN const INT32 nSrcWidth, IN const INT32 nSrcHeight, IN const INT32 nSrcStride,
                                              OUT UINT8 *pDst, IN const INT32 nDstWidth, INT32 nDstHeight, IN const INT32 nDstStride)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    if((nSrcWidth == (2 * nDstWidth)) && (nSrcHeight == (2 * nDstHeight)))
    {
        // DownScaling 1/2
        _Scale_Subsample_1_2(pSrc, nSrcWidth, nSrcHeight, nSrcStride, pDst, nDstWidth, nDstHeight, nDstStride);
    }
    else if((nSrcWidth == (4 * nDstWidth)) && (nSrcHeight == (4 * nDstHeight)))
    {
        // DownScaling 1/4
        _Scale_Subsample_1_4(pSrc, nSrcWidth, nSrcHeight, nSrcStride, pDst, nDstWidth, nDstHeight, nDstStride);
    }
    else
    {
        // DownScaling Arbitrary Size
        _Scale_Subsample_Arbitrary(pSrc, nSrcWidth, nSrcHeight, nSrcStride, pDst, nDstWidth, nDstHeight, nDstStride);
    }
    
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _Scaler_BilinearCoreByte(IN const UINT8 *pSrc, IN const INT32 nSrcWidth, IN const INT32 nSrcHeight, IN const INT32 nSrcStride,
                                             OUT UINT8 *pDst, IN const INT32 nDstWidth, IN const INT32 nDstHeight, IN const INT32 nDstStride)
#if 1
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    UINT8                   nRow[1920 + 16];
    
    INT32           dx = (nSrcWidth << 16) / nDstWidth;
    INT32           dy = (nSrcHeight << 16) / nDstHeight;
    INT32           x = (dx >= 65536) ? ((dx >> 1) - 32768) : (dx >> 1);
    INT32           y = (dy >= 65536) ? ((dy >> 1) - 32768) : (dy >> 1);
    INT32           maxy = (nSrcHeight > 1) ? ((nSrcHeight - 1) << 16) - 1 : 0;
    INT32           j = 0;
    
    for(j=0 ; j<nDstHeight ; ++j)
    {
        INT32               yi = y >> 16;
        INT32               yf = (y >> 8) & 255;
        const UINT8         *pLocalSrc = pSrc + yi * nSrcStride;
        
        OneRow_ScaleBilinear(nRow, pLocalSrc, nSrcStride, nSrcWidth, yf);
        OneCol_ScaleBilinear(pDst, nRow, nDstWidth, x, dx);
        pDst += nDstStride;
        y += dy;
        if (y > maxy)
            y = maxy;
    }
    
    nRet = KAKAO_STAT_OK;

    return nRet;
}
#else
{
    INT32                   i = 0, j = 0;
    INT32                   nDeltaX = 0, nDeltaY = 0;
    INT32                   nSrcCurrPos0 = 0, nSrcCurrPos1 = 0, nSrcNextPos0 = 0, nSrcNextPos1 = 0, nDstPos = 0;
    INT32                   nXCurrOffset = 0, nXNextOffset = 0;
    INT32                   nYCurrOffset = 0, nYNextOffset = 0;
    F32                     nRatioX = FLOATPRECISION * ((F32)(nSrcWidth) / (F32)(nDstWidth));
    F32                     nRatioY = FLOATPRECISION * ((F32)(nSrcHeight) / (F32)(nDstHeight));
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    for(j=0 ; j<nDstHeight ; j++)
    {
        nYCurrOffset = ((UINT32)(j * nRatioY) >> FLOATFACTOR) * nSrcStride;
        nYNextOffset = ((UINT32)((j + 1) * nRatioY) >> FLOATFACTOR) * nSrcStride;
        nDeltaY = ((UINT32)(j * nRatioY) & FLOATMASK);
        
        nDstPos = j * nDstStride;
        
        for(i=0 ; i<nDstWidth ; i++)
        {
            nXCurrOffset = ((UINT32)(i * nRatioX) >> FLOATFACTOR);
            nXNextOffset = ((UINT32)((i + 1) * nRatioX) >> FLOATFACTOR);
            nDeltaX = ((UINT32)(i * nRatioX) & FLOATMASK);
            
            nSrcCurrPos0 = nYCurrOffset + nXCurrOffset;
            nSrcCurrPos1 = nYCurrOffset + nXNextOffset;
            nSrcNextPos0 = nYNextOffset + nXCurrOffset;
            nSrcNextPos1 = nYNextOffset + nXNextOffset;
            
            pDst[nDstPos] = (UINT8)(((FLOATPRECISION - nDeltaX)*(FLOATPRECISION - nDeltaY)*pSrc[nSrcCurrPos0]
                                 +  (                 nDeltaX)*(FLOATPRECISION - nDeltaY)*pSrc[nSrcCurrPos1]
                                 +  (FLOATPRECISION - nDeltaX)*(                 nDeltaY)*pSrc[nSrcNextPos0]
                                 +  (                 nDeltaX)*(                 nDeltaY)*pSrc[nSrcNextPos1]) >> (2 * FLOATFACTOR));
            
            nDstPos++;
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}
#endif

static Kakao_Status _Scaler_BilinearCoreDouble(IN const D64 *pSrc, D64 *pDst, IN const INT32 nWidth, IN const INT32 nHeight,
                                               IN const INT32 nSrcStride, IN const INT32 nDstStride, IN const F32 nRatioX, IN const F32 nRatioY)
{
    INT32                   i = 0, j = 0;
    INT32                   nDeltaX = 0, nDeltaY = 0;
    INT32                   nSrcCurrPos0 = 0, nSrcCurrPos1 = 0, nSrcNextPos0 = 0, nSrcNextPos1 = 0, nDstPos = 0;
    INT32                   nXCurrOffset = 0, nXNextOffset = 0;
    INT32                   nYCurrOffset = 0, nYNextOffset = 0;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    // TODO: Make Table of Interpolated KIPoint
    
    for(j=0 ; j<nHeight ; j++)
    {
        nYCurrOffset = ((UINT32)(j * nRatioY) >> FLOATFACTOR) * nSrcStride;
        nYNextOffset = ((UINT32)((j + 1) * nRatioY) >> FLOATFACTOR) * nSrcStride;
        nDeltaY = ((UINT32)(j * nRatioY) & FLOATMASK);
        
        nDstPos = j * nDstStride;
        
        for(i=0 ; i<nWidth ; i++)
        {
            nXCurrOffset = ((UINT32)(i * nRatioX) >> FLOATFACTOR);
            nXCurrOffset = ((UINT32)((i + 1) * nRatioX) >> FLOATFACTOR);
            nDeltaX = ((UINT32)(i * nRatioX) & FLOATMASK);
            
            nSrcCurrPos0 = nYCurrOffset + nXCurrOffset;
            nSrcNextPos0 = nYNextOffset + nXCurrOffset;
            nSrcCurrPos1 = nYCurrOffset + nXNextOffset;
            nSrcNextPos1 = nYNextOffset + nXNextOffset;
            
            pDst[nDstPos] = (((FLOATPRECISION - nDeltaX)*(FLOATPRECISION - nDeltaY)*pSrc[nSrcCurrPos0]
                         +  (                 nDeltaX)*(FLOATPRECISION - nDeltaY)*pSrc[nSrcCurrPos1]
                         +  (FLOATPRECISION - nDeltaX)*(                 nDeltaY)*pSrc[nSrcNextPos0]
                         +  (                 nDeltaX)*(                 nDeltaY)*pSrc[nSrcNextPos1]) / (FLOATPRECISION * FLOATPRECISION));
            
            nDstPos++;
        }
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _Scaler_Subsample_Gray(IN const ImgScaler_T *pScaler, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    INT32                   nSrcWidth = 0, nSrcHeight = 0;
    INT32                   nDstWidth = 0, nDstHeight = 0;
    INT32                   nSrcStride = 0, nDstStride = 0;
    UINT8                   *pSrc = NULL, *pDst = NULL;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    pSrc = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
    pDst = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0);
    nSrcWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    nSrcHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    nDstWidth = pDstImg->GetImgWidth(pDstImg, IMG_PLANE_0);
    nDstHeight = pDstImg->GetImgHeight(pDstImg, IMG_PLANE_0);
    nSrcStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    nDstStride = pDstImg->GetImgStride(pDstImg, IMG_PLANE_0);
    
    _Scaler_SubsampleCoreByte(pSrc, nSrcWidth, nSrcHeight, nSrcStride, pDst, nDstWidth, nDstHeight, nDstStride);
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _Scaler_Subsample_I420(IN const ImgScaler_T *pScaler, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    INT32                   i;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    for(i=0 ; i<pSrcImg->GetNumofImgPlanes(pSrcImg) ; i++)
    {
        INT32                   nSrcWidth = pSrcImg->GetImgWidth(pSrcImg, i);
        INT32                   nSrcHeight = pSrcImg->GetImgHeight(pSrcImg, i);
        INT32                   nDstWidth = pDstImg->GetImgWidth(pDstImg, i);
        INT32                   nDstHeight = pDstImg->GetImgHeight(pDstImg, i);
        INT32                   nSrcStride = pSrcImg->GetImgStride(pSrcImg, i);
        INT32                   nDstStride = pDstImg->GetImgStride(pDstImg, i);
        UINT8                   *pSrc = NULL, *pDst = NULL;
        
        pSrc = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, i);
        pDst = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, i);
        
        _Scaler_SubsampleCoreByte(pSrc, nSrcWidth, nSrcHeight, nSrcStride, pDst, nDstWidth, nDstHeight, nDstStride);
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _Scaler_Bilinear_Gray(IN const ImgScaler_T *pScaler, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    INT32                   nSrcWidth = 0, nSrcHeight = 0;
    INT32                   nDstWidth = 0, nDstHeight = 0;
    INT32                   nSrcStride = 0, nDstStride = 0;
    UINT8                   *pSrc = NULL, *pDst = NULL;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    pSrc = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
    pDst = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0);
    nSrcWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    nSrcHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    nDstWidth = pDstImg->GetImgWidth(pDstImg, IMG_PLANE_0);
    nDstHeight = pDstImg->GetImgHeight(pDstImg, IMG_PLANE_0);
    nSrcStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    nDstStride = pDstImg->GetImgStride(pDstImg, IMG_PLANE_0);
    
    _Scaler_BilinearCoreByte(pSrc, nSrcWidth, nSrcHeight, nSrcStride, pDst, nDstWidth, nDstHeight, nDstStride);
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _Scaler_Bilinear_I420(IN const ImgScaler_T *pScaler, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    UINT32                  i;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    for(i=0 ; i<pSrcImg->GetNumofImgPlanes(pSrcImg) ; i++)
    {
        INT32               nSrcWidth = pSrcImg->GetImgWidth(pSrcImg, i);
        INT32               nSrcHeight = pSrcImg->GetImgHeight(pSrcImg, i);
        INT32               nDstWidth = pDstImg->GetImgWidth(pDstImg, i);
        INT32               nDstHeight = pDstImg->GetImgHeight(pDstImg, i);
        INT32               nSrcStride = pSrcImg->GetImgStride(pSrcImg, i);
        INT32               nDstStride = pDstImg->GetImgStride(pDstImg, i);
        UINT8               *pSrc = NULL, *pDst = NULL;
        
        pSrc = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, i);
        pDst = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, i);
        
        _Scaler_BilinearCoreByte(pSrc, nSrcWidth, nSrcHeight, nSrcStride, pDst, nDstWidth, nDstHeight, nDstStride);
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _Scaler_Bilinear_HSV(IN const ImgScaler_T *pScaler, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    INT32                   i;
    INT32                   nWidth = pDstImg->GetImgWidth(pDstImg, IMG_PLANE_0), nHeight = pDstImg->GetImgHeight(pDstImg, IMG_PLANE_0);
    INT32                   nSrcStride = 0, nDstStride = 0;
    F32                     nRatioX = FLOATPRECISION * ((F32)(pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0)) / (F32)(pDstImg->GetImgWidth(pDstImg, IMG_PLANE_0)));
    F32                     nRatioY = FLOATPRECISION * ((F32)(pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0)) / (F32)(pDstImg->GetImgHeight(pDstImg, IMG_PLANE_0)));
    D64                     *pSrc = NULL, *pDst = NULL;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    for(i=0 ; i<pSrcImg->GetNumofImgPlanes(pSrcImg) ; i++)
    {
        pSrc = (D64 *)pSrcImg->GetImgPlanes(pSrcImg, i);
        pDst = (D64 *)pDstImg->GetImgPlanes(pDstImg, i);
        nSrcStride = pSrcImg->GetImgStride(pSrcImg, i);
        nDstStride = pDstImg->GetImgStride(pDstImg, i);
        
        _Scaler_BilinearCoreDouble(pSrc, pDst, nWidth, nHeight, nSrcStride, nDstStride, nRatioX, nRatioY);
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}


static Kakao_Status _Scaler_Bilinear_HSL(IN const ImgScaler_T *pScaler, IN const Img_T *pSrcImg, OUT Img_T *pDstImg)
{
    INT32                   i;
    INT32                   nWidth = pDstImg->GetImgWidth(pDstImg, IMG_PLANE_0), nHeight = pDstImg->GetImgHeight(pDstImg, IMG_PLANE_0);
    INT32                   nSrcStride = 0, nDstStride = 0;
    F32                     nRatioX = FLOATPRECISION * ((F32)(pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0)) / (F32)(pDstImg->GetImgWidth(pDstImg, IMG_PLANE_0)));
    F32                     nRatioY = FLOATPRECISION * ((F32)(pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0)) / (F32)(pDstImg->GetImgHeight(pDstImg, IMG_PLANE_0)));
    D64                     *pSrc = NULL, *pDst = NULL;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    for(i=0 ; i<pSrcImg->GetNumofImgPlanes(pSrcImg) ; i++)
    {
        pSrc = (D64 *)pSrcImg->GetImgPlanes(pSrcImg, i);
        pDst = (D64 *)pDstImg->GetImgPlanes(pDstImg, i);
        nSrcStride = pSrcImg->GetImgStride(pSrcImg, i);
        nDstStride = pDstImg->GetImgStride(pDstImg, i);
        
        _Scaler_BilinearCoreDouble(pSrc, pDst, nWidth, nHeight, nSrcStride, nDstStride, nRatioX, nRatioY);
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
}











