


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
#if DO_VISUALATTEND
INT32 VisualAttention_Init(Preproc_T *pPreproc, VisualAttend_T *VisualAttend)
{
    Img_T      *pSrc = &VisualAttend->pVisualAttendImg;
    INT32             nImg_SizeY, nImg_SizeUV;
    KISize            nPaddedSizeY, nPaddedSizeUV;
    
    nPaddedSizeY = VisualAttend->pVisualAttendImg.nYSize;
    nPaddedSizeUV = VisualAttend->pVisualAttendImg.nUVSize;
    
    nImg_SizeY = (nPaddedSizeY.nPaddedWidth) * (nPaddedSizeY.nPaddedHeight);
    nImg_SizeUV = (nPaddedSizeUV.nPaddedWidth) * (nPaddedSizeUV.nPaddedHeight);        
    
    SAFEALLOC(VisualAttend->pVisialAttension.pIn, (VisualAttend->pVisialAttension.nWidth*VisualAttend->pVisialAttension.nHeight), 32, UINT8);
    SAFEALLOC(VisualAttend->pVisualAttendMBwiseMap, (nImg_SizeY>>8), 32, UINT8);
    SAFEALLOC(pSrc->pFrame_Y, nImg_SizeY, 32, UINT8);
    SAFEALLOC(pSrc->pFrame_Cb, nImg_SizeUV, 32, UINT8);
    SAFEALLOC(pSrc->pFrame_Cr, nImg_SizeUV, 32, UINT8);          
            
    Kakao_VC_Init_VisualAttention(&(VisualAttend->pHandle), &(VisualAttend->pVisialAttension));

    return KAKAO_STAT_OK;
}


INT32 VisualAttention_Destroy(Preproc_T *pPreproc, VisualAttend_T *VisualAttend)
{
    Img_T        *pSrc = &VisualAttend->pVisualAttendImg;

    SAFEFREE(VisualAttend->pVisialAttension.pIn);
    SAFEFREE(VisualAttend->pVisualAttendMBwiseMap);
    SAFEFREE(pSrc->pFrame_Y);
    SAFEFREE(pSrc->pFrame_Cb);
    SAFEFREE(pSrc->pFrame_Cr);
    
    Kakao_VC_Destroy_VisualAttention(VisualAttend->pHandle, &(VisualAttend->pVisialAttension));
        
    return KAKAO_STAT_OK;
}


void SetHighVisualAtten(UINT8 *ROIImage, UINT8 *pROIMBwiseMap, KISize *nSize)
{
    INT32     nPaddedWidth = nSize->nPaddedWidth;
    INT32     nPaddedHeight = nSize->nPaddedHeight;
    INT32     nROICount;
    INT32     nPos;
    INT32     i, j, x, y;

    nPos = (nPaddedWidth>>4) + 1;
    for(y=MACRO_BLOCK_SIZE ; y<nPaddedHeight-MACRO_BLOCK_SIZE ; y+=MACRO_BLOCK_SIZE)
    {
        for(x=MACRO_BLOCK_SIZE ; x<nPaddedWidth-MACRO_BLOCK_SIZE ; x+=MACRO_BLOCK_SIZE)
        {
            nROICount = 0;
            for(j=0 ; j<MACRO_BLOCK_SIZE ; j++)
            {
                for(i=0 ; i<MACRO_BLOCK_SIZE ; i++)
                {
                    if(ROIImage[(y+j)*nPaddedWidth + (x+i)] >= 60)
                        nROICount++;
                }
            }

            if(nROICount > 100)
                pROIMBwiseMap[nPos] = 1;
            else
                pROIMBwiseMap[nPos] = 0;

            nPos++;
        }
        nPos += 2;
    }    
}
 


void Kakao_VC_VisualAttend_Filter(ImgScaler_T *Resizer, VisualAttend_T *VisualAttend)
{
    KISize       nSize;
    UINT8   *pSrcY = Resizer->pResizedImg.pFrame_Y;
    UINT8   *pDstY = VisualAttend->pVisualAttendImg.pFrame_Y;
    UINT8   *pDstU = VisualAttend->pVisualAttendImg.pFrame_Cb;
    UINT8   *pDstV = VisualAttend->pVisualAttendImg.pFrame_Cr;
    UINT8   *pS_FeatureMap = NULL;

    nSize.nWidth = nSize.nPaddedWidth = VisualAttend->pVisialAttension.nWidth;
    nSize.nHeight = nSize.nPaddedHeight = VisualAttend->pVisialAttension.nHeight;    
    Resize_Bilinear(pSrcY, VisualAttend->pVisialAttension.pIn, &Resizer->pResizedImg.nYSize, &nSize);

    pS_FeatureMap = (UINT8 *)Kakao_VC_VisualAttention(VisualAttend->pHandle, &(VisualAttend->pVisialAttension));     

    nSize.nWidth = nSize.nPaddedWidth = VisualAttend->pVisialAttension.nWidth / 4;
    nSize.nHeight = nSize.nPaddedHeight = VisualAttend->pVisialAttension.nHeight / 4;
    Resize_Bilinear(pS_FeatureMap, pDstY, &nSize, &VisualAttend->pVisualAttendImg.nYSize);
    MEMSET(pDstU, 128, (VisualAttend->pVisualAttendImg.nYSize.nPaddedWidth*VisualAttend->pVisualAttendImg.nYSize.nPaddedHeight/4));
    MEMSET(pDstV, 128, (VisualAttend->pVisualAttendImg.nYSize.nPaddedWidth*VisualAttend->pVisualAttendImg.nYSize.nPaddedHeight/4));

    SetHighVisualAtten(pDstY, VisualAttend->pVisualAttendMBwiseMap, &VisualAttend->pVisualAttendImg.nYSize);       
}

#endif







