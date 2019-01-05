#include "PT_VisualAttention.h"

void convert(unsigned char *Inimage, int height, int width)
{

    int i, j;
    int num=0, mean, var, threshold;

    for(i=0; i<height; i++)
    {
        for(j=0; j<width; j++)
        {
            num += Inimage[i*width+j];
        }
    }
    mean = num/height/width;
    num=0;
    for(i=0; i<height; i++)
    {
        for(j=0; j<width; j++)
        {
            num += (int)(sqrt((double)(Inimage[i*width+j]-mean)*(double)(Inimage[i*width+j]-mean)));
        }
    }
    var = num/height/width;
    threshold = mean+var;
    for(i=0; i<height; i++)
    {
        for(j=0; j<width; j++)
        {
            if(Inimage[i*width+j]>threshold) Inimage[i*width+j] = Inimage[i*width+j];
            else Inimage[i*width+j] = 0;
        }
    }
}
