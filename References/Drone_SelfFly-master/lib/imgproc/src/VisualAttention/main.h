#include<stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <malloc.h>
#include "tiff.h"
#include "tiffio.h"
#include "rmatrix.h"
#include "math.h"

#define pi 3.141592

typedef struct
{
    unsigned char R;
    unsigned char G;
    unsigned char B;
} COLOR;

typedef struct 
{
    int l_x, r_x;
    int u_y, d_y;        
} cordi;

void median(unsigned char *in, unsigned char *OutImage, int height, int width);
void gauss_pyramid(unsigned char *inData, unsigned char *outData, int height, int width);
void expand_pyramid (unsigned char *inData, unsigned char *outData, int height, int width, int number);
void Intensity_CSM(unsigned char *In, unsigned char *i_FeatureMap, int h, int w);
void convert_gray(unsigned char *In, unsigned char *out, int h, int w);
void normalization(unsigned char *inData, int height, int width);
double normal(unsigned char *inData, int height, int width);
void new_RBG(unsigned char *In, unsigned char *new_R, unsigned char *new_G, unsigned char *new_B, unsigned char *new_Y, int h, int w);
void Color_CSM(unsigned char *In, unsigned char *c_FeatureMap, int h, int w);
void Gabor_CSM(unsigned char *In, unsigned char *o_FeatureMap, int h, int w);
void Gabor_filter(unsigned char *inData, unsigned char *outData, int height, int width, double angle, double frequency, double distance);
void LucasKanade(unsigned char *in0, unsigned char *in1, int height, int width, int mask, unsigned char *obj);
void Gauss(unsigned char *in, unsigned char *out, int height, int width, double sigma);
void sobel(unsigned char *in, double *out_x, double *out_y, int height, int width);
void sobel1(unsigned char *in, unsigned char *out, int height, int width);
void line_draw_b(unsigned char *outData, int height, int width, int x1, int x2, int y1, int y2, int value);
void Motion_CSM(unsigned char *In, unsigned char *In1, unsigned char *Out, int h, int w);
void nor(unsigned char *In, int height, int width);
void order(unsigned char *In, unsigned char *out, unsigned char *re3, int h, int w);
void select(unsigned char *In, unsigned char *out, unsigned char *re3, unsigned char *re2, int h, int w, int *max);
void mexican(unsigned char *In, unsigned char *Out, int height, int width);
unsigned char * Otsu_option(unsigned char *blockData, int subwidth, int subheight, int option);
void convert(unsigned char *Inimage, int height, int width);
void orientation_CSM(unsigned char *In, unsigned char *o_FeatureMap, int h, int w);
unsigned char *Labeling(unsigned char *in, unsigned char * org, int h, int w, cordi cor[], int *digit);
int track(unsigned char *In1, unsigned char *In2, int h, int w, cordi cor[], int each);
int scene_change(unsigned char *InImage, int height, int width);