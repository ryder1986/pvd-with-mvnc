#pragma once
#include "defines.h"
//#include <opencv/cv.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/tracking.hpp>
// http://www.morethantechnical.com/2011/06/17/simple-kalman-filter-for-tracking-using-opencv-2-2-w-code/
using namespace cv;
class TKalmanFilter
{
public:
    KalmanFilter* kalman;
	track_t deltatime; //приращение времени
	Point_t LastResult;
	TKalmanFilter(Point_t p, track_t dt = 0.2, track_t Accel_noise_mag = 0.5);
	~TKalmanFilter();
	Point_t GetPrediction();
	Point_t Update(Point_t p, bool DataCorrect);
};
