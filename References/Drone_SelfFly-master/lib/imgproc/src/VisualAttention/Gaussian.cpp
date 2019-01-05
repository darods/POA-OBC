#include "PT_VisualAttention.h"
#define masksize 5

void Gauss(unsigned char *in, unsigned char *out, int height, int width, double sigma)
{
    int i, j, m, n;
    double *mask, sum=0.0;

    mask = (double*)malloc(sizeof(double)*masksize*masksize);

    for(i=0; i<height*width; i++) out[i]=0;
    
    for(i=0; i<masksize; i++)
    {
        for(j=0; j<masksize; j++)
        {
            mask[i*masksize+j] = exp(-((-(masksize-1.0)/2.0+i)*(-(masksize-1.0)/2.0+i)+(-(masksize-1.0)/2.0+j)*(-(masksize-1.0)/2.0+j) )/(2.0*sigma*sigma));
            sum += mask[i*masksize+j];
        }
    }

    for(i=0; i<masksize; i++)
    {
        for(j=0; j<masksize; j++)
        {
            mask[i*masksize+j] /= sum;
        }
    }
    
    for(i=2; i<(height-2); i++)
    {
        for(j=2; j<(width-2); j++)
        {
            for(m=-2; m<3; m++)
            {
                for(n=-2; n<3; n++)
                {
                    sum += (double)in[(i+m)*width+(j+n)]*mask[(m+2)*5+(n+2)];
                }
            }

            if(sum>255.0) sum = 255.0;
            out[i*width+j] = (unsigned char)sum;
            sum = 0.0;
        }
    }
    free(mask);
}




    







