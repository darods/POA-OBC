

#include "PT_VisualAttention.h"


extern COLOR    *TmpBuffer;


void convert_gray(unsigned char *In, unsigned char *out, int h, int w)
{
    int                       i, j;
    int                       nPos;
    unsigned char       *pSrc;

    for(i=0 ; i<h ; i++)
    {
        for(j=0 ; j<w ; j++)
        {
            nPos = (i*w+j);
            pSrc = &In[3*nPos];
            
            out[nPos] = (pSrc[0] + pSrc[1] + pSrc[2])/3;
        }
    }
}





void new_RBG(unsigned char *In, unsigned char *new_R, unsigned char *new_G, unsigned char *new_B, unsigned char *new_Y, int h, int w)
{
    int                       i, j;
    int                       nPos;
    int                       x=0, m, n, k, l;
    unsigned char       *pSrc;

    for(i=0 ; i<h ; i++)
    {
        for(j=0 ; j<w ; j++)
        {
            nPos = (i*w+j);
            pSrc = &In[3*nPos];

            m = pSrc[0] - (pSrc[1] + pSrc[2])/2;
            n = pSrc[1] - (pSrc[0] + pSrc[2])/2;
            k = pSrc[2] - (pSrc[1] + pSrc[0])/2;
            l = (pSrc[0] + pSrc[1]) - 2*(abs(pSrc[0] - pSrc[1]) + pSrc[2]);
        
            if(m<0) new_R[nPos] = 0;
            else new_R[nPos] = m;
            if(n<0) new_G[nPos] = 0;
            else new_G[nPos] = n;
            if(k<0) new_B[nPos] = 0;
            else new_B[nPos] = k;
            if(l<0) new_Y[nPos] = 0;
            else new_Y[nPos] = l;
        }
    }
}





void normalization(unsigned char *inData, int height, int width)
{
    int i, j, max = 0;
    
    for(j=0; j<height; j++)
    {
        for(i=0; i<width; i++)
        {
            if(inData[j*width+i]>max) 
                max = inData[j*width+i];
        }
    }

    if(max == 0) 
        return;

    for(j=0; j<height; j++)
    {
        for(i=0; i<width; i++)
        {
            inData[j*width+i] = inData[j*width+i]*255/max;
        }
    }
}

/*
void normalization(unsigned char *inData, int height, int width)
{
    int i, j, max = 0, max1 = 0;
    double *temp;
    double k=0.0, factor;

    temp = (double *)malloc(sizeof(double)*height*width);
    
    for(i=0; i<height; i++)
    {
        for(j=0; j<width; j++)
        {
            if(inData[i*width+j]>max) max = inData[i*width+j];
        }
    }

    for(i=0; i<height; i++)
    {
        for(j=0; j<width; j++)
        {
            inData[i*width+j] = inData[i*width+j]*10/max;
            k = log(inData[i*width+j]*inData[i*width+j])/log(2);
            temp[i*width+j] = k;
            if(temp[i*width+j]>max1) max1 = temp[i*width+j];
        }
    }

    for(i=0; i<height; i++)
    {
        for(j=0; j<width; j++)
        {
            if(temp[i*width+j]*255/max1>255) inData[i*width+j] = 255;
            else inData[i*width+j] = temp[i*width+j]*255/max1;
        }
    }
}
*/
/*
void normalization(unsigned char *inData, int height, int width)
{
    int i, j, max = 0, h, w, x=0, y=0, k;
    int local_max[256] = {0,};
    double tot=0, var=0, max2=0.0;
    double *temp;

    temp = (double *)calloc(sizeof(double),height*width);

    for(i=0; i<height; i++)
    {
        for(j=0; j<width; j++)
        {
            if(inData[i*width+j]>max) max = inData[i*width+j];
        }
    }

    h = height/16; w = width/16;

    for(k=0; k<256; k++)
    {
        for(i=x; i<x+h; i++)
        {
            for(j=y; j<y+w; j++)
            {
                if(inData[i*width+j] > local_max[k]) local_max[k] = inData[i*width+j];
            }
        }
        y += w;
        if(k%16==15)
        {
            y = 0; 
            x += h;
        }
        tot += (double)local_max[k];
    }
    tot = tot/256;
    var = (max - tot)*(max - tot);
    
    for(i=0; i<height; i++)
    {
        for(j=0; j<width; j++)
        {
            temp[i*width+j] = (double)inData[i*width+j]/max*var;
            if(temp[i*width+j]>max2) max2 = temp[i*width+j];
        }
    }
    
    for(i=0; i<height; i++)
    {
        for(j=0; j<width; j++)
        {
            inData[i*width+j] = temp[i*width+j]*255/max2;
        }
    }

}
*/
/*
void normalization(unsigned char *inData, int height, int width)
{
    int i, j;

    for(i=0; i<height; i++)
    {
        for(j=0; j<width; j++)
        {
            if(inData[i*width+j]>=0 && inData[i*width+j]<25) inData[i*width+j] = 0;
            else if(inData[i*width+j]>=25 && inData[i*width+j]<50) inData[i*width+j] = 25;
            else if(inData[i*width+j]>=50 && inData[i*width+j]<75) inData[i*width+j] = 50;
            else if(inData[i*width+j]>=75 && inData[i*width+j]<100) inData[i*width+j] = 75;
            else if(inData[i*width+j]>=100 && inData[i*width+j]<125) inData[i*width+j] = 100;
            else if(inData[i*width+j]>=125 && inData[i*width+j]<150) inData[i*width+j] = 125;
            else if(inData[i*width+j]>=150 && inData[i*width+j]<175) inData[i*width+j] = 150;
            else if(inData[i*width+j]>=175 && inData[i*width+j]<200) inData[i*width+j] = 175;
            else if(inData[i*width+j]>=200 && inData[i*width+j]<225) inData[i*width+j] = 200;
            else inData[i*width+j] = 225;
        }
    }
}
*/
/*
double normal(unsigned char *inData, int height, int width)
{
    int i, j, max = 0, h, w, x=0, y=0, k;
    int local_max[256] = {0,};
    double tot=0, var=0;

    for(i=0; i<height; i++)
    {
        for(j=0; j<width; j++)
        {
            if(inData[i*width+j]>max) max = inData[i*width+j];
        }
    }

    h = height/16; w = width/16;

    for(k=0; k<256; k++)
    {
        for(i=x; i<x+h; i++)
        {
            for(j=y; j<y+w; j++)
            {
                if(inData[i*width+j] > local_max[k]) local_max[k] = inData[i*width+j];
            }
        }
        y += w;
        if(k%16==15)
        {
            y = 0; 
            x += h;
        }
        tot += (double)local_max[k];
    }
    tot = tot/256;
    var = (max - tot)*(max - tot)/10000;
    return(var);
}
*/
/*
void nor(unsigned char *In, int height, int width)
{
    int i, j, temp;

    unsigned char *te, *gauss0, *gauss1;

    gauss0 = (unsigned char *)malloc(sizeof(char)*height*width);
    gauss1 = (unsigned char *)malloc(sizeof(char)*height*width);

    te = (unsigned char *)malloc(sizeof(char)*height*width);

    Gauss(In, gauss0, height, width, 2.5);
    Gauss(In, gauss1, height, width, 2.5/1.75);

    for(i=0; i<height*width; i++)
    {
        te[i] = abs(gauss0[i] - gauss1[i]);
        temp = In[i] + (gauss0[i] - 1.6*gauss1[i]);
        if(temp < 0) In[i] = 0;
        else if(temp > 255) In[i] = 255;
        else In[i] = temp;
    }
}
*/
double normal(unsigned char *inData, int height, int width)
{
    int i, j, max = 0, x=0, var;
    double tot=0.0;

    for(i=0; i<height; i++)
    {
        for(j=0; j<width; j++)
        {
            if(inData[i*width+j]>max) 
                max = inData[i*width+j];
        }
    }

    for(i=1; i<height-1; i++)
    {
        for(j=1; j<width-1; j++)
        {
            if((inData[i*width+j]>inData[(i-1)*width+(j-1)])&&(inData[i*width+j]>inData[(i-1)*width+j])
                &&(inData[i*width+j]>inData[(i-1)*width+(j+1)])&&(inData[i*width+j]>inData[i*width+(j-1)])
                &&(inData[i*width+j]>inData[i*width+(j+1)])&&(inData[i*width+j]>inData[(i+1)*width+(j-1)])
                &&(inData[i*width+j]>inData[(i+1)*width+j])&&(inData[i*width+j]>inData[(i+1)*width+(j+1)]))
            {
                tot += inData[i*width+j];
                x++;
            }
        }
    }
    
    var = (int)((max - tot/x) * (max - tot/x));
    
    return  var;
}

void mexican(unsigned char *In, unsigned char *Out, int height, int width)
{
    int i, j, m, n;
    double temp=0.0;
    double weight[5][5] = {-0.1, -0.1, -0.1, -0.1, -0.1,
                           -0.1,  0.1,  0.1,  0.1, -0.1,
                           -0.1,  0.1,  1.0,  0.1, -0.1,
                           -0.1,  0.1,  0.1,  0.1, -0.1,
                           -0.1, -0.1, -0.1, -0.1, -0.1};

    unsigned char *pad;

    pad = (unsigned char *) calloc(sizeof(char), (height+4)*(width+4));

    for(i=2; i<height+2; i++)
    {
        pad[i*(width+4)+0] = In[(i-2)*width+0];
        pad[i*(width+4)+1] = In[(i-2)*width+0];
        pad[i*(width+4)+(width+2)] = In[(i-2)*width+(width-1)];
        pad[i*(width+4)+(width+3)] = In[(i-2)*width+(width-1)];
    }
    for(j=2; j<width+2; j++)
    {
        pad[0*(width+4)+j] = In[0*width+(j-2)];
        pad[1*(width+4)+j] = In[0*width+(j-2)];
        pad[(height+2)*(width+4)+j] = In[(height-1)*width+(j-2)];
        pad[(height+2)*(width+4)+j] = In[(height-1)*width+(j-2)];
    }
    for(i=0; i<2; i++)
    {
        for(j=0; j<2; j++)
        {
            pad[i*(width+4)+j] = In[0];
        }
    }
    for(i=0; i<2; i++)
    {
        for(j=width+2; j<width+4; j++)
        {
            pad[i*(width+4)+j] = In[width-1];
        }
    }
    for(i=height+2; i<height+4; i++)
    {
        for(j=0; j<2; j++)
        {
            pad[i*(width+4)+j] = In[(height-1)*width];
        }
    }
    for(i=height+2; i<height+4; i++)
    {
        for(j=width+2; j<width+4; j++)
        {
            pad[i*(width+4)+j] = In[(height-1)*width+(width-1)];
        }
    }


    for(i=2; i<height+2; i++)
    {
        for(j=2; j<width+2; j++)
        {
            pad[i*(width+4)+j] = In[(i-2)*width+(j-2)];
        }
    }

    for(i=2; i<height+2; i++)
    {
        for(j=2; j<width+2; j++)
        {
            for(m=-2; m<3; m++)
            {
                for(n=-2; n<3; n++)
                {
                    temp += (double)pad[(i+m)*(width+4)+(j+n)]*weight[m+2][n+2];
                }
            }
            if(temp<0.0) Out[(i-2)*width+(j-2)]=0;
            else if(temp>255.0) Out[(i-2)*width+(j-2)]=255;
            else Out[(i-2)*width+(j-2)] = (unsigned char)temp;
            temp = 0.0;
        }
    }
//    write_tiff_image("pad.tif", pad, height+4, width+4, 0);
//    write_tiff_image("te.tif", Out, height, width, 0);
    free(pad);
}
