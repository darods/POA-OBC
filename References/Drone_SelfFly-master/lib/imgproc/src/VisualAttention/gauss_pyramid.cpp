#include "PT_VisualAttention.h"

#define LERP(a,l,h)     ((l)+(((h)-(l))*(a)))
#define MIN(a,b)    ((a) <= (b) ? (a) : (b))

extern COLOR    *TmpBuffer;
extern double     pGaucoeffi[5][5];


void gauss_pyramid(unsigned char *inData, unsigned char *outData, int height, int width)
{
    int i, j, k, l;
    double sum;
    double temp_coeff;        // a는 가우시안 weight에서 0일때 최대값


    for (j=0; j<height/2; j++)
    {
        for (i=0; i<width/2; i++)
        {
            sum = 0; temp_coeff = 1;
            for (k=-2; k<=2; k++)
            {
                for (l=-2; l<=2; l++)
                {
                    if (2*j+k<0 || 2*j+k>=height || 2*i+l<0 || 2*i+l>= width)
                    {
                        temp_coeff -= pGaucoeffi[k+2][l+2];
                        continue;
                    }
                    sum += pGaucoeffi[k+2][l+2]*inData[(2*j+k)*width+2*i+l];
                }
            }
            sum /= temp_coeff;
            outData[j*(width/2)+i] = Clip3(0, 255, (unsigned char)sum);
        }
    }
}





void expand_pyramid(unsigned char *inData, unsigned char *outData, int height, int width, int number)
{
    int i, j;
    double x, y; 
    int x0, y0, x1, y1, scale;
    double d00, d01, d10, d11, dx0, dx1, fx, fy;
    double sw = (float)1/pow((float)2, number);
    double sh = (float)1/pow((float)2, number);
    scale = (int)pow((double)2,number);


    for(j=0; j<height*scale; j++)
    {
        for(i=0; i<width*scale; i++)
        {
            x = ((double)i) * sw - 0.5;
            y = ((double)j) * sh - 0.5;
            if (x < 0.0) x = 0.0;
            if (y < 0.0) y = 0.0;
            x0 = (int)floor(x); fx = x - (double)x0;
            y0 = (int)floor(y); fy = y - (double)y0;
            x1 = MIN(x0 + 1, width - 1);
            y1 = MIN(y0 + 1, height - 1);
            d00 = (double)(inData[y0*width+x0]);
            d10 = (double)(inData[y0*width+x1]);
            d01 = (double)(inData[y1*width+x0]);
            d11 = (double)(inData[y1*width+x1]);
            dx0 = LERP(fx, d00, d10);
            dx1 = LERP(fx, d01, d11);
            outData[j*width*scale+i] = (unsigned char)(LERP(fy, dx0, dx1));    
        }
    }
}
