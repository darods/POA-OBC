#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <memory.h>
#include <pthread.h>
#include "cv.h"
#include "highgui.h"

#include <OpenGL/gl.h> // OpenGL framework on Mac (version 1.5.10.7.0)
//#include <GL/gl.h> // OpenGL library on Windows

#include <GLUT/glut.h> // GLUT framework on Mac (version 3.4.2)
//#include <GL/glut.h> // GLUT library on Windows

#define IMG_WIDTH                       (720)
#define IMG_HEIGHT                      (401)
#define ESTIMATE_ARR_SIZE               (50)
#define AXIS_X_OFFSET                   (5)
#define AXIS_X_START                    (60)
#define AXIS_Y_START                    (200)
#define AXIS_X_LEN                      (600 + AXIS_X_OFFSET)
#define AXIS_Y_LEN                      (180)


int                         nCoeffiPID[3] = {130, 3, 1500};
int                         nInitVal = 35;
int                         nTargetVal = 30;
float                       nEstimatedVal[ESTIMATE_ARR_SIZE];
IplImage                    *pImg = NULL;

void _PIDControl_Cb(int pos);
static void _Get_EstimatedVal(float *pDst, const float nTargetVal, const float nInitVal, const float nPCoeffi, const float nICoeffi, const float nDCoeffi);
static void _DrawGraph(IplImage *pImg, float *pDst);
static void _DrawAxis(IplImage *pImg);
float _Clip3Float(const float nValue, const int MIN, const int MAX);


int main(int argc, char * argv[])
{
    int                     i = 0, j = 0, k = 0;
    
    pImg = cvCreateImage(cvSize(IMG_WIDTH, IMG_HEIGHT), IPL_DEPTH_8U, 3);
    
    cvNamedWindow("PID Control Simulator", 0);
    cvResizeWindow("PID Control Simulator", IMG_WIDTH, IMG_HEIGHT);
    
    cvCreateTrackbar(" P_Coeffi", "PID Control Simulator", &(nCoeffiPID[0]), 3000, _PIDControl_Cb);
    cvCreateTrackbar(" I_Coeffi", "PID Control Simulator", &(nCoeffiPID[1]), 3000, _PIDControl_Cb);
    cvCreateTrackbar(" D_Coeffi", "PID Control Simulator", &(nCoeffiPID[2]), 3000, _PIDControl_Cb);
    cvCreateTrackbar("  InitVal", "PID Control Simulator", &nInitVal, 60, _PIDControl_Cb);
    cvCreateTrackbar("TargetVal", "PID Control Simulator", &nTargetVal, 60, _PIDControl_Cb);
    
//    for(i=0 ; i<30 ; i++)
//    {
//        //for(j=0 ; j<30 ; j++)
//        {
//            //for(k=0 ; k<30 ; k++)
//            {
//                nCoeffiPID[1] = 500 + i * 50;
//                //nCoeffiPID[1] = 500 + j * 50;
//                //nCoeffiPID[2] = 500 + k * 50;
//                
//                _PIDControl_Cb(0);
//                
//                cvWaitKey(1);
//            }
//        }
//    }

    _PIDControl_Cb(0);

    cvWaitKey(0);
    
    cvDestroyWindow("PID Control Simulator");
    
    return 0;
}


void _PIDControl_Cb(int pos)
{
    
    _Get_EstimatedVal(&(nEstimatedVal[0]), (const float)(nTargetVal - 30), (const float)(nInitVal - 30),
                      (const float)((float)nCoeffiPID[0] / 100.0f), (const float)((float)nCoeffiPID[1] / 100.0f), (const float)((float)nCoeffiPID[2] / 100.0f));
    
    _DrawAxis(pImg);
    _DrawGraph(pImg, &(nEstimatedVal[0]));
    
    cvShowImage("PID Control Simulator", pImg);
}


static void _Get_EstimatedVal(float *pDst, const float nTargetVal, const float nInitVal, const float nPCoeffi, const float nICoeffi, const float nDCoeffi)
{
    int                         i = 0;
    const float                 nLoopDuration = 1.0f;
    float                       nCurrVal = nInitVal;
    float                       nPrevErr = 0.0f;
    float                       nCurrErr = 0.0f;
    float                       nCurrErrRate = 0.0f;
    float                       nPrevErrRate = 0.0f;
    float                       nEstimated[3] = {0, 0, 0};
    
    memset(pDst, 0, ESTIMATE_ARR_SIZE * sizeof(float));

    printf("%03.05f     %03.05f     %03.05f     %03.05f     %03.05f         \n", nTargetVal, nInitVal, nPCoeffi, nICoeffi, nDCoeffi);
    
    pDst[0] = nCurrVal;
    for(i=0 ; i<ESTIMATE_ARR_SIZE ; i++)
    {
        nCurrErr = nTargetVal - nCurrVal;
        nCurrErrRate =  nCurrVal - nTargetVal;
        nEstimated[0] = 0;//nCurrErrRate * nPCoeffi;
        nEstimated[1] += nCurrErrRate * nLoopDuration * nICoeffi;
        nEstimated[1] = 0;//_Clip3Float(nEstimated[1], -100, 100);
        nEstimated[2] = (nCurrErrRate - nPrevErrRate) / nLoopDuration * nDCoeffi;
        pDst[i] = nCurrVal - (nEstimated[0] + nEstimated[1] + nEstimated[2]);
        
        nCurrVal = pDst[i];
        nPrevErrRate = nCurrErrRate;
    }
}


static void _DrawGraph(IplImage *pImg, float *pDst)
{
    int                         i = 0;
    CvScalar                    nScalar = {255, 255, 0, 0};
    int                         nScale = 10;
    CvPoint                     nStartPos = {AXIS_X_START, AXIS_Y_START};
    CvPoint                     nEndPos = {0, 0};
    
    for(i=0 ; i<(ESTIMATE_ARR_SIZE-1) ; i++)
    {
        nStartPos.x = AXIS_X_START + (i * AXIS_X_OFFSET);
        nStartPos.y = AXIS_Y_START + (pDst[i] * nScale);
        nEndPos.x = nStartPos.x + AXIS_X_OFFSET;
        nEndPos.y =  AXIS_Y_START + (pDst[i+1] * nScale);
        
        cvLine(pImg, nStartPos, nEndPos, nScalar, 1, 8, 0);
    }
}


static void _DrawAxis(IplImage *pImg)
{
    CvScalar                nScalar = {255, 255, 255, 255};
    CvPoint                 nStartPos = {0, 0};
    CvPoint                 nEndPos = {0, 0};
    
    memset(pImg->imageData, 0, pImg->imageSize);
    
    // Draw X-Axis
    nStartPos.x = AXIS_X_START;
    nStartPos.y = AXIS_Y_START;
    nEndPos.x = AXIS_X_START+AXIS_X_LEN;
    nEndPos.y = AXIS_Y_START;
    cvLine(pImg, nStartPos, nEndPos, nScalar, 1, 8, 0);

    // Draw Y-Axis
    nStartPos.x = AXIS_X_START + AXIS_X_OFFSET;
    nStartPos.y = AXIS_Y_START;
    nEndPos.x = AXIS_X_START + AXIS_X_OFFSET;
    nEndPos.y = AXIS_Y_START - AXIS_Y_LEN;
    cvLine(pImg, nStartPos, nEndPos, nScalar, 1, 8, 0);
    nEndPos.x = AXIS_X_START + AXIS_X_OFFSET;
    nEndPos.y = AXIS_Y_START + AXIS_Y_LEN;
    cvLine(pImg, nStartPos, nEndPos, nScalar, 1, 8, 0);
}


float _Clip3Float(const float nValue, const int MIN, const int MAX)
{
    float               nClipVal = nValue;
    
    if(nValue < MIN)
        nClipVal = MIN;
    else if(nValue > MAX)
        nClipVal = MAX;
    
    return nClipVal;
}






