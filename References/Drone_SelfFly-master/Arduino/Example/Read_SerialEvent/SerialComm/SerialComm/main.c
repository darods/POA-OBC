//
//  main.c
//  SerialComm
//
//  Created by Maverick on 2016. 7. 5..
//  Copyright © 2016년 Minhohihi. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <pthread.h>


int OpenSerialPort(char *pDeviceName);
int CloseSerialPort(int fd);
void *AnotherPortReceive(void *fd);

int main()
{
    int nSerialPort0 = 0, nSerialPort1 = 0;
    int nReadMsgSize = 0;
    int nRunOtherSerial = 0;
    int nAngle = 10;
    int nSignFlag = 0;
    unsigned char nDataBuf[50] = {0, };
    
    pthread_t   nThread;
    
    nSerialPort0 = OpenSerialPort("/dev/cu.wchusbserial1410");
    //nSerialPort1 = OpenSerialPort("/dev/tty.wchusbserial1410");
    
    memset(nDataBuf, 0, 50 * sizeof(unsigned char));
    
    //nRunOtherSerial = pthread_create(&nThread, NULL, AnotherPortReceive, (void *)nSerialPort1);
    
    while(1)
    {
        //nReadMsgSize = read(nSerialPort0, nDataBuf, 2);
        //if(nReadMsgSize > 0)
        {
            printf("WiFi Module: %02x %02x %d-Byte  Angle:%d\n", nDataBuf[0], nDataBuf[1], nReadMsgSize, nAngle);
        
            if((nAngle < 10) || (nAngle > 170))
                nSignFlag = !nSignFlag;
            
            nDataBuf[0] = nAngle;
            write(nSerialPort0, nDataBuf, 1);
            write(nSerialPort0, nDataBuf, 1);
            write(nSerialPort0, "\n", 1);
            //write(nSerialPort0, "\n", 1);
            
            nAngle = nAngle + (0 == nSignFlag ? 1 : -1);
        }
        
        usleep(20000);
    }
    
    CloseSerialPort(nSerialPort0);
    //CloseSerialPort(nSerialPort1);
}


int OpenSerialPort(char *pDeviceName)
{
    int             fd;
    struct termios  newtio;
    speed_t         nBaudRate = B115200;
    
    fd = open(pDeviceName, O_RDWR | O_NOCTTY | O_NDELAY);
    
    if(fd < 0)
        return -1;
    
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
    
    return fd;
}


int CloseSerialPort(int fd)
{
    close(fd);
    
    return 0;
}


void *AnotherPortReceive(void *fd)
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


