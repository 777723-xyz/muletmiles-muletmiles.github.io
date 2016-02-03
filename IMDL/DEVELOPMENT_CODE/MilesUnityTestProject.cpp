// MilesUnityTestProject.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

//Notes on project properties
//When I built opencv I didn't build it with the include header files apparently
//So for the Additional include directories, I pointed it at header files that
//come with opencv when you download it.
//I set an environmental variable for it called OPENCV_INCLUDE_DIR
// which on my computer is C:\opencv\build\include

//Then for the files you link to I built opencv from source, because that is what
//you are doing. I built it at C:\openCVCustom which is accessed by OPENCV_BUILD_DIR

//I point the linker at OPENCV_BUILDDIR\lib\$(Configuration), the configuration being
//Debug or Release because you have to build OPENCV twice, once as DEBUG and once as release.
//Visual Studio will automatically fill in the right term

//Also important is adding the shared libs to your computer's Path
//I added %OPENCV_BUILDDIR%\bin\Debug and %OPENCV_BUILDDIR%\bin\Release
//to my computer's path

//Last but not least, if you change any of the environmental variables, including Path
//, you have to restart Visual Studio for it to take effect. Just because

//Also make sure to build as 64 or win32 as needed

//Important, declaring this will allow you to export
//functions, classes, and variables from the dll
#define DllExport   __declspec( dllexport )

#include <sstream>
#include <string.h>
#include <iostream>
#include <opencv2/highgui.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/opencv.hpp>
#include "fruit.h"
#include <vector>

using namespace std;
using namespace cv;

//initial min and max HSV filter values.
//these will be changed using trackbars
int H_MIN = 153;//0;
int H_MAX = 164;//256;
int S_MIN = 0;
int S_MAX = 256;
int V_MIN = 0;
int V_MAX = 256;
//default capture width and height
const int FRAME_WIDTH = 640;
const int FRAME_HEIGHT = 480;
//max number of objects to be detected in frame
const int MAX_NUM_OBJECTS = 50;
//minimum and maximum object area
const int MIN_OBJECT_AREA = 40 * 40;
const int MAX_OBJECT_AREA = FRAME_HEIGHT*FRAME_WIDTH / 1.5;
//names that will appear at the top of each window
const string windowName = "Original Image";
const string windowName1 = "HSV Image";
const string windowName2 = "Thresholded Image";
const string windowName3 = "After Morphological Operations";
const string trackbarWindowName = "Trackbars";

//These are used to send back to C Sharp
int OUTPUT_XPos = 0;
int OUTPUT_YPos = 0;
int OUTPUT_Area = 0;
int OUTPUT_detected = 0;

string intToString(int number) {


	std::stringstream ss;
	ss << number;
	return ss.str();
}

void drawObject(vector<fruit> theFruits, Mat &frame) {

	for (int i = 0; i < theFruits.size(); i++) {

		fruit theFruit = theFruits[i];

		cv::circle(frame, cv::Point(theFruit.GetXPos(), theFruit.GetYPos()), 10, cv::Scalar(0, 0, 255));
		cv::putText(frame, intToString(theFruit.GetXPos()) + " , " + intToString(theFruit.GetYPos()), cv::Point(theFruit.GetXPos(), theFruit.GetYPos() + 20), 1, 1, Scalar(0, 255, 0));
		cv::putText(frame, intToString(theFruit.GetArea()), cv::Point(theFruit.GetXPos(), theFruit.GetYPos() + 50), 1, 1, Scalar(0, 255, 0));

		OUTPUT_XPos = theFruit.GetXPos();
		OUTPUT_YPos = theFruit.GetYPos();
		OUTPUT_Area = theFruit.GetArea();

	}

}

void on_trackbar(int, void*)
{//This function gets called whenever a
 // trackbar position is changed





}

void createTrackbars() {
	//create window for trackbars


	namedWindow(trackbarWindowName, 0);
	//create memory to store trackbar name on window
	char TrackbarName[50];
	sprintf_s(TrackbarName, "H_MIN", H_MIN);
	sprintf_s(TrackbarName, "H_MAX", H_MAX);
	sprintf_s(TrackbarName, "S_MIN", S_MIN);
	sprintf_s(TrackbarName, "S_MAX", S_MAX);
	sprintf_s(TrackbarName, "V_MIN", V_MIN);
	sprintf_s(TrackbarName, "V_MAX", V_MAX);
	//create trackbars and insert them into window
	//3 parameters are: the address of the variable that is changing when the trackbar is moved(eg.H_LOW),
	//the max value the trackbar can move (eg. H_HIGH), 
	//and the function that is called whenever the trackbar is moved(eg. on_trackbar)
	//                                  ---->    ---->     ---->      
	createTrackbar("H_MIN", trackbarWindowName, &H_MIN, H_MAX, on_trackbar);
	createTrackbar("H_MAX", trackbarWindowName, &H_MAX, H_MAX, on_trackbar);
	createTrackbar("S_MIN", trackbarWindowName, &S_MIN, S_MAX, on_trackbar);
	createTrackbar("S_MAX", trackbarWindowName, &S_MAX, S_MAX, on_trackbar);
	createTrackbar("V_MIN", trackbarWindowName, &V_MIN, V_MAX, on_trackbar);
	createTrackbar("V_MAX", trackbarWindowName, &V_MAX, V_MAX, on_trackbar);


}

void morphOps(Mat &thresh) {

	//create structuring element that will be used to "dilate" and "erode" image.
	//the element chosen here is a 3px by 3px rectangle

	Mat erodeElement = getStructuringElement(MORPH_RECT, Size(3, 3));
	//dilate with larger element so make sure object is nicely visible
	Mat dilateElement = getStructuringElement(MORPH_RECT, Size(8, 8));

	erode(thresh, thresh, erodeElement);
	erode(thresh, thresh, erodeElement);


	dilate(thresh, thresh, dilateElement);
	dilate(thresh, thresh, dilateElement);



}

void trackFilteredObject(Mat threshold, Mat HSV, Mat &cameraFeed) {

	int x, y;
	//putText(cameraFeed, "Got to the place", Point(0, 50), 1, 2, Scalar(0, 0, 255), 2);

	vector<fruit> apples;

	Mat temp;
	threshold.copyTo(temp);
	//these two vectors needed for output of findContours
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;
	//find contours of filtered image using openCV findContours function
	findContours(temp, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
	//use moments method to find our filtered object
	double refArea = 0;
	bool objectFound = false;
	if (hierarchy.size() > 0) {
		int numObjects = hierarchy.size();
		//if number of objects greater than MAX_NUM_OBJECTS we have a noisy filter
		if (numObjects<MAX_NUM_OBJECTS) {
			for (int index = 0; index >= 0; index = hierarchy[index][0]) {

				Moments moment = moments((cv::Mat)contours[index]);
				double area = moment.m00;

				//if the area is less than 20 px by 20px then it is probably just noise
				//if the area is the same as the 3/2 of the image size, probably just a bad filter
				//we only want the object with the largest area so we safe a reference area each
				//iteration and compare it to the area in the next iteration.
				if (area>8){//MIN_OBJECT_AREA) {

					fruit apple;

					apple.SetXPos(moment.m10 / area);
					apple.SetYPos(moment.m01 / area);
					apple.SetArea(area);

					apples.push_back(apple);
					//x = moment.m10/area;
					//y = moment.m01/area;



					objectFound = true;

				}
				else objectFound = false;


			}
			//let user know you found an object
			if (objectFound == true) {
				//draw object location on screen
				putText(cameraFeed, "Got to the place", Point(0, 50), 1, 2, Scalar(0, 0, 255), 2);
				drawObject(apples, cameraFeed);
				
				OUTPUT_detected = true;
			}

		}
		else putText(cameraFeed, "TOO MUCH NOISE! ADJUST FILTER", Point(0, 50), 1, 2, Scalar(0, 0, 255), 2);
	}
}

//This is also important because if you don't export
//with C linkage then name mangling might cause everything 
// to fail
extern "C" {
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(25, 25));
	DllExport void __stdcall PassMeAnArray(BYTE* inArray, int size, int &XPos, int &YPos, int &Area, int &detected){
		
		bool debugging = false;

		Mat threshold;
		Mat HSV;

		fruit apple;
		//apple.SetHSVmin(Scalar(46, 112, 0));		//----- RED!
		//apple.SetHSVmax(Scalar(256, 256, 256));	//----- RED!

		apple.SetHSVmin(Scalar(153, 93, 0));			//----- PINK!
		apple.SetHSVmax(Scalar(164, 255, 255));		//----- PINK!

		//assign opencv Mat from the data
		//this does not copy the data, only creates openCV information about it
		cv::Mat newMat(480, 640, CV_8UC3, inArray);

		//internally opencv does everything in bgr so use this
		//when doing internal openCV operations
		//don't necessarily need it now if you are 
		//going to display the image in unity
		cv::cvtColor(newMat, newMat, CV_RGB2BGR);

		if(debugging)
			createTrackbars();

		cvtColor(newMat, HSV, CV_BGR2HSV);
		//cvtColor(newMat, newMat, COLOR_BGR2HSV);
		if(!debugging)
			inRange(HSV, apple.GetHSVmin(), apple.GetHSVmax(), threshold);
		else
			inRange(HSV, Scalar(H_MIN, S_MIN, V_MIN), Scalar(H_MAX, S_MAX, V_MAX), threshold);
		morphOps(threshold);
		trackFilteredObject(threshold, HSV, newMat);

		//inRange(HSV, apple.GetHSVmin(), apple.GetHSVmax(), newMat);

		imshow("Thresholded Image", threshold);
		
		//putText(newMat, "TOO MUCH NOISE! ADJUST FILTER", Point(0, 50), 1, 2, Scalar(0, 0, 255), 2);
		//fun image effect
		//cv::dilate(newMat, newMat, kernel);
		//yay
		//putText(newMat, "Got to the place", Point(0, 50), 1, 2, Scalar(0, 0, 255), 2);

		XPos = OUTPUT_XPos;
		YPos = OUTPUT_YPos;
		Area = OUTPUT_Area;
		detected = OUTPUT_detected;

		cv::cvtColor(newMat, newMat, CV_BGR2RGB);
		
	}
}