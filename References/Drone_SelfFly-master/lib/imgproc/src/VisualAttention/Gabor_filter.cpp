#include "PT_VisualAttention.h"
#include "Gabor.h"

extern COMPLEX  *TmpComplexBuf;

//unsigned char *Gabor_filter(unsigned char *inData, int height, int width, double angle, double frequency, double distance)
void Gabor_filter(unsigned char *inData, unsigned char *out, int height, int width, double angle, double frequency, double distance)
{
    int i;  
//    unsigned char *out;
    COMPLEX *complex_data = TmpComplexBuf;

    
    /* 필터링 결과 이미지를 저장할 메모리 할당 받기. */
/*
    if((out = (unsigned char *)malloc(sizeof(unsigned char)*height*width))==NULL)
    {
        printf("Not enough memory to allocate buffer \n");
        exit(1);
    }
*/
    for(i=0; i<height*width; i++)
    {
        out[i]=0; // initializing
    }

    // Fourier Transform
    
//    printf("Process : Fourier Transform......\n");
    FFT_2D(complex_data, inData, out, height,  width);

    
//    IFFT(gbong, outData, height, width);
//    write_tiff_image("BIDFourier.tif", outData, height, width);

    //Fourier(inData, outData, height, width);
    //write_tiff_image("Fourier.tif", outData, height, width);

    for(i=0; i<height*width; i++)
    {
        out[i]=0;
    }
    
    GaborF(complex_data, height, width, angle, frequency, distance);
    
            
            
    //
    //
    IFFT(complex_data, out, height, width);
}



