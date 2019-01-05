#include "PT_VisualAttention.h"

void Gabor_CSM(G_CMS *pGCMS, unsigned char *In, unsigned char *o_FeatureMap, int h, int w)
{
    int i, j, Lw = lW, Lh = lH;
    double angle;

    unsigned char *tran, *trans, *g0_out, *g1_out, *g2_out, *g3_out;
    unsigned char *Org_0, *Org_1, *Org_2, *Org_3, *Org_4, *Org_5, *Org_6, *Org_7, *Org_8;
    unsigned char *ExOrg_23, *ExOrg_24, *ExOrg_25, *ExOrg_26, *ExOrg_27, *ExOrg_28;
    unsigned char *Gabor_2_0, *Gabor_3_0, *Gabor_4_0, *Gabor_5_0, *Gabor_6_0, *Gabor_7_0, *Gabor_8_0;
    unsigned char *Gabor_2_45, *Gabor_3_45, *Gabor_4_45, *Gabor_5_45, *Gabor_6_45, *Gabor_7_45, *Gabor_8_45;
    unsigned char *Gabor_2_90, *Gabor_3_90, *Gabor_4_90, *Gabor_5_90, *Gabor_6_90, *Gabor_7_90, *Gabor_8_90;
    unsigned char *Gabor_2_135, *Gabor_3_135, *Gabor_4_135, *Gabor_5_135, *Gabor_6_135, *Gabor_7_135, *Gabor_8_135;
    unsigned char *ExGabor_2_0, *ExGabor_3_0, *ExGabor_4_0, *ExGabor_5_0, *ExGabor_6_0, *ExGabor_7_0, *ExGabor_8_0;
    unsigned char *ExGabor_2_45, *ExGabor_3_45, *ExGabor_4_45, *ExGabor_5_45, *ExGabor_6_45, *ExGabor_7_45, *ExGabor_8_45;
    unsigned char *ExGabor_2_90, *ExGabor_3_90, *ExGabor_4_90, *ExGabor_5_90, *ExGabor_6_90, *ExGabor_7_90, *ExGabor_8_90;
    unsigned char *ExGabor_2_135, *ExGabor_3_135, *ExGabor_4_135, *ExGabor_5_135, *ExGabor_6_135, *ExGabor_7_135, *ExGabor_8_135;

    unsigned char *Gabor_25_0, *Gabor_26_0, *Gabor_36_0, *Gabor_37_0, *Gabor_47_0, *Gabor_48_0;
    unsigned char *Gabor_25_45, *Gabor_26_45, *Gabor_36_45, *Gabor_37_45, *Gabor_47_45, *Gabor_48_45;
    unsigned char *Gabor_25_90, *Gabor_26_90, *Gabor_36_90, *Gabor_37_90, *Gabor_47_90, *Gabor_48_90;
    unsigned char *Gabor_25_135, *Gabor_26_135, *Gabor_36_135, *Gabor_37_135, *Gabor_47_135, *Gabor_48_135;

    double var_g0=0, var_g1=0, var_g2=0, var_g3=0, var_g4=0, var_g5=0, var_g6=0, var_g7=0, var_g8=0, var_g9=0,
                var_g10=0, var_g11=0, var_g12=0, var_g13=0, var_g14=0, var_g15=0, var_g16=0, var_g17=0, var_g18=0,
                var_g19=0, var_g20=0, var_g21=0, var_g22=0, var_g23=0, var_g24=0, var_g25=0, var_g26=0, var_g27=0,
                g_t0=0, g_t1=0, g_t2=0, g_t3=0, g=0;
    

    tran          = pGCMS->tran;
    trans         = pGCMS->trans;
    ExGabor_2_0   = pGCMS->ExGabor_2_0;
    ExGabor_3_0   = pGCMS->ExGabor_3_0;
    ExGabor_4_0   = pGCMS->ExGabor_4_0;
    ExGabor_5_0   = pGCMS->ExGabor_5_0;
    ExGabor_6_0   = pGCMS->ExGabor_6_0;
    ExGabor_7_0   = pGCMS->ExGabor_7_0;
    ExGabor_8_0   = pGCMS->ExGabor_8_0;
    ExGabor_2_45  = pGCMS->ExGabor_2_45;
    ExGabor_3_45  = pGCMS->ExGabor_3_45;
    ExGabor_4_45  = pGCMS->ExGabor_4_45;
    ExGabor_5_45  = pGCMS->ExGabor_5_45;
    ExGabor_6_45  = pGCMS->ExGabor_6_45;
    ExGabor_7_45  = pGCMS->ExGabor_7_45;
    ExGabor_8_45  = pGCMS->ExGabor_8_45;
    ExGabor_2_90  = pGCMS->ExGabor_2_90;
    ExGabor_3_90  = pGCMS->ExGabor_3_90;
    ExGabor_4_90  = pGCMS->ExGabor_4_90;
    ExGabor_5_90  = pGCMS->ExGabor_5_90;
    ExGabor_6_90  = pGCMS->ExGabor_6_90;
    ExGabor_7_90  = pGCMS->ExGabor_7_90;
    ExGabor_8_90  = pGCMS->ExGabor_8_90;
    ExGabor_2_135 = pGCMS->ExGabor_2_135;
    ExGabor_3_135 = pGCMS->ExGabor_3_135;
    ExGabor_4_135 = pGCMS->ExGabor_4_135;
    ExGabor_5_135 = pGCMS->ExGabor_5_135;
    ExGabor_6_135 = pGCMS->ExGabor_6_135;
    ExGabor_7_135 = pGCMS->ExGabor_7_135;
    ExGabor_8_135 = pGCMS->ExGabor_8_135;
    Gabor_25_0    = pGCMS->Gabor_25_0;
    Gabor_26_0    = pGCMS->Gabor_26_0;
    Gabor_36_0    = pGCMS->Gabor_36_0;
    Gabor_37_0    = pGCMS->Gabor_37_0;
    Gabor_47_0    = pGCMS->Gabor_47_0;
    Gabor_48_0    = pGCMS->Gabor_48_0;
    Gabor_25_45   = pGCMS->Gabor_25_45;
    Gabor_26_45   = pGCMS->Gabor_26_45;
    Gabor_36_45   = pGCMS->Gabor_36_45;
    Gabor_37_45   = pGCMS->Gabor_37_45;
    Gabor_47_45   = pGCMS->Gabor_47_45;
    Gabor_48_45   = pGCMS->Gabor_48_45;
    Gabor_25_90   = pGCMS->Gabor_25_90;
    Gabor_26_90   = pGCMS->Gabor_26_90;
    Gabor_36_90   = pGCMS->Gabor_36_90;
    Gabor_37_90   = pGCMS->Gabor_37_90;
    Gabor_47_90   = pGCMS->Gabor_47_90;
    Gabor_48_90   = pGCMS->Gabor_48_90;
    Gabor_25_135  = pGCMS->Gabor_25_135;
    Gabor_26_135  = pGCMS->Gabor_26_135;
    Gabor_36_135  = pGCMS->Gabor_36_135;
    Gabor_37_135  = pGCMS->Gabor_37_135;
    Gabor_47_135  = pGCMS->Gabor_47_135;
    Gabor_48_135  = pGCMS->Gabor_48_135;
    g0_out        = pGCMS->g0_out;
    g1_out        = pGCMS->g1_out;
    g2_out        = pGCMS->g2_out;
    g3_out        = pGCMS->g3_out;
    ExOrg_23      = pGCMS->ExOrg_23;
    ExOrg_24      = pGCMS->ExOrg_24;
    ExOrg_25      = pGCMS->ExOrg_25;
    ExOrg_26      = pGCMS->ExOrg_26;
    ExOrg_27      = pGCMS->ExOrg_27;
    ExOrg_28      = pGCMS->ExOrg_28;
    Gabor_2_0     = pGCMS->Gabor_2_0;
    Gabor_3_0     = pGCMS->Gabor_3_0;
    Gabor_4_0     = pGCMS->Gabor_4_0;
    Gabor_5_0     = pGCMS->Gabor_5_0;
    Gabor_6_0     = pGCMS->Gabor_6_0;
    Gabor_7_0     = pGCMS->Gabor_7_0;
    Gabor_8_0     = pGCMS->Gabor_8_0;
    Gabor_2_45    = pGCMS->Gabor_2_45;
    Gabor_3_45    = pGCMS->Gabor_3_45;
    Gabor_4_45    = pGCMS->Gabor_4_45;
    Gabor_5_45    = pGCMS->Gabor_5_45;
    Gabor_6_45    = pGCMS->Gabor_6_45;
    Gabor_7_45    = pGCMS->Gabor_7_45;
    Gabor_8_45    = pGCMS->Gabor_8_45;
    Gabor_2_90    = pGCMS->Gabor_2_90;
    Gabor_3_90    = pGCMS->Gabor_3_90;
    Gabor_4_90    = pGCMS->Gabor_4_90;
    Gabor_5_90    = pGCMS->Gabor_5_90;
    Gabor_6_90    = pGCMS->Gabor_6_90;
    Gabor_7_90    = pGCMS->Gabor_7_90;
    Gabor_8_90    = pGCMS->Gabor_8_90;
    Gabor_2_135   = pGCMS->Gabor_2_135;
    Gabor_3_135   = pGCMS->Gabor_3_135;
    Gabor_4_135   = pGCMS->Gabor_4_135;
    Gabor_5_135   = pGCMS->Gabor_5_135;
    Gabor_6_135   = pGCMS->Gabor_6_135;
    Gabor_7_135   = pGCMS->Gabor_7_135;
    Gabor_8_135   = pGCMS->Gabor_8_135;
    Org_0         = pGCMS->Org_0;
    Org_1         = pGCMS->Org_1;
    Org_2         = pGCMS->Org_2;
    Org_3         = pGCMS->Org_3;
    Org_4         = pGCMS->Org_4;
    Org_5         = pGCMS->Org_5;
    Org_6         = pGCMS->Org_6;
    Org_7         = pGCMS->Org_7;
    Org_8         = pGCMS->Org_8;

    convert_gray(In, trans, h, w);

    for(i=0; i<lH; i++)
    {
        for(j=0; j<lW; j++)
        {
            if(j<w && i<h)
            //    if(i<512 && j<768)
            {
                Org_0[i*lW+j] = trans[i*w+j];
            }
            else
            {
                Org_0[i*lW+j] = Org_0[i*lW+j];
            }
        }
    }

    gauss_pyramid(Org_0, Org_1, Lh, Lw );
    gauss_pyramid(Org_1, Org_2, Lh/2, Lw/2);
    gauss_pyramid(Org_2, Org_3, Lh/4, Lw/4);
    gauss_pyramid(Org_3, Org_4, Lh/8, Lw/8);
    gauss_pyramid(Org_4, Org_5, Lh/16, Lw/16);
    gauss_pyramid(Org_5, Org_6, Lh/32, Lw/32);
    gauss_pyramid(Org_6, Org_7, Lh/64, Lw/64);
    gauss_pyramid(Org_7, Org_8, Lh/128, Lw/128);

    expand_pyramid(Org_3, ExOrg_23, Lh/8, Lw/8, 1);
    expand_pyramid(Org_4, ExOrg_24, Lh/16, Lw/16, 2);
    expand_pyramid(Org_5, ExOrg_25, Lh/32, Lw/32, 3);
    expand_pyramid(Org_6, ExOrg_26, Lh/64, Lw/64, 4);
    expand_pyramid(Org_7, ExOrg_27, Lh/128, Lw/128, 5);
    expand_pyramid(Org_8, ExOrg_28, Lh/256, Lw/256, 6);

    ////////////////////////////////////////
    // Gabor filter Orientation 측정 함수 //
    ////////////////////////////////////////
    // distance가 주파수를 의미하고, frequency는 표준편차를 의미한다. 
    // 현재 x,y축으로 같은 표준편차를 가지도록 세팅되어 있다. 

    double distance = 13, frequency = 0.09;//0.03;//2/distance ;
    angle = 0;

    //Gabor_0_0 = Gabor_filter (inputData, 512, 512, angle);
    //    Gabor_2_0 = Gabor_filter(Org_2, Lh/4, Lw/4, angle, frequency, distance);
    Gabor_filter(Org_2, Gabor_2_0, Lh/4, Lw/4, angle, frequency, distance);
    //free(Rec_2);

    //    distance = 15; frequency = 0.07;
    /*
    Gabor_3_0 = Gabor_filter(ExOrg_23, Lh/4, Lw/4, angle, frequency, distance);
    Gabor_4_0 = Gabor_filter(ExOrg_24, Lh/4, Lw/4, angle, frequency, distance);
    Gabor_5_0 = Gabor_filter(ExOrg_25, Lh/4, Lw/4, angle, frequency, distance);
    Gabor_6_0 = Gabor_filter(ExOrg_26, Lh/4, Lw/4, angle, frequency, distance);
    Gabor_7_0 = Gabor_filter(ExOrg_27, Lh/4, Lw/4, angle, frequency, distance);
    Gabor_8_0 = Gabor_filter(ExOrg_28, Lh/4, Lw/4, angle, frequency, distance);
    */
    Gabor_filter(ExOrg_23, Gabor_3_0, Lh/4, Lw/4, angle, frequency, distance);
    Gabor_filter(ExOrg_24, Gabor_4_0, Lh/4, Lw/4, angle, frequency, distance);
    Gabor_filter(ExOrg_25, Gabor_5_0, Lh/4, Lw/4, angle, frequency, distance);
    Gabor_filter(ExOrg_26, Gabor_6_0, Lh/4, Lw/4, angle, frequency, distance);
    Gabor_filter(ExOrg_27, Gabor_7_0, Lh/4, Lw/4, angle, frequency, distance);
    Gabor_filter(ExOrg_28, Gabor_8_0, Lh/4, Lw/4, angle, frequency, distance);

    for(i=0; i<h/4; i++)
    {
        for (j=0; j<w/4; j++)
        {
            ExGabor_2_0[i*w/4+j] = Gabor_2_0[i*1024/4+j];
            ExGabor_3_0[i*w/4+j] = Gabor_3_0[i*1024/4+j];        //Ex는 Expand의 뜻, 맞지는 않지만 -_-;
            ExGabor_4_0[i*w/4+j] = Gabor_4_0[i*1024/4+j];
            ExGabor_5_0[i*w/4+j] = Gabor_5_0[i*1024/4+j];
            ExGabor_6_0[i*w/4+j] = Gabor_6_0[i*1024/4+j];
            ExGabor_7_0[i*w/4+j] = Gabor_7_0[i*1024/4+j];
            ExGabor_8_0[i*w/4+j] = Gabor_8_0[i*1024/4+j];
            }
    }

    angle = pi*0.25;
    //Gabor_0_0 = Gabor_filter (inputData, 512, 512, angle);
    //    Gabor_2_45 = Gabor_filter(Org_2, Lh/4, Lw/4, angle, frequency, distance);
    Gabor_filter(Org_2, Gabor_2_45, Lh/4, Lw/4, angle, frequency, distance);
    //free(Rec_2);

    //    distance = 15; frequency = 0.05;
    //expand_pyramid (Gabor_3_0, ExRec_3, Lheight/8, Lwidth/8, 1);
    /*
    Gabor_3_45 = Gabor_filter(ExOrg_23, Lh/4, Lw/4, angle, frequency, distance);
    Gabor_4_45 = Gabor_filter(ExOrg_24, Lh/4, Lw/4, angle, frequency, distance);
    Gabor_5_45 = Gabor_filter(ExOrg_25, Lh/4, Lw/4, angle, frequency, distance);
    Gabor_6_45 = Gabor_filter(ExOrg_26, Lh/4, Lw/4, angle, frequency, distance);
    Gabor_7_45 = Gabor_filter(ExOrg_27, Lh/4, Lw/4, angle, frequency, distance);
    Gabor_8_45 = Gabor_filter(ExOrg_28, Lh/4, Lw/4, angle, frequency, distance);
    */
    Gabor_filter(ExOrg_23, Gabor_3_45, Lh/4, Lw/4, angle, frequency, distance);
    Gabor_filter(ExOrg_24, Gabor_4_45, Lh/4, Lw/4, angle, frequency, distance);
    Gabor_filter(ExOrg_25, Gabor_5_45, Lh/4, Lw/4, angle, frequency, distance);
    Gabor_filter(ExOrg_26, Gabor_6_45, Lh/4, Lw/4, angle, frequency, distance);
    Gabor_filter(ExOrg_27, Gabor_7_45, Lh/4, Lw/4, angle, frequency, distance);
    Gabor_filter(ExOrg_28, Gabor_8_45, Lh/4, Lw/4, angle, frequency, distance);

    for(i=0; i<h/4; i++)
    {
        for (j=0; j<w/4; j++)
        {
            ExGabor_2_45[i*w/4+j] = Gabor_2_45[i*1024/4+j];
            ExGabor_3_45[i*w/4+j] = Gabor_3_45[i*1024/4+j];        //Ex는 Expand의 뜻, 맞지는 않지만 -_-;
            ExGabor_4_45[i*w/4+j] = Gabor_4_45[i*1024/4+j];
            ExGabor_5_45[i*w/4+j] = Gabor_5_45[i*1024/4+j];
            ExGabor_6_45[i*w/4+j] = Gabor_6_45[i*1024/4+j];
            ExGabor_7_45[i*w/4+j] = Gabor_7_45[i*1024/4+j];
            ExGabor_8_45[i*w/4+j] = Gabor_8_45[i*1024/4+j];
        }
    }

    angle = pi*0.5;
    //Gabor_0_0 = Gabor_filter (inputData, 512, 512, angle);
    //    Gabor_2_90 = Gabor_filter(Org_2, Lh/4, Lw/4, angle, frequency, distance);
    Gabor_filter(Org_2, Gabor_2_90, Lh/4, Lw/4, angle, frequency, distance);
    //free(Rec_2);

    //    distance = 15; frequency = 0.05;
    //expand_pyramid (Gabor_3_0, ExRec_3, Lheight/8, Lwidth/8, 1);
    /*
    Gabor_3_90 = Gabor_filter(ExOrg_23, Lh/4, Lw/4, angle, frequency, distance);
    Gabor_4_90 = Gabor_filter(ExOrg_24, Lh/4, Lw/4, angle, frequency, distance);
    Gabor_5_90 = Gabor_filter(ExOrg_25, Lh/4, Lw/4, angle, frequency, distance);
    Gabor_6_90 = Gabor_filter(ExOrg_26, Lh/4, Lw/4, angle, frequency, distance);
    Gabor_7_90 = Gabor_filter(ExOrg_27, Lh/4, Lw/4, angle, frequency, distance);
    Gabor_8_90 = Gabor_filter(ExOrg_28, Lh/4, Lw/4, angle, frequency, distance);
    */
    Gabor_filter(ExOrg_23, Gabor_3_90, Lh/4, Lw/4, angle, frequency, distance);
    Gabor_filter(ExOrg_24, Gabor_4_90, Lh/4, Lw/4, angle, frequency, distance);
    Gabor_filter(ExOrg_25, Gabor_5_90, Lh/4, Lw/4, angle, frequency, distance);
    Gabor_filter(ExOrg_26, Gabor_6_90, Lh/4, Lw/4, angle, frequency, distance);
    Gabor_filter(ExOrg_27, Gabor_7_90, Lh/4, Lw/4, angle, frequency, distance);
    Gabor_filter(ExOrg_28, Gabor_8_90, Lh/4, Lw/4, angle, frequency, distance);

    for(i=0; i<h/4; i++)
    {
        for (j=0; j<w/4; j++)
        {
            ExGabor_2_90[i*w/4+j] = Gabor_2_90[i*1024/4+j];
            ExGabor_3_90[i*w/4+j] = Gabor_3_90[i*1024/4+j];        //Ex는 Expand의 뜻, 맞지는 않지만 -_-;
            ExGabor_4_90[i*w/4+j] = Gabor_4_90[i*1024/4+j];
            ExGabor_5_90[i*w/4+j] = Gabor_5_90[i*1024/4+j];
            ExGabor_6_90[i*w/4+j] = Gabor_6_90[i*1024/4+j];
            ExGabor_7_90[i*w/4+j] = Gabor_7_90[i*1024/4+j];
            ExGabor_8_90[i*w/4+j] = Gabor_8_90[i*1024/4+j];
        }
    }

    angle = pi*0.75;
    //Gabor_0_0 = Gabor_filter (inputData, 512, 512, angle);
    //    Gabor_2_135 = Gabor_filter(Org_2, Lh/4, Lw/4, angle, frequency, distance);
    Gabor_filter(Org_2, Gabor_2_135, Lh/4, Lw/4, angle, frequency, distance);
    //free(Rec_2);

    //    distance = 25; frequency = 0.055;
    //expand_pyramid (Gabor_3_0, ExRec_3, Lheight/8, Lwidth/8, 1);
    /*
    Gabor_3_135 = Gabor_filter(ExOrg_23, Lh/4, Lw/4, angle, frequency, distance);
    Gabor_4_135 = Gabor_filter(ExOrg_24, Lh/4, Lw/4, angle, frequency, distance);
    Gabor_5_135 = Gabor_filter(ExOrg_25, Lh/4, Lw/4, angle, frequency, distance);
    Gabor_6_135 = Gabor_filter(ExOrg_26, Lh/4, Lw/4, angle, frequency, distance);
    Gabor_7_135 = Gabor_filter(ExOrg_27, Lh/4, Lw/4, angle, frequency, distance);
    Gabor_8_135 = Gabor_filter(ExOrg_28, Lh/4, Lw/4, angle, frequency, distance);
    */
    Gabor_filter(ExOrg_23, Gabor_3_135, Lh/4, Lw/4, angle, frequency, distance);
    Gabor_filter(ExOrg_24, Gabor_4_135, Lh/4, Lw/4, angle, frequency, distance);
    Gabor_filter(ExOrg_25, Gabor_5_135, Lh/4, Lw/4, angle, frequency, distance);
    Gabor_filter(ExOrg_26, Gabor_6_135, Lh/4, Lw/4, angle, frequency, distance);
    Gabor_filter(ExOrg_27, Gabor_7_135, Lh/4, Lw/4, angle, frequency, distance);
    Gabor_filter(ExOrg_28, Gabor_8_135, Lh/4, Lw/4, angle, frequency, distance);


    for(i=0; i<h/4; i++)
    {
        for (j=0; j<w/4; j++)
        {
            ExGabor_2_135[i*w/4+j] = Gabor_2_135[i*1024/4+j];
            ExGabor_3_135[i*w/4+j] = Gabor_3_135[i*1024/4+j];        //Ex는 Expand의 뜻, 맞지는 않지만 -_-;
            ExGabor_4_135[i*w/4+j] = Gabor_4_135[i*1024/4+j];
            ExGabor_5_135[i*w/4+j] = Gabor_5_135[i*1024/4+j];
            ExGabor_6_135[i*w/4+j] = Gabor_6_135[i*1024/4+j];
            ExGabor_7_135[i*w/4+j] = Gabor_7_135[i*1024/4+j];
            ExGabor_8_135[i*w/4+j] = Gabor_8_135[i*1024/4+j];
        }
    }

    // 인제 Center-Surround Difference를 구해볼까나! //
    for(i=0; i<h/4; i++)
    {
        for(j=0; j<w/4; j++)
        {
            /*
            temp = ExGabor_1_0[i*w/2+j] - ExGabor_4_0[i*w/2+j];
            if(temp<=0) Gabor_14_0[i*w/2+j] = 0;
            else Gabor_14_0[i*w/2+j] = temp;
            temp = ExGabor_1_0[i*w/2+j] - ExGabor_5_0[i*w/2+j];
            if(temp<=0) Gabor_15_0[i*w/2+j] = 0;
            else Gabor_15_0[i*w/2+j] = temp;
            temp = ExGabor_2_0[i*w/2+j] - ExGabor_5_0[i*w/2+j];
            if(temp<=0) Gabor_25_0[i*w/2+j] = 0;
            else Gabor_25_0[i*w/2+j] = temp;
            temp = ExGabor_2_0[i*w/2+j] - ExGabor_6_0[i*w/2+j];
            if(temp<=0) Gabor_26_0[i*w/2+j] = 0;
            else Gabor_26_0[i*w/2+j] = temp;
            temp = ExGabor_3_0[i*w/2+j] - ExGabor_6_0[i*w/2+j];
            if(temp<=0) Gabor_36_0[i*w/2+j] = 0;
            else Gabor_36_0[i*w/2+j] = temp;
            temp = ExGabor_3_0[i*w/2+j] - ExGabor_7_0[i*w/2+j];
            if(temp<=0) Gabor_37_0[i*w/2+j] = 0;
            else Gabor_37_0[i*w/2+j] = temp;

            temp = ExGabor_1_45[i*w/2+j] - ExGabor_4_45[i*w/2+j];
            if(temp<=0) Gabor_14_45[i*w/2+j] = 0;
            else Gabor_14_45[i*w/2+j] = temp;
            temp = ExGabor_1_45[i*w/2+j] - ExGabor_5_45[i*w/2+j];
            if(temp<=0) Gabor_15_45[i*w/2+j] = 0;
            else Gabor_15_45[i*w/2+j] = temp;
            temp = ExGabor_2_45[i*w/2+j] - ExGabor_5_45[i*w/2+j];
            if(temp<=0) Gabor_25_45[i*w/2+j] = 0;
            else Gabor_25_45[i*w/2+j] = temp;
            temp = ExGabor_2_45[i*w/2+j] - ExGabor_6_45[i*w/2+j];
            if(temp<=0) Gabor_26_45[i*w/2+j] = 0;
            else Gabor_26_45[i*w/2+j] = temp;
            temp = ExGabor_3_45[i*w/2+j] - ExGabor_6_45[i*w/2+j];
            if(temp<=0) Gabor_36_45[i*w/2+j] = 0;
            else Gabor_36_45[i*w/2+j] = temp;
            temp = ExGabor_3_45[i*w/2+j] - ExGabor_7_45[i*w/2+j];
            if(temp<=0) Gabor_37_45[i*w/2+j] = 0;
            else Gabor_37_45[i*w/2+j] = temp;

            temp = ExGabor_1_90[i*w/2+j] - ExGabor_4_90[i*w/2+j];
            if(temp<=0) Gabor_14_90[i*w/2+j] = 0;
            else Gabor_14_90[i*w/2+j] = temp;
            temp = ExGabor_1_90[i*w/2+j] - ExGabor_5_90[i*w/2+j];
            if(temp<=0) Gabor_15_90[i*w/2+j] = 0;
            else Gabor_15_90[i*w/2+j] = temp;
            temp = ExGabor_2_90[i*w/2+j] - ExGabor_5_90[i*w/2+j];
            if(temp<=0) Gabor_25_90[i*w/2+j] = 0;
            else Gabor_25_90[i*w/2+j] = temp;
            temp = ExGabor_2_90[i*w/2+j] - ExGabor_6_90[i*w/2+j];
            if(temp<=0) Gabor_26_90[i*w/2+j] = 0;
            else Gabor_26_90[i*w/2+j] = temp;
            temp = ExGabor_3_90[i*w/2+j] - ExGabor_6_90[i*w/2+j];
            if(temp<=0) Gabor_36_90[i*w/2+j] = 0;
            else Gabor_36_90[i*w/2+j] = temp;
            temp = ExGabor_3_90[i*w/2+j] - ExGabor_7_90[i*w/2+j];
            if(temp<=0) Gabor_37_90[i*w/2+j] = 0;
            else Gabor_37_90[i*w/2+j] = temp;

            temp = ExGabor_1_135[i*w/2+j] - ExGabor_4_135[i*w/2+j];
            if(temp<=0) Gabor_14_135[i*w/2+j] = 0;
            else Gabor_14_135[i*w/2+j] = temp;
            temp = ExGabor_1_135[i*w/2+j] - ExGabor_5_135[i*w/2+j];
            if(temp<=0) Gabor_15_135[i*w/2+j] = 0;
            else Gabor_15_135[i*w/2+j] = temp;
            temp = ExGabor_2_135[i*w/2+j] - ExGabor_5_135[i*w/2+j];
            if(temp<=0) Gabor_25_135[i*w/2+j] = 0;
            else Gabor_25_135[i*w/2+j] = temp;
            temp = ExGabor_2_135[i*w/2+j] - ExGabor_6_135[i*w/2+j];
            if(temp<=0) Gabor_26_135[i*w/2+j] = 0;
            else Gabor_26_135[i*w/2+j] = temp;
            temp = ExGabor_3_135[i*w/2+j] - ExGabor_6_135[i*w/2+j];
            if(temp<=0) Gabor_36_135[i*w/2+j] = 0;
            else Gabor_36_135[i*w/2+j] = temp;
            temp = ExGabor_3_135[i*w/2+j] - ExGabor_7_135[i*w/2+j];
            if(temp<=0) Gabor_37_135[i*w/2+j] = 0;
            else Gabor_37_135[i*w/2+j] = temp;
            */

            Gabor_25_0[i*w/4+j] = abs(ExGabor_2_0[i*w/4+j] - ExGabor_5_0[i*w/4+j]);
            Gabor_26_0[i*w/4+j] = abs(ExGabor_2_0[i*w/4+j] - ExGabor_6_0[i*w/4+j]);
            Gabor_36_0[i*w/4+j] = abs(ExGabor_3_0[i*w/4+j] - ExGabor_6_0[i*w/4+j]);
            Gabor_37_0[i*w/4+j] = abs(ExGabor_3_0[i*w/4+j] - ExGabor_7_0[i*w/4+j]);
            Gabor_47_0[i*w/4+j] = abs(ExGabor_4_0[i*w/4+j] - ExGabor_7_0[i*w/4+j]);
            Gabor_48_0[i*w/4+j] = abs(ExGabor_4_0[i*w/4+j] - ExGabor_8_0[i*w/4+j]);

            Gabor_25_45[i*w/4+j] = abs(ExGabor_2_45[i*w/4+j] - ExGabor_5_45[i*w/4+j]);
            Gabor_26_45[i*w/4+j] = abs(ExGabor_2_45[i*w/4+j] - ExGabor_6_45[i*w/4+j]);
            Gabor_36_45[i*w/4+j] = abs(ExGabor_3_45[i*w/4+j] - ExGabor_6_45[i*w/4+j]);
            Gabor_37_45[i*w/4+j] = abs(ExGabor_3_45[i*w/4+j] - ExGabor_7_45[i*w/4+j]);
            Gabor_47_45[i*w/4+j] = abs(ExGabor_4_45[i*w/4+j] - ExGabor_7_45[i*w/4+j]);
            Gabor_48_45[i*w/4+j] = abs(ExGabor_4_45[i*w/4+j] - ExGabor_8_45[i*w/4+j]);

            Gabor_25_90[i*w/4+j] = abs(ExGabor_2_90[i*w/4+j] - ExGabor_5_90[i*w/4+j]);
            Gabor_26_90[i*w/4+j] = abs(ExGabor_2_90[i*w/4+j] - ExGabor_6_90[i*w/4+j]);
            Gabor_36_90[i*w/4+j] = abs(ExGabor_3_90[i*w/4+j] - ExGabor_6_90[i*w/4+j]);
            Gabor_37_90[i*w/4+j] = abs(ExGabor_3_90[i*w/4+j] - ExGabor_7_90[i*w/4+j]);
            Gabor_47_90[i*w/4+j] = abs(ExGabor_4_90[i*w/4+j] - ExGabor_7_90[i*w/4+j]);
            Gabor_48_90[i*w/4+j] = abs(ExGabor_4_90[i*w/4+j] - ExGabor_8_90[i*w/4+j]);

            Gabor_25_135[i*w/4+j] = abs(ExGabor_2_135[i*w/4+j] - ExGabor_5_135[i*w/4+j]);
            Gabor_26_135[i*w/4+j] = abs(ExGabor_2_135[i*w/4+j] - ExGabor_6_135[i*w/4+j]);
            Gabor_36_135[i*w/4+j] = abs(ExGabor_3_135[i*w/4+j] - ExGabor_6_135[i*w/4+j]);
            Gabor_37_135[i*w/4+j] = abs(ExGabor_3_135[i*w/4+j] - ExGabor_7_135[i*w/4+j]);
            Gabor_47_135[i*w/4+j] = abs(ExGabor_4_135[i*w/4+j] - ExGabor_7_135[i*w/4+j]);
            Gabor_48_135[i*w/4+j] = abs(ExGabor_4_135[i*w/4+j] - ExGabor_8_135[i*w/4+j]);
        }
    }

    normalization(Gabor_25_0, h/4, w/4); normalization(Gabor_26_0, h/4, w/4); normalization(Gabor_36_0, h/4, w/4);
    normalization(Gabor_37_0, h/4, w/4); normalization(Gabor_47_0, h/4, w/4); normalization(Gabor_48_0, h/4, w/4);
    normalization(Gabor_25_45, h/4, w/4); normalization(Gabor_26_45, h/4, w/4); normalization(Gabor_36_45, h/4, w/4);
    normalization(Gabor_37_45, h/4, w/4); normalization(Gabor_47_45, h/4, w/4); normalization(Gabor_48_45, h/4, w/4);
    normalization(Gabor_25_90, h/4, w/4); normalization(Gabor_26_90, h/4, w/4); normalization(Gabor_36_90, h/4, w/4);
    normalization(Gabor_37_90, h/4, w/4); normalization(Gabor_47_90, h/4, w/4); normalization(Gabor_48_90, h/4, w/4);
    normalization(Gabor_25_135, h/4, w/4); normalization(Gabor_26_135, h/4, w/4); normalization(Gabor_36_135, h/4, w/4);
    normalization(Gabor_37_135, h/4, w/4); normalization(Gabor_47_135, h/4, w/4); normalization(Gabor_48_135, h/4, w/4);

    var_g0=normal(Gabor_25_0, h/4, w/4);var_g1=normal(Gabor_26_0, h/4, w/4);var_g2=normal(Gabor_36_0, h/4, w/4); 
    var_g3=normal(Gabor_37_0, h/4, w/4);var_g4=normal(Gabor_47_0, h/4, w/4);var_g5=normal(Gabor_48_0, h/4, w/4); 
    var_g6=normal(Gabor_25_45, h/4, w/4);var_g7=normal(Gabor_26_45, h/4, w/4);var_g8=normal(Gabor_36_45, h/4, w/4); 
    var_g9=normal(Gabor_37_45, h/4, w/4);var_g10=normal(Gabor_47_45, h/4, w/4);var_g11=normal(Gabor_48_45, h/4, w/4); 
    var_g12=normal(Gabor_25_90, h/4, w/4);var_g13=normal(Gabor_26_90, h/4, w/4);var_g14=normal(Gabor_36_90, h/4, w/4); 
    var_g15=normal(Gabor_37_90, h/4, w/4);var_g16=normal(Gabor_47_90, h/4, w/4);var_g17=normal(Gabor_48_90, h/4, w/4); 
    var_g18=normal(Gabor_25_135, h/4, w/4);var_g19=normal(Gabor_26_135, h/4, w/4);var_g20=normal(Gabor_36_135, h/4, w/4); 
    var_g21=normal(Gabor_37_135, h/4, w/4);var_g22=normal(Gabor_47_135, h/4, w/4);var_g23=normal(Gabor_48_135, h/4, w/4); 

    g_t0 = var_g0 + var_g1 + var_g2 + var_g3 + var_g4 + var_g5;
    g_t1 = var_g6 + var_g7 + var_g8 + var_g9 + var_g10 + var_g11;
    g_t2 = var_g12 + var_g13 + var_g14 + var_g15 + var_g16 + var_g17;
    g_t3 = var_g18 + var_g19 + var_g20 + var_g21 + var_g22 + var_g23;


    for(i=0; i<h/4; i++)
    {
        for(j=0; j<w/4; j++)
        {
            g0_out[i*w/4+j] = (unsigned char)((Gabor_25_0[i*w/4+j]*var_g0+Gabor_26_0[i*w/4+j]*var_g1+Gabor_36_0[i*w/4+j]*var_g2
                                        +Gabor_37_0[i*w/4+j]*var_g3+Gabor_47_0[i*w/4+j]*var_g4+Gabor_48_0[i*w/4+j]*var_g5)/g_t0);
            g1_out[i*w/4+j] = (unsigned char)((Gabor_25_45[i*w/4+j]*var_g6+Gabor_26_45[i*w/4+j]*var_g7+Gabor_36_45[i*w/4+j]*var_g8
                                        +Gabor_37_45[i*w/4+j]*var_g9+Gabor_47_45[i*w/4+j]*var_g10+Gabor_48_45[i*w/4+j]*var_g11)/g_t1);
            g2_out[i*w/4+j] = (unsigned char)((Gabor_25_90[i*w/4+j]*var_g12+Gabor_26_90[i*w/4+j]*var_g13+Gabor_36_90[i*w/4+j]*var_g14
                                        +Gabor_37_90[i*w/4+j]*var_g15+Gabor_47_90[i*w/4+j]*var_g16+Gabor_48_90[i*w/4+j]*var_g17)/g_t2);
            g3_out[i*w/4+j] = (unsigned char)((Gabor_25_135[i*w/4+j]*var_g18+Gabor_26_135[i*w/4+j]*var_g19+Gabor_36_135[i*w/4+j]*var_g20
                                        +Gabor_37_135[i*w/4+j]*var_g21+Gabor_47_135[i*w/4+j]*var_g22+Gabor_48_135[i*w/4+j]*var_g23)/g_t2);
        }
    }
    /*
    for(i=0; i<h/2; i++)
    {
    for(j=0; j<w/2; j++)
    {
    g0_out[i*w/2+j] = (Gabor_14_0[i*w/2+j]+Gabor_15_0[i*w/2+j]+Gabor_25_0[i*w/2+j]
    +Gabor_26_0[i*w/2+j]+Gabor_36_0[i*w/2+j]+Gabor_37_0[i*w/2+j])/6;
    g1_out[i*w/2+j] = (Gabor_14_45[i*w/2+j]+Gabor_15_45[i*w/2+j]+Gabor_25_45[i*w/2+j]
    +Gabor_26_45[i*w/2+j]+Gabor_36_45[i*w/2+j]+Gabor_37_45[i*w/2+j])/6;
    g2_out[i*w/2+j] = (Gabor_14_90[i*w/2+j]+Gabor_15_90[i*w/2+j]+Gabor_25_90[i*w/2+j]
    +Gabor_26_90[i*w/2+j]+Gabor_36_90[i*w/2+j]+Gabor_37_90[i*w/2+j])/6;
    g3_out[i*w/2+j] = (Gabor_14_135[i*w/2+j]+Gabor_15_135[i*w/2+j]+Gabor_25_135[i*w/2+j]
    +Gabor_26_135[i*w/2+j]+Gabor_36_135[i*w/2+j]+Gabor_37_135[i*w/2+j])/6;
    }
    }
    */
    normalization(g0_out, h/4, w/4);
    normalization(g1_out, h/4, w/4);
    normalization(g2_out, h/4, w/4);
    normalization(g3_out, h/4, w/4);    

    var_g24 = normal(g0_out, h/4, w/4);
    var_g25 = normal(g1_out, h/4, w/4);
    var_g26 = normal(g2_out, h/4, w/4); 
    var_g27 = normal(g3_out, h/4, w/4);

    g = var_g24 + var_g25 + var_g26 + var_g27;

    for(i=0; i<h/4; i++)
    {
        for(j=0; j<w/4; j++)
        {
            o_FeatureMap[i*w/4+j] = (unsigned char)((g0_out[i*w/4+j]*var_g24 + g1_out[i*w/4+j]*var_g25
                                                    + g2_out[i*w/4+j]*var_g26 + g3_out[i*w/4+j]*var_g27)/g);
        }
    }
}    
    
