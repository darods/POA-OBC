/*----------------------------------------------------------------------------------------
 File Inclusions
 ----------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <memory.h>
#include <fcntl.h>
#include <termios.h>
#include <pthread.h>
#include "cv.h"
#include "highgui.h"
#include "Kakao_ImgProc_Modules.h"

#include <OpenGL/gl.h> // OpenGL framework on Mac (version 1.5.10.7.0)
//#include <GL/gl.h> // OpenGL library on Windows

#include <GLUT/glut.h> // GLUT framework on Mac (version 3.4.2)
//#include <GL/glut.h> // GLUT library on Windows


/*----------------------------------------------------------------------------------------
 Constant Definitions
 ----------------------------------------------------------------------------------------*/
#define ENC_IMG_WIDTH               (608)
#define ENC_IMG_HEIGHT              (352)
#define ENC_FRAME_RATE              (1500)


/*----------------------------------------------------------------------------------------
 Macro Definitions
 ----------------------------------------------------------------------------------------*/
#define Thread                      signed int
#define TSK_YIELD()                 usleep(1)        //Sleep(0)      // take care of YIELD
#define TSK_SLEEP(n)                usleep(n)
#define CREATE_THREAD               CreateThread
#define RESUME_THREAD               ResumeThread
#define EXIT_THREAD                 GetExitCodeThread
#define SUSPEND_THREAD              SuspendThread

//#define __USE_LOCK_OBJ__

#ifdef __USE_LOCK_OBJ__
    #define CS                      CRITICAL_SECTION
    #define InitCS(a)               InitializeCriticalSection(&(a))
    #define EnterCS(a)              EnterCriticalSection(&(a))
    #define ExitCS(a)               LeaveCriticalSection(&(a))
    #define UnInitCS(a)             DeleteCriticalSection(&(a))
#else
    #define CS                      pthread_mutex_t
    #define InitCS(a)               pthread_mutex_init(&(a), NULL)
    #define EnterCS(a)              pthread_mutex_lock(&(a))    // 100 : time out
    #define ExitCS(a)               pthread_mutex_unlock(&(a))
    #define UnInitCS(a)             pthread_mutex_destroy(&(a))
#endif

#define ROUND_BASE(x, base)         (x - (x & (base - 1)))

/*----------------------------------------------------------------------------------------
 Type Definitions
 ----------------------------------------------------------------------------------------*/
typedef enum _GLTexture
{
    GLTEXTURE_ORGIMG,
    GLTEXTURE_DECIMG,
    GLTEXTURE_IMGPROC,
    GLTEXTURE_STAT_DATA,
    GLTEXTURE_IMG_DATA0,
    GLTEXTURE_IMG_DATA1,
    GLTEXTURE_MAXNUM,
}GLTexture;


typedef struct _MainStruct_T
{
    IplImage                        *pCapturedImg_IPL[2];
    IplImage                        *pDecodedImg_IPL[2];
    IplImage                        *pDisplayImg_IPL[2];
    IplImage                        *pEncodingStatisticsImg_IPL[2];
    IplImage                        *pImgData0_IPL;
    IplImage                        *pImgData1_IPL;
    IplImage                        *pEdgeMapImg_IPL;
    IplImage                        *pImgProcessedImg_IPL;
    IplImage                        *pTextureImg_IPL;
    IplImage                        *pOpneGLSurfaceImg_IPL;
    IplImage                        *pCmpScaledModeImg_IPL[IMG_SCALEMODE_MAX];
    GLuint                          nTexture[GLTEXTURE_MAXNUM];
    CvSize                          nYImgSize, nUVImgSize, nOpenGLImgSize;
    
    Img_T                           *pOrgRGBImg_T;
    Img_T                           *pOrgYUV420Img_T;
    Img_T                           *pOrgScaledRGBImg_T;
    Img_T                           *pDstRGBImg_T[2];
    
    Img_T                           *pEncSrcImg_T[2][2];
    Img_T                           *pScaledImg_T[2];
    Img_T                           *pCmpScaledModeYUV420Img_T;
    Img_T                           *pCmpScaledModeRGBImg_T;
    Img_T                           *pImgForImgProc_T;
    Img_T                           *pTmpEncSrcImg_T;
    Img_T                           *pDummyEncImg;
    Img_T                           *pDummyDecImg[2];
    
    unsigned int                    nCurrEncodingFramePos;                              // 현재 encoding하려고 하는 frame의 위치
    unsigned int                    nPrevEncodingFramePos;                              // 직전에 encoding 한 frame의 위치
    unsigned int                    nCurrBufferFramePos;                                // 현재 들어온 data를 저장할 위치
    unsigned int                    nTotalBufferedFrames[2];                               // 현재까지 buffering된 전체 frame 수
    
    unsigned char                   *pEncodedBuf;
    
    unsigned int                    key, nCount, i;
    unsigned int                    nEncodedStreamLen[2];
    unsigned int                    nIsKeyFrame[2];
    unsigned int                    nCurrFrame_Q[2];
    unsigned int                    nBitPerSec[2];
    long long                       nProcessedFrameCnt;
	   
    unsigned int                    nBitrateArr[2][4][30];
    int                             nEncBufferLevel[2][30];
    int                             nEstimatedBufferLevel[2][30];
    float                           IQA[2][30];                        // Image Quality Assessment
    
    unsigned int                    nAvgBitratePerSec[2];
    unsigned long long              nAvgBitrateTotal[2];
    unsigned int                    MinBR, MaxBR;
    float                           nSumPSNR[2];
    signed int                      nTargetBR[2];
    
    CvFont                          nFont[2];
    char                            text[200];
    
    // Capture In / Out Img
    CvVideoWriter                   *pVideoOut;
    FILE                            *fpInImg;
    FILE                            *fpOutImg;
    
    // For Labeling
    BlobLabel                       *pBlobLabel;
    UINT32                          nNumofBlob;
    BlobInfo                        nBlobInfo[500];
    
    SkinDet_T                       *pSkinDetector;
    Filter_T                        *pFilter;
    EdgeDet_T                       *pEdgeDetector;
    IQA_T                           *pIQA;
    ColorTool_T                     *pColorTool;
    ObjDetect_T                     *pObjDetector;
    ImgScaler_T                     *pScaler;
    MeanShift_T                     *pMeanShift;
    
    DetectedObjDataSet_T            *pDetectedObj;
    ImgSizeType                     nImgSizeType;                       // Type of ImgSize (QCIF / QVGA / CIF / VGA / 720P / 1080P)
    INT32                           nDegree;                            // Calculated Degree (= (((CameraRotate - 1) + (DeviceRotate - 1)) * 90) - 360)
    UINT32                          nScaleFactor;
    
    int                             nSerialPort;
    unsigned char                   nDataBuf[50];
}MainStruct_T;


/*----------------------------------------------------------------------------------------
 Static Function
 ----------------------------------------------------------------------------------------*/
void Thread_TrackingObj_MeanShift();
static void _EncDecSet_Create(MainStruct_T **ppMainStruct, const unsigned int nSrcWidth, const unsigned int nSrcHeight,
                              const unsigned int nTargetWidth, const unsigned int nTargetHeight);
static void _EncDecSet_Destroy(MainStruct_T **ppMainStruct);
static void _Copy_RGBIplImg_to_RGBImgT(IplImage *pSrcImg, Img_T *pDstImg);
static void _Copy_RGBImgT_to_RGBIplImg(Img_T *pSrcImg, IplImage *pDstImg);
static void _Copy_GrayIplImg_to_GrayImgT(IplImage *pSrcImg, Img_T *pDstImg);
static void _Copy_GrayImgT_to_GrayIplImg(Img_T *pSrcImg, IplImage *pDstImg);
static void _Copy_GrayImgT_to_RGBIplImg(Img_T *pSrcImg, IplImage *pDstImg);
static void _Copy_EdgeImgT_to_IplImg(Img_T *pSrcImg, IplImage *pDstImg);
static void _Copy_SkinImgT_to_IplImg(Img_T *pSrcImg, IplImage *pDstImg);
static void _Init();
static void _Idle();
static void _DrawImage(MainStruct_T *pMainStruct, IplImage *pSrcImg, GLuint nTextureIdx);
static void _Keyboard(UINT8 nKey, INT32 nMouseX, INT32 nMouseY);
static void _Reshape(INT32 nWidth, INT32 nHeight);
static void _DrawRect(IplImage *pDstImg, UINT32 x, UINT32 y, UINT32 nWidth, UINT32 nHeight, CvScalar nColor);
static void _OpenSerialPort(char *pDeviceName, int *pFp);
static void _CloseSerialPort(int fd);
static void *_AnotherPortReceive(void *fd);


/*----------------------------------------------------------------------------------------
 Static Variable
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 External Variable
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Global Variable
 ----------------------------------------------------------------------------------------*/
int                             fps = 0;
int                             _gnWidth = 0;
int                             _gnHeight = 0;
MainStruct_T                    *_gpMainStruct = NULL;
CvCapture                       *_gpCapture = NULL;


/*----------------------------------------------------------------------------------------
 Function Implementation
 ----------------------------------------------------------------------------------------*/
int main(int argc, const char * argv[])
{
    MainStruct_T                *pMainStruct = NULL;
    IplImage                    *image = 0;
    IplImage                    *pSrcImg = NULL, *pScaledImg = NULL;
    IplImage                    *pHistogramImg = NULL;
    static UINT8                *pLocalDst = NULL;
    int                         i = 0;
    F32                         *pRef1DHistogram = NULL;
    F32                         *pRef2DHistogram = NULL;
    INT32                       nBins = 16;
    UINT32                      nFrameCnt = 0;
    INT32                       nDimension = 2;
    
    //cvNamedWindow("Source", 0);
    //cvResizeWindow("Source", ENC_IMG_WIDTH, ENC_IMG_HEIGHT);
    
    _gpCapture = cvCaptureFromCAM(0);
    
    pSrcImg = cvQueryFrame(_gpCapture);
    
    fps = 30;
    _gnWidth = (int)cvGetCaptureProperty(_gpCapture, CV_CAP_PROP_FRAME_WIDTH);
    _gnHeight = (int)cvGetCaptureProperty(_gpCapture, CV_CAP_PROP_FRAME_HEIGHT);
    
    pHistogramImg = cvCreateImage(cvSize(256, 256), IPL_DEPTH_8U, 1);
    pScaledImg = cvCreateImage(cvSize(ENC_IMG_WIDTH, ENC_IMG_HEIGHT), IPL_DEPTH_8U, 3);

    _EncDecSet_Create(&pMainStruct, _gnWidth, _gnHeight, ENC_IMG_WIDTH, ENC_IMG_HEIGHT);
    _gpMainStruct = pMainStruct;
    
    {
        glutInit(&argc, argv);
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

        glutInitWindowSize(pMainStruct->nOpenGLImgSize.width, pMainStruct->nOpenGLImgSize.height);
        glutInitWindowPosition(0, 0);
        glutCreateWindow("OpenCV to OpenGL");
        
        _Init();
        
        glutIdleFunc(_Idle);
        glutDisplayFunc(Thread_TrackingObj_MeanShift);
        glutKeyboardFunc(_Keyboard);
        glutReshapeFunc(_Reshape);
        
        glutMainLoop ();
    }
    
    _EncDecSet_Destroy(&pMainStruct);
    
    cvReleaseCapture(&_gpCapture);
    cvDestroyWindow("Source");
    
    return 0;
}


void Thread_TrackingObj_MeanShift()
{
    IplImage                    *pSrcImg = NULL;
    static KIPointF2            nCurrCenterPtr = {0, };
    CvPoint                     nTL = {0, 0};
    CvPoint                     nBR = {0, 0};
    const CvScalar              nColor = {0, 0, 255, 0};
    
    {
        unsigned int        nCurrBufferFramePos = _gpMainStruct->nCurrBufferFramePos;
        
        pSrcImg = cvQueryFrame(_gpCapture);
        
        /* display frame */
        sprintf(_gpMainStruct->text, "%d", (int)_gpMainStruct->nProcessedFrameCnt);
        cvPutText(pSrcImg, _gpMainStruct->text, cvPoint(50, 20), &(_gpMainStruct->nFont[1]), CV_RGB(255, 0, 0));
        
        // Convert Color (RGB -> YUV420)
        _Copy_RGBIplImg_to_RGBImgT(pSrcImg, _gpMainStruct->pOrgRGBImg_T);
        Kakao_ImgProc_ColorTool_Cvt(_gpMainStruct->pColorTool, _gpMainStruct->pOrgRGBImg_T, _gpMainStruct->pOrgYUV420Img_T);
        Kakao_ImgProc_Scaler_External(_gpMainStruct->pScaler, _gpMainStruct->pOrgYUV420Img_T, _gpMainStruct->pEncSrcImg_T[0][nCurrBufferFramePos], IMG_SCALEMODE_BOX);
        
        if(20 > _gpMainStruct->nProcessedFrameCnt)
            goto Skip;
        if(22 > _gpMainStruct->nProcessedFrameCnt)
        {
            const Img_T         *pEncSrcImg = _gpMainStruct->pEncSrcImg_T[0][nCurrBufferFramePos];
            const INT32         nWidth = pEncSrcImg->GetImgWidth(pEncSrcImg, IMG_PLANE_0);
            const INT32         nHeight = pEncSrcImg->GetImgHeight(pEncSrcImg, IMG_PLANE_0);
            const INT32         nROIWidth = nWidth / 10;
            const INT32         nROIHeight = nHeight / 10;
            ROI                 nTargetROI = {0, };
            KIPointF2           nCenterPtr = {(F32)(nWidth / 2), (F32)(nHeight / 2)};
            
            nTargetROI.nStartX = nCenterPtr.x - nROIWidth;
            nTargetROI.nEndX = nCenterPtr.x + nROIWidth;
            nTargetROI.nStartY = nCenterPtr.y - nROIHeight;
            nTargetROI.nEndY = nCenterPtr.y + nROIHeight;
            
            Kakao_ImgProc_MeanShift_Track_SetReference(_gpMainStruct->pMeanShift, _gpMainStruct->pEncSrcImg_T[0][nCurrBufferFramePos], nTargetROI);
            
            nCurrCenterPtr = nCenterPtr;
            
            goto Skip;
        }
        
        Kakao_ImgProc_MeanShift_DoTracking(_gpMainStruct->pMeanShift, _gpMainStruct->pEncSrcImg_T[0][nCurrBufferFramePos]);
        
        Kakao_ImgProc_MeanShift_GetCenterPtr(_gpMainStruct->pMeanShift, &nCurrCenterPtr);
        
    Skip:
        nTL.x = nCurrCenterPtr.x - 12;
        nTL.y = nCurrCenterPtr.y - 12;
        nBR.x = nCurrCenterPtr.x + 12;
        nBR.y = nCurrCenterPtr.y + 12;
        
        // Output ROI Position to Serial Port
        //_gpMainStruct->nDataBuf[0] = 180 - 180 * (nTL.x + nBR.x) / 2 / ENC_IMG_WIDTH;
        //_gpMainStruct->nDataBuf[1] = 180 * (nTL.y + nBR.y) / 2 / ENC_IMG_HEIGHT;
        //_gpMainStruct->nDataBuf[2] = '\n';
        //write(_gpMainStruct->nSerialPort, &(_gpMainStruct->nDataBuf[0]), 1);
        //write(_gpMainStruct->nSerialPort, &(_gpMainStruct->nDataBuf[1]), 1);
        //write(_gpMainStruct->nSerialPort, &(_gpMainStruct->nDataBuf[2]), 1);
        
        Kakao_ImgProc_ColorTool_Cvt(_gpMainStruct->pColorTool, _gpMainStruct->pEncSrcImg_T[0][nCurrBufferFramePos], _gpMainStruct->pOrgScaledRGBImg_T);
        _Copy_RGBImgT_to_RGBIplImg(_gpMainStruct->pOrgScaledRGBImg_T, _gpMainStruct->pCapturedImg_IPL[nCurrBufferFramePos]);
        
        cvRectangle(_gpMainStruct->pCapturedImg_IPL[nCurrBufferFramePos], nTL, nBR, nColor, 1, 8, 0);
        
        _DrawImage(_gpMainStruct, _gpMainStruct->pCapturedImg_IPL[nCurrBufferFramePos], GLTEXTURE_ORGIMG);
        //_DrawImage(_gpMainStruct, _gpMainStruct->pCapturedImg_IPL[nCurrBufferFramePos], GLTEXTURE_STAT_DATA);
        
        _gpMainStruct->nProcessedFrameCnt++;
        _gpMainStruct->nCurrBufferFramePos = (_gpMainStruct->nCurrBufferFramePos + 1) % 2;
    }
    
    return;
}


static void _EncDecSet_Create(MainStruct_T **ppMainStruct, const unsigned int nSrcWidth, const unsigned int nSrcHeight,
                              const unsigned int nTargetWidth, const unsigned int nTargetHeight)
{
    unsigned int        i = 0, j = 0;
    MainStruct_T        *pMainStruct = NULL;
    CvSize              EncodingStatisticsSize = {720, 480};
    
    pMainStruct = (MainStruct_T *)calloc(1, sizeof(MainStruct_T));
    
    pMainStruct->MinBR = 0xFFFFFFFF;
    pMainStruct->MaxBR = 0;
    
    cvInitFont(&(pMainStruct->nFont[0]), CV_FONT_VECTOR0, 0.2f, 0.2f, 0, 1, 1);
    cvInitFont(&(pMainStruct->nFont[1]), CV_FONT_VECTOR0, 0.3f, 0.4f, 0, 1, 1);
    
    pMainStruct->nYImgSize.width = nTargetWidth;
    pMainStruct->nYImgSize.height = nTargetHeight;
    pMainStruct->nOpenGLImgSize.width = pMainStruct->nYImgSize.width;
    pMainStruct->nOpenGLImgSize.height = pMainStruct->nYImgSize.height;
    
    for(j=0 ; j<2 ; j++)
        pMainStruct->pCapturedImg_IPL[j] = cvCreateImage(pMainStruct->nYImgSize, IPL_DEPTH_8U, 3);
    
    for(j=0 ; j<2 ; j++)
    {
        pMainStruct->pDecodedImg_IPL[j] = cvCreateImage(pMainStruct->nYImgSize, IPL_DEPTH_8U, 3);
        pMainStruct->pDisplayImg_IPL[j] = cvCreateImage(pMainStruct->nYImgSize, IPL_DEPTH_8U, 3);
        pMainStruct->pEncodingStatisticsImg_IPL[j] = cvCreateImage(EncodingStatisticsSize, IPL_DEPTH_8U, 3);
        
        Kakao_ImgProc_Util_CreateImg(&(pMainStruct->pDstRGBImg_T[j]), pMainStruct->nYImgSize.width, pMainStruct->nYImgSize.height, IMG_FORMAT_BGR24, IMG_DEPTH_U8);
        Kakao_ImgProc_Util_CreateDummyImg(&(pMainStruct->pDummyDecImg[j]), nSrcWidth, nSrcHeight, IMG_FORMAT_I420, IMG_DEPTH_U8);
    }
    
    pMainStruct->pImgData0_IPL = cvCreateImage(pMainStruct->nYImgSize, IPL_DEPTH_8U, 3);
    pMainStruct->pImgData1_IPL = cvCreateImage(pMainStruct->nYImgSize, IPL_DEPTH_8U, 3);
    pMainStruct->pEdgeMapImg_IPL = cvCreateImage(pMainStruct->nYImgSize, IPL_DEPTH_8U, 1);
    pMainStruct->pImgProcessedImg_IPL = cvCreateImage(pMainStruct->nYImgSize, IPL_DEPTH_8U, 3);
    for(j=0 ; j<IMG_SCALEMODE_MAX ; j++)
        pMainStruct->pCmpScaledModeImg_IPL[j] = cvCreateImage(pMainStruct->nYImgSize, IPL_DEPTH_8U, 3);
    pMainStruct->pOpneGLSurfaceImg_IPL = cvCreateImage(pMainStruct->nOpenGLImgSize, IPL_DEPTH_8U, 3);
    
    // Allocating buffers
    {
        for(i=0 ; i<2 ; i++)
            for(j=0 ; j<2 ; j++)
                Kakao_ImgProc_Util_CreateImg(&(pMainStruct->pEncSrcImg_T[i][j]), pMainStruct->nYImgSize.width, pMainStruct->nYImgSize.height, IMG_FORMAT_I420, IMG_DEPTH_U8);
        
        Kakao_ImgProc_Util_CreateImg(&(pMainStruct->pOrgRGBImg_T), nSrcWidth, nSrcHeight, IMG_FORMAT_BGR24, IMG_DEPTH_U8);
        Kakao_ImgProc_Util_CreateImg(&(pMainStruct->pOrgYUV420Img_T), nSrcWidth, nSrcHeight, IMG_FORMAT_I420, IMG_DEPTH_U8);
        Kakao_ImgProc_Util_CreateImg(&(pMainStruct->pOrgScaledRGBImg_T), pMainStruct->nYImgSize.width, pMainStruct->nYImgSize.height, IMG_FORMAT_BGR24, IMG_DEPTH_U8);
        Kakao_ImgProc_Util_CreateImg(&(pMainStruct->pImgForImgProc_T), pMainStruct->nYImgSize.width, pMainStruct->nYImgSize.height, IMG_FORMAT_I420, IMG_DEPTH_U8);
        Kakao_ImgProc_Util_CreateImg(&(pMainStruct->pTmpEncSrcImg_T), pMainStruct->nYImgSize.width, pMainStruct->nYImgSize.height, IMG_FORMAT_I420, IMG_DEPTH_U8);
        
        Kakao_ImgProc_Util_CreateImg(&(pMainStruct->pScaledImg_T[0]), pMainStruct->nYImgSize.width/3, pMainStruct->nYImgSize.height/3, IMG_FORMAT_I420, IMG_DEPTH_U8);
        Kakao_ImgProc_Util_CreateImg(&(pMainStruct->pScaledImg_T[1]), pMainStruct->nYImgSize.width/3, pMainStruct->nYImgSize.height/3, IMG_FORMAT_I420, IMG_DEPTH_U8);
        
        Kakao_ImgProc_Util_CreateImg(&(pMainStruct->pCmpScaledModeYUV420Img_T), pMainStruct->nYImgSize.width, pMainStruct->nYImgSize.height, IMG_FORMAT_I420, IMG_DEPTH_U8);
        Kakao_ImgProc_Util_CreateImg(&(pMainStruct->pCmpScaledModeRGBImg_T), pMainStruct->nYImgSize.width, pMainStruct->nYImgSize.height, IMG_FORMAT_BGR24, IMG_DEPTH_U8);
    }
    
    Kakao_ImgProc_Util_CreateDummyImg(&(pMainStruct->pDummyEncImg), nSrcWidth, nSrcHeight, IMG_FORMAT_I420, IMG_DEPTH_U8);
    
    pMainStruct->pEncodedBuf = (unsigned char *)malloc(1000 * 500);
    
    //pMainStruct->fpInImg = FOPEN("/Users/Maverick/Downloads/Input.yuv", "wb");
    //pMainStruct->fpOutImg = FOPEN("/Users/Maverick/Downloads/Output.yuv", "wb");
    
    #if SAVE_ENCODED_STREAM_AVI
    {
        char                            nLocalFilePath[200] = {0, };
        CvSize                          nYImgSize;
        
        sprintf(nLocalFilePath, "%s/VP8Encoded_Video_%.01ffps_%dkbps.mpg", nFilePath, (float)nFrameRate/100.0f, nTargetBitrate);
        
        pMainStruct->pVideoOut = cvCreateVideoWriter(nLocalFilePath, CV_FOURCC('M','P','E','G'),
                                                      30.0f/*(D64)nFrameRate/100.0f*/, pMainStruct->nOpenGLImgSize, 1);
    }
    #endif
    
    Kakao_ImgProc_BlobLabeling_Create(&(pMainStruct->pBlobLabel), pMainStruct->nYImgSize.width, pMainStruct->nYImgSize.height);
    Kakao_ImgProc_SkinDetector_Create(&(pMainStruct->pSkinDetector), pMainStruct->nYImgSize.width, pMainStruct->nYImgSize.height);
    Kakao_ImgProc_Filter_Create(&(pMainStruct->pFilter), pMainStruct->nYImgSize.width, pMainStruct->nYImgSize.height);
    Kakao_ImgProc_Edge_Create(&(pMainStruct->pEdgeDetector), nTargetWidth, nTargetHeight);
    Kakao_ImgProc_IQA_Create(&(pMainStruct->pIQA), nTargetWidth, nTargetHeight);
    Kakao_ImgProc_ColorTool_Create(&(pMainStruct->pColorTool), nTargetWidth, nTargetHeight);
    Kakao_ImgProc_ObjDetector_Create(&(pMainStruct->pObjDetector), nTargetWidth, nTargetHeight);
    Kakao_ImgProc_Scaler_Create(&(pMainStruct->pScaler), nTargetWidth, nTargetHeight);
    Kakao_ImgProc_MeanShift_Create(&(pMainStruct->pMeanShift), nTargetWidth, nTargetHeight);
    Kakao_ImgProc_Profiler_Create();
    
    pMainStruct->pDetectedObj = (DetectedObjDataSet_T *) malloc(sizeof(DetectedObjDataSet_T));
    
    Kakao_ImgProc_ObjDetector_Update_RotateBasedFeature(pMainStruct->pObjDetector, OBJMODEL_FACE_FRONT, 270);
    
    _OpenSerialPort("/dev/cu.wchusbserial1410", &(pMainStruct->nSerialPort));
    MEMSET(pMainStruct->nDataBuf, 0, 50 * sizeof(unsigned char));
    
    *ppMainStruct = pMainStruct;
    
    return;
}


static void _EncDecSet_Destroy(MainStruct_T **ppMainStruct)
{
    unsigned int            i = 0, j = 0;
    MainStruct_T            *pMainStruct = *ppMainStruct;
    
    free(pMainStruct->pDetectedObj);
    pMainStruct->pDetectedObj = NULL;
    
    Kakao_ImgProc_BlobLabeling_Destroy(&(pMainStruct->pBlobLabel));
    Kakao_ImgProc_SkinDetector_Destroy(&(pMainStruct->pSkinDetector));
    Kakao_ImgProc_Filter_Destroy(&(pMainStruct->pFilter));
    Kakao_ImgProc_Edge_Destroy(&(pMainStruct->pEdgeDetector));
    Kakao_ImgProc_IQA_Destroy(&(pMainStruct->pIQA));
    Kakao_ImgProc_ColorTool_Destroy(&(pMainStruct->pColorTool));
    Kakao_ImgProc_ObjDetector_Destroy(&(pMainStruct->pObjDetector));
    Kakao_ImgProc_Scaler_Destroy(&(pMainStruct->pScaler));
    Kakao_ImgProc_MeanShift_Destroy(&(pMainStruct->pMeanShift));
    Kakao_ImgProc_Profiler_Destroy();
    
    cvReleaseVideoWriter(&(pMainStruct->pVideoOut));
    pMainStruct->pVideoOut = NULL;
    
    free(pMainStruct->pEncodedBuf);
    pMainStruct->pEncodedBuf = NULL;
    
    // Release all buffers
    for(i=0 ; i<2 ; i++)
        for(j=0 ; j<2 ; j++)
            Kakao_ImgProc_Util_ReleaseImg(&(pMainStruct->pEncSrcImg_T[i][j]));
    
    Kakao_ImgProc_Util_ReleaseImg(&(pMainStruct->pTmpEncSrcImg_T));
    Kakao_ImgProc_Util_ReleaseImg(&(pMainStruct->pOrgRGBImg_T));
    Kakao_ImgProc_Util_ReleaseImg(&(pMainStruct->pOrgYUV420Img_T));
    Kakao_ImgProc_Util_ReleaseImg(&(pMainStruct->pImgForImgProc_T));
    Kakao_ImgProc_Util_ReleaseImg(&(pMainStruct->pScaledImg_T[0]));
    Kakao_ImgProc_Util_ReleaseImg(&(pMainStruct->pScaledImg_T[1]));
    Kakao_ImgProc_Util_ReleaseImg(&(pMainStruct->pCmpScaledModeYUV420Img_T));
    Kakao_ImgProc_Util_ReleaseImg(&(pMainStruct->pCmpScaledModeRGBImg_T));
    Kakao_ImgProc_Util_ReleaseImg(&(pMainStruct->pDummyEncImg));
    
    for(j=0 ; j<2 ; j++)
        cvReleaseImage(&pMainStruct->pCapturedImg_IPL[j]);
    
    for(j=0 ; j<IMG_SCALEMODE_MAX ; j++)
        cvReleaseImage(&pMainStruct->pCmpScaledModeImg_IPL[j]);
    
    cvReleaseImage(&pMainStruct->pOpneGLSurfaceImg_IPL);
    cvReleaseImage(&pMainStruct->pImgData0_IPL);
    cvReleaseImage(&pMainStruct->pImgData1_IPL);
    cvReleaseImage(&pMainStruct->pEdgeMapImg_IPL);
    cvReleaseImage(&pMainStruct->pImgProcessedImg_IPL);
    
    for(j=0 ; j<2 ; j++)
    {
        cvReleaseImage(&pMainStruct->pDecodedImg_IPL[j]);
        cvReleaseImage(&pMainStruct->pDisplayImg_IPL[j]);
        cvReleaseImage(&pMainStruct->pEncodingStatisticsImg_IPL[j]);
        
        Kakao_ImgProc_Util_ReleaseImg(&(pMainStruct->pDstRGBImg_T[j]));
        Kakao_ImgProc_Util_ReleaseImg(&(pMainStruct->pDummyDecImg[j]));
    }

    _CloseSerialPort(pMainStruct->nSerialPort);
    
    FCLOSE(pMainStruct->fpInImg);
    FCLOSE(pMainStruct->fpOutImg);
    
    free(*ppMainStruct);
    
    return;
}


static void _Copy_RGBIplImg_to_RGBImgT(IplImage *pSrcImg, Img_T *pDstImg)
{
    INT32               j = 0;
    INT32               nWidth = pSrcImg->width;
    INT32               nHeight = pSrcImg->height;
    INT32               nSrcStride = pSrcImg->widthStep;
    INT32               nDstStride = pDstImg->GetImgStride(pDstImg, IMG_PLANE_0);
    UINT8               *pSrc = NULL, *pDst = NULL;
    
    pSrc = (UINT8 *)pSrcImg->imageData;
    pDst = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0);
    for(j=0 ; j<nHeight ; j++)
        MEMCPY(pDst + (j * nDstStride), pSrc + (j * nSrcStride), nWidth * 3 * sizeof(UINT8));
    
    return;
}


static void _Copy_RGBImgT_to_RGBIplImg(Img_T *pSrcImg, IplImage *pDstImg)
{
    INT32               j = 0;
    INT32               nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32               nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32               nSrcStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    INT32               nDstStride = pDstImg->widthStep;
    UINT8               *pSrc = NULL, *pDst = NULL;
    
    pSrc = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
    pDst = (UINT8 *)pDstImg->imageData;
    for(j=0 ; j<nHeight ; j++)
        MEMCPY(pDst + (j * nDstStride), pSrc + (j * nSrcStride), nWidth * 3 * sizeof(UINT8));
    
    return;
}


static void _Copy_GrayIplImg_to_GrayImgT(IplImage *pSrcImg, Img_T *pDstImg)
{
    INT32               j = 0;
    INT32               nWidth = pSrcImg->width;
    INT32               nHeight = pSrcImg->height;
    INT32               nSrcStride = pSrcImg->widthStep;
    INT32               nDstStride = pDstImg->GetImgStride(pDstImg, IMG_PLANE_0);
    UINT8               *pSrc = NULL, *pDst = NULL;
    
    pSrc = (UINT8 *)pSrcImg->imageData;
    pDst = (UINT8 *)pDstImg->GetImgPlanes(pDstImg, IMG_PLANE_0);
    for(j=0 ; j<nHeight ; j++)
        MEMCPY(pDst + (j * nDstStride), pSrc + (j * nSrcStride), nWidth * sizeof(UINT8));
    
    return;
}


static void _Copy_GrayImgT_to_GrayIplImg(Img_T *pSrcImg, IplImage *pDstImg)
{
    INT32               j = 0;
    INT32               nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32               nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32               nSrcStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    INT32               nDstStride = pDstImg->widthStep;
    UINT8               *pSrc = NULL, *pDst = NULL;
    
    pSrc = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
    pDst = (UINT8 *)pDstImg->imageData;
    for(j=0 ; j<nHeight ; j++)
        MEMCPY(pDst + (j * nDstStride), pSrc + (j * nSrcStride), nWidth * sizeof(UINT8));
    
    return;
}


static void _Copy_GrayImgT_to_RGBIplImg(Img_T *pSrcImg, IplImage *pDstImg)
{
    INT32               i = 0, j = 0;
    INT32               nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32               nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32               nSrcStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    INT32               nDstStride = pDstImg->widthStep;
    INT32               nSrcLocalPos = 0;
    INT32               nDstLocalPos = 0;
    UINT8               *pSrc = NULL, *pDst = NULL;
    
    pSrc = (UINT8 *)pSrcImg->GetImgPlanes(pSrcImg, IMG_PLANE_0);
    pDst = (UINT8 *)pDstImg->imageData;
    for(j=0 ; j<nHeight ; j++)
    {
        nSrcLocalPos = j * nSrcStride;
        nDstLocalPos = j * nDstStride;
        for(i=0 ; i<nWidth ; i++)
        {
            pDst[nDstLocalPos] = pSrc[nSrcLocalPos];
            pDst[nDstLocalPos++] = pSrc[nSrcLocalPos];
            pDst[nDstLocalPos++] = pSrc[nSrcLocalPos++];
        }
    }
    
    return;
}


static void _Copy_EdgeImgT_to_IplImg(Img_T *pSrcImg, IplImage *pDstImg)
{
    INT32               j = 0;
    INT32               nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32               nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32               nSrcStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    INT32               nDstStride = pDstImg->widthStep;
    UINT8               *pSrc = NULL, *pDst = NULL;
    
    pSrc = (UINT8 *)pSrcImg->GetImgBinaryMap(pSrcImg, IMG_BINARYMAP_EDGE) + PADDING_SIZE;
    pDst = (UINT8 *)pDstImg->imageData;
    for(j=0 ; j<nHeight ; j++)
        MEMCPY(pDst + (j * nDstStride), pSrc + (j * nSrcStride), nWidth * sizeof(UINT8));
    
    return;
}


static void _Copy_SkinImgT_to_IplImg(Img_T *pSrcImg, IplImage *pDstImg)
{
    INT32               j = 0;
    INT32               nWidth = pSrcImg->GetImgWidth(pSrcImg, IMG_PLANE_0);
    INT32               nHeight = pSrcImg->GetImgHeight(pSrcImg, IMG_PLANE_0);
    INT32               nSrcStride = pSrcImg->GetImgStride(pSrcImg, IMG_PLANE_0);
    INT32               nDstStride = pDstImg->widthStep;
    UINT8               *pSrc = NULL, *pDst = NULL;
    
    pSrc = (UINT8 *)pSrcImg->GetImgBinaryMap(pSrcImg, IMG_BINARYMAP_SKIN) + PADDING_SIZE;
    pDst = (UINT8 *)pDstImg->imageData;
    for(j=0 ; j<nHeight ; j++)
        MEMCPY(pDst + (j * nDstStride), pSrc + (j * nSrcStride), nWidth * sizeof(UINT8));
    
    return;
}


static void _Init()
{
    glViewport(0, 0, ENC_IMG_WIDTH, ENC_IMG_HEIGHT);
    
    glGenTextures(GLTEXTURE_MAXNUM, &(_gpMainStruct->nTexture[0]));
    glBindTexture(GL_TEXTURE_2D, _gpMainStruct->nTexture[0]);
}


static void _Idle()
{
    glutPostRedisplay();
}


static void _DrawImage(MainStruct_T *pMainStruct, IplImage *pSrcImg, GLuint nTextureIdx)
{
    const int               nWidth = ENC_IMG_WIDTH;
    const int               nHeight = ENC_IMG_HEIGHT;
    
    glBindTexture(GL_TEXTURE_2D, pMainStruct->nTexture[0]);
    
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glViewport(0, 0, nWidth, nHeight);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    cvFlip(pSrcImg, NULL, 0);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, nWidth, nHeight, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, pSrcImg->imageData);
    
    glEnable(GL_TEXTURE_2D);
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    {
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 0.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f, 0.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f, 1.0f, 0.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, 0.0f);
    }
    glEnd();
    
    glutSwapBuffers();
}


static void _Keyboard(UINT8 nKey, INT32 nMouseX, INT32 nMouseY)
{
    switch(nKey)
    {
        case 27: // Press ESC to exit
            exit (0);
    }
}


static void _Reshape(INT32 nWidth, INT32 nHeight)
{
    glViewport (0, 0, nWidth, nHeight);
}


static void _DrawRect(IplImage *pDstImg, UINT32 x, UINT32 y, UINT32 nWidth, UINT32 nHeight, CvScalar nColor)
{
    CvPoint                         nTL = {(INT32)x, (INT32)y};
    CvPoint                         nBR = {(INT32)(x + nWidth), (INT32)(y + nHeight)};
    
    cvRectangle(pDstImg, nTL, nBR, nColor, 1, 8, 0);
}


void _OpenSerialPort(char *pDeviceName, int *pFp)
{
    int             fd;
    struct termios  newtio;
    speed_t         nBaudRate = B115200;
    
    fd = open(pDeviceName, O_RDWR | O_NOCTTY | O_NDELAY);
    
    if(fd < 0)
    {
        *pFp = NULL;
        return;
    }
    
    memset(&newtio, 0, sizeof(newtio));
    
    cfsetispeed(&newtio, nBaudRate);                   // Set the baud rate
    cfsetospeed(&newtio, nBaudRate);
    
    newtio.c_iflag = IGNPAR | IGNBRK;
    newtio.c_oflag = 0;
    newtio.c_cflag = CS8 | CLOCAL | CREAD;
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 0;
    newtio.c_cc[VMIN] = 0;
    
    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &newtio);
    
    *pFp = fd;
}


void _CloseSerialPort(int fd)
{
    close(fd);
}


void *_AnotherPortReceive(void *fd)
{
    int nFileDescriptor = 0;
    int nReadMsgSize = 0;
    unsigned char nDataBuf[50] = {0, };
    
    memset(nDataBuf, 0, 50 * sizeof(unsigned char));
    nFileDescriptor = (int)fd;
    
    while(1)
    {
        nReadMsgSize = read(nFileDescriptor, nDataBuf, 1);
        if(nReadMsgSize > 0)
            printf("Zigbee Module: %02X %d-Byte\n", nDataBuf[0], nReadMsgSize);
    }
}


