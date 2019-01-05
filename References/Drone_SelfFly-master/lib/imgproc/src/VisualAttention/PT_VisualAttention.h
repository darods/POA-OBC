#ifndef     _PT_VISUALATTENTION_FUNC_
#define     _PT_VISUALATTENTION_FUNC_

#include<stdio.h>
#include <stdlib.h>
//#include <conio.h>
//#include <malloc.h>
//#include "rmatrix.h"
#include "math.h"

#define pi 3.141592

#define     PT_OK           1
#define     PT_FAIL        0

#define     Min(a,b)                          (((a)<(b)) ? (a) : (b))
#define     Max(a,b)                         (((a)<(b)) ? (b) : (a))
#define     Clip3(nMin, nMax, x)         x > nMax ? nMax : (x < nMin ? nMin : x)
#define     SQRT(a)                           sqrt(a)
#define     POW(x, y)                        pow(x, y)
#define     EXP(x)                             exp(x)
#define     absm(A)                          ((A)<(0) ? (-(A)):(A)) //!< abs macro, faster than procedure


#define     FWRITE(buffer, sizeoftype, sizeofdata, filepointer)   fwrite(buffer, sizeoftype, sizeofdata, filepointer) 
#define     MEMCPY(dst, src, length)                                             memcpy(dst, src, length)
#define     MEMSET(dst, var, length)                                             memset(dst, var, length)
#define     SAFEALLOC(pHandle, size, type)\
                                    {\
                                        type *Handle = (type *) calloc(size, sizeof(type));\
                                        if(NULL != Handle)\
                                        {\
                                            MEMSET(Handle, 0, (size*sizeof(type)));\
                                            pHandle = Handle;\
                                         }\
                                         else\
                                            pHandle = NULL;\
                                     }
#define     SAFEFREE(x)\
                                    if(NULL != x)\
                                    {\
                                        free(x);\
                                        x = NULL;\
                                    }



#define     lW      512
#define     lH      512
#define     __SIMPLIFICATION_MODE__       1


typedef void *HANDLE;


typedef struct _COLOR
{
    unsigned char R;
    unsigned char G;
    unsigned char B;
} COLOR;




typedef struct _cordi
{
    int l_x, r_x;
    int u_y, d_y;        
} cordi;




typedef struct _I_CMS
{
    unsigned char     *level_0;
    unsigned char     *level_1;
    unsigned char     *level_2;
    unsigned char     *level_3;
    unsigned char     *level_4;
    unsigned char     *level_5;
    unsigned char     *level_6;          
    unsigned char     *level_7;          
    unsigned char     *level_8;          
    unsigned char     *Expand_23;
    unsigned char     *Expand_24;
    unsigned char     *Expand_25;
    unsigned char     *Expand_26;
    unsigned char     *Expand_27;
    unsigned char     *Expand_28;
    unsigned char     *Intensity_25;
    unsigned char     *Intensity_26;
    unsigned char     *Intensity_36;
    unsigned char     *Intensity_37;
    unsigned char     *Intensity_47;
    unsigned char     *Intensity_48;    
}I_CMS;




typedef struct _C_CMS
{
    unsigned char     *new_R;
    unsigned char     *new_G;
    unsigned char     *new_B;
    unsigned char     *new_Y;
    unsigned char     *new_R_1;
    unsigned char     *new_R_2;
    unsigned char     *new_R_3;
    unsigned char     *new_R_4;
    unsigned char     *new_R_5;
    unsigned char     *new_R_6;
    unsigned char     *new_R_7;
    unsigned char     *new_R_8;
    unsigned char     *new_G_1;
    unsigned char     *new_G_2;
    unsigned char     *new_G_3;
    unsigned char     *new_G_4;
    unsigned char     *new_G_5;
    unsigned char     *new_G_6;
    unsigned char     *new_G_7;
    unsigned char     *new_G_8;
    unsigned char     *new_B_1;
    unsigned char     *new_B_2;
    unsigned char     *new_B_3;
    unsigned char     *new_B_4;
    unsigned char     *new_B_5;
    unsigned char     *new_B_6;
    unsigned char     *new_B_7;
    unsigned char     *new_B_8;
    unsigned char     *new_Y_1;
    unsigned char     *new_Y_2;
    unsigned char     *new_Y_3;
    unsigned char     *new_Y_4;
    unsigned char     *new_Y_5;
    unsigned char     *new_Y_6;
    unsigned char     *new_Y_7;
    unsigned char     *new_Y_8;
    unsigned char     *Expand_R_23;
    unsigned char     *Expand_R_24;
    unsigned char     *Expand_R_25;
    unsigned char     *Expand_R_26;
    unsigned char     *Expand_R_27;
    unsigned char     *Expand_R_28;
    unsigned char     *Expand_G_23;
    unsigned char     *Expand_G_24;
    unsigned char     *Expand_G_25;
    unsigned char     *Expand_G_26;
    unsigned char     *Expand_G_27;
    unsigned char     *Expand_G_28;
    unsigned char     *Expand_B_23;
    unsigned char     *Expand_B_24;
    unsigned char     *Expand_B_25;
    unsigned char     *Expand_B_26;
    unsigned char     *Expand_B_27;
    unsigned char     *Expand_B_28;
    unsigned char     *Expand_Y_23;
    unsigned char     *Expand_Y_24;
    unsigned char     *Expand_Y_25;
    unsigned char     *Expand_Y_26;
    unsigned char     *Expand_Y_27;
    unsigned char     *Expand_Y_28;
    unsigned char     *RG_25;
    unsigned char     *RG_26;
    unsigned char     *RG_36;
    unsigned char     *RG_37;
    unsigned char     *RG_47;
    unsigned char     *RG_48;
    unsigned char     *BY_25;
    unsigned char     *BY_26;
    unsigned char     *BY_36;
    unsigned char     *BY_37;
    unsigned char     *BY_47;
    unsigned char     *BY_48;
    unsigned char     *RG_out;
    unsigned char     *BY_out;    
}C_CMS;




typedef struct _G_CMS
{
    unsigned char     *tran;
    unsigned char     *trans;
    unsigned char     *ExGabor_2_0;
    unsigned char     *ExGabor_3_0;
    unsigned char     *ExGabor_4_0;
    unsigned char     *ExGabor_5_0;
    unsigned char     *ExGabor_6_0;
    unsigned char     *ExGabor_7_0;
    unsigned char     *ExGabor_8_0;
    unsigned char     *ExGabor_2_45;
    unsigned char     *ExGabor_3_45;
    unsigned char     *ExGabor_4_45;
    unsigned char     *ExGabor_5_45;
    unsigned char     *ExGabor_6_45;
    unsigned char     *ExGabor_7_45;
    unsigned char     *ExGabor_8_45;
    unsigned char     *ExGabor_2_90;
    unsigned char     *ExGabor_3_90;
    unsigned char     *ExGabor_4_90;
    unsigned char     *ExGabor_5_90;
    unsigned char     *ExGabor_6_90;
    unsigned char     *ExGabor_7_90;
    unsigned char     *ExGabor_8_90;
    unsigned char     *ExGabor_2_135;
    unsigned char     *ExGabor_3_135;
    unsigned char     *ExGabor_4_135;
    unsigned char     *ExGabor_5_135;
    unsigned char     *ExGabor_6_135;
    unsigned char     *ExGabor_7_135;
    unsigned char     *ExGabor_8_135;
    unsigned char     *Gabor_25_0;
    unsigned char     *Gabor_26_0;
    unsigned char     *Gabor_36_0;
    unsigned char     *Gabor_37_0;
    unsigned char     *Gabor_47_0;
    unsigned char     *Gabor_48_0;
    unsigned char     *Gabor_25_45;
    unsigned char     *Gabor_26_45;
    unsigned char     *Gabor_36_45;
    unsigned char     *Gabor_37_45;
    unsigned char     *Gabor_47_45;
    unsigned char     *Gabor_48_45;
    unsigned char     *Gabor_25_90;
    unsigned char     *Gabor_26_90;
    unsigned char     *Gabor_36_90;
    unsigned char     *Gabor_37_90;
    unsigned char     *Gabor_47_90;
    unsigned char     *Gabor_48_90;
    unsigned char     *Gabor_25_135;
    unsigned char     *Gabor_26_135;
    unsigned char     *Gabor_36_135;
    unsigned char     *Gabor_37_135;
    unsigned char     *Gabor_47_135;
    unsigned char     *Gabor_48_135;
    unsigned char     *g0_out;
    unsigned char     *g1_out;
    unsigned char     *g2_out;
    unsigned char     *g3_out;
    unsigned char     *ExOrg_23;
    unsigned char     *ExOrg_24;
    unsigned char     *ExOrg_25;
    unsigned char     *ExOrg_26;
    unsigned char     *ExOrg_27;
    unsigned char     *ExOrg_28;
    unsigned char     *Gabor_2_0;
    unsigned char     *Gabor_3_0;
    unsigned char     *Gabor_4_0;
    unsigned char     *Gabor_5_0;
    unsigned char     *Gabor_6_0;
    unsigned char     *Gabor_7_0;
    unsigned char     *Gabor_8_0;
    unsigned char     *Gabor_2_45;
    unsigned char     *Gabor_3_45;
    unsigned char     *Gabor_4_45;
    unsigned char     *Gabor_5_45;
    unsigned char     *Gabor_6_45;
    unsigned char     *Gabor_7_45;
    unsigned char     *Gabor_8_45;
    unsigned char     *Gabor_2_90;
    unsigned char     *Gabor_3_90;
    unsigned char     *Gabor_4_90;
    unsigned char     *Gabor_5_90;
    unsigned char     *Gabor_6_90;
    unsigned char     *Gabor_7_90;
    unsigned char     *Gabor_8_90;
    unsigned char     *Gabor_2_135;
    unsigned char     *Gabor_3_135;
    unsigned char     *Gabor_4_135;
    unsigned char     *Gabor_5_135;
    unsigned char     *Gabor_6_135;
    unsigned char     *Gabor_7_135;
    unsigned char     *Gabor_8_135;
    unsigned char     *Org_0;
    unsigned char     *Org_1;
    unsigned char     *Org_2;
    unsigned char     *Org_3;
    unsigned char     *Org_4;
    unsigned char     *Org_5;
    unsigned char     *Org_6;
    unsigned char     *Org_7;
    unsigned char     *Org_8;     
}G_CMS;



typedef struct _PT_VisualAttention_Main
{
    I_CMS   *pICMS;
    C_CMS   *pCCMS;
    G_CMS   *pGCMS;
}PT_VisualAttention_Main;











void median(unsigned char *in, unsigned char *OutImage, int height, int width);
void gauss_pyramid(unsigned char *inData, unsigned char *outData, int height, int width);
void expand_pyramid (unsigned char *inData, unsigned char *outData, int height, int width, int number);
void Intensity_CSM(I_CMS *pICMS, unsigned char *In, unsigned char *i_FeatureMap, int h, int w);
void convert_gray(unsigned char *In, unsigned char *out, int h, int w);
void normalization(unsigned char *inData, int height, int width);
double normal(unsigned char *inData, int height, int width);
void new_RBG(unsigned char *In, unsigned char *new_R, unsigned char *new_G, unsigned char *new_B, unsigned char *new_Y, int h, int w);
void Color_CSM(C_CMS *pCCMS, unsigned char *In, unsigned char *c_FeatureMap, int h, int w);
void Gabor_CSM(G_CMS *pGCMS, unsigned char *In, unsigned char *o_FeatureMap, int h, int w);
void Gabor_filter(unsigned char *inData, unsigned char *outData, int height, int width, double angle, double frequency, double distance);
void LucasKanade(unsigned char *in0, unsigned char *in1, int height, int width, int mask, unsigned char *obj);
void Gauss(unsigned char *in, unsigned char *out, int height, int width, double sigma);
void sobel(unsigned char *in, double *out_x, double *out_y, int height, int width);
void sobel1(unsigned char *in, unsigned char *out, int height, int width);
void line_draw_b(unsigned char *outData, int height, int width, int x1, int x2, int y1, int y2, int value);
void Motion_CSM(unsigned char *In, unsigned char *In1, unsigned char *Out, int h, int w);
void nor(unsigned char *In, int height, int width);
void order(unsigned char *In, unsigned char *out, unsigned char *re3, int h, int w);
void select(unsigned char *In, unsigned char *out, unsigned char *re3, unsigned char *re2, int h, int w, int *max);
void mexican(unsigned char *In, unsigned char *Out, int height, int width);
unsigned char * Otsu_option(unsigned char *blockData, int subwidth, int subheight, int option);
void convert(unsigned char *Inimage, int height, int width);
void orientation_CSM(unsigned char *In, unsigned char *o_FeatureMap, int h, int w);
unsigned char *Labeling(unsigned char *in, unsigned char * org, int h, int w, cordi cor[], int *digit);
int track(unsigned char *In1, unsigned char *In2, int h, int w, cordi cor[], int each);
int scene_change(unsigned char *InImage, int height, int width);



void Init_ICMS(I_CMS *pICMS, struct _PT_VisualAttention_Struct *pVisialAttension);
void Init_CCMS(C_CMS *pCCMS, struct _PT_VisualAttention_Struct *pVisialAttension);
void Init_GCMS(G_CMS *pGCMS, struct _PT_VisualAttention_Struct *pVisialAttension);    
void Destroy_ICMS(I_CMS *pICMS, struct _PT_VisualAttention_Struct *pVisialAttension);
void Destroy_CCMS(C_CMS *pCCMS, struct _PT_VisualAttention_Struct *pVisialAttension);
void Destroy_GCMS(G_CMS *pGCMS, struct _PT_VisualAttention_Struct *pVisialAttension);    




#endif
