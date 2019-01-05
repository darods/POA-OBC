#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#define pi 3.1415926543


void Orientation (unsigned char *inData, unsigned char *out_orient, int height, int width)
{
    //unsigned char seg_sum=0;
    int i, j, m=0, min=100000, max=0;
    int *gradient_x_8,  gauss_size = 15, *gradient_y_8;
    double duration, *orientation, *theta;
    double sum=0;
    clock_t start, finish;

    
    gradient_x_8 = (int*)malloc(sizeof(int)*width*height);
    gradient_y_8 = (int*)malloc(sizeof(int)*width*height);
    orientation = (double*)malloc(sizeof(double)*width*height);
    theta = (double*)malloc(sizeof(double)*width*height);
    
    /* 각 픽셀에서의 gradient를 계산 */
    for (j=0; j<height; j++)
    {
        for (i=0; i<width; i++)                                                                                                                                                                                        
        {
            orientation[j*width+i] = 0;
            if (i==0 || i==width-1 || j==0 || j==height-1) {
                gradient_x_8[j*width+i] = 0;
                gradient_y_8[j*width+i] = 0;
            }
            else {
                gradient_x_8[j*width+i] = //(-1)*inData[(j-1)*width+i-1] + inData[(j-1)*width+i+1]+
                                        - inData[j*width+i-1]  + inData[j*width+i+1];
                                        //(-1)*inData[(j+1)*width+i-1] + inData[(j+1)*width+i+1];
                //gradient_x_8[j*width+i] = (-1)*inData[(j-1)*width+i-1] + (-2)*inData[(j-1)*width+i] + (-1)*inData[(j-1)*width+i+1]+
                                        //inData[(j+1)*width+i-1] + 2*inData[(j+1)*width+i] + inData[(j+1)*width+i+1];
                gradient_y_8[j*width+i] = -inData[(j-1)*width+i] + inData[(j+1)*width+i] ;
                //r_k[j*width+i] = sqrt( gradient_x_8[j*width+i]*gradient_x_8[j*width+i]
                     //+ gradient_y_8[j*width+i]*gradient_y_8[j*width+i]);
            }
        }
    }
    for (j=0; j<height; j++)
    {
        for (i=0; i<width; i++)
        {
            if (i==0 || i==width-1 || j==0 || j==height-1 || (gradient_y_8[j*width+ i]==0 && gradient_x_8[j*width+ i]==0)) {
                theta[j*width + i]=-10;
            }
            else if (abs(gradient_y_8[j*width+ i]) + abs(gradient_x_8[j*width+ i]) > 40){
                theta[j*width+ i] = atan2((double)(gradient_y_8[j*width+ i]) ,(double)(gradient_x_8[j*width+ i])) ;
                if (theta[j*width+ i] > pi/2)
                    theta[j*width+ i] -= pi;
                else if (theta[j*width+ i] <-pi/2)
                    theta[j*width+ i] += pi;
            }
            else theta[j*width+i]=-10;
        }
    }

    
    int SIZE_MASK = gauss_size, l,orient[8],t, count, angle, mean;
    double dev;
    start = clock();
    for (j=SIZE_MASK; j<height-SIZE_MASK; j+=SIZE_MASK)
    {
        for (i=SIZE_MASK; i<width-SIZE_MASK; i+=SIZE_MASK)
        {
            max = 0;
            for (t=0; t<8; t++) orient[t]=0; count=0;dev=0;
            for (m=-SIZE_MASK; m<=SIZE_MASK; m++)
            {
                for (l=-SIZE_MASK; l<=SIZE_MASK; l++)
                {
                    if (theta[(j+m)*width+i+l]>-10) {
                        angle = (int)((theta[(j+m)*width+i+l]+pi/2)*8/pi);
                        orient[angle]++;
                        count++;
                    }
                }
            }
            //for (t=0; t<8; t++) 
            //    if (orient[t]>max) max = orient[t];
            if (count>(2*SIZE_MASK+1)*(2*SIZE_MASK+1)*0.3)
            {
                //mean = (count-max)/7;
                
                mean = count / 8;
                for (t=0; t<8; t++)
                {
                    dev += (orient[t] - mean)*(orient[t] - mean);
                }
                
                if (mean==0) {
                    for (m=j; m<j+SIZE_MASK; m++)
                        for (l=i; l<i+SIZE_MASK; l++)
                            orientation[m*width+l] = 0;
                }
                else {
                    for (m=j; m<j+SIZE_MASK; m++)
                        for (l=i; l<i+SIZE_MASK; l++)
                            orientation[m*width+l] = dev/mean;
                    
                }
                
            }
            else {
                for (m=j; m<j+SIZE_MASK; m++)
                        for (l=i; l<i+SIZE_MASK; l++)
                            orientation[m*width+l] = 0;
            }

        }
    }

    finish = clock();
    duration = (double)(finish - start) / CLOCKS_PER_SEC;

    printf( "\n main function time = %2.1f seconds\n", duration );

    // nomalization
    
    for (j=0; j<height; j++) 
        for (i=0; i<width; i++)
        {
            if (max<orientation[j*width+i]) 
                max=(int)orientation[j*width+i];
            if (min>orientation[j*width+i] && orientation[j*width+i]>0) 
                min=(int)orientation[j*width+i];
        }
    for (j=0; j<height; j++) {
        for (i=0; i<width; i++) {
            if (orientation[j*width+i]==0)
                out_orient[j*width+i] =0;
            else 
                out_orient[j*width+i] = (int)(255-((double)(orientation[j*width+i]-min)/(double)(max-min))*255);
        }
    }

    free(gradient_x_8);
    free(gradient_y_8);
    free(orientation);
}
