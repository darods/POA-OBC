//
//  Kakao_ImgProc_Structure.h
//
//  Created by Maverick Park on 2014. 8. 10.
//  Copyright (c) 2015년 Kakao. All rights reserved.
//

#ifndef     __KAKAO_IMGPROC_PRE_STRUCTURE_H__
#define     __KAKAO_IMGPROC_PRE_STRUCTURE_H__

#include "Kakao_ImgProc_Typedefs.h"


typedef MEM_ALIGNED(4) struct _Size
{
    INT32                           nWidth[4];                   // Image Width KISize
    INT32                           nHeight[4];                  // Image Height KISize
    INT32                           nStride[4];                  // Image Stride
    INT32                           nBaseWidth;                  // Base Width For Restoring From ROI Setting
    INT32                           nBaseHeight;                 // Base Height For Restoring From ROI Setting
    INT32                           nAllocatedWidth;             // Allocated Width
    INT32                           nAllocatedHeight;            // Allocated Height
}KISize;


typedef MEM_ALIGNED(4) struct _Point2
{
    INT32                           x;
    INT32                           y;
}KIPoint2;


typedef MEM_ALIGNED(4) struct _Point3
{
    INT32                           x;
    INT32                           y;
    INT32                           z;
}KIPoint3;


typedef MEM_ALIGNED(4) struct _PointF2
{
    F32                             x;
    F32                             y;
}KIPointF2;


typedef MEM_ALIGNED(4) struct _PointF3
{
    F32                             x;
    F32                             y;
    F32                             z;
}KIPointF3;


typedef MEM_ALIGNED(4) struct _Color_YUV
{
    UINT8                           y;
    UINT8                           u;
    UINT8                           v;
}Color_YUV;


typedef MEM_ALIGNED(4) struct _Color_RGB
{
    UINT8                           r;
    UINT8                           g;
    UINT8                           b;
}Color_RGB;


typedef MEM_ALIGNED(4) struct _Color_HSL
{
    D64                             h;
    D64                             s;
    D64                             l;
}Color_HSL;


typedef MEM_ALIGNED(4) struct _Color_HSV
{
    D64                             h;
    D64                             s;
    D64                             v;
}Color_HSV;


typedef MEM_ALIGNED(4) struct _Rect
{
    INT32                           x;
    INT32                           y;
    INT32                           nWidth;
    INT32                           nHeight;
}KIRect;


typedef MEM_ALIGNED(4) struct _ROI
{
    INT32                           nStartX;
    INT32                           nStartY;
    INT32                           nEndX;
    INT32                           nEndY;
}ROI;


typedef MEM_ALIGNED(4) struct _ROI_DB
{
    UINT8                           *pQPMap;                                    // An id between 0 and 3 for each 16x16 region within a frame
    INT32                           nRows;                                      // Number of rows
    INT32                           nCols;                                      // Number of columns
    INT32                           nDelta_Q[4];                                // Quantizer deltas
    INT32                           nDelta_LF[4];                               // Loop filter deltas
                                                                                // Static breakout threshold for each segment.
    UINT32                          nStatic_Th[4];
}ROI_DB;


typedef struct _Img_T               Img_T;
typedef struct _BaseImg_T           BaseImg_T;

typedef INT32 (*fpGetImgWidth)(IN const Img_T *pSrcImg, IN const INT32 nPlaneIdx);
typedef INT32 (*fpSetImgWidth)(IN Img_T *pSrcImg, IN const INT32 nPlaneIdx, IN const INT32 nWidth);
typedef INT32 (*fpGetImgHeight)(IN const Img_T *pSrcImg, IN const INT32 nPlaneIdx);
typedef INT32 (*fpSetImgHeight)(IN Img_T *pSrcImg, IN const INT32 nPlaneIdx, IN const INT32 nHeight);
typedef INT32 (*fpGetImgStride)(IN const Img_T *pSrcImg, IN const INT32 nPlaneIdx);
typedef INT32 (*fpSetImgStride)(IN Img_T *pSrcImg, IN const INT32 nPlaneIdx, IN const INT32 nStride);
typedef KISize (*fpGetImgSize)(IN const Img_T *pSrcImg);
typedef ImgFormat (*fpGetImgFormat)(IN const Img_T *pSrcImg);
typedef INT32 (*fpSetImgFormat)(IN const Img_T *pSrcImg, IN const ImgFormat nImgFormat);
typedef ImgDepth (*fpGetImgDepth)(IN const Img_T *pSrcImg);
typedef INT32 (*fpSetImgDepth)(IN const Img_T *pSrcImg, IN const ImgDepth nImgDepth);
typedef INT32 (*fpGetNumofImgPlanes)(IN const Img_T *pSrcImg);
typedef INT32 (*fpSetNumofImgPlanes)(IN const Img_T *pSrcImg, IN const INT32 nNumofImgPlanes);
typedef ROI (*fpGetImgROI)(IN const Img_T *pSrcImg);
typedef INT32 (*fpSetImgROI)(IN Img_T *pSrcImg, IN const INT32 nStartX, IN const INT32 nEndX, IN const INT32 nStartY, IN const INT32 nEndY);
typedef UINT8* (*fpGetImgMBWiseMap)(IN const Img_T *pSrcImg);
typedef INT32* (*fpGetImgMBWiseSADMap)(IN const Img_T *pSrcImg);
typedef UINT8* (*fpGetImgBinaryMap)(IN Img_T *pSrcImg, IN const BinaryMapType nBinaryMap);
typedef F32* (*fpGetImgHistMap)(IN const Img_T *pSrcImg, IN const INT32 nPlaneIdx, IN const INT32 nDimension);
typedef HANDLE (*fpGetImgPlanes)(IN const Img_T *pSrcImg, IN const INT32 nPlaneIdx);
typedef INT32 (*fpSetImgPlanes)(IN Img_T *pSrcImg, IN const INT32 nPlaneIdx, IN HANDLE pPtr);
typedef INT32 (*fpGetImgBins)(IN const Img_T *pSrcImg);
typedef INT32 (*fpSetImgBins)(IN Img_T *pSrcImg, IN const INT32 nBins);
typedef IMGPROC_BOOL (*fpGetImgROIStatus)(IN const Img_T *pSrcImg);
typedef INT32 (*fpSetImgROIStatus)(IN Img_T *pSrcImg, IN const IMGPROC_BOOL bIsROIEnabled);


struct MEM_ALIGNED(4) _Img_T
{
    fpGetImgWidth                   GetImgWidth;
    fpSetImgWidth                   SetImgWidth;
    fpGetImgHeight                  GetImgHeight;
    fpSetImgHeight                  SetImgHeight;
    fpGetImgStride                  GetImgStride;
    fpSetImgStride                  SetImgStride;
    fpGetImgSize                    GetImgSize;
    fpGetImgFormat                  GetImgFormat;
    fpSetImgFormat                  SetImgFormat;
    fpGetImgDepth                   GetImgDepth;
    fpSetImgDepth                   SetImgDepth;
    fpGetNumofImgPlanes             GetNumofImgPlanes;
    fpSetNumofImgPlanes             SetNumofImgPlanes;
    fpGetImgROI                     GetImgROI;
    fpSetImgROI                     SetImgROI;
    fpGetImgMBWiseMap               GetImgMBWiseMap;
    fpGetImgMBWiseSADMap            GetImgMBWiseSADMap;
    fpGetImgBinaryMap               GetImgBinaryMap;
    fpGetImgHistMap                 GetImgHistMap;
    fpGetImgPlanes                  GetImgPlanes;
    fpSetImgPlanes                  SetImgPlanes;
    fpGetImgBins                    GetImgBins;
    fpSetImgBins                    SetImgBins;
    fpGetImgROIStatus               GetImgROIStatus;
    fpSetImgROIStatus               SetImgROIStatus;
};


struct MEM_ALIGNED(4) _BaseImg_T
{
    Img_T                           nImgT;                                      // Base Handle of Img_T
    
    KISize                          nImgSize;                                   // Current Img Size
    ImgFormat                       nImgFormat;                                 // Image Color Format
    ImgDepth                        nImgDepth;                                  // Image Color BitDepth Type
    INT32                           nNumofPlanes;                               // Total Number of Image Planes
    ROI                             nROI;                                       // Region of Interest

    HANDLE                          pImgData;                                   // Pointer Indicates Start Point of Total Image Buffer
    HANDLE                          pRealImgData;                               // Pointer Indicates Start Point of Actual Image Buffer (Jump a Padding Region)
    HANDLE                          pPlanes[4];                                 /* IMG_PLANE_RGB_R / IMG_PLANE_RGB_G / IMG_PLANE_RGB_B
                                                                                   IMG_PLANE_YUV_Y / IMG_PLANE_YUV_U / IMG_PLANE_YUV_V
                                                                                   IMG_PLANE_YUV_Y / IMG_PLANE_YUV_UV
                                                                                   IMG_PLANE_YUV_Y / IMG_PLANE_YUV_VU
                                                                                   IMG_PLANE_HSV_H / IMG_PLANE_HSV_S / IMG_PLANE_HSV_V
                                                                                   IMG_PLANE_HSL_H / IMG_PLANE_HSL_S / IMG_PLANE_HSL_L */
    HANDLE                          pOrgImgPointer[4];                          // For Restore From ROI Setting
    
    INT32                           *pMBwiseSADMap;
    UINT8                           *pMBwiseMap;
    UINT8                           *pBinaryMap[IMG_BINARYMAP_MAX];
    UINT8                           *pOneRowBuf;
    F32                             *p1DHist[4];                                // 256 Bytes Array for 8bit Color Space
    INT32                           *pTmp1DHist;
    F32                             *p2DHist;                                   // 256 * 256 Bytes Array for 8bit Color Space
    INT32                           *pTmp2DHist;
    UINT8                           *p1DHistBPTable;                            // 256 Bytes Array for 8bit Color Space
    UINT8                           *p2DHistBPTable;                            // 256 * 256 Bytes Array for 8bit Color Space
    INT32                           nBins;
    IMGPROC_BOOL                    bIsROIEnabled;
};


#endif
