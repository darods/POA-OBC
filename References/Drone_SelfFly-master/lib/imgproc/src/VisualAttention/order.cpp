#include "PT_VisualAttention.h"

void order(unsigned char *In, unsigned char *out, unsigned char *re3, int h, int w)
{
    unsigned char *re, *re1, *re2;
    int i, j, m=0, n=0, x, max=0, k, height, width;
    double local_max[256]={0,};
    double avg=0.0;

    re = (unsigned char *)malloc(sizeof(unsigned char)*h*w/16);
    re1 = (unsigned char *)malloc(sizeof(unsigned char)*h*w/64);
    re2 = (unsigned char *)malloc(sizeof(unsigned char)*h*w/256);

    gauss_pyramid(out, re, h/2, w/2);
    gauss_pyramid(re, re1, h/4, w/4);
    gauss_pyramid(re1, re2, h/8, w/8);
/*
    for(i=0; i<h/16; i++)
    {
        for(j=0; j<w/16; j++)
        {
            avg += (double)re2[i*(w/16)+j];
        }
    }
    
    avg = avg/(h/16)/(w/16);

    avg = 0.0;
*/

    height=h/16/16; width=w/16/16;

    for(k=0; k<256; k++)
    {
        for(i=m; i<m+height; i++)
        {
            for(j=n; j<n+width; j++)
                if(re2[i*w/16+j] > local_max[k]) local_max[k] = re[i*w/16+j];
        }
        n += width;
        if(k%16==15)
        {
            n = 0; 
            m += height;
        }
        avg += (double)local_max[k];
    }
    avg = avg/256;

    for(i=1; i<h/16; i++)
    {
        for(j=1; j<w/16; j++)
        {
            if(re2[i*(w/16)+j]>avg+20)
            {
                for(m=-16; m<17; m++)
                {
                    for(n=-16; n<17; n++)
                    {
                        x = (i*16+m)*w+(j*16+n);
                        re3[x*3] = In[x*3];
                        re3[x*3+1] = In[x*3+1];
                        re3[x*3+2] = In[x*3+2];
                    }
                }
            }
        }
    }
}