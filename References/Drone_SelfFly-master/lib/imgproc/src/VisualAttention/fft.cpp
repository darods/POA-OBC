#include "Gabor.h"

extern COMPLEX  *TmpComplexBuf_COL;
extern COMPLEX  *TmpComplexBuf_ROW;
extern unsigned char      *pFFTView;


void FFT_2D(COMPLEX *complex_data, unsigned char *inData, unsigned char *outData,  int height, int width)
{
    int index=0,i=0,j=0,k=0, N;
    COMPLEX *col_data = TmpComplexBuf_COL;
    COMPLEX *row_data = TmpComplexBuf_ROW;
    N=0;

    for(i=0; i<height; i++){
        col_data[i].real=0;
        col_data[i].imagin=0;
    }
    for(i=0; i<width; i++){
        row_data[i].real=0;
        row_data[i].imagin=0;
    }
    for(i=0; i<height; i++){
        for(j=0; j<width; j++){
        // 실제 영상의 값 대입
            complex_data[i*width+j].real=inData[i*width+j];
            complex_data[i*width+j].imagin=0;  // 허수부는 없다...
        }
    }
    k=0;
    // 
    for(int y=0; y<height; y++){
        // 행부터 FFT 연산 수행...
        index=y*width;
        for(int x=0; x<width; x++)
        {
            row_data[x].real = complex_data[index].real;
            row_data[x].imagin = complex_data[index++].imagin;
        }
        // 함수 호출...
        // N 은 stage 수가 된다.
        N=(int)(log((double)width)/log((double)2));
        fft(row_data,N, width,1);  // 계산할 포인트는 width
        
        index=y*width;
        for(int x=0; x<width; x++)
        {    // 계산 결과 저장.... 
            complex_data[index].real = row_data[x].real;
            complex_data[index++].imagin = row_data[x].imagin;
        }
    }
                                                                  
    //////////////////////////////////////////////////////////////  
    //              .열방향으로 FFT 연산 수행.                      //  
    //////////////////////////////////////////////////////////////  

    for(int x=0; x<width; x++){
        index=x;
        for(int y=0; y<height; y++)
        {
            // data 저장한다..
            col_data[y].real = complex_data[index].real;
            col_data[y].imagin = complex_data[index].imagin;
            index += width;
        }
        // 함수 호출..
        N=(int)(log((double)height)/log((double)2));
        fft(col_data,N, height,1);

        index=x;
        for(int y=0; y<height; y++)
        {
            complex_data[index].real = col_data[y].real;
            complex_data[index].imagin = col_data[y].imagin;
            index += width;
        }
    }
    
    //  결과 저장...
    k=0;
    for(i=0; i<height; i++){
        for(j=0; j<width; j++){
            // scaling....
            outData[i*width+j]=(unsigned char)(18*log(1+abs(complex_data[k+j].real)));
            if(outData[i*width+j]>=190)outData[i*width+j]=255;
            else 
            {
                outData[i*width+j]=10;
            }
        }
        k+=width;
    }
    fftview(outData, height, width);
}

// numpoint는.... 계산할 포인트 수..
void fft(COMPLEX *f, int logN, int numpoints, int dir)
{
    scramble(numpoints,f);
    butterflies(numpoints,logN,dir,f);
}

void scramble(int numpoints, COMPLEX *f)
{
    int i,j,m;    
    double temp;    

    j=0;
    for(i=0;i<numpoints;i++)    
    {
        if(i>j)
        {
            //swap real
            temp=f[j].real;
            f[j].real=f[i].real;
            f[i].real=(float)temp;
            //swap imagenary
            temp=f[j].imagin;
            f[j].imagin=f[i].imagin;
            f[i].imagin=(float)temp;
        }
        m=numpoints>>1;
        while(( j >= m ) & ( m >= 2 ))
        {
            j -=m;
            m = m >> 1;
        }
        j +=m;
    }    
}

void butterflies(int numpoints, int logN, int dir, COMPLEX *f)
{
    double angle;        //polar angle
    COMPLEX w,wp,temp;    //intermediate complex numbers
    int i,j,k,offset;    //loop variables
    int N,half_N;        //storage for powers of 2
    double wtemp;        //temporary storage for w.re

    N=1;
    for(k=0; k<logN; k++)
    {
        half_N=N;
        N <<= 1;
        angle = -2.0 * PI / N * dir;
        wtemp = sin(0.5*angle);
        //wp.real = (float)sin(0.5*angle);//변경.
        wp.real = (float)(-2.0*wtemp*wtemp);
        
        wp.imagin = (float)sin(angle);//변경
        w.real = 1.0;
        w.imagin = 0.0;
        for(offset=0; offset<half_N; offset++)
        {
            for(i=offset; i<numpoints; i+=N)
            {
                j=i+half_N;
                temp.real = (w.real*f[j].real)-(w.imagin*f[j].imagin);
                temp.imagin = (w.imagin*f[j].real)+(w.real*f[j].imagin);
                f[j].real = f[i].real-temp.real;
                f[i].real += temp.real;
                f[j].imagin = f[i].imagin-temp.imagin;
                f[i].imagin += temp.imagin;
            }
            wtemp = w.real;
            w.real = (float)(wtemp*wp.real-w.imagin*wp.imagin+w.real);//변경.
            w.imagin = (float)(w.imagin*wp.real+wtemp*wp.imagin+w.imagin);//변경.
        }
    }
    if(dir == -1)
        for(i=0;i<numpoints;i++)
        {
            f[i].real /= numpoints;
            f[i].imagin /= numpoints;
        }
}

void fftview(unsigned char *outData, int height, int width)
{
    unsigned char *fftview = pFFTView;
    
    //convert
    for(int i=0; i<height; i+=height/2){
        for(int j=0; j<width; j+=width/2){
            for(int row=0; row<height/2; row++){
                for(int col=0; col<width/2; col++){
                    fftview[((height/2-1)-row+i)*width+(width/2-1)-col+j]=outData[(i+row)*width+j+col];
                }
            }
        }
    }
    for(int y=0; y<height; y++){
        for(int x=0; x<width; x++){
            outData[y*width+x]=fftview[y*width+x];
        }
    }
}

void IFFT(COMPLEX *complex_data, unsigned char *outData,  int height, int width)
{
    int index=0,i=0,j=0,k=0, N;
    COMPLEX *col_data = TmpComplexBuf_COL; 
       COMPLEX *row_data = TmpComplexBuf_ROW;
    
    for(i=0; i<height; i++){
        col_data[i].real=0;
        col_data[i].imagin=0;
    }
    for(i=0; i<width; i++){
        row_data[i].real=0;
        row_data[i].imagin=0;
    }
    
    for(int y=0; y<height; y++)
    {
        index=y*width;
        for(int x=0; x<width; x++)
        {
            row_data[x].real = complex_data[index].real;
            row_data[x].imagin = complex_data[index++].imagin;
        }
        // row inverse transform
        N=(int)(log((double)width)/log((double)2));
        fft(row_data,N, width,-1);
        
        index=y*width;
        for(int x=0; x<width; x++)
        {
            complex_data[index].real = row_data[x].real;
            complex_data[index++].imagin = row_data[x].imagin;
        }
    }

    for(int x=0; x<width; x++)
    {
        index=x;
        for(int y=0; y<height; y++)
        {
            col_data[y].real = complex_data[index].real;
            col_data[y].imagin = complex_data[index].imagin;
            index += width;
        }
        N=(int)(log((double)height)/log((double)2));
        fft(col_data,N,height,-1);  // 열 변환...한다.

        index=x;
        for(int y=0; y<height; y++)
        {
            complex_data[index].real = col_data[y].real;
            complex_data[index].imagin = col_data[y].imagin;
            index += width;
        }
    }
    
    for(i=0; i<height; i++)
    {
        for(j=0; j<width; j++)
        {
            if(complex_data[k+j].real>=255)
                outData[i*width+j]=255;
            else if(complex_data[k+j].real<=0)
                outData[i*width+j]=0;
            else
                outData[i*width+j]=(unsigned char)(complex_data[k+j].real);
        }
        k+=width;
    }
}

