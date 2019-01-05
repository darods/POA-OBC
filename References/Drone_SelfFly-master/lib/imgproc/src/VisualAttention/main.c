

#include    <stdio.h>
#include    <stdlib.h>
#include    <memory.h>
#include    <cv.h>
#include    <cxcore.h>
#include    <highgui.h>
#include    <math.h>
#include    <vfw.h>
#include    "PT_VisualAttention_Export.h"



int Load_YUVFromFile(FILE *fp, int nWidth, int nHeight, unsigned char *pDstY);
int Save_DecodedFrameToYUV(FILE *fp,  int nWidth, int nHeight, unsigned char *pSrcY);
void ColorConvert_YUV420toRGB(unsigned char *pInYUV, unsigned char *pRGB, int nWidth, int nHeight);
void ColorConvert_YUV420toRGB_ForOpenCV(unsigned char *pInYUV, unsigned char *pRGB, int nWidth, int nHeight);
void ColorConvert_RGBtoYUV420(unsigned char *pInYUV, unsigned char *pRGB, int nWidth, int nHeight);
void Resize_Bilinear(unsigned char *pSrc, unsigned char *pDst, int  nSrcWidth, int nSrcHeight, int nDstWidth, int nDstHeight);
void Show_ROIImg(IplImage *pInImg, IplImage *pOutImg, unsigned char *pInYUV, unsigned char *pOutResizedYUV, unsigned char *pROIYUV, int nWidth, int nHeight);



#define IN_YUV_FILE                  "../export/input/src_mcube_new.yuv"
#define OUT_STRM_FILE             "../export/input/Test_Seq_352x288_Out.yuv"
#define     FLOATFACTOR                 (10)
#define     FLOATPRECISION            (1 << FLOATFACTOR)
#define     FLOATMASK                     (FLOATPRECISION - 1)
#define     Clip3(nMin, nMax, x)         x > nMax ? nMax : (x < nMin ? nMin : x)
#define     FWRITE(buffer, sizeoftype, sizeofdata, filepointer)   fwrite(buffer, sizeoftype, sizeofdata, filepointer) 
#define     MEMCPY(dst, src, length)                                             memcpy(dst, src, length)
#define     MEMSET(dst, var, length)                                             memset(dst, var, length)
#define     SAFEALLOC(pHandle, size, type)\
                                    {\
                                        type *Handle = (type *) calloc(size, sizeof(type));\
                                        if(NULL != Handle)\
                                        {\
                                            MEMSET(Handle, 0, (size*sizeof(type)));\
                                            pHandle = Handle;\
                                         }\
                                         else\
                                            pHandle = NULL;\
                                     }
#define     SAFEFREE(x)\
                                    if(NULL != x)\
                                    {\
                                        free(x);\
                                        x = NULL;\
                                    }



#define     INPUT_IMG           "Input_Img"
#define     OUTPUT_IMG           "Output_Img"
#define     lW      512
#define     lH      512


void main()
{
    int             nSrcWidth, nSrcHeight; //color imageÀÇ height¿Í width
    int             nDstWidth, nDstHeight;
    int             i, j;
    int             nFrameCount = 0;
    int             nPos;
    unsigned char *pInYUV, *pOutYUV, *pInRGB, *pInResizedRYUV, *pOutResizedYUV, *pROIYUV;
    unsigned char *pI_FeatureMap, *pC_FeatureMap, *pO_FeatureMap, *pS_FeatureMap;
    double      var0, var1, var2, nTemp=0;
    double      t, temp;    
    PT_VisualAttention_Struct   pVisialAttension;
    void    *pHandle = NULL;
    IplImage    *pInImg, *pOutImg;
    CvSize      nSizeY;

    FILE *pSrcFile;
    FILE *pDstFile;

    nSrcWidth = 320;
    nSrcHeight = 240;
    nDstWidth = lW;
    nDstHeight = lH;
 
    if((pSrcFile = fopen(IN_YUV_FILE, "rb")) == NULL) 
    {
        printf("Error: %s open failed.\n", IN_YUV_FILE);
        exit(0);
    }
    else
        printf("     2. Open %s file\n", IN_YUV_FILE);  

    if( (pDstFile = fopen(OUT_STRM_FILE, "wb+")) == NULL ) 
    {
        printf("Error: %s open failed.\n", OUT_STRM_FILE);
        exit(0);
    }
    else
        printf("     3. Open %s file\n", OUT_STRM_FILE);


    nSizeY.width = nSrcWidth;
    nSizeY.height = nSrcHeight;
    pInImg = cvCreateImage(nSizeY, IPL_DEPTH_8U, 3);
    pOutImg = cvCreateImage(nSizeY, IPL_DEPTH_8U, 3);
    cvNamedWindow(INPUT_IMG, CV_WINDOW_AUTOSIZE);
    cvNamedWindow(OUTPUT_IMG, CV_WINDOW_AUTOSIZE);
    SAFEALLOC(pROIYUV, (int)(1.5*nSrcHeight*nSrcWidth), unsigned char);
    SAFEALLOC(pInRGB, (3*nDstWidth*nDstHeight), unsigned char);
    SAFEALLOC(pInResizedRYUV, (3*nDstWidth*nDstHeight/2), unsigned char);    
    SAFEALLOC(pInYUV, (int)(1.5*nSrcHeight*nSrcWidth), unsigned char);    
    SAFEALLOC(pOutYUV, (int)(1.5*nDstWidth*nDstHeight/16), unsigned char);        
    SAFEALLOC(pOutResizedYUV, (int)(1.5*nSrcWidth*nSrcHeight), unsigned char);            

    pVisialAttension.nWidth = nDstWidth;
    pVisialAttension.nHeight = nDstHeight;
    pVisialAttension.pIn = pInRGB;

    PT_Init_VisualAttention(&pHandle, &pVisialAttension);

    while(nFrameCount < 400)
    {
        // Load Input Seq.
        Load_YUVFromFile(pSrcFile, nSrcWidth, nSrcHeight, pInYUV);

        Resize_Bilinear(pInYUV, pInResizedRYUV, nSrcWidth, nSrcHeight, nDstWidth, nDstHeight);        
        Resize_Bilinear(pInYUV+(int)(nSrcWidth*nSrcHeight), pInResizedRYUV+(nDstWidth*nDstHeight), nSrcWidth/2, nSrcHeight/2, nDstWidth/2, nDstHeight/2);        
        Resize_Bilinear(pInYUV+(int)(1.25*nSrcWidth*nSrcHeight), pInResizedRYUV+(int)(1.25*nDstWidth*nDstHeight), nSrcWidth/2, nSrcHeight/2, nDstWidth/2, nDstHeight/2);                
        
        ColorConvert_YUV420toRGB(pInResizedRYUV, pInRGB, nDstWidth, nDstHeight);
        
        pS_FeatureMap = (unsigned char *)PT_VisualAttention(pHandle, &pVisialAttension);

        ColorConvert_RGBtoYUV420(pOutYUV, pS_FeatureMap, nDstWidth/4, nDstHeight/4);

        Resize_Bilinear(pOutYUV, pOutResizedYUV, nDstWidth/4, nDstHeight/4, nSrcWidth, nSrcHeight);        
        Resize_Bilinear(pOutYUV+(int)(nDstWidth*nDstHeight/16), pOutResizedYUV+(nSrcWidth*nSrcHeight), nDstWidth/8, nDstHeight/8, nSrcWidth/2, nDstHeight/2);
        Resize_Bilinear(pOutYUV+(int)(1.25*nDstWidth*nDstHeight/16), pOutResizedYUV+(int)(1.25*nSrcWidth*nSrcHeight), nDstWidth/8, nDstHeight/8, nSrcWidth/2, nSrcHeight/2);

        Show_ROIImg(pInImg, pOutImg, pInYUV, pOutResizedYUV, pROIYUV, nSrcWidth, nSrcHeight);

        Save_DecodedFrameToYUV(pDstFile, nSrcWidth, nSrcHeight, pOutResizedYUV);
        //Save_DecodedFrameToYUV(pDstFile, nSrcWidth, nSrcHeight, pROIYUV);        

        printf("%d\n", nFrameCount);

        nFrameCount++;
    }

    PT_Destroy_VisualAttention(pHandle, &pVisialAttension);

    fclose(pSrcFile);
    fclose(pDstFile);

    cvDestroyWindow(INPUT_IMG);
    cvDestroyWindow(OUTPUT_IMG);

    
    SAFEFREE(pInYUV);
    SAFEFREE(pOutYUV);
    SAFEFREE(pInRGB);    
    SAFEFREE(pInResizedRYUV);
    SAFEFREE(pOutResizedYUV);
    SAFEFREE(pI_FeatureMap); 
    SAFEFREE(pC_FeatureMap); 
    SAFEFREE(pO_FeatureMap); 
    SAFEFREE(pS_FeatureMap);    
    SAFEFREE(pROIYUV);
    cvReleaseImage(&pInImg);
    cvReleaseImage(&pOutImg);
}





void Show_ROIImg(IplImage *pInImg, IplImage *pOutImg, unsigned char *pInYUV, unsigned char *pOutResizedYUV, unsigned char *pROIYUV, int nWidth, int nHeight)
{
    int                      x, y, i, j;
    int                      nROICount;
    unsigned char      *pDstInRGB = (unsigned char *)(pInImg->imageData);
    unsigned char      *pDstOutRGB = (unsigned char *)(pOutImg->imageData);   

    x = 0;
    do
    {
        if(60 < pOutResizedYUV[x])
            pROIYUV[x] = 255;
        else
            pROIYUV[x] = 0;

        x++;
    }while(x < nWidth * nHeight);


    for(y=0 ; y<nHeight ; y+=16)
    {
        for(x=0 ; x<nWidth ; x+=16)
        {
            nROICount = 0;
            for(j=0 ; j<16 ; j++)
            {
                for(i=0 ; i<16 ; i++)
                {
                    if(pROIYUV[(y+j)*nWidth + (x+i)] == 255)
                        nROICount++;
                }
            }

            if(nROICount > 20)
            {
                for(j=0 ; j<16 ; j++)
                    for(i=0 ; i<16 ; i++)
                        pROIYUV[(y+j)*nWidth + (x+i)] = pInYUV[(y+j)*nWidth + (x+i)];
            }
            else
            {
                for(j=0 ; j<16 ; j++)
                    for(i=0 ; i<16 ; i++)
                        pROIYUV[(y+j)*nWidth + (x+i)] = 0;                
            }
        }
    }


    MEMSET(&pROIYUV[nWidth * nHeight], 128, (nWidth * nHeight / 2));

    ColorConvert_YUV420toRGB_ForOpenCV(pInYUV, pDstInRGB, nWidth, nHeight);
    ColorConvert_YUV420toRGB_ForOpenCV(pROIYUV, pDstOutRGB, nWidth, nHeight);    

    cvShowImage(INPUT_IMG, pInImg);
    cvShowImage(OUTPUT_IMG, pOutImg);

    cvWaitKey(10);
}




int Load_YUVFromFile(FILE *fp,  
                                            int     nWidth, 
                                            int     nHeight,
                                            unsigned char *pDstYUV)
{
    int                     i;
    int                     nWidthUV = nWidth/2;
    int                     nHeightUV = nHeight/2;    
    unsigned char    *ptr;


    ptr = pDstYUV;
    for(i=0 ; i<nHeight ; i++) 
    {
        if( fread(ptr, 1, nWidth , fp) < nWidth )
        return 0;
        ptr += nWidth;
    }

    ptr = pDstYUV + (nWidth * nHeight);
    for(i=0 ; i<nHeightUV ; i++) 
    {
        if( fread(ptr, 1, nWidthUV, fp) < nWidthUV )
        return 0;
        ptr += nWidthUV;
    }
    
    ptr = pDstYUV + (int)(1.25 * nWidth * nHeight);;
    for(i=0 ; i<nHeightUV ; i++) 
    {
        if( fread(ptr, 1, nWidthUV, fp) < nWidthUV )
        return 0;
        ptr += nWidthUV;
    }

    #if 0
    {
        FILE      *fp;
        int         nImageSize = (3 * nWidth * nHeight);

        fp = fopen("test.raw", "wb");
        fwrite(pDstYUV, sizeof(unsigned char), nImageSize/2, fp);
        fclose(fp);
    }
    #endif

    return 1;
}


int Save_DecodedFrameToYUV(FILE *fp,  
                                                            int     nWidth, 
                                                            int     nHeight,
                                                            unsigned char *pSrcYUV)
{
    int     i;
    int     nWidthUV = nWidth/2;
    int     nHeightUV = nHeight/2;        
    unsigned char *tmp_ptr;    
    
    tmp_ptr = pSrcYUV;
    for(i=0 ; i<nHeight ; i++) 
    {
        fwrite(tmp_ptr, 1, nWidth, fp);
        tmp_ptr += nWidth;
    }

    tmp_ptr = pSrcYUV + (nWidth * nHeight);
    for(i=0 ; i<nHeightUV ; i++) 
    {
        fwrite(tmp_ptr, 1, nWidthUV, fp);
        tmp_ptr += nWidthUV;
    }

    tmp_ptr = pSrcYUV + (int)(1.25 * nWidth * nHeight);
    for(i=0 ; i<nHeightUV ; i++) 
    {
        fwrite(tmp_ptr, 1, nWidthUV, fp);
        tmp_ptr += nWidthUV;
    }

    #if 0
    {
        FILE      *fp;
        int         nImageSize = (3 * nWidth * nHeight);

        fp = fopen("test.raw", "wb");
        fwrite(pSrcYUV, sizeof(unsigned char), nImageSize/2, fp);
        fclose(fp);
    }
    #endif    

    return 0;
}



void ColorConvert_YUV420toRGB(unsigned char *pInYUV, unsigned char *pRGB, int nWidth, int nHeight)
{
    int            i, j;
    int            nPixPos;

    unsigned char      *pSrcY = pInYUV;
    unsigned char      *pSrcU = pInYUV+(nHeight*nWidth);
    unsigned char      *pSrcV = pInYUV+(int)(1.25*nHeight*nWidth);
    unsigned char      *pDstRGB = pRGB;


    for(j=0 ; j<nHeight ; j++)
    {
        for(i=0 ; i<nWidth ; i++)
        {
            nPixPos = ((j * nWidth) + i) * 3;
            
            pDstRGB[nPixPos]     = Clip3(0, 255, (int)(1.164*(double)(*(pSrcY) - 16) + 1.596*(double)(*(pSrcV) - 128)));               // R
            pDstRGB[nPixPos+1] = Clip3(0, 255, (int)(1.164*(double)(*(pSrcY) - 16) - 0.391*(double)(*(pSrcU) - 128) - 0.813*(double)(*(pSrcV) - 128)));
            pDstRGB[nPixPos+2] = Clip3(0, 255, (int)(1.164*(double)(*(pSrcY) - 16) + 2.018*(double)(*(pSrcU) - 128)));              // B

            pSrcY++;
            if((i & 0x01) == 1)
            {
                pSrcU++;
                pSrcV++;
            }
        }
        
        if((j & 0x01) == 1)
        {
            pSrcU -= (nWidth >> 1);
            pSrcV -= (nWidth >> 1);
        }
    }  

    #if 0
    {
        FILE      *fp;
        int         nImageSize = (3 * nWidth * nHeight);

        fp = fopen("test.raw", "wb");
        fwrite(pRGB, sizeof(unsigned char), nImageSize, fp);
        fclose(fp);
    }
    #endif
}





void ColorConvert_YUV420toRGB_ForOpenCV(unsigned char *pInYUV, unsigned char *pRGB, int nWidth, int nHeight)
{
    int            i, j;
    int            nPixPos;

    unsigned char      *pSrcY = pInYUV;
    unsigned char      *pSrcU = pInYUV+(nHeight*nWidth);
    unsigned char      *pSrcV = pInYUV+(int)(1.25*nHeight*nWidth);
    unsigned char      *pDstRGB = pRGB;


    for(j=0 ; j<nHeight ; j++)
    {
        for(i=0 ; i<nWidth ; i++)
        {
            nPixPos = ((j * nWidth) + i) * 3;
            
            pDstRGB[nPixPos+2]     = Clip3(0, 255, (int)(1.164*(double)(*(pSrcY) - 16) + 1.596*(double)(*(pSrcV) - 128)));               // R
            pDstRGB[nPixPos+1] = Clip3(0, 255, (int)(1.164*(double)(*(pSrcY) - 16) - 0.391*(double)(*(pSrcU) - 128) - 0.813*(double)(*(pSrcV) - 128)));
            pDstRGB[nPixPos] = Clip3(0, 255, (int)(1.164*(double)(*(pSrcY) - 16) + 2.018*(double)(*(pSrcU) - 128)));              // B

            pSrcY++;
            if((i & 0x01) == 1)
            {
                pSrcU++;
                pSrcV++;
            }
        }
        
        if((j & 0x01) == 1)
        {
            pSrcU -= (nWidth >> 1);
            pSrcV -= (nWidth >> 1);
        }
    }  

    #if 0
    {
        FILE      *fp;
        int         nImageSize = (3 * nWidth * nHeight);

        fp = fopen("test.raw", "wb");
        fwrite(pRGB, sizeof(unsigned char), nImageSize, fp);
        fclose(fp);
    }
    #endif
}







void ColorConvert_RGBtoYUV420(unsigned char *pInYUV, unsigned char *pRGB, int nWidth, int nHeight)
{
    int            i, j;
    int            nPixPos;

    unsigned char      *pSrcRGB = pRGB;
    unsigned char      *pDstY = pInYUV;
    unsigned char      *pDstU = pInYUV+(nHeight*nWidth);
    unsigned char      *pDstV = pInYUV+(int)(1.25*nHeight*nWidth);
    
    for(j=0 ; j<nHeight ; j++)
    {
        for(i=0 ; i<nWidth ; i++)
        {            
            nPixPos = (j * nWidth) + i;
            *(pDstY++) = Clip3(0, 255, (int)( (0.257 * (double)pSrcRGB[nPixPos]) + (0.504 * (double)pSrcRGB[nPixPos]) + (0.098 * (double)pSrcRGB[nPixPos]) + 16));
            
            if((((i) & 0x01) == 0) && ((j & 0x01) == 0))
            {
                *(pDstU++) = Clip3(0, 255, (int)(-(0.148 * (double)pSrcRGB[nPixPos]) - (0.291 * (double)pSrcRGB[nPixPos]) + (0.439 * (double)pSrcRGB[nPixPos]) + 128));
                *(pDstV++) = Clip3(0, 255, (int)(  (0.439 * (double)pSrcRGB[nPixPos]) - (0.368 * (double)pSrcRGB[nPixPos]) - (0.071 * (double)pSrcRGB[nPixPos]) + 128));
            }
        }
    }

    #if 0
    {
        FILE         *fp;
        int            nImageSize = (int)(1.5*nWidth * nHeight);

        fp = fopen("test.raw", "wb");
        fwrite(pInYUV, sizeof(unsigned char), nImageSize, fp);
        fclose(fp);
    }
    #endif
}



void Resize_Bilinear(unsigned char *pSrc, unsigned char *pDst, int  nSrcWidth, int nSrcHeight, int nDstWidth, int nDstHeight)
{
    int     i, j;
    int     ii, jj;
    int     nDeltaX, nDeltaY;
    int     nSrcPaddedWidth = nSrcWidth;
    int     nDstPaddedWidth = nDstWidth;    
    int     nRatioX = (int)(FLOATPRECISION * ((float)nSrcWidth / (float)nDstWidth)); 
    int     nRatioY = (int)(FLOATPRECISION * ((float)nSrcHeight / (float)nDstHeight));

    for(j=0 ; j<nDstHeight ; j++)
    {
        jj = ((j * nRatioY) >> FLOATFACTOR);
        nDeltaY = ((j * nRatioY) & FLOATMASK);
        
        for(i=0 ; i<nDstWidth ; i++)
        {
            ii = ((i * nRatioX) >> FLOATFACTOR);
            nDeltaX = ((i * nRatioX) & FLOATMASK);

            if((i<nSrcWidth-1) && (j<nSrcHeight-1))
            {
                pDst[(j*nDstPaddedWidth) + i] = (unsigned char)(
                    ((FLOATPRECISION - nDeltaX)*(FLOATPRECISION - nDeltaY)*pSrc[(jj * nSrcPaddedWidth) + ii]
                + (                             nDeltaX)*(FLOATPRECISION - nDeltaY)*pSrc[(jj * nSrcPaddedWidth) + ii + 1]
                + (FLOATPRECISION - nDeltaX)*(                             nDeltaY)*pSrc[((jj+1) * nSrcPaddedWidth) + ii]
                + (                             nDeltaX)*(                             nDeltaY)*pSrc[((jj+1) * nSrcPaddedWidth) + ii + 1]) >> (2 * FLOATFACTOR));
            }
            else
                pDst[(j*nDstPaddedWidth) + i] = pSrc[(jj * nSrcPaddedWidth) + ii];
        }
    }

    #if 0
    {
        FILE *fp;
        int            nImageSize = (nDstWidth * nDstHeight);        
        
        fp = fopen("test.raw", "wb");
        fwrite(&(pDst[0]), sizeof(unsigned char), nImageSize, fp);
        fclose(fp);      
    }
    #endif
}
