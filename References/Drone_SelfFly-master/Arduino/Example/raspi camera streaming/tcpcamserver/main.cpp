
#include <iostream>
#include "../opencvcom/streamingsender.h"

using namespace std;
using namespace cv;

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		cout << "commandline <bind port>" << endl;
		return 0;
	}

	const int bindPort = atoi(argv[1]);

	cout << "streaming server init... " << endl;
	cvproc::cStreamingSender sender;
	if (!sender.Init(bindPort, true, true))
	{
		cout << "bind error !!" << endl;
		return 0;
	}

	cout << "bind success.. " << endl;

	cout << "check camera...";
	CvCapture *capture = cvCaptureFromCAM(0);
	if (capture == NULL)
	{
		cout << " error" << endl;
		perror("OpenCV : open WebCam\n");
		return -1;
	}

	cout << "success" << endl;

	long oldT = GetTickCount();
	int fps = 0;
	while (1)
	{
		++fps;

		long curT = GetTickCount();
		if (curT - oldT > 1000)
		{
			//printf("fps = %d\n", fps);
			oldT = curT;
			fps = 0;
		}

		sender.CheckPacket();

		cvGrabFrame(capture);
		IplImage *frame = cvRetrieveFrame(capture);
		sender.Send(Mat(frame));
		cvWaitKey(1);
	}

	return 0;
}
