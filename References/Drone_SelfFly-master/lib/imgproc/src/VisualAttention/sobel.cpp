#include "PT_VisualAttention.h"

void sobel(unsigned char *in, double *out_x, double *out_y, int height, int width)
{

    int i, j, m, n;
    double sum_x=0.0, sum_y=0.0;
    
    double sobel_x[3][3] = {-1, 0, 1,
                            -2, 0, 2,
                            -1, 0, 1};
    double sobel_y[3][3] = {-1, -2, -1,
                            0, 0, 0,
                            1, 2, 1};

    for(i=0; i<height*width; i++)
    {
        out_x[i] = in[i];
        out_y[i] = in[i];
    }

    for(i=3; i<(height-3); i++)
    {
        for(j=3; j<(width-3); j++)
        {
            for(m=-1; m<2; m++)
            {
                for(n=-1; n<2; n++)
                {
                    sum_x += (in[(i+m)*width+(j+n)]*sobel_x[m+1][n+1]);
                    sum_y += (in[(i+m)*width+(j+n)]*sobel_y[m+1][n+1]);
                }
            }
            out_x[i*width+j] = sum_x;
            out_y[i*width+j] = sum_y;
            sum_x=0.0; sum_y=0.0;
        }
    }
}

void sobel1(unsigned char *in, unsigned char *out, int height, int width)
{
    int sum = 0;
    
    int mask1[3][3] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
    int mask2[3][3] = {-1, -2, -1, 0, 0, 0, 1, 2, 1};
             
    for(int i=1; i<height-1; i++)
    {
        for(int j=1; j<width-1; j++)
        {
            for (int k=0 ;k<3; k++)
            {
                for(int l=0; l<3; l++)
                {
                    sum += (mask1[k][l] * in[(i+k-1)*width+(j+l-1)]) + (mask2[k][l] * in[(i+k-1)*width+(j+l-1)]);
                }
            }
    
            if(abs(sum) > 255) sum = 255;
            if(abs(sum) < 0) sum = 0;
            out[i*width+j] = abs(sum);
            sum = 0;
        }
    }
}
