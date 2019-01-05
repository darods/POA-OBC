

#include    "PT_VisualAttention.h"
#include    "PT_VisualAttention_Export.h"
#include    "Gabor.h"


COMPLEX          *TmpComplexBuf;
COMPLEX          *TmpComplexBuf_COL;
COMPLEX          *TmpComplexBuf_ROW;
unsigned char   *pLPF0;
unsigned char   *pLPF1;
unsigned char   *pFFTView;
double              pGaucoeffi[5][5];


API_EXPORT void PT_Init_VisualAttention(HANDLE *pHandle, PT_VisualAttention_Struct *pVisialAttension)
{
    int     nImgSize = (pVisialAttension->nWidth * pVisialAttension->nHeight);
    PT_VisualAttention_Main     *pVisAtten;


    SAFEALLOC(pVisAtten, 1, PT_VisualAttention_Main);
    *pHandle = (PT_VisualAttention_Main *) pVisAtten;


    SAFEALLOC(pVisialAttension->pI_FeatureMap, (nImgSize>>4), unsigned char);
    SAFEALLOC(pVisialAttension->pC_FeatureMap, (nImgSize>>4), unsigned char);
    SAFEALLOC(pVisialAttension->pO_FeatureMap, (nImgSize>>4), unsigned char);
    SAFEALLOC(pVisialAttension->pS_FeatureMap, (nImgSize>>4), unsigned char); 


    SAFEALLOC(pVisAtten->pICMS, 1, I_CMS);
    #if !__SIMPLIFICATION_MODE__
    SAFEALLOC(pVisAtten->pCCMS, 1, C_CMS);
    SAFEALLOC(pVisAtten->pGCMS, 1, G_CMS);    
    #endif

    Init_ICMS(pVisAtten->pICMS, pVisialAttension);

    #if !__SIMPLIFICATION_MODE__
    Init_CCMS(pVisAtten->pCCMS, pVisialAttension);

    Init_GCMS(pVisAtten->pGCMS, pVisialAttension);

    SAFEALLOC(TmpComplexBuf, (sizeof(COMPLEX)* nImgSize / 16), COMPLEX);    
    SAFEALLOC(TmpComplexBuf_COL, (sizeof(COMPLEX)* pVisialAttension->nHeight / 4), COMPLEX);        
    SAFEALLOC(TmpComplexBuf_ROW, (sizeof(COMPLEX)* pVisialAttension->nWidth / 4), COMPLEX);        
    SAFEALLOC(pLPF0, (sizeof(unsigned char)* nImgSize / 16), unsigned char);    
    SAFEALLOC(pLPF1, (sizeof(unsigned char)* nImgSize / 16), unsigned char);        
    SAFEALLOC(pFFTView, (sizeof(unsigned char)* nImgSize / 16), unsigned char);
    #endif
    
    {
        int         k, l;
        double  coef[3] = {0.4, 0.25, 0.05};

        // weight °ª ÀúÀå
        for (k=-2; k<=2; k++)
            for (l=-2; l<=2; l++)
                pGaucoeffi[k+2][l+2] = coef[abs(k)]*coef[abs(l)];    
    }
}



API_EXPORT unsigned char * PT_VisualAttention(HANDLE pHandle, PT_VisualAttention_Struct *pVisialAttension)
{
    PT_VisualAttention_Main     *pVisAtten = (PT_VisualAttention_Main *)pHandle;
    unsigned char           *pIn = pVisialAttension->pIn;
    int                            i, j;
    int                            nWidth = pVisialAttension->nWidth;
    int                            nHeight = pVisialAttension->nHeight;
    int                            nQuadWidth = nWidth / 4;
    int                            nQuadHeight = nHeight / 4;
    unsigned char            temp;        


    #if __SIMPLIFICATION_MODE__
    {
        unsigned char *pI_FeatureMap = pVisialAttension->pI_FeatureMap;
        unsigned char *pS_FeatureMap = pVisialAttension->pS_FeatureMap;        
        
        Intensity_CSM(pVisAtten->pICMS, pIn, pI_FeatureMap, nHeight, nWidth);

        normalization(pI_FeatureMap, nQuadHeight, nQuadWidth);

        normal(pI_FeatureMap, nQuadHeight, nQuadWidth);

        for(i=0; i<nHeight/4; i++)
        {
            for(j=0; j<nWidth/4; j++)
            {
                temp = pI_FeatureMap[i*nWidth/4+j];
                
                if(temp > 255) 
                    pS_FeatureMap[i*nWidth/4+j] = 255;
                else 
                    pS_FeatureMap[i*nWidth/4+j] = temp;
            }
        }
        
        return pS_FeatureMap;
    }
    #else
    {
        double            nVar0, nVar1, nVar2, t;
        unsigned char *pI_FeatureMap = pVisialAttension->pI_FeatureMap;
        unsigned char *pC_FeatureMap = pVisialAttension->pC_FeatureMap;        
        unsigned char *pO_FeatureMap = pVisialAttension->pO_FeatureMap;                
        unsigned char *pS_FeatureMap = pVisialAttension->pS_FeatureMap;        

        Intensity_CSM(pVisAtten->pICMS, pIn, pI_FeatureMap, nHeight, nWidth);
        Color_CSM(pVisAtten->pCCMS, pIn, pC_FeatureMap, nHeight, nWidth);
        Gabor_CSM(pVisAtten->pGCMS, pIn, pO_FeatureMap, nHeight, nWidth);

        normalization(pI_FeatureMap, nQuadHeight, nQuadWidth);
        normalization(pC_FeatureMap, nQuadHeight, nQuadWidth);
        normalization(pO_FeatureMap, nQuadHeight, nQuadWidth);

        nVar0 = normal(pI_FeatureMap, nQuadHeight, nQuadWidth);
        nVar1 = normal(pC_FeatureMap, nQuadHeight, nQuadWidth);
        nVar2 = normal(pO_FeatureMap, nQuadHeight, nQuadWidth);

        t = nVar0 + nVar1 + nVar2;

        for(i=0; i<nHeight/4; i++)
        {
            for(j=0; j<nWidth/4; j++)
            {
                temp = (pI_FeatureMap[i*nWidth/4+j]*nVar0 + pC_FeatureMap[i*nWidth/4+j]*nVar1 + pO_FeatureMap[i*nWidth/4+j]*nVar2)/t;
                if(temp > 255) 
                    pS_FeatureMap[i*nWidth/4+j] = 255;
                else 
                    pS_FeatureMap[i*nWidth/4+j] = temp;
            }
        }

        return pS_FeatureMap;
    }
    #endif    

}




API_EXPORT void PT_Destroy_VisualAttention(HANDLE pHandle, PT_VisualAttention_Struct *pVisialAttension)
{
    PT_VisualAttention_Main     *pVisAtten = (PT_VisualAttention_Main *)pHandle;

    Destroy_ICMS(pVisAtten->pICMS, pVisialAttension);

    #if !__SIMPLIFICATION_MODE__
    Destroy_CCMS(pVisAtten->pCCMS, pVisialAttension);

    Destroy_GCMS(pVisAtten->pGCMS, pVisialAttension);    
    #endif

    SAFEFREE(pVisAtten->pICMS);
    #if !__SIMPLIFICATION_MODE__
    SAFEFREE(pVisAtten->pCCMS);
    SAFEFREE(pVisAtten->pGCMS);

    SAFEFREE(TmpComplexBuf);
    SAFEFREE(TmpComplexBuf_COL);
    SAFEFREE(TmpComplexBuf_ROW);    
    SAFEFREE(pLPF0);
    SAFEFREE(pLPF1);    
    SAFEFREE(pFFTView);
    #endif

    SAFEFREE(pVisialAttension->pI_FeatureMap);
    SAFEFREE(pVisialAttension->pC_FeatureMap);
    SAFEFREE(pVisialAttension->pO_FeatureMap);
    SAFEFREE(pVisialAttension->pS_FeatureMap);

    SAFEFREE(pVisAtten);
}





void Init_ICMS(I_CMS *pICMS, PT_VisualAttention_Struct *pVisialAttension)
{
    int     nWidth = pVisialAttension->nWidth;
    int     nHeight = pVisialAttension->nHeight;
    int     nImgSize = nWidth * nHeight;

    SAFEALLOC(pICMS->level_0, nImgSize, unsigned char);          
    SAFEALLOC(pICMS->level_1, nImgSize/4, unsigned char);        
    SAFEALLOC(pICMS->level_2, nImgSize/16, unsigned char);       
    SAFEALLOC(pICMS->level_3, nImgSize/64, unsigned char);       
    SAFEALLOC(pICMS->level_4, nImgSize/256, unsigned char);      
    SAFEALLOC(pICMS->level_5, nImgSize/1024, unsigned char);     
    SAFEALLOC(pICMS->Expand_23, nImgSize/16, unsigned char);     
    SAFEALLOC(pICMS->Expand_24, nImgSize/16, unsigned char);     
    SAFEALLOC(pICMS->Expand_25, nImgSize/16, unsigned char);     
    SAFEALLOC(pICMS->Intensity_25, nImgSize/16, unsigned char);  

    #if !__SIMPLIFICATION_MODE__
    SAFEALLOC(pICMS->level_6, nImgSize/4096, unsigned char);     
    SAFEALLOC(pICMS->level_7, nImgSize/16384, unsigned char);    
    SAFEALLOC(pICMS->level_8, nImgSize/65536, unsigned char);    
    SAFEALLOC(pICMS->Expand_26, nImgSize/16, unsigned char);     
    SAFEALLOC(pICMS->Expand_27, nImgSize/16, unsigned char);     
    SAFEALLOC(pICMS->Expand_28, nImgSize/16, unsigned char);     
    SAFEALLOC(pICMS->Intensity_26, nImgSize/16, unsigned char);  
    SAFEALLOC(pICMS->Intensity_36, nImgSize/16, unsigned char);  
    SAFEALLOC(pICMS->Intensity_37, nImgSize/16, unsigned char);  
    SAFEALLOC(pICMS->Intensity_47, nImgSize/16, unsigned char);  
    SAFEALLOC(pICMS->Intensity_48, nImgSize/16, unsigned char);  
    #endif
}


    #if !__SIMPLIFICATION_MODE__
void Init_CCMS(C_CMS *pCCMS, PT_VisualAttention_Struct *pVisialAttension)
{
    int     nWidth = pVisialAttension->nWidth;
    int     nHeight = pVisialAttension->nHeight;
    int     nImgSize = nWidth * nHeight;    
    
    SAFEALLOC(pCCMS->new_R, nImgSize, unsigned char);            
    SAFEALLOC(pCCMS->new_G, nImgSize, unsigned char);            
    SAFEALLOC(pCCMS->new_B, nImgSize, unsigned char);            
    SAFEALLOC(pCCMS->new_Y, nImgSize, unsigned char);            
    SAFEALLOC(pCCMS->new_R_1, nImgSize/4, unsigned char);        
    SAFEALLOC(pCCMS->new_R_2, nImgSize/16, unsigned char);       
    SAFEALLOC(pCCMS->new_R_3, nImgSize/64, unsigned char);       
    SAFEALLOC(pCCMS->new_R_4, nImgSize/256, unsigned char);      
    SAFEALLOC(pCCMS->new_R_5, nImgSize/1024, unsigned char);     
    SAFEALLOC(pCCMS->new_R_6, nImgSize/4096, unsigned char);     
    SAFEALLOC(pCCMS->new_R_7, nImgSize/16384, unsigned char);    
    SAFEALLOC(pCCMS->new_R_8, nImgSize/65536, unsigned char);    
    SAFEALLOC(pCCMS->new_G_1, nImgSize/4, unsigned char);        
    SAFEALLOC(pCCMS->new_G_2, nImgSize/16, unsigned char);       
    SAFEALLOC(pCCMS->new_G_3, nImgSize/64, unsigned char);       
    SAFEALLOC(pCCMS->new_G_4, nImgSize/256, unsigned char);      
    SAFEALLOC(pCCMS->new_G_5, nImgSize/1024, unsigned char);     
    SAFEALLOC(pCCMS->new_G_6, nImgSize/4096, unsigned char);     
    SAFEALLOC(pCCMS->new_G_7, nImgSize/16384, unsigned char);    
    SAFEALLOC(pCCMS->new_G_8, nImgSize/65536, unsigned char);    
    SAFEALLOC(pCCMS->new_B_1, nImgSize/4, unsigned char);        
    SAFEALLOC(pCCMS->new_B_2, nImgSize/16, unsigned char);       
    SAFEALLOC(pCCMS->new_B_3, nImgSize/64, unsigned char);       
    SAFEALLOC(pCCMS->new_B_4, nImgSize/256, unsigned char);      
    SAFEALLOC(pCCMS->new_B_5, nImgSize/1024, unsigned char);     
    SAFEALLOC(pCCMS->new_B_6, nImgSize/4096, unsigned char);     
    SAFEALLOC(pCCMS->new_B_7, nImgSize/16384, unsigned char);    
    SAFEALLOC(pCCMS->new_B_8, nImgSize/65536, unsigned char);    
    SAFEALLOC(pCCMS->new_Y_1, nImgSize/4, unsigned char);        
    SAFEALLOC(pCCMS->new_Y_2, nImgSize/16, unsigned char);       
    SAFEALLOC(pCCMS->new_Y_3, nImgSize/64, unsigned char);       
    SAFEALLOC(pCCMS->new_Y_4, nImgSize/256, unsigned char);      
    SAFEALLOC(pCCMS->new_Y_5, nImgSize/1024, unsigned char);     
    SAFEALLOC(pCCMS->new_Y_6, nImgSize/4096, unsigned char);     
    SAFEALLOC(pCCMS->new_Y_7, nImgSize/16384, unsigned char);    
    SAFEALLOC(pCCMS->new_Y_8, nImgSize/65536, unsigned char);    
    SAFEALLOC(pCCMS->Expand_R_23, nImgSize/16, unsigned char);   
    SAFEALLOC(pCCMS->Expand_R_24, nImgSize/16, unsigned char);   
    SAFEALLOC(pCCMS->Expand_R_25, nImgSize/16, unsigned char);   
    SAFEALLOC(pCCMS->Expand_R_26, nImgSize/16, unsigned char);   
    SAFEALLOC(pCCMS->Expand_R_27, nImgSize/16, unsigned char);   
    SAFEALLOC(pCCMS->Expand_R_28, nImgSize/16, unsigned char);   
    SAFEALLOC(pCCMS->Expand_G_23, nImgSize/16, unsigned char);   
    SAFEALLOC(pCCMS->Expand_G_24, nImgSize/16, unsigned char);   
    SAFEALLOC(pCCMS->Expand_G_25, nImgSize/16, unsigned char);   
    SAFEALLOC(pCCMS->Expand_G_26, nImgSize/16, unsigned char);   
    SAFEALLOC(pCCMS->Expand_G_27, nImgSize/16, unsigned char);   
    SAFEALLOC(pCCMS->Expand_G_28, nImgSize/16, unsigned char);   
    SAFEALLOC(pCCMS->Expand_B_23, nImgSize/16, unsigned char);   
    SAFEALLOC(pCCMS->Expand_B_24, nImgSize/16, unsigned char);   
    SAFEALLOC(pCCMS->Expand_B_25, nImgSize/16, unsigned char);   
    SAFEALLOC(pCCMS->Expand_B_26, nImgSize/16, unsigned char);   
    SAFEALLOC(pCCMS->Expand_B_27, nImgSize/16, unsigned char);   
    SAFEALLOC(pCCMS->Expand_B_28, nImgSize/16, unsigned char);   
    SAFEALLOC(pCCMS->Expand_Y_23, nImgSize/16, unsigned char);   
    SAFEALLOC(pCCMS->Expand_Y_24, nImgSize/16, unsigned char);   
    SAFEALLOC(pCCMS->Expand_Y_25, nImgSize/16, unsigned char);   
    SAFEALLOC(pCCMS->Expand_Y_26, nImgSize/16, unsigned char);   
    SAFEALLOC(pCCMS->Expand_Y_27, nImgSize/16, unsigned char);   
    SAFEALLOC(pCCMS->Expand_Y_28, nImgSize/16, unsigned char);   
    SAFEALLOC(pCCMS->RG_25, nImgSize/16, unsigned char);         
    SAFEALLOC(pCCMS->RG_26, nImgSize/16, unsigned char);         
    SAFEALLOC(pCCMS->RG_36, nImgSize/16, unsigned char);         
    SAFEALLOC(pCCMS->RG_37, nImgSize/16, unsigned char);         
    SAFEALLOC(pCCMS->RG_47, nImgSize/16, unsigned char);         
    SAFEALLOC(pCCMS->RG_48, nImgSize/16, unsigned char);         
    SAFEALLOC(pCCMS->BY_25, nImgSize/16, unsigned char);         
    SAFEALLOC(pCCMS->BY_26, nImgSize/16, unsigned char);         
    SAFEALLOC(pCCMS->BY_36, nImgSize/16, unsigned char);         
    SAFEALLOC(pCCMS->BY_37, nImgSize/16, unsigned char);         
    SAFEALLOC(pCCMS->BY_47, nImgSize/16, unsigned char);         
    SAFEALLOC(pCCMS->BY_48, nImgSize/16, unsigned char);         
    SAFEALLOC(pCCMS->RG_out, nImgSize/16, unsigned char);        
    SAFEALLOC(pCCMS->BY_out, nImgSize/16, unsigned char);        
 
}


void Init_GCMS(G_CMS *pGCMS, PT_VisualAttention_Struct *pVisialAttension)
{
    int     nWidth = pVisialAttension->nWidth;
    int     nHeight = pVisialAttension->nHeight;
    int     nLWidth = lW;
    int     nLHeight = lH;
    int     nImgSize = nWidth * nHeight;
    
    SAFEALLOC(pGCMS->tran, nImgSize/16, unsigned char);       
    SAFEALLOC(pGCMS->trans, nImgSize, unsigned char);      
    SAFEALLOC(pGCMS->ExGabor_2_0, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->ExGabor_3_0, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->ExGabor_4_0, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->ExGabor_5_0, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->ExGabor_6_0, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->ExGabor_7_0, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->ExGabor_8_0, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->ExGabor_2_45, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->ExGabor_3_45, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->ExGabor_4_45, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->ExGabor_5_45, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->ExGabor_6_45, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->ExGabor_7_45, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->ExGabor_8_45, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->ExGabor_2_90, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->ExGabor_3_90, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->ExGabor_4_90, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->ExGabor_5_90, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->ExGabor_6_90, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->ExGabor_7_90, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->ExGabor_8_90, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->ExGabor_2_135, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->ExGabor_3_135, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->ExGabor_4_135, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->ExGabor_5_135, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->ExGabor_6_135, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->ExGabor_7_135, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->ExGabor_8_135, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->Gabor_25_0, nImgSize/16, unsigned char); 
    SAFEALLOC(pGCMS->Gabor_26_0, nImgSize/16, unsigned char); 
    SAFEALLOC(pGCMS->Gabor_36_0, nImgSize/16, unsigned char); 
    SAFEALLOC(pGCMS->Gabor_37_0, nImgSize/16, unsigned char); 
    SAFEALLOC(pGCMS->Gabor_47_0, nImgSize/16, unsigned char); 
    SAFEALLOC(pGCMS->Gabor_48_0, nImgSize/16, unsigned char); 
    SAFEALLOC(pGCMS->Gabor_25_45, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->Gabor_26_45, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->Gabor_36_45, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->Gabor_37_45, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->Gabor_47_45, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->Gabor_48_45, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->Gabor_25_90, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->Gabor_26_90, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->Gabor_36_90, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->Gabor_37_90, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->Gabor_47_90, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->Gabor_48_90, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->Gabor_25_135, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->Gabor_26_135, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->Gabor_36_135, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->Gabor_37_135, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->Gabor_47_135, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->Gabor_48_135, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->g0_out, nImgSize/16, unsigned char);     
    SAFEALLOC(pGCMS->g1_out, nImgSize/16, unsigned char);     
    SAFEALLOC(pGCMS->g2_out, nImgSize/16, unsigned char);     
    SAFEALLOC(pGCMS->g3_out, nImgSize/16, unsigned char);     

    nImgSize = nLWidth * nLHeight;
    SAFEALLOC(pGCMS->ExOrg_23, nImgSize/16, unsigned char);  
    SAFEALLOC(pGCMS->ExOrg_24, nImgSize/16, unsigned char);  
    SAFEALLOC(pGCMS->ExOrg_25, nImgSize/16, unsigned char);  
    SAFEALLOC(pGCMS->ExOrg_26, nImgSize/16, unsigned char);  
    SAFEALLOC(pGCMS->ExOrg_27, nImgSize/16, unsigned char);  
    SAFEALLOC(pGCMS->ExOrg_28, nImgSize/16, unsigned char);  
    SAFEALLOC(pGCMS->Gabor_2_0, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->Gabor_3_0, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->Gabor_4_0, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->Gabor_5_0, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->Gabor_6_0, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->Gabor_7_0, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->Gabor_8_0, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->Gabor_2_45, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->Gabor_3_45, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->Gabor_4_45, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->Gabor_5_45, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->Gabor_6_45, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->Gabor_7_45, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->Gabor_8_45, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->Gabor_2_90, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->Gabor_3_90, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->Gabor_4_90, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->Gabor_5_90, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->Gabor_6_90, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->Gabor_7_90, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->Gabor_8_90, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->Gabor_2_135, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->Gabor_3_135, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->Gabor_4_135, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->Gabor_5_135, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->Gabor_6_135, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->Gabor_7_135, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->Gabor_8_135, nImgSize/16, unsigned char);
    SAFEALLOC(pGCMS->Org_0, nImgSize, unsigned char);       
    SAFEALLOC(pGCMS->Org_1, nImgSize/4, unsigned char);     
    SAFEALLOC(pGCMS->Org_2, nImgSize/16, unsigned char);    
    SAFEALLOC(pGCMS->Org_3, nImgSize/64, unsigned char);    
    SAFEALLOC(pGCMS->Org_4, nImgSize/256, unsigned char);   
    SAFEALLOC(pGCMS->Org_5, nImgSize/1024, unsigned char);  
    SAFEALLOC(pGCMS->Org_6, nImgSize/4096, unsigned char);  
    SAFEALLOC(pGCMS->Org_7, nImgSize/16384, unsigned char); 
    SAFEALLOC(pGCMS->Org_8, nImgSize/65536, unsigned char);  
}
#endif

void Destroy_ICMS(I_CMS *pICMS, PT_VisualAttention_Struct *pVisialAttension)
{
    SAFEFREE(pICMS->level_0);
    SAFEFREE(pICMS->level_1);
    SAFEFREE(pICMS->level_2);
    SAFEFREE(pICMS->level_3);
    SAFEFREE(pICMS->level_4);
    SAFEFREE(pICMS->level_5);
    SAFEFREE(pICMS->Expand_23);
    SAFEFREE(pICMS->Expand_24);
    SAFEFREE(pICMS->Expand_25);
    SAFEFREE(pICMS->Intensity_25);
    #if !__SIMPLIFICATION_MODE__
    SAFEFREE(pICMS->level_6);          
    SAFEFREE(pICMS->level_7);          
    SAFEFREE(pICMS->level_8);          
    SAFEFREE(pICMS->Expand_26);
    SAFEFREE(pICMS->Expand_27);
    SAFEFREE(pICMS->Expand_28);
    SAFEFREE(pICMS->Intensity_26);
    SAFEFREE(pICMS->Intensity_36);
    SAFEFREE(pICMS->Intensity_37);
    SAFEFREE(pICMS->Intensity_47);
    SAFEFREE(pICMS->Intensity_48);
    #endif
}


#if !__SIMPLIFICATION_MODE__
void Destroy_CCMS(C_CMS *pCCMS, PT_VisualAttention_Struct *pVisialAttension)
{
    SAFEFREE(pCCMS->new_R);
    SAFEFREE(pCCMS->new_G);
    SAFEFREE(pCCMS->new_B);
    SAFEFREE(pCCMS->new_Y);
    SAFEFREE(pCCMS->new_R_1);
    SAFEFREE(pCCMS->new_R_2);
    SAFEFREE(pCCMS->new_R_3);
    SAFEFREE(pCCMS->new_R_4);
    SAFEFREE(pCCMS->new_R_5);
    SAFEFREE(pCCMS->new_R_6);
    SAFEFREE(pCCMS->new_R_7);
    SAFEFREE(pCCMS->new_R_8);
    SAFEFREE(pCCMS->new_G_1);
    SAFEFREE(pCCMS->new_G_2);
    SAFEFREE(pCCMS->new_G_3);
    SAFEFREE(pCCMS->new_G_4);
    SAFEFREE(pCCMS->new_G_5);
    SAFEFREE(pCCMS->new_G_6);
    SAFEFREE(pCCMS->new_G_7);
    SAFEFREE(pCCMS->new_G_8);
    SAFEFREE(pCCMS->new_B_1);
    SAFEFREE(pCCMS->new_B_2);
    SAFEFREE(pCCMS->new_B_3);
    SAFEFREE(pCCMS->new_B_4);
    SAFEFREE(pCCMS->new_B_5);
    SAFEFREE(pCCMS->new_B_6);
    SAFEFREE(pCCMS->new_B_7);
    SAFEFREE(pCCMS->new_B_8);
    SAFEFREE(pCCMS->new_Y_1);
    SAFEFREE(pCCMS->new_Y_2);
    SAFEFREE(pCCMS->new_Y_3);
    SAFEFREE(pCCMS->new_Y_4);
    SAFEFREE(pCCMS->new_Y_5);
    SAFEFREE(pCCMS->new_Y_6);
    SAFEFREE(pCCMS->new_Y_7);
    SAFEFREE(pCCMS->new_Y_8);
    SAFEFREE(pCCMS->Expand_R_23);
    SAFEFREE(pCCMS->Expand_R_24);
    SAFEFREE(pCCMS->Expand_R_25);
    SAFEFREE(pCCMS->Expand_R_26);
    SAFEFREE(pCCMS->Expand_R_27);
    SAFEFREE(pCCMS->Expand_R_28);
    SAFEFREE(pCCMS->Expand_G_23);
    SAFEFREE(pCCMS->Expand_G_24);
    SAFEFREE(pCCMS->Expand_G_25);
    SAFEFREE(pCCMS->Expand_G_26);
    SAFEFREE(pCCMS->Expand_G_27);
    SAFEFREE(pCCMS->Expand_G_28);
    SAFEFREE(pCCMS->Expand_B_23);
    SAFEFREE(pCCMS->Expand_B_24);
    SAFEFREE(pCCMS->Expand_B_25);
    SAFEFREE(pCCMS->Expand_B_26);
    SAFEFREE(pCCMS->Expand_B_27);
    SAFEFREE(pCCMS->Expand_B_28);
    SAFEFREE(pCCMS->Expand_Y_23);
    SAFEFREE(pCCMS->Expand_Y_24);
    SAFEFREE(pCCMS->Expand_Y_25);
    SAFEFREE(pCCMS->Expand_Y_26);
    SAFEFREE(pCCMS->Expand_Y_27);
    SAFEFREE(pCCMS->Expand_Y_28);
    SAFEFREE(pCCMS->RG_25);
    SAFEFREE(pCCMS->RG_26);
    SAFEFREE(pCCMS->RG_36);
    SAFEFREE(pCCMS->RG_37);
    SAFEFREE(pCCMS->RG_47);
    SAFEFREE(pCCMS->RG_48);
    SAFEFREE(pCCMS->BY_25);
    SAFEFREE(pCCMS->BY_26);
    SAFEFREE(pCCMS->BY_36);
    SAFEFREE(pCCMS->BY_37);
    SAFEFREE(pCCMS->BY_47);
    SAFEFREE(pCCMS->BY_48);
    SAFEFREE(pCCMS->RG_out);
    SAFEFREE(pCCMS->BY_out);
}


void Destroy_GCMS(G_CMS *pGCMS, PT_VisualAttention_Struct *pVisialAttension)
{
    SAFEFREE(pGCMS->tran);
    SAFEFREE(pGCMS->trans);
    SAFEFREE(pGCMS->ExGabor_2_0);
    SAFEFREE(pGCMS->ExGabor_3_0);
    SAFEFREE(pGCMS->ExGabor_4_0);
    SAFEFREE(pGCMS->ExGabor_5_0);
    SAFEFREE(pGCMS->ExGabor_6_0);
    SAFEFREE(pGCMS->ExGabor_7_0);
    SAFEFREE(pGCMS->ExGabor_8_0);
    SAFEFREE(pGCMS->ExGabor_2_45);
    SAFEFREE(pGCMS->ExGabor_3_45);
    SAFEFREE(pGCMS->ExGabor_4_45);
    SAFEFREE(pGCMS->ExGabor_5_45);
    SAFEFREE(pGCMS->ExGabor_6_45);
    SAFEFREE(pGCMS->ExGabor_7_45);
    SAFEFREE(pGCMS->ExGabor_8_45);
    SAFEFREE(pGCMS->ExGabor_2_90);
    SAFEFREE(pGCMS->ExGabor_3_90);
    SAFEFREE(pGCMS->ExGabor_4_90);
    SAFEFREE(pGCMS->ExGabor_5_90);
    SAFEFREE(pGCMS->ExGabor_6_90);
    SAFEFREE(pGCMS->ExGabor_7_90);
    SAFEFREE(pGCMS->ExGabor_8_90);
    SAFEFREE(pGCMS->ExGabor_2_135);
    SAFEFREE(pGCMS->ExGabor_3_135);
    SAFEFREE(pGCMS->ExGabor_4_135);
    SAFEFREE(pGCMS->ExGabor_5_135);
    SAFEFREE(pGCMS->ExGabor_6_135);
    SAFEFREE(pGCMS->ExGabor_7_135);
    SAFEFREE(pGCMS->ExGabor_8_135);
    SAFEFREE(pGCMS->Gabor_25_0);
    SAFEFREE(pGCMS->Gabor_26_0);
    SAFEFREE(pGCMS->Gabor_36_0);
    SAFEFREE(pGCMS->Gabor_37_0);
    SAFEFREE(pGCMS->Gabor_47_0);
    SAFEFREE(pGCMS->Gabor_48_0);
    SAFEFREE(pGCMS->Gabor_25_45);
    SAFEFREE(pGCMS->Gabor_26_45);
    SAFEFREE(pGCMS->Gabor_36_45);
    SAFEFREE(pGCMS->Gabor_37_45);
    SAFEFREE(pGCMS->Gabor_47_45);
    SAFEFREE(pGCMS->Gabor_48_45);
    SAFEFREE(pGCMS->Gabor_25_90);
    SAFEFREE(pGCMS->Gabor_26_90);
    SAFEFREE(pGCMS->Gabor_36_90);
    SAFEFREE(pGCMS->Gabor_37_90);
    SAFEFREE(pGCMS->Gabor_47_90);
    SAFEFREE(pGCMS->Gabor_48_90);
    SAFEFREE(pGCMS->Gabor_25_135);
    SAFEFREE(pGCMS->Gabor_26_135);
    SAFEFREE(pGCMS->Gabor_36_135);
    SAFEFREE(pGCMS->Gabor_37_135);
    SAFEFREE(pGCMS->Gabor_47_135);
    SAFEFREE(pGCMS->Gabor_48_135);
    SAFEFREE(pGCMS->g0_out);
    SAFEFREE(pGCMS->g1_out);
    SAFEFREE(pGCMS->g2_out);
    SAFEFREE(pGCMS->g3_out);
    SAFEFREE(pGCMS->ExOrg_23);
    SAFEFREE(pGCMS->ExOrg_24);
    SAFEFREE(pGCMS->ExOrg_25);
    SAFEFREE(pGCMS->ExOrg_26);
    SAFEFREE(pGCMS->ExOrg_27);
    SAFEFREE(pGCMS->ExOrg_28);
    SAFEFREE(pGCMS->Gabor_2_0);
    SAFEFREE(pGCMS->Gabor_3_0);
    SAFEFREE(pGCMS->Gabor_4_0);
    SAFEFREE(pGCMS->Gabor_5_0);
    SAFEFREE(pGCMS->Gabor_6_0);
    SAFEFREE(pGCMS->Gabor_7_0);
    SAFEFREE(pGCMS->Gabor_8_0);
    SAFEFREE(pGCMS->Gabor_2_45);
    SAFEFREE(pGCMS->Gabor_3_45);
    SAFEFREE(pGCMS->Gabor_4_45);
    SAFEFREE(pGCMS->Gabor_5_45);
    SAFEFREE(pGCMS->Gabor_6_45);
    SAFEFREE(pGCMS->Gabor_7_45);
    SAFEFREE(pGCMS->Gabor_8_45);
    SAFEFREE(pGCMS->Gabor_2_90);
    SAFEFREE(pGCMS->Gabor_3_90);
    SAFEFREE(pGCMS->Gabor_4_90);
    SAFEFREE(pGCMS->Gabor_5_90);
    SAFEFREE(pGCMS->Gabor_6_90);
    SAFEFREE(pGCMS->Gabor_7_90);
    SAFEFREE(pGCMS->Gabor_8_90);
    SAFEFREE(pGCMS->Gabor_2_135);
    SAFEFREE(pGCMS->Gabor_3_135);
    SAFEFREE(pGCMS->Gabor_4_135);
    SAFEFREE(pGCMS->Gabor_5_135);
    SAFEFREE(pGCMS->Gabor_6_135);
    SAFEFREE(pGCMS->Gabor_7_135);
    SAFEFREE(pGCMS->Gabor_8_135);
    SAFEFREE(pGCMS->Org_0);
    SAFEFREE(pGCMS->Org_1);
    SAFEFREE(pGCMS->Org_2);
    SAFEFREE(pGCMS->Org_3);
    SAFEFREE(pGCMS->Org_4);
    SAFEFREE(pGCMS->Org_5);
    SAFEFREE(pGCMS->Org_6);
    SAFEFREE(pGCMS->Org_7);
    SAFEFREE(pGCMS->Org_8);    
}
#endif




