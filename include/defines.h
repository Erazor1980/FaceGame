#pragma once

#include <opencv2/opencv.hpp>
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"

// for timer 
#include <ctime>

// for playing sounds
#include <windows.h>
#include <mmsystem.h>


#define BLUE	cv::Scalar( 255, 0, 0 )
#define RED		cv::Scalar( 0, 0, 255 )
#define GREEN	cv::Scalar( 0, 255, 0 )
#define WHITE	cv::Scalar( 255, 255, 255 )
#define BLACK	cv::Scalar( 0, 0, 0 )
#define CYAN    cv::Scalar( 255, 255, 0 )

#define DEBUG_INFOS (0)