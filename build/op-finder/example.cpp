
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "suace.h"
using namespace cv;
using namespace std;

int a = 21;
int b = 36;
int intensityMax = 255;

int main()
{
	namedWindow("SUACE", 1);
	createTrackbar("distance", "SUACE", &a, intensityMax);
	createTrackbar("sigma", "SUACE", &b, intensityMax);
	char filename[100];
	Mat suaceResult;
	Mat frame;

	while (true)
	{
		frame = imread("C:\\path\\to\\your\\image.png", CV_LOAD_IMAGE_GRAYSCALE);
		performSUACE(frame, suaceResult, a, (b + 1) / 8.0);
		imshow("SUACE", suaceResult);
		imshow("Original", frame);
		int response = waitKey(0);
		if (response == 32)
			break;
	}
	return 0;
}
