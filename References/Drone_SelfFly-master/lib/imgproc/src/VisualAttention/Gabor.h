/* Program for image Binarization */
/* programmed by bong su */
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define    SCALE(x)    ((((int)x)*((1L<<16)-1))/255)
#define pi 3.141592

int read_tiff_image(char *, unsigned char **, int*, int*);
int write_tiff_image(char *, unsigned char *,int, int);

/* 이미지 조작함수 선언. */
void Fourier(unsigned char *x, unsigned char *y,  int height , int width);
#define PI 3.1415926536

typedef struct{
    double real;
    double imagin;
} COMPLEX;

// FT와 IFT 그리고 LPF
void FFT_2D(COMPLEX *complex_data, unsigned char *inData, unsigned char *outData, int height, int width);
void fft(COMPLEX *f, int logN, int numpoints, int dir);
void butterflies(int numpoints, int logN, int dir, COMPLEX *f);
void scramble(int numpoints, COMPLEX *f);
void fftview(unsigned char* outData, int height, int width);
void IFFT(COMPLEX *complex_data,  unsigned char *outData,  int height, int width);
void Lpf(COMPLEX *complex_data, int height, int width);
void GaborF(COMPLEX *complex_data, int h, int w, double, double, double);