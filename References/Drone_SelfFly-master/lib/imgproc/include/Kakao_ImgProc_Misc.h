//
//  Kakao_ImgProc_Misc.h
//
//  Created by Maverick Park on 2014. 8. 10.
//  Copyright (c) 2015년 Kakao. All rights reserved.
//

#ifndef     __KAKAO_IMGPROC_PRE_MISCELLANEA_H__
#define     __KAKAO_IMGPROC_PRE_MISCELLANEA_H__


//#include "tgmath.h"


#define GET_MIN(a,b)                                                (((a)<(b)) ? (a) : (b))
#define GET_MAX(a,b)                                                (((a)<(b)) ? (b) : (a))
#define CLIP3(nMin, nMax, x)                                        ((x > nMax) ? nMax : (x < nMin ? nMin : x))
#define SQRT(a)                                                     sqrt(a)
#define POW(x, y)                                                   pow(x, y)
#define EXP(x)                                                      exp((x))
#define EXP_F(x)                                                    expf((x))
#define ABSM(A)                                                     abs(A)          //((A)<(0) ? (-(A)):(A)) //!< abs macro, faster than procedure
#define ABSM_F(A)                                                   fabsf(A)        //((A)<(0) ? (-(A)):(A)) //!< abs macro, faster than procedure
#define ABSM_D(A)                                                   fabs(A)         //((A)<(0) ? (-(A)):(A)) //!< abs macro, faster than procedure
#define ROUND(x)                                                    (INT32)((F32)(x) + 0.5f)


#define GET_PIXEL_SIZE(nType)                                       ((IMG_DEPTH_U8 == nType) ? 1 :\
                                                                    (IMG_DEPTH_U32 == nType) ? 4 :\
                                                                    (IMG_DEPTH_F32 == nType) ? 4 :\
                                                                    (IMG_DEPTH_D64 == nType) ? 8 : 1)

#define STRCPY(extension, address)                                  strcpy(extension, address)
#define STRCMP(address, extension)                                  strcmp(address, extension)


#define FFLUSH(fp)                                                  fflush(fp)
#define FSEEK(fp, var, position)                                    fseek(fp, var, position)
#define FTELL(fp)                                                   ftell(fp)
#define FOPEN(filename, mode)                                       fopen(filename, mode)
#define FCLOSE(fp)                                                  fclose(fp)
#ifndef FREAD
    #define FREAD(buffer, sizeoftype, sizeofdata, filepoint)        fread(buffer, sizeoftype, sizeofdata, filepoint)
#endif
#ifndef FWRITE
    #define FWRITE(buffer, sizeoftype, sizeofdata, filepointer)     fwrite(buffer, sizeoftype, sizeofdata, filepointer)
#endif


#define MEMCPY(dst, src, length)                                    memcpy((dst), (src), (length))
#define MEMSET(dst, var, length)                                    memset((dst), (var), (length))


#define MALLOC(size)                                                malloc(size)


#define ADDRESS_STORAGE_SIZE                                        sizeof(size_t)
#define ALIGN_ADDRESS(addr, align)                                  (void*)(((size_t)(addr) + ((align) - 1)) & (size_t)-(align))


#define MAKE_STRIDE(nWidth)                                         ((((nWidth) + (2 * PADDING_SIZE)) + (PADDING_SIZE - 1)) & ~(PADDING_SIZE - 1))
#define GET_ALIGNED_LENGTH(value, align)                            ((value + (align - 1)) & ~(align - 1))
#define ROUND_BASE(x, base)                                         (x - (x & (base - 1)))


#if (defined(__GNUC__) && __GNUC__) || defined(__SUNPRO_C)
    #define MEM_ALIGNED(n)                                          __attribute__ ((aligned(n)))
#elif defined(_MSC_VER)
    #define MEM_ALIGNED(n)
#else
    #define MEM_ALIGNED(n)
#endif


#if (!defined (__INLINE)) && (!defined (__FORCE_INLINE))
    #if (defined (__GNUC__)) && (!defined (__GNUC_STDC_INLINE__))
        #define __FORCE_INLINE                                      __attribute__((always_inline))
        #define __INLINE                                            static inline
    #elif defined(_MSC_VER)
        #define __FORCE_INLINE                                      __forceinline
        #define __INLINE                                            __inline
    #else
        #define __FORCE_INLINE                                      static inline __attribute__((always_inline))
        #define __INLINE                                            static inline
    #endif
#endif


#define GET_INTEGRAL_PIX_POS(pIntegralSrc, x, y, nStride)           (&(pIntegralSrc[(y) * nStride + (x)]))
#define CALCULATE_INTEGRAL_SUM(nIntTable, nOffset)                  (nIntTable.pBR[nOffset] + nIntTable.pTL[nOffset] - nIntTable.pTR[nOffset] - nIntTable.pBL[nOffset])

#define SET_UNUSED_VARIABLE(variable)                               (void)variable


#define BILINEAR_APPX(nVal0, nVal1, nRatio, nPrecision)             (((nVal0 * nRatio) + (nVal1 * ((1 << nPrecision) - nRatio))) >> nPrecision)


#define CHECK_POINTER_VALIDATION(pointer, retval)\
                                    {\
                                        if(NULL == pointer)\
                                        {\
                                            nRet = retval;\
                                            goto Error;\
                                        }\
                                    }

#define CHECK_PARAM_VALIDATION(condition, retval)\
                                    {\
                                        if(IMGPROC_FALSE == condition)\
                                        {\
                                            nRet = retval;\
                                            goto Error;\
                                        }\
                                    }

#define SAFEALLOC(pHandle, size, align, type)\
                                    {\
                                        if(NULL == pHandle)\
                                        {\
                                            void    *pAlignedHandle = NULL;\
                                            void    *pTmpHandle = (void *) MALLOC((size + align - 1 + ADDRESS_STORAGE_SIZE) * sizeof(type));\
                                            if(NULL != pTmpHandle)\
                                            {\
                                                MEMSET(pTmpHandle, 0, ((size + align - 1 + ADDRESS_STORAGE_SIZE) * sizeof(type)));\
                                                pAlignedHandle = ALIGN_ADDRESS((UINT8 *)pTmpHandle + ADDRESS_STORAGE_SIZE, (int)align);\
                                                ((size_t *)pAlignedHandle)[-1] = (size_t)pTmpHandle;\
                                                pHandle = pAlignedHandle;\
                                            }\
                                            else\
                                            {\
                                                pHandle = NULL;\
                                                nRet = KAKAO_STAT_ALLOC_FAIL;\
                                                goto Error;\
                                            }\
                                        }\
                                    }


#define SAFEALLOC_NO_RET(pHandle, size, align, type)\
                                    {\
                                        if(NULL == pHandle)\
                                        {\
                                            void    *pAlignedHandle = NULL;\
                                            void    *pTmpHandle = (void *) MALLOC((size + align - 1 + ADDRESS_STORAGE_SIZE) * sizeof(type));\
                                            if(NULL != pTmpHandle)\
                                            {\
                                                MEMSET(pTmpHandle, 0, ((size + align - 1 + ADDRESS_STORAGE_SIZE) * sizeof(type)));\
                                                pAlignedHandle = ALIGN_ADDRESS((UINT8 *)pTmpHandle + ADDRESS_STORAGE_SIZE, (int)align);\
                                                ((size_t *)pAlignedHandle)[-1] = (size_t)pTmpHandle;\
                                                pHandle = pAlignedHandle;\
                                            }\
                                        }\
                                    }

#define SAFEFREE(pHandle)\
                                    if(NULL != pHandle)\
                                    {\
                                        void    *pRealHandle = (void *)(((size_t *)pHandle)[-1]);\
                                        free(pRealHandle);\
                                        pHandle = NULL;\
                                    }

#define SAVEImage(filename, ImgSize)\
                                    {\
                                        FILE    *fp;\
                                        fp = fopen(filename, "wb");\
                                        fwrite(pDst, sizeof(UINT8), ImgSize, fp);\
                                        fclose(fp);\
                                    }                                    

#define BLENDER(a, b, f) ((INT32)(a) + ((f) * ((INT32)(b) - (INT32)(a)) >> 16))

#define GET_PROPER_THREAD_NUM(nNumofCPUCore)                                ((nNumofCPUCore <= 1) ? 1 :\
                                                                            (nNumofCPUCore <= 2) ? 2 :\
                                                                            (nNumofCPUCore <= 3) ? 2 :\
                                                                            (nNumofCPUCore <= 4) ? 3 :\
                                                                            (nNumofCPUCore <= 6) ? 4 :\
                                                                            (nNumofCPUCore <= 8) ? 4 :\
                                                                            (nNumofCPUCore <= 16) ? 8 : 10)


#define GET_COLORFORMAT_IDX(nColorFormat)                                   ((IMG_FORMAT_GRAY == nColorFormat) ? 0 :\
                                                                            (IMG_FORMAT_RGB24 == nColorFormat) ? 1 :\
                                                                            (IMG_FORMAT_BGR24 == nColorFormat) ? 2 :\
                                                                            (IMG_FORMAT_RGBA32 == nColorFormat) ? 3 :\
                                                                            (IMG_FORMAT_ARGB32 == nColorFormat) ? 4 :\
                                                                            (IMG_FORMAT_BGRA32 == nColorFormat) ? 5 :\
                                                                            (IMG_FORMAT_ABGR32 == nColorFormat) ? 6 :\
                                                                            (IMG_FORMAT_I420 == nColorFormat) ? 7 : \
                                                                            (IMG_FORMAT_I422 == nColorFormat) ? 8 : \
                                                                            (IMG_FORMAT_HSV == nColorFormat) ?  9 : \
                                                                            (IMG_FORMAT_HSL == nColorFormat) ? 10 : \
                                                                            (IMG_FORMAT_NV12 == nColorFormat) ? 11 : \
                                                                            (IMG_FORMAT_NV21 == nColorFormat) ? 12 : \
                                                                            (IMG_FORMAT_UYVY == nColorFormat) ? 13 : -1);

#endif


