#include "PT_VisualAttention.h"

void orientation_CSM(unsigned char *In, unsigned char *o_FeatureMap, int h, int w)
{
    int i, j, temp;
    unsigned char *level, *level_0, *level_1, *level_2, *level_3, *level_4, *level_5, *level_6, *level_7, *level_8;
    unsigned char *Expand_23, *Expand_24, *Expand_25, *Expand_26, *Expand_27, *Expand_28;
    unsigned char *Intensity_25, *Intensity_26, *Intensity_36, *Intensity_37, *Intensity_47, *Intensity_48;
    double var_i0=0, var_i1=0, var_i2=0, var_i3=0, var_i4=0, var_i5=0, i_t=0;

    // 메모리 할당
    level = (unsigned char *)malloc(sizeof(unsigned char)*h*w);
    level_0 = (unsigned char *)malloc(sizeof(unsigned char)*h*w);
    level_1 = (unsigned char *)malloc(sizeof(unsigned char)*h*w/4);
    level_2 = (unsigned char *)malloc(sizeof(unsigned char)*h*w/16);
    level_3 = (unsigned char *)malloc(sizeof(unsigned char)*h*w/64);
    level_4 = (unsigned char *)malloc(sizeof(unsigned char)*h*w/256);
    level_5 = (unsigned char *)malloc(sizeof(unsigned char)*h*w/1024);
    level_6 = (unsigned char *)malloc(sizeof(unsigned char)*h*w/4096);
    level_7 = (unsigned char *)malloc(sizeof(unsigned char)*h*w/16384);
    level_8 = (unsigned char *)malloc(sizeof(unsigned char)*h*w/65536);

    Expand_23 = (unsigned char *)malloc(sizeof(unsigned char)*h*w/16);
    Expand_24 = (unsigned char *)malloc(sizeof(unsigned char)*h*w/16);
    Expand_25 = (unsigned char *)malloc(sizeof(unsigned char)*h*w/16);
    Expand_26 = (unsigned char *)malloc(sizeof(unsigned char)*h*w/16);
    Expand_27 = (unsigned char *)malloc(sizeof(unsigned char)*h*w/16);
    Expand_28 = (unsigned char *)malloc(sizeof(unsigned char)*h*w/16);
    
    Intensity_25 = (unsigned char *)malloc(sizeof(unsigned char)*h*w/16);
    Intensity_26 = (unsigned char *)malloc(sizeof(unsigned char)*h*w/16);
    Intensity_36 = (unsigned char *)malloc(sizeof(unsigned char)*h*w/16);
    Intensity_37 = (unsigned char *)malloc(sizeof(unsigned char)*h*w/16);
    Intensity_47 = (unsigned char *)malloc(sizeof(unsigned char)*h*w/16);
    Intensity_48 = (unsigned char *)malloc(sizeof(unsigned char)*h*w/16);

    convert_gray(In, level, h, w); // color영상을 gray영상으로 변환
    sobel1(level, level_0, h, w);
    
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
    

    normalization(Intensity_25, h/4, w/4); normalization(Intensity_26, h/4, w/4);
    normalization(Intensity_36, h/4, w/4); normalization(Intensity_37, h/4, w/4);
    normalization(Intensity_47, h/4, w/4); normalization(Intensity_48, h/4, w/4);
    

    var_i0 = normal(Intensity_25, h/4, w/4); var_i1 = normal(Intensity_26, h/4, w/4);
    var_i2 = normal(Intensity_36, h/4, w/4); var_i3 = normal(Intensity_37, h/4, w/4);
    var_i4 = normal(Intensity_47, h/4, w/4); var_i5 = normal(Intensity_48, h/4, w/4);

    i_t = var_i0+var_i1+var_i2+var_i3+var_i4+var_i5;

    for (j=0; j<h/4; j++)
    {
        for (i=0; i<w/4; i++)
        {
            temp = (int)((Intensity_25[j*(w/4)+i]*var_i0 + Intensity_26[j*(w/4)+i]*var_i1 + Intensity_36[j*(w/4)+i]*var_i2
                + Intensity_37[j*(w/4)+i]*var_i3 + Intensity_47[j*(w/4)+i]*var_i4 + Intensity_48[j*(w/4)+i]*var_i5)/i_t);
            if(temp > 255) o_FeatureMap[j*(w/4)+i]=255;
            else o_FeatureMap[j*(w/4)+i] = (unsigned char)temp;
        }
    }
/*
    for (j=0; j<h/2; j++)
    {
        for (i=0; i<w/2; i++)
        {
            temp = (Intensity_14[j*(w/2)+i] + Intensity_15[j*(w/2)+i] + Intensity_25[j*(w/2)+i]
                + Intensity_26[j*(w/2)+i] + Intensity_36[j*(w/2)+i] + Intensity_37[j*(w/2)+i])/6;
            if(temp > 255) i_FeatureMap[j*(w/2)+i] = 255;
            else i_FeatureMap[j*(w/2)+i] = (unsigned char)temp;
        }
    }
/*
    nor(Intensity_14, h/2, w/2);
    nor(Intensity_15, h/2, w/2);
    nor(Intensity_25, h/2, w/2);
    nor(Intensity_26, h/2, w/2);
    nor(Intensity_36, h/2, w/2);
    nor(Intensity_37, h/2, w/2);

    for (j=0; j<h/2; j++)
    {
        for (i=0; i<w/2; i++)
        {
            temp = (Intensity_14[j*(w/2)+i] + Intensity_15[j*(w/2)+i] + Intensity_25[j*(w/2)+i]
                + Intensity_26[j*(w/2)+i] + Intensity_36[j*(w/2)+i] + Intensity_37[j*(w/2)+i])/6;
            i_FeatureMap[j*(w/2)+i] = (unsigned char)temp;
        }
    }
*/
/*
    write_tiff_image("output10000.tif", level_0, h, w, 0);
    write_tiff_image("output10001.tif", level_1, h/2, w/2, 0);
    write_tiff_image("output10002.tif", level_2, h/4, w/4, 0);
    write_tiff_image("output10003.tif", level_3, h/8, w/8, 0);
    write_tiff_image("output10004.tif", level_4, h/16, w/16, 0);
    write_tiff_image("output10005.tif", level_5, h/32, w/32, 0);
    write_tiff_image("output10006.tif", level_6, h/64, w/64, 0);
    write_tiff_image("output10007.tif", level_7, h/128, w/128, 0);
    write_tiff_image("output10008.tif", Expand_23, h/4, w/4, 0);
    write_tiff_image("output10009.tif", Expand_24, h/4, w/4, 0);
    write_tiff_image("output10010.tif", Expand_25, h/4, w/4, 0);
    write_tiff_image("output10011.tif", Expand_26, h/4, w/4, 0);
    write_tiff_image("output10012.tif", Expand_27, h/4, w/4, 0);
    write_tiff_image("output10013.tif", Expand_28, h/4, w/4, 0);

    write_tiff_image("output10014.tif", Intensity_25, h/4, w/4, 0);
    write_tiff_image("output10015.tif", Intensity_26, h/4, w/4, 0);
    write_tiff_image("output10016.tif", Intensity_36, h/4, w/4, 0);
    write_tiff_image("output10017.tif", Intensity_37, h/4, w/4, 0);
    write_tiff_image("output10018.tif", Intensity_47, h/4, w/4, 0);
    write_tiff_image("output10019.tif", Intensity_48, h/4, w/4, 0);
//    write_tiff_image("output10020.tif", FeatureMap1, h/2, w/2, 0);
//    write_tiff_image("output10020_test.tif", i_FeatureMap, h/2, w/2, 0);
*/

    free(level);free(level_0);free(level_1);free(level_2);free(level_3);free(level_4);free(level_5);free(level_6);free(level_7);
    free(level_8);free(Expand_23);free(Expand_24);free(Expand_25);free(Expand_26);free(Expand_27);free(Expand_28);
    free(Intensity_25);free(Intensity_26);free(Intensity_36);free(Intensity_37);free(Intensity_47);free(Intensity_48);
}