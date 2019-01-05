#include "PT_VisualAttention.h"
// CSD(center-surround difference)





#if __SIMPLIFICATION_MODE__
void Color_CSM(C_CMS *pCCMS, unsigned char *In, unsigned char *c_FeatureMap, int h, int w)
{
    int temp2, temp3;
    unsigned char *RG_out, *BY_out;
    unsigned char *new_R_2, *new_G_2, *new_B_2, *new_Y_2;
    unsigned char *Expand_R_25, *Expand_G_25, *Expand_B_25, *Expand_Y_25;

    new_R_2     = pCCMS->new_R_2;
    new_G_2     = pCCMS->new_G_2;
    new_B_2     = pCCMS->new_B_2;
    new_Y_2     = pCCMS->new_Y_2;
    RG_out       = pCCMS->RG_out;
    BY_out       = pCCMS->BY_out;  
    Expand_R_25 = pCCMS->Expand_R_25;
    Expand_G_25 = pCCMS->Expand_G_25;
    Expand_B_25 = pCCMS->Expand_B_25;
    Expand_Y_25 = pCCMS->Expand_Y_25;

    new_RBG(In, pCCMS->new_R, pCCMS->new_G, pCCMS->new_B, pCCMS->new_Y, h, w);

    gauss_pyramid(pCCMS->new_R, pCCMS->new_R_1, h, w);
    gauss_pyramid(pCCMS->new_R_1, pCCMS->new_R_2, h/2, w/2);
    gauss_pyramid(pCCMS->new_R_2, pCCMS->new_R_3, h/4, w/4);
    gauss_pyramid(pCCMS->new_R_3, pCCMS->new_R_4, h/8, w/8);
    gauss_pyramid(pCCMS->new_R_4, pCCMS->new_R_5, h/16, w/16);

    gauss_pyramid(pCCMS->new_G, pCCMS->new_G_1, h, w);
    gauss_pyramid(pCCMS->new_G_1, pCCMS->new_G_2, h/2, w/2);
    gauss_pyramid(pCCMS->new_G_2, pCCMS->new_G_3, h/4, w/4);
    gauss_pyramid(pCCMS->new_G_3, pCCMS->new_G_4, h/8, w/8);
    gauss_pyramid(pCCMS->new_G_4, pCCMS->new_G_5, h/16, w/16);

    gauss_pyramid(pCCMS->new_B, pCCMS->new_B_1, h, w);
    gauss_pyramid(pCCMS->new_B_1, pCCMS->new_B_2, h/2, w/2);
    gauss_pyramid(pCCMS->new_B_2, pCCMS->new_B_3, h/4, w/4);
    gauss_pyramid(pCCMS->new_B_3, pCCMS->new_B_4, h/8, w/8);
    gauss_pyramid(pCCMS->new_B_4, pCCMS->new_B_5, h/16, w/16);

    gauss_pyramid(pCCMS->new_Y, pCCMS->new_Y_1, h, w);
    gauss_pyramid(pCCMS->new_Y_1, pCCMS->new_Y_2, h/2, w/2);
    gauss_pyramid(pCCMS->new_Y_2, pCCMS->new_Y_3, h/4, w/4);
    gauss_pyramid(pCCMS->new_Y_3, pCCMS->new_Y_4, h/8, w/8);
    gauss_pyramid(pCCMS->new_Y_4, pCCMS->new_Y_5, h/16, w/16);

    expand_pyramid(pCCMS->new_R_5, pCCMS->Expand_R_25, h/32, w/32, 3);
    expand_pyramid(pCCMS->new_G_5, pCCMS->Expand_G_25, h/32, w/32, 3);
    expand_pyramid(pCCMS->new_B_5, pCCMS->Expand_B_25, h/32, w/32, 3);
    expand_pyramid(pCCMS->new_Y_5, pCCMS->Expand_Y_25, h/32, w/32, 3);

    {
        int     nQWidth = w / 4;
        int     nQHeight = h / 4;
        int     nPos, temp;
        int     i, j, x;
        
        for(j=0 ; j<nQHeight ; j++)
        {
            for(i=0 ; i<nQWidth ; i++)
            {
                nPos = j*nQWidth + i;

                temp = (new_G_2[nPos] - new_R_2[nPos]);
                temp2 = (Expand_G_25[nPos] - Expand_R_25[nPos]);
                temp3 = abs(temp - temp2);
                if (temp3>255) RG_out[nPos] = 255;
                else RG_out[nPos] = temp3;


                temp = (new_B_2[nPos] - new_Y_2[nPos]);
                temp2 = (Expand_B_25[nPos] - Expand_Y_25[nPos]);
                temp3 = abs(temp - temp2);
                if (temp3>255) BY_out[nPos] = 255;
                else BY_out[nPos] = temp3;
            }
        }

        normalization(pCCMS->RG_out, nQHeight, nQWidth); 
        normalization(pCCMS->BY_out, nQHeight, nQWidth); 

        {
            double var_c0=0, var_c1=0, c=0;

            var_c0 = normal(pCCMS->RG_out, nQHeight, nQWidth); 
            var_c1 = normal(pCCMS->BY_out, nQHeight, nQWidth); 

            c = var_c0 + var_c1;
            
            x = 0;
            do
            {
                c_FeatureMap[x] = (unsigned char)((RG_out[x]*var_c0 + BY_out[x]*var_c1) / c);
                x++;
            }while(x < (nQWidth * nQHeight));
        }
    }
}
#else
void Color_CSM(C_CMS *pCCMS, unsigned char *In, unsigned char *c_FeatureMap, int h, int w)
{
    int i, j, temp, temp2, temp3;
    unsigned char *RG_out, *BY_out;
    unsigned char *new_R, *new_R_1, *new_R_2, *new_R_3, *new_R_4, *new_R_5, *new_R_6, *new_R_7, *new_R_8;
    unsigned char *new_G, *new_G_1, *new_G_2, *new_G_3, *new_G_4, *new_G_5, *new_G_6, *new_G_7, *new_G_8;
    unsigned char *new_B, *new_B_1, *new_B_2, *new_B_3, *new_B_4, *new_B_5, *new_B_6, *new_B_7, *new_B_8;
    unsigned char *new_Y, *new_Y_1, *new_Y_2, *new_Y_3, *new_Y_4, *new_Y_5, *new_Y_6, *new_Y_7, *new_Y_8;
    unsigned char *Expand_R_23, *Expand_R_24, *Expand_R_25, *Expand_R_26, *Expand_R_27, *Expand_R_28;
    unsigned char *Expand_G_23, *Expand_G_24, *Expand_G_25, *Expand_G_26, *Expand_G_27, *Expand_G_28;
    unsigned char *Expand_B_23, *Expand_B_24, *Expand_B_25, *Expand_B_26, *Expand_B_27, *Expand_B_28;
    unsigned char *Expand_Y_23, *Expand_Y_24, *Expand_Y_25, *Expand_Y_26, *Expand_Y_27, *Expand_Y_28;
    unsigned char *RG_25, *RG_26, *RG_36, *RG_37, *RG_47, *RG_48, *BY_25, *BY_26, *BY_36, *BY_37, *BY_47, *BY_48;
    double var_c0=0, var_c1=0, var_c2=0, var_c3=0, var_c4=0, var_c5=0, var_c6=0, var_c7=0,
    var_c8=0, var_c9=0, var_c10=0, var_c11=0, var_c12=0, var_c13=0, c=0, c_t0=0, c_t1=0;

    new_R       = pCCMS->new_R;
    new_G       = pCCMS->new_G;
    new_B       = pCCMS->new_B;
    new_Y       = pCCMS->new_Y;
    new_R_1     = pCCMS->new_R_1;
    new_R_2     = pCCMS->new_R_2;
    new_R_3     = pCCMS->new_R_3;
    new_R_4     = pCCMS->new_R_4;
    new_R_5     = pCCMS->new_R_5;
    new_R_6     = pCCMS->new_R_6;
    new_R_7     = pCCMS->new_R_7;
    new_R_8     = pCCMS->new_R_8;
    new_G_1     = pCCMS->new_G_1;
    new_G_2     = pCCMS->new_G_2;
    new_G_3     = pCCMS->new_G_3;
    new_G_4     = pCCMS->new_G_4;
    new_G_5     = pCCMS->new_G_5;
    new_G_6     = pCCMS->new_G_6;
    new_G_7     = pCCMS->new_G_7;
    new_G_8     = pCCMS->new_G_8;
    new_B_1     = pCCMS->new_B_1;
    new_B_2     = pCCMS->new_B_2;
    new_B_3     = pCCMS->new_B_3;
    new_B_4     = pCCMS->new_B_4;
    new_B_5     = pCCMS->new_B_5;
    new_B_6     = pCCMS->new_B_6;
    new_B_7     = pCCMS->new_B_7;
    new_B_8     = pCCMS->new_B_8;
    new_Y_1     = pCCMS->new_Y_1;
    new_Y_2     = pCCMS->new_Y_2;
    new_Y_3     = pCCMS->new_Y_3;
    new_Y_4     = pCCMS->new_Y_4;
    new_Y_5     = pCCMS->new_Y_5;
    new_Y_6     = pCCMS->new_Y_6;
    new_Y_7     = pCCMS->new_Y_7;
    new_Y_8     = pCCMS->new_Y_8;
    Expand_R_23 = pCCMS->Expand_R_23;
    Expand_R_24 = pCCMS->Expand_R_24;
    Expand_R_25 = pCCMS->Expand_R_25;
    Expand_R_26 = pCCMS->Expand_R_26;
    Expand_R_27 = pCCMS->Expand_R_27;
    Expand_R_28 = pCCMS->Expand_R_28;
    Expand_G_23 = pCCMS->Expand_G_23;
    Expand_G_24 = pCCMS->Expand_G_24;
    Expand_G_25 = pCCMS->Expand_G_25;
    Expand_G_26 = pCCMS->Expand_G_26;
    Expand_G_27 = pCCMS->Expand_G_27;
    Expand_G_28 = pCCMS->Expand_G_28;
    Expand_B_23 = pCCMS->Expand_B_23;
    Expand_B_24 = pCCMS->Expand_B_24;
    Expand_B_25 = pCCMS->Expand_B_25;
    Expand_B_26 = pCCMS->Expand_B_26;
    Expand_B_27 = pCCMS->Expand_B_27;
    Expand_B_28 = pCCMS->Expand_B_28;
    Expand_Y_23 = pCCMS->Expand_Y_23;
    Expand_Y_24 = pCCMS->Expand_Y_24;
    Expand_Y_25 = pCCMS->Expand_Y_25;
    Expand_Y_26 = pCCMS->Expand_Y_26;
    Expand_Y_27 = pCCMS->Expand_Y_27;
    Expand_Y_28 = pCCMS->Expand_Y_28;
    RG_25       = pCCMS->RG_25;
    RG_26       = pCCMS->RG_26;
    RG_36       = pCCMS->RG_36;
    RG_37       = pCCMS->RG_37;
    RG_47       = pCCMS->RG_47;
    RG_48       = pCCMS->RG_48;
    BY_25       = pCCMS->BY_25;
    BY_26       = pCCMS->BY_26;
    BY_36       = pCCMS->BY_36;
    BY_37       = pCCMS->BY_37;
    BY_47       = pCCMS->BY_47;
    BY_48       = pCCMS->BY_48;
    RG_out      = pCCMS->RG_out;
    BY_out      = pCCMS->BY_out;  

    new_RBG(In, new_R, new_G, new_B, new_Y, h, w);

    gauss_pyramid(new_R, new_R_1, h, w);
    gauss_pyramid(new_R_1, new_R_2, h/2, w/2);
    gauss_pyramid(new_R_2, new_R_3, h/4, w/4);
    gauss_pyramid(new_R_3, new_R_4, h/8, w/8);
    gauss_pyramid(new_R_4, new_R_5, h/16, w/16);
    gauss_pyramid(new_R_5, new_R_6, h/32, w/32);
    gauss_pyramid(new_R_6, new_R_7, h/64, w/64);
    gauss_pyramid(new_R_7, new_R_8, h/128, w/128);

    gauss_pyramid(new_G, new_G_1, h, w);
    gauss_pyramid(new_G_1, new_G_2, h/2, w/2);
    gauss_pyramid(new_G_2, new_G_3, h/4, w/4);
    gauss_pyramid(new_G_3, new_G_4, h/8, w/8);
    gauss_pyramid(new_G_4, new_G_5, h/16, w/16);
    gauss_pyramid(new_G_5, new_G_6, h/32, w/32);
    gauss_pyramid(new_G_6, new_G_7, h/64, w/64);
    gauss_pyramid(new_G_7, new_G_8, h/128, w/128);

    gauss_pyramid(new_B, new_B_1, h, w);
    gauss_pyramid(new_B_1, new_B_2, h/2, w/2);
    gauss_pyramid(new_B_2, new_B_3, h/4, w/4);
    gauss_pyramid(new_B_3, new_B_4, h/8, w/8);
    gauss_pyramid(new_B_4, new_B_5, h/16, w/16);
    gauss_pyramid(new_B_5, new_B_6, h/32, w/32);
    gauss_pyramid(new_B_6, new_B_7, h/64, w/64);
    gauss_pyramid(new_B_7, new_B_8, h/128, w/128);

    gauss_pyramid(new_Y, new_Y_1, h, w);
    gauss_pyramid(new_Y_1, new_Y_2, h/2, w/2);
    gauss_pyramid(new_Y_2, new_Y_3, h/4, w/4);
    gauss_pyramid(new_Y_3, new_Y_4, h/8, w/8);
    gauss_pyramid(new_Y_4, new_Y_5, h/16, w/16);
    gauss_pyramid(new_Y_5, new_Y_6, h/32, w/32);
    gauss_pyramid(new_Y_6, new_Y_7, h/64, w/64);
    gauss_pyramid(new_Y_7, new_Y_8, h/128, w/128);

    expand_pyramid(new_R_3, Expand_R_23, h/8, w/8, 1);
    expand_pyramid(new_R_4, Expand_R_24, h/16, w/16, 2);
    expand_pyramid(new_R_5, Expand_R_25, h/32, w/32, 3);
    expand_pyramid(new_R_6, Expand_R_26, h/64, w/64, 4);
    expand_pyramid(new_R_7, Expand_R_27, h/128, w/128, 5);
    expand_pyramid(new_R_8, Expand_R_28, h/256, w/256, 6);

    expand_pyramid(new_G_3, Expand_G_23, h/8, w/8, 1);
    expand_pyramid(new_G_4, Expand_G_24, h/16, w/16, 2);
    expand_pyramid(new_G_5, Expand_G_25, h/32, w/32, 3);
    expand_pyramid(new_G_6, Expand_G_26, h/64, w/64, 4);
    expand_pyramid(new_G_7, Expand_G_27, h/128, w/128, 5);
    expand_pyramid(new_G_8, Expand_G_28, h/256, w/256, 6);

    expand_pyramid(new_B_3, Expand_B_23, h/8, w/8, 1);
    expand_pyramid(new_B_4, Expand_B_24, h/16, w/16, 2);
    expand_pyramid(new_B_5, Expand_B_25, h/32, w/32, 3);
    expand_pyramid(new_B_6, Expand_B_26, h/64, w/64, 4);
    expand_pyramid(new_B_7, Expand_B_27, h/128, w/128, 5);
    expand_pyramid(new_B_8, Expand_B_28, h/256, w/256, 6);

    expand_pyramid(new_Y_3, Expand_Y_23, h/8, w/8, 1);
    expand_pyramid(new_Y_4, Expand_Y_24, h/16, w/16, 2);
    expand_pyramid(new_Y_5, Expand_Y_25, h/32, w/32, 3);
    expand_pyramid(new_Y_6, Expand_Y_26, h/64, w/64, 4);
    expand_pyramid(new_Y_7, Expand_Y_27, h/128, w/128, 5);
    expand_pyramid(new_Y_8, Expand_Y_28, h/256, w/256, 6);

    // 현재는 scale 1에로 모든 영상 크기를 맞추어 놓았음
    
    for (j=0; j<h/4; j++)
    {
        for (i=0; i<w/4; i++)
        {
        //    temp = (new_R_2[j*(w/4)+i] - new_G_2[j*(w/4)+i]);
            temp = (new_G_2[j*(w/4)+i] - new_R_2[j*(w/4)+i]);
        //    temp = temp >= 0 ? temp : 0;
            temp2 = (Expand_G_25[j*(w/4)+i] - Expand_R_25[j*(w/4)+i]);
        //    temp2 = (Expand_R_25[j*(w/4)+i] - Expand_G_25[j*(w/4)+i]);
            temp3 = abs(temp - temp2);
            if (temp3>255) RG_25[j*(w/4)+i] = 255;
            else RG_25[j*(w/4)+i] = temp3;

            temp2 = (Expand_G_26[j*(w/4)+i] - Expand_R_26[j*(w/4)+i]);
        //    temp2 = (Expand_R_26[j*(w/4)+i] - Expand_G_26[j*(w/4)+i]);
            temp3 = abs(temp - temp2);
            if (temp3>255) RG_26[j*(w/4)+i] = 255;
            else RG_26[j*(w/4)+i] = temp3;

        //    temp = (Expand_R_23[j*(w/4)+i] - Expand_G_23[j*(w/4)+i]);
            temp = (Expand_G_23[j*(w/4)+i] - Expand_R_23[j*(w/4)+i]);
        //    temp = temp >= 0 ? temp : 0;
            temp2 = (Expand_G_26[j*(w/4)+i] - Expand_R_26[j*(w/4)+i]);
        //    temp2 = (Expand_R_26[j*(w/4)+i] - Expand_G_26[j*(w/4)+i]);
            temp3 = abs(temp - temp2);
            if (temp3>255) RG_36[j*(w/4)+i] = 255;
            else RG_36[j*(w/4)+i] = temp3;

            temp2 = (Expand_G_27[j*(w/4)+i] - Expand_R_27[j*(w/4)+i]);
        //    temp2 = (Expand_R_27[j*(w/4)+i] - Expand_G_27[j*(w/4)+i]);
            temp3 = abs(temp - temp2);
            if (temp3>255) RG_37[j*(w/4)+i] = 255;
            else RG_37[j*(w/4)+i] = temp3;

        //    temp = (Expand_R_24[j*(w/4)+i] - Expand_G_24[j*(w/4)+i]);
            temp = (Expand_G_24[j*(w/4)+i] - Expand_R_24[j*(w/4)+i]);
        //    temp = temp >= 0 ? temp : 0;
            temp2 = (Expand_G_27[j*(w/4)+i] - Expand_R_27[j*(w/4)+i]);
        //    temp2 = (Expand_R_27[j*(w/4)+i] - Expand_G_27[j*(w/4)+i]);
            temp3 = abs(temp - temp2);
            if (temp3>255) RG_47[j*(w/4)+i] = 255;
            else RG_47[j*(w/4)+i] = temp3;

            temp2 = (Expand_G_28[j*(w/4)+i] - Expand_R_28[j*(w/4)+i]);
        //    temp2 = (Expand_R_28[j*(w/4)+i] - Expand_G_28[j*(w/4)+i]);
            temp3 = abs(temp - temp2);
            if (temp3>255) RG_48[j*(w/4)+i] = 255;
            else RG_48[j*(w/4)+i] = temp3;

            temp = (new_B_2[j*(w/4)+i] - new_Y_2[j*(w/4)+i]);
        //    temp = temp >= 0 ? temp : 0;
        //    temp2 = (Expand_Y_25[j*(w/4)+i] - Expand_B_25[j*(w/4)+i]);
            temp2 = (Expand_B_25[j*(w/4)+i] - Expand_Y_25[j*(w/4)+i]);
            temp3 = abs(temp - temp2);
            if (temp3>255) BY_25[j*(w/4)+i] = 255;
            else BY_25[j*(w/4)+i] = temp3;

        //    temp2 = (Expand_Y_26[j*(w/4)+i] - Expand_B_26[j*(w/4)+i]);
            temp2 = (Expand_B_26[j*(w/4)+i] - Expand_Y_26[j*(w/4)+i]);
            temp3 = abs(temp - temp2);
            if (temp3>255) BY_26[j*(w/4)+i] = 255;
            else BY_26[j*(w/4)+i] = temp3;

            temp = (Expand_B_23[j*(w/4)+i] - Expand_Y_23[j*(w/4)+i]);
        //    temp = temp >= 0 ? temp : 0;
        //    temp2 = (Expand_Y_26[j*(w/4)+i] - Expand_B_26[j*(w/4)+i]);
            temp2 = (Expand_B_26[j*(w/4)+i] - Expand_Y_26[j*(w/4)+i]);
            temp3 = abs(temp - temp2);
            if (temp3>255) BY_36[j*(w/4)+i] = 255;
            else BY_36[j*(w/4)+i] = temp3;

        //    temp2 = (Expand_Y_27[j*(w/4)+i] - Expand_B_27[j*(w/4)+i]);
            temp2 = (Expand_B_27[j*(w/4)+i] - Expand_Y_27[j*(w/4)+i]);
            temp3 = abs(temp - temp2);
            if (temp3>255) BY_37[j*(w/4)+i] = 255;
            else BY_37[j*(w/4)+i] = temp3;

            temp = (Expand_B_24[j*(w/4)+i] - Expand_Y_24[j*(w/4)+i]);
        //    temp = temp >= 0 ? temp : 0;
        //    temp2 = (Expand_Y_27[j*(w/4)+i] - Expand_B_27[j*(w/4)+i]);
            temp2 = (Expand_B_27[j*(w/4)+i] - Expand_Y_27[j*(w/4)+i]);
            temp3 = abs(temp - temp2);
            if (temp3>255) BY_47[j*(w/4)+i] = 255;
            else BY_47[j*(w/4)+i] = temp3;

        //    temp2 = (Expand_Y_28[j*(w/4)+i] - Expand_B_28[j*(w/4)+i]);
            temp2 = (Expand_B_28[j*(w/4)+i] - Expand_Y_28[j*(w/4)+i]);
            temp3 = abs(temp - temp2);
            if (temp3>255) BY_48[j*(w/4)+i] = 255;
            else BY_48[j*(w/4)+i] = temp3;
        }
    }

    normalization(RG_25, h/4, w/4); normalization(BY_25, h/4, w/4);
    normalization(RG_26, h/4, w/4); normalization(BY_26, h/4, w/4);
    normalization(RG_36, h/4, w/4); normalization(BY_36, h/4, w/4);
    normalization(RG_37, h/4, w/4); normalization(BY_37, h/4, w/4);
    normalization(RG_47, h/4, w/4); normalization(BY_47, h/4, w/4);
    normalization(RG_48, h/4, w/4); normalization(BY_48, h/4, w/4);


    var_c0 = normal(RG_25, h/4, w/4); var_c6 = normal(BY_25, h/4, w/4);
    var_c1 = normal(RG_26, h/4, w/4); var_c7 = normal(BY_26, h/4, w/4);
    var_c2 = normal(RG_36, h/4, w/4); var_c8 = normal(BY_36, h/4, w/4);
    var_c3 = normal(RG_37, h/4, w/4); var_c9 = normal(BY_37, h/4, w/4);
    var_c4 = normal(RG_47, h/4, w/4); var_c10 = normal(BY_47, h/4, w/4);
    var_c5 = normal(RG_48, h/4, w/4); var_c11 = normal(BY_48, h/4, w/4);

    c_t0 = var_c0+var_c1+var_c2+var_c3+var_c4+var_c5;
    c_t1 = var_c6+var_c7+var_c8+var_c9+var_c10+var_c11;

    for(i=0; i<h/4; i++)
    {
        for(j=0; j<w/4; j++)
        {
            RG_out[i*w/4+j] = (RG_25[i*w/4+j]*var_c0+RG_26[i*w/4+j]*var_c1+RG_36[i*w/4+j]*var_c2+RG_37[i*w/4+j]*var_c3
                                            +RG_47[i*w/4+j]*var_c4+RG_48[i*w/4+j]*var_c5)/c_t0;
            BY_out[i*w/4+j] = (BY_25[i*w/4+j]*var_c6+BY_26[i*w/4+j]*var_c7+BY_36[i*w/4+j]*var_c8+BY_37[i*w/4+j]*var_c9
                                            +BY_47[i*w/4+j]*var_c10+BY_48 [i*w/4+j]*var_c11)/c_t1;
        }
    }


    normalization(RG_out, h/4, w/4);
    normalization(BY_out, h/4, w/4);

    var_c12 = (RG_out, h/4, w/4);
    var_c13 = (BY_out, h/4, w/4);

    c = var_c12 + var_c13;

    for(i=0; i<h/4; i++)
        for(j=0; j<w/4; j++)
            c_FeatureMap[i*w/4+j] = (RG_out[i*w/4+j]*var_c12 + BY_out[i*w/4+j]*var_c13)/c;
}

    

#endif
    
