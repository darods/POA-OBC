#include "PT_VisualAttention.h"

void select(unsigned char *In, unsigned char *out, unsigned char *re3, unsigned char *re2,
            int h, int w, int *max)
{

    int i, j, m, n, x, max_i, max_j, digit=0, k;

    for(i=1; i<h/16-1; i++)
    {
        for(j=1; j<w/16-1; j++)
        {
        //    k = (re2[i*(w/16)+j] + re2[i*(w/16)+(j+1)] + re2[(i+1)*(w/16)+j] + re2[(i+1)*(w/16)+(j+1)])/4;
            k = re2[i*(w/16)+j];
            if(k>digit)
            {
                digit = k;
                max_i=i;
                max_j=j;
            }
        }
    }

    for(m=-24; m<25; m++)
    {
        for(n=-24; n<25; n++)
        {
            x = (max_i*16+m)*w+(max_j*16+n);
            if((max_i*16+m>=0) && (max_i*16+m<h) && (max_j*16+n>=0) && (max_j*16+n<w))
            {
                re3[x*3] = In[x*3];
                re3[x*3+1] = In[x*3+1];
                re3[x*3+2] = In[x*3+2];
            }
        }
    }

    for(i=-2; i<3; i++)
    {
        for(j=-2; j<3; j++)
        {
            if((max_i+i>=0) && (max_i+i<h) && (max_j+j>=0) && (max_j+j<w))
            {
                re2[(max_i+i)*w/16+(max_j+j)] = 0;
            }
        }
    }

    *max = digit;
}