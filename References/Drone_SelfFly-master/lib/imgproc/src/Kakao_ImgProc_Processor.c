

#if 0
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
INT32 Input(Preproc_T *pPreproc)
{
    KISize                  nPaddedSizeY, nPaddedSizeUV;
    Img_T                   *pDst = &pPreproc->pOutPic_plane;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    
    /* -----------------------------------------
    /               Allocate Resize memory
    /-----------------------------------------*/
    if(KAKAO_STAT_OK != pPreproc->nCheckMemAllocated)
    {
        INT32     nImg_SizeY, nImg_SizeUV;
        
        nPaddedSizeY.nWidth = pPreproc->nInYSize.nPaddedWidth;
        nPaddedSizeY.nHeight = pPreproc->nInYSize.nPaddedHeight;
        nPaddedSizeUV.nWidth = pPreproc->nInUVSize.nPaddedWidth;
        nPaddedSizeUV.nHeight = pPreproc->nInUVSize.nPaddedHeight;
        
        nImg_SizeY = (nPaddedSizeY.nWidth) * (nPaddedSizeY.nHeight);
        nImg_SizeUV = (nPaddedSizeUV.nWidth) * (nPaddedSizeUV.nHeight);
        
        SAFEALLOC(pDst->pFrame_Y, nImg_SizeY, 32, UINT8);
        SAFEALLOC(pDst->pFrame_Cb, nImg_SizeUV, 32, UINT8);
        SAFEALLOC(pDst->pFrame_Cr, nImg_SizeUV, 32, UINT8);
        SAFEALLOC(pPreproc->pROI, (nImg_SizeY>>8), 32, UINT8);
        
        pPreproc->nCheckMemAllocated = KAKAO_STAT_OK;
    }
    else if(KAKAO_STAT_OK == pPreproc->nCheckMemAllocated)
    {              
        SAFEFREE(pDst->pFrame_Y);
        SAFEFREE(pDst->pFrame_Cb);
        SAFEFREE(pDst->pFrame_Cr);
        SAFEFREE(pPreproc->pROI);
        
        pPreproc->nCheckMemAllocated = KAKAO_STAT_FAIL;        
    }

    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


INT32 Input(Preproc_T *pPreproc)
{
    KISize                  nPaddedSizeY, nPaddedSizeUV;
    Img_T                   *pDst = &pPreproc->pOutPic_plane;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    
    /* -----------------------------------------
     /               Allocate Resize memory
     /-----------------------------------------*/
    if(KAKAO_STAT_OK != pPreproc->nCheckMemAllocated)
    {
        INT32     nImg_SizeY, nImg_SizeUV;
        
        nPaddedSizeY.nWidth = pPreproc->nInYSize.nPaddedWidth;
        nPaddedSizeY.nHeight = pPreproc->nInYSize.nPaddedHeight;
        nPaddedSizeUV.nWidth = pPreproc->nInUVSize.nPaddedWidth;
        nPaddedSizeUV.nHeight = pPreproc->nInUVSize.nPaddedHeight;
        
        nImg_SizeY = (nPaddedSizeY.nWidth) * (nPaddedSizeY.nHeight);
        nImg_SizeUV = (nPaddedSizeUV.nWidth) * (nPaddedSizeUV.nHeight);
        
        SAFEALLOC(pDst->pFrame_Y, nImg_SizeY, 32, UINT8);
        SAFEALLOC(pDst->pFrame_Cb, nImg_SizeUV, 32, UINT8);
        SAFEALLOC(pDst->pFrame_Cr, nImg_SizeUV, 32, UINT8);
        SAFEALLOC(pPreproc->pROI, (nImg_SizeY>>8), 32, UINT8);
        
        pPreproc->nCheckMemAllocated = KAKAO_STAT_OK;
    }
    else if(KAKAO_STAT_OK == pPreproc->nCheckMemAllocated)
    {
        SAFEFREE(pDst->pFrame_Y);
        SAFEFREE(pDst->pFrame_Cb);
        SAFEFREE(pDst->pFrame_Cr);
        SAFEFREE(pPreproc->pROI);
        
        pPreproc->nCheckMemAllocated = KAKAO_STAT_FAIL;
    }
    
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


void Init_FuncParam(Preproc_T *pPreproc, IN OUT struct _Preproc_Interface *Pre_Interface)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    Blur_Gaussian_Filter_MakeCoeffi(&pPreproc->nGaussianBlurMaskSize, pPreproc->nSigma, &pPreproc->nTemp_Filter[0][0]);

    pPreproc->EdgeDetInfo.nGaussianBlurMaskSize = pPreproc->nGaussianBlurMaskSize;
    pPreproc->EdgeDetInfo.nTemp_Filter= &pPreproc->nTemp_Filter[0][0];

    {   
        INT32     nFilterSize = 0;
        INT32     i;

        for(i=0 ; i<5 ; i++)
        {
            Blur_Gaussian_Filter_MakeCoeffi(&nFilterSize, (pPreproc->nSigma/(6/pow(2,i))), &pPreproc->nTemp_Filter[i+1][0]);
            pPreproc->ImgEnhanceInfo.nFilterCoeffi[i] = &pPreproc->nTemp_Filter[i+1][0];
            pPreproc->ImgEnhanceInfo.nFilterSize[i] = nFilterSize;
        }        
    }
    
    return;
}


INT32 Init_InputImg_Param(Preproc_T *pPreproc, IN OUT struct _Preproc_Interface *Pre_Interface)
{
    INT32                   nRet = KAKAO_STAT_OK;
    INT32                   nPaddingSize = 0;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    // Set Padding KISize
    nPaddingSize = pPreproc->nPaddingSize = MACRO_BLOCK_SIZE;

    // Set Input Image KISize
    pPreproc->nInYSize.nWidth = Pre_Interface->nInWidth;
    pPreproc->nInYSize.nHeight = Pre_Interface->nInHeight;
    pPreproc->nInYSize.nPaddedWidth = Pre_Interface->nInWidth + 2*nPaddingSize;
    pPreproc->nInYSize.nPaddedHeight = Pre_Interface->nInHeight + 2*nPaddingSize;

    pPreproc->nInUVSize.nWidth = (Pre_Interface->nInWidth / 2);
    pPreproc->nInUVSize.nHeight = (Pre_Interface->nInHeight/ 2);    
    pPreproc->nInUVSize.nPaddedWidth = pPreproc->nInUVSize.nWidth + nPaddingSize;
    pPreproc->nInUVSize.nPaddedHeight = pPreproc->nInUVSize.nHeight + nPaddingSize;

    
    // Set Output Image KISize
    pPreproc->nOutYSize.nWidth = Pre_Interface->nOutWidth;
    pPreproc->nOutYSize.nHeight = Pre_Interface->nOutHeight;
    pPreproc->nOutYSize.nPaddedWidth = Pre_Interface->nOutWidth + 2*nPaddingSize;
    pPreproc->nOutYSize.nPaddedHeight = Pre_Interface->nOutHeight + 2*nPaddingSize;

    pPreproc->nOutUVSize.nWidth = (Pre_Interface->nOutWidth / 2);
    pPreproc->nOutUVSize.nHeight = (Pre_Interface->nOutHeight/ 2);    
    pPreproc->nOutUVSize.nPaddedWidth = pPreproc->nOutUVSize.nWidth + nPaddingSize;
    pPreproc->nOutUVSize.nPaddedHeight = pPreproc->nOutUVSize.nHeight + nPaddingSize;


    // Set Image KISize of each module
    pPreproc->pInPic_plane.nYSize.nWidth = Pre_Interface->nInWidth;
    pPreproc->pInPic_plane.nYSize.nHeight = Pre_Interface->nInHeight;
    pPreproc->pInPic_plane.nYSize.nPaddedWidth = Pre_Interface->nInWidth;
    pPreproc->pInPic_plane.nYSize.nPaddedHeight = Pre_Interface->nInHeight;    
    pPreproc->pInPic_plane.nUVSize.nWidth = (Pre_Interface->nInWidth / 2);
    pPreproc->pInPic_plane.nUVSize.nHeight = (Pre_Interface->nInHeight / 2);;
    pPreproc->pInPic_plane.nUVSize.nPaddedWidth = (Pre_Interface->nInWidth / 2);
    pPreproc->pInPic_plane.nUVSize.nPaddedHeight = (Pre_Interface->nInHeight / 2);
    pPreproc->pInPic_plane.nPaddingsize = 0;
    

    pPreproc->pOutPic_plane.nYSize = pPreproc->nOutYSize;
    pPreproc->pOutPic_plane.nUVSize = pPreproc->nOutUVSize;    
    pPreproc->pOutPic_plane.nPaddingsize = pPreproc->nPaddingSize;


    pPreproc->ImgEnhanceInfo.pEnhancedImg.nYSize = pPreproc->nOutYSize;
    pPreproc->ImgEnhanceInfo.pEnhancedImg.nUVSize = pPreproc->nOutUVSize;    
    pPreproc->ImgEnhanceInfo.pEnhancedImg.nPaddingsize = pPreproc->nPaddingSize;

    pPreproc->EdgeDetInfo.pEdgeDetectedImg.nYSize = pPreproc->nOutYSize;
    pPreproc->EdgeDetInfo.pEdgeDetectedImg.nUVSize = pPreproc->nOutUVSize;    
    pPreproc->EdgeDetInfo.pEdgeDetectedImg.nPaddingsize = pPreproc->nPaddingSize;
    
    pPreproc->SkinDetInfo.pSkinDetectedImg.nYSize = pPreproc->nOutYSize;
    pPreproc->SkinDetInfo.pSkinDetectedImg.nUVSize = pPreproc->nOutUVSize;    
    pPreproc->SkinDetInfo.pSkinDetectedImg.nPaddingsize = pPreproc->nPaddingSize;
    
    pPreproc->ColorCorrInfo.nColorCorrectorImg.nYSize = pPreproc->nOutYSize;
    pPreproc->ColorCorrInfo.nColorCorrectorImg.nUVSize = pPreproc->nOutUVSize;
    pPreproc->ColorCorrInfo.nColorCorrectorImg.nPaddingsize = pPreproc->nPaddingSize;
    
    pPreproc->ResizerInfo.pResizedImg.nYSize = pPreproc->nOutYSize;
    pPreproc->ResizerInfo.pResizedImg.nUVSize = pPreproc->nOutUVSize;    
    pPreproc->ResizerInfo.pResizedImg.nPaddingsize = pPreproc->nPaddingSize;
    
    pPreproc->SBDInfo.pSBDImg.nYSize = pPreproc->nOutYSize;
    pPreproc->SBDInfo.pSBDImg.nUVSize = pPreproc->nOutUVSize;    
    pPreproc->SBDInfo.pSBDImg.nPaddingsize = pPreproc->nPaddingSize;

    pPreproc->AutoROIDetect.pROIImg.nYSize = pPreproc->nOutYSize;
    pPreproc->AutoROIDetect.pROIImg.nUVSize = pPreproc->nOutUVSize;    
    pPreproc->AutoROIDetect.pROIImg.nPaddingsize = pPreproc->nPaddingSize;

    pPreproc->AutoFaceDetect.pFaceDetectedImg.nYSize.nWidth = 352;
    pPreproc->AutoFaceDetect.pFaceDetectedImg.nYSize.nHeight = 288;
    pPreproc->AutoFaceDetect.pFaceDetectedImg.nYSize.nPaddedWidth = 384;
    pPreproc->AutoFaceDetect.pFaceDetectedImg.nYSize.nPaddedHeight = 320;    
    pPreproc->AutoFaceDetect.pFaceDetectedImg.nUVSize.nWidth = 176;
    pPreproc->AutoFaceDetect.pFaceDetectedImg.nUVSize.nHeight = 144;
    pPreproc->AutoFaceDetect.pFaceDetectedImg.nUVSize.nPaddedWidth = 192;
    pPreproc->AutoFaceDetect.pFaceDetectedImg.nUVSize.nPaddedHeight = 160;
    pPreproc->AutoFaceDetect.pFaceDetectedImg.nPaddingsize = pPreproc->nPaddingSize;

    pPreproc->VisualAttend.pVisialAttension.nWidth = 512;
    pPreproc->VisualAttend.pVisialAttension.nHeight = 512;    
    pPreproc->VisualAttend.pVisualAttendImg.nYSize = pPreproc->nOutYSize;
    pPreproc->VisualAttend.pVisualAttendImg.nUVSize = pPreproc->nOutUVSize;
    pPreproc->VisualAttend.pVisualAttendImg.nPaddingsize = pPreproc->nPaddingSize;


    // Set Edge detection Mode
    pPreproc->EdgeDetInfo.nEdgeMode = pPreproc->nEdgeMode = Canny;

    
    pPreproc->nSigma = pPreproc->ImgEnhanceInfo.nSigma = pPreproc->EdgeDetInfo.nSigma = 0.6;

    return nRet;
}


Kakao_Status Pre_ReadDataFromAPP(Preproc_T *pPreproc, IN OUT struct _Preproc_Interface *Pre_Interface)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    // Set Input Image pointer
    if(Pre_Interface->pInPic_plane[0] != NULL)
    {
        pPreproc->pInPic_plane.pFrame_Y = Pre_Interface->pInPic_plane[0];
        pPreproc->pInPic_plane.pFrame_Cb = Pre_Interface->pInPic_plane[1];
        pPreproc->pInPic_plane.pFrame_Cr = Pre_Interface->pInPic_plane[2];        
    }
                                 
    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}


Kakao_Status Pre_WriteDataToEncoder(Preproc_T *pPreproc, IN OUT struct _Preproc_Interface *Pre_Interface)
{
    Kakao_Status            nRet = KAKAO_STAT_FAIL;

    // Data to Encoder
    Pre_Interface->nOutWidth = pPreproc->nOutYSize.nWidth;                                                          // Resized width size
    Pre_Interface->nOutHeight = pPreproc->nOutYSize.nHeight;                                                        // Resized height size    

    Pre_Interface->pEdgeMBwiseMap = &pPreproc->EdgeDetInfo.pEdgeDetectMBwiseMap0[0];             // Detected Edge map (MBwise)
    Pre_Interface->pSkinMBwiseMap = &pPreproc->SkinDetInfo.pMBwiseMap0[0];               // Detected Skin map (MBwise)
    Pre_Interface->pMVInfoMBwiseMapEnc = &pPreproc->SBDInfo.pMVInfoMBwise[0];                     // Estimated motion vector map (MBwise)
    Pre_Interface->pImgAnalysis[0] = &pPreproc->SBDInfo.pImgAnalysis[0][0];
    Pre_Interface->pImgAnalysis[1] = &pPreproc->SBDInfo.pImgAnalysis[1][0];  
    Pre_Interface->pROIMBwiseMapEnc = &pPreproc->AutoROIDetect.pAutoROIMBwiseMap[0];
    Pre_Interface->nForceIntraFlag = pPreproc->SBDInfo.nForceIntraFlag;                                         // Reference info for RC       

    Pre_Interface->pOutPic_plane[0] = &pPreproc->ImgEnhanceInfo.pEnhancedImg.pFrame_Y[0];//&pPreproc->pOutPic_plane.pFrame_Y[0];                                // Output Image Y
    Pre_Interface->pOutPic_plane[1] = &pPreproc->ImgEnhanceInfo.pEnhancedImg.pFrame_Cb[0];//&pPreproc->pOutPic_plane.pFrame_Cb[0];                              // Output Image U
    Pre_Interface->pOutPic_plane[2] = &pPreproc->ImgEnhanceInfo.pEnhancedImg.pFrame_Cr[0];//&pPreproc->pOutPic_plane.pFrame_Cr[0];                               // Output Image V

    nRet = KAKAO_STAT_OK;
    return nRet;
    
Error:
    return nRet;
}
#endif

