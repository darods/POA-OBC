#include "PT_VisualAttention.h"

#if __SIMPLIFICATION_MODE__
void Intensity_CSM(I_CMS *pICMS, unsigned char *In, unsigned char *i_FeatureMap, int h, int w)
{
    unsigned char *level_2, *Expand_25, *Intensity_25;
    double var_i0=0, i_t=0;

    level_2 = pICMS->level_2;
    Expand_25    = pICMS->Expand_25;
    Intensity_25 = pICMS->Intensity_25;

    #if __SIMPLIFICATION_MODE__
    MEMCPY(pICMS->level_0, In, (h* w));
    #else
    convert_gray(In, pICMS->level_0, h, w); // color영상을 gray영상으로 변환
    #endif

    gauss_pyramid(pICMS->level_0, pICMS->level_1, h, w);
    gauss_pyramid(pICMS->level_1, pICMS->level_2, h/2, w/2);
    gauss_pyramid(pICMS->level_2, pICMS->level_3, h/4, w/4);
    gauss_pyramid(pICMS->level_3, pICMS->level_4, h/8, w/8);
    gauss_pyramid(pICMS->level_4, pICMS->level_5, h/16, w/16);

    expand_pyramid(pICMS->level_3, pICMS->Expand_23, h/8, w/8, 1);
    expand_pyramid(pICMS->level_4, pICMS->Expand_24, h/16, w/16, 2);
    expand_pyramid(pICMS->level_5, pICMS->Expand_25, h/32, w/32, 3);

    {
        int     nQWidth = w / 4;
        int     nQHeight = h / 4;
        int     nPos, temp;
        int     i, j;
        
        for(j=0 ; j<nQHeight ; j++)
        {
            for(i=0 ; i<nQWidth ; i++)
            {
                nPos = j*nQWidth + i;
                temp = abs(level_2[nPos] - Expand_25[nPos]);
                if(temp>255) Intensity_25[nPos] = 255;
                else Intensity_25[nPos] = temp;
            }
        }
        
        normalization(Intensity_25, nQHeight, nQWidth); 
        var_i0 = normal(Intensity_25, nQHeight, nQWidth); 
        i_t = var_i0;

        for(j=0 ; j<nQHeight ; j++)
        {
            for(i=0 ; i<nQWidth ; i++)
            {
                nPos = j*nQWidth + i;
                temp = (int)((Intensity_25[nPos]*var_i0) / i_t);
                if(temp > 255) i_FeatureMap[nPos]=255;
                else i_FeatureMap[nPos] = (unsigned char)temp;
            }
        }
    }
}
#else
void Intensity_CSM(I_CMS *pICMS, unsigned char *In, unsigned char *i_FeatureMap, int h, int w)
{
    int i, j, temp;
    unsigned char *level_0, *level_1, *level_2, *level_3, *level_4, *level_5, *level_6, *level_7, *level_8;
    unsigned char *Expand_23, *Expand_24, *Expand_25, *Expand_26, *Expand_27, *Expand_28;
    unsigned char *Intensity_25, *Intensity_26, *Intensity_36, *Intensity_37, *Intensity_47, *Intensity_48;
    double var_i0=0, var_i1=0, var_i2=0, var_i3=0, var_i4=0, var_i5=0, i_t=0;

    level_0      = pICMS->level_0;
    level_1      = pICMS->level_1;
    level_2      = pICMS->level_2;
    level_3      = pICMS->level_3;
    level_4      = pICMS->level_4;
    level_5      = pICMS->level_5;
    level_6      = pICMS->level_6;    
    level_7      = pICMS->level_7;   
    level_8      = pICMS->level_8;    
    Expand_23    = pICMS->Expand_23;
    Expand_24    = pICMS->Expand_24;
    Expand_25    = pICMS->Expand_25;
    Expand_26    = pICMS->Expand_26;
    Expand_27    = pICMS->Expand_27;
    Expand_28    = pICMS->Expand_28;
    Intensity_25 = pICMS->Intensity_25;
    Intensity_26 = pICMS->Intensity_26;
    Intensity_36 = pICMS->Intensity_36;
    Intensity_37 = pICMS->Intensity_37;
    Intensity_47 = pICMS->Intensity_47;
    Intensity_48 = pICMS->Intensity_48;

    convert_gray(In, level_0, h, w); // color영상을 gray영상으로 변환

    gauss_pyramid(level_0, level_1, h, w);
    gauss_pyramid(level_1, level_2, h/2, w/2);
    gauss_pyramid(level_2, level_3, h/4, w/4);
    gauss_pyramid(level_3, level_4, h/8, w/8);
    gauss_pyramid(level_4, level_5, h/16, w/16);
    gauss_pyramid(level_5, level_6, h/32, w/32);
    gauss_pyramid(level_6, level_7, h/64, w/64);
    gauss_pyramid(level_7, level_8, h/128, w/128);

    expand_pyramid(level_3, Expand_23, h/8, w/8, 1);
    expand_pyramid(level_4, Expand_24, h/16, w/16, 2);
    expand_pyramid(level_5, Expand_25, h/32, w/32, 3);
    expand_pyramid(level_6, Expand_26, h/64, w/64, 4);
    expand_pyramid(level_7, Expand_27, h/128, w/128, 5);
    expand_pyramid(level_8, Expand_28, h/256, w/256, 6);

    for(i=0; i<h/4; i++)
    {
        for(j=0; j<w/4; j++)
        {
            temp = abs(level_2[i*w/4+j] - Expand_25[i*w/4+j]);
            if(temp>255) Intensity_25[i*w/4+j] = 255;
            else Intensity_25[i*w/4+j] = temp;

            temp = abs(level_2[i*w/4+j] - Expand_26[i*w/4+j]);
            if(temp>255) Intensity_26[i*w/4+j] = 255;
            else Intensity_26[i*w/4+j] = temp;

            temp = abs(Expand_23[i*w/4+j] - Expand_26[i*w/4+j]);
            if(temp>255) Intensity_36[i*w/4+j] = 255;
            else Intensity_36[i*w/4+j] = temp;

            temp = abs(Expand_23[i*w/4+j] - Expand_27[i*w/4+j]);
            if(temp>255) Intensity_37[i*w/4+j] = 255;
            else Intensity_37[i*w/4+j] = temp;

            temp = abs(Expand_24[i*w/4+j] - Expand_27[i*w/4+j]);
            if(temp>255) Intensity_47[i*w/4+j] = 255;
            else Intensity_47[i*w/4+j] = temp;

            temp = abs(Expand_24[i*w/4+j] - Expand_28[i*w/4+j]);
            if(temp>255) Intensity_48[i*w/4+j] = 255;
            else Intensity_48[i*w/4+j] = temp;
        }
    }
    

    normalization(Intensity_25, h/4, w/4); 
    normalization(Intensity_26, h/4, w/4);
    normalization(Intensity_36, h/4, w/4); 
    normalization(Intensity_37, h/4, w/4);
    normalization(Intensity_47, h/4, w/4); 
    normalization(Intensity_48, h/4, w/4);


    var_i0 = normal(Intensity_25, h/4, w/4); 
    var_i1 = normal(Intensity_26, h/4, w/4);
    var_i2 = normal(Intensity_36, h/4, w/4); 
    var_i3 = normal(Intensity_37, h/4, w/4);
    var_i4 = normal(Intensity_47, h/4, w/4); 
    var_i5 = normal(Intensity_48, h/4, w/4);

    i_t = var_i0+var_i1+var_i2+var_i3+var_i4+var_i5;

    for (j=0; j<h/4; j++)
    {
        for (i=0; i<w/4; i++)
        {
            temp = (Intensity_25[j*(w/4)+i]*var_i0 + Intensity_26[j*(w/4)+i]*var_i1 + Intensity_36[j*(w/4)+i]*var_i2
            + Intensity_37[j*(w/4)+i]*var_i3 + Intensity_47[j*(w/4)+i]*var_i4 + Intensity_48[j*(w/4)+i]*var_i5)/i_t;
            if(temp > 255) i_FeatureMap[j*(w/4)+i]=255;
            else i_FeatureMap[j*(w/4)+i] = (unsigned char)temp;
        }
    }
}

#endif
