/****************************************************************************************************
										Fish Eye Effect Video
										[ CPP and OpenCV ]
*****************************************************************************************************/

/**	Header Files	*/
#include <iostream>
#include <opencv2/opencv.hpp>
#include <string.h>
#include <conio.h>

using namespace std;
using namespace cv;

/**	Function to calculate shift*/
float calc_shift(float x1, float x2, float cx, float k)
{
	float thresh = 1;
	float x3 = x1 + (x2 - x1)*0.5;
	float res1 = x1 + ((x1 - cx)*k*((x1 - cx)*(x1 - cx)));
	float res3 = x3 + ((x3 - cx)*k*((x3 - cx)*(x3 - cx)));

	if (res1>-thresh && res1 < thresh)
		return x1;
	if (res3<0)
	{
		return calc_shift(x3, x2, cx, k);
	}
	else
	{
		return calc_shift(x1, x3, cx, k);
	}
}

float getRadialX(float x, float y, float cx, float cy, float k, bool scale, Vec4f props)
{
	float result;
	if (scale)
	{
		float xshift = props[0];
		float yshift = props[1];
		float xscale = props[2];
		float yscale = props[3];

		x = (x*xscale + xshift);
		y = (y*yscale + yshift);
		result = x + ((x - cx)*k*((x - cx)*(x - cx) + (y - cy)*(y - cy)));
	}
	else
	{
		result = x + ((x - cx)*k*((x - cx)*(x - cx) + (y - cy)*(y - cy)));
	}
	return result;
}

float getRadialY(float x, float y, float cx, float cy, float k, bool scale, Vec4f props)
{
	float result;
	if (scale)
	{
		float xshift = props[0];
		float yshift = props[1];
		float xscale = props[2];
		float yscale = props[3];

		x = (x*xscale + xshift);
		y = (y*yscale + yshift);
		result = y + ((y - cy)*k*((x - cx)*(x - cx) + (y - cy)*(y - cy)));
	}
	else
	{
		result = y + ((y - cy)*k*((x - cx)*(x - cx) + (y - cy)*(y - cy)));
	}
	return result;
}

//	Fish Eye Function							Cx, Cy Center of x & y
//	_src : Input image, _dst : Output image, Cx,Cy coordinates from where the distorted image will have as initial point, k : distortion factor
void fishEye(InputArray _src, OutputArray _dst, double Cx, double Cy, double k, bool scale = true)
{
	Mat src = _src.getMat();
	Mat mapx = Mat(src.size(), CV_32FC1);
	Mat mapy = Mat(src.size(), CV_32FC1);

	int w = src.cols;	//	Width
	int h = src.rows;	//	Height

	Vec4f props;
	//Calculating x and y shifts to be applied
	float xShift = calc_shift(0, Cx - 1, Cx, k);
	props[0] = xShift;
	float newCenterX = w - Cx;
	float xShift2 = calc_shift(0, newCenterX - 1, newCenterX, k);

	float yShift = calc_shift(0, Cy - 1, Cy, k);
	props[1] = yShift;
	float newCenterY = w - Cy;
	float yShift2 = calc_shift(0, newCenterY - 1, newCenterY, k);

	//	Calculating the scale factor from the x & y shifts accordingly
	float xScale = (w - xShift - xShift2) / w;
	props[2] = xScale;
	float yScale = (h - yShift - yShift2) / h;
	props[3] = yScale;

	float* p = mapx.ptr<float>(0);

	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			*p++ = getRadialX((float)x, (float)y, Cx, Cy, k, scale, props);
		}
	}

	p = mapy.ptr<float>(0);

	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			*p++ = getRadialY((float)x, (float)y, Cx, Cy, k, scale, props);
		}
	}

	remap(src, _dst, mapx, mapy, CV_INTER_LINEAR, BORDER_CONSTANT);

}

/**	Main Function*/
int main()
{
	VideoCapture capWebcam(0);            // declare a VideoCapture object and associate to webcam, 0 => use 1st webcam
	char charCheckForEscKey = 0;
	if (capWebcam.isOpened() == false)                                 // check if VideoCapture object was associated to webcam successfully
	{
		std::cout << "error: capWebcam not accessed successfully\n\n";      // if not, print error message to std out
		_getch();                                                           // may have to modify this line if not using Windows
		return(0);                                                          // and exit program
	}
	Mat input_image;	
	Mat output_image;
	//	Resizing video to 640x640
	capWebcam.set(CV_CAP_PROP_FRAME_WIDTH, 640);
	capWebcam.set(CV_CAP_PROP_FRAME_HEIGHT, 640);
	capWebcam.read(input_image);	//	Read the image
	
	while (charCheckForEscKey != 27 && capWebcam.isOpened())             // until the Esc key is pressed or webcam connection is lost
	{
		bool blnFrameReadSuccessfully = capWebcam.read(input_image);            // get next frame
		resize(input_image, input_image, Size(640, 640), 0, 0, 3);

		if (!blnFrameReadSuccessfully || input_image.empty())                  // if frame not read successfully
		{
			std::cout << "error: frame not read from webcam\n";                 // print error message to std out
			break;                                                              // and jump out of while loop
		}

		imshow("Input Image", input_image);

		fishEye(input_image, output_image, input_image.cols / 2, input_image.rows / 2, 0.0001);	// Fish Eye Function

		imshow("Output Image", output_image);

		charCheckForEscKey = cv::waitKey(1);        // delay (in ms) and get key press, if any
	}

	return 0;
}