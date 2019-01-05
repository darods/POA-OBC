#include "Gabor.h"


extern unsigned char    *pLPF0;
extern unsigned char    *pLPF1;

void GaborF(COMPLEX *complex_data, int h, int w, double orientation, double frequency, double distance)
{
    unsigned long x, y, height, width;
    unsigned long tempx, tempy, midheight, midwidth;     
    unsigned long index, max;
    unsigned char *lpf = pLPF0;
       unsigned char *lpf_2 = pLPF1;
    double gaus, D;
    unsigned int check_x, check_y;
    double dv, du;
    double dx, dy;
    double freq;
    double U, V, u, v, uu, vv;
    
    
    U = distance*cos(orientation) ;//2.0*pi*cos(orientation)*freq;
    V = distance*sin(orientation);//2.0*pi*sin(orientation)*freq;
    u=v=uu=vv=0.0;
    check_x=check_y=0;
    gaus=D=0.0;
//    DO=30;
    height=(unsigned long)h;
    width=(unsigned long)w;

    dx = frequency;
    dy = frequency;
//    u0 = sqrt(2.0);
    du=0.0;
    dv=0.0;
    freq = 15.0;
    

    midheight = height / 2;
    midwidth = width / 2;

    for(unsigned int i=0; i<height*width; i++)
    {
        lpf[i]=0;
        lpf_2[i]=0;
    }
    
    
    max = 0;
    // 좌표 보정..
    for(y=0; y<height; y++)
    {
        if(y >= midheight)
            tempy = y-midheight;   // 반대쪽으로..
        else
            tempy = y+midheight;



        for(x=0; x<width; x++)
        {
            if(x >= midwidth)
                tempx = x-midwidth;
            else
                tempx = x+midwidth;
            index = y*width+x;            
            // gaussian lowpass filter
            // 원점으로부터의 거리..
            //u_pi = (double)((double)tempx-(double)midwidth)*cos(orientation)+((double)tempy-(double)midheight)*sin(orientation);
            //v_pi = -(double)((double)tempx-(double)midwidth)*sin(orientation)+((double)tempy-(double)midheight)*cos(orientation);
        //    printf("cos(0.1) is %f   sin(or) = %f\n  v_pi = %f", cos(orientation), sin(orientation), (tempx-midwidth)*cos(orientation));
    
            //u0 = 2.0*pi*cos(orientation)/freq;
            //v0 = 2.0*pi*sin(orientation)/freq;
            
            du = 1.0/(2.0*pi*dx);
            dv = 1.0/(2.0*pi*dy);

            u = (double)tempx - (double)midwidth;
            v = (double)tempy - (double)midheight;
            //uu = (u - U)*cos(orientation) + (v - V)*sin(orientation);
            //vv = -(u - U)*sin(orientation) + (v - V)*cos(orientation);
            

            //uu_n = (u + U)*cos(orientation) + (v + V)*sin(orientation);
            //vv_n = -(u + U)*sin(orientation) + (v + V)*cos(orientation);


            //D = 2.0*pi*dx*dy*exp(-0.5*(dx*dx*uu*uu/(double)midwidth + dy*dy*vv*vv/(double)midheight))
            //    +2.0*pi*dx*dy*exp(-0.5*(dx*dx*uu_n*uu_n/(double)midwidth + dy*dy*vv_n*vv_n/(double)midheight));

            D = 2.0*pi*exp(-2*(dx*dx*(u-U)*(u-U) + dy*dy*(v-V)*(v-V)));


        //    D = 2.0*pi*dx*dy*(exp(-0.5*((u_pi-u0)*(u_pi-u0)/(du*du)+(v_pi-v0)*(v_pi-v0)/(dv*dv))) 
        //        + exp(-0.5*((u_pi+u0)*(u_pi+u0)/(du*du)+(v_pi+v0)*(v_pi+v0)/(dv*dv))));
            gaus = (unsigned long)D;
            if (max<D) max = (unsigned long)D;
            // filter값과 영상의 주파수 특성을 곱하면..
            // 계산 끝...
    
            if(gaus>0)
            {
                lpf[y*width+x]=(unsigned char)gaus;
            }
            else if (gaus>255)
                lpf[y*width+x]=255;
            else
            {
                lpf[y*width+x]=0;
            }
            complex_data[index].real *= gaus;
            complex_data[index].imagin *= gaus;
            //complex_data[index].imagin *= gaus;
        }
    }

    //unsigned char *_temp;
    //_temp = (unsigned char*)malloc(height*width*sizeof(unsigned char));
    for(unsigned int i=0; i<height; i++)
        for(unsigned int j=0; j<width; j++)
        {
            //complex_data[j*width+i].real = (complex_data[j*width+i].real*255)/max;
            if (max == 0) lpf[i*width+j] = 0;
            else lpf[i*width+j] = (unsigned char)(lpf[i*width+j]*255/max);
            //_temp[j*width+i]=(unsigned char) (complex_data[j*width+i].real);
        }


    //write_tiff_image("GaborF.tiff", lpf, height, width);

    // 가운데로 정렬 시킨 gabor filter 모양
    for(unsigned int i=0; i<height; i++)
    {
        for(unsigned int j=0; j<width; j++)
        {
            check_x = j+width/2;
            check_y = i+height/2;
            if((check_y)>=height)
            {
                check_y-=height;
            }
            if((check_x)>=width)
            {
                check_x-=width;
            }
            lpf_2[check_y*width+check_x]=lpf[i*width+j];
            
            if(lpf_2[check_y*width+check_x]>255)
            {
                lpf[check_y*width+check_x]=255;
            }
            else if(lpf[check_y*width+check_x]<=0)
            {
                lpf[check_y*width+check_x]=0;
            }
            else 
            {;
            }
        }
    }
    //write_tiff_image("Gabor_2.tiff", lpf_2, height, width);
}
