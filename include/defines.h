#pragma once

// opencv
#include <opencv2/opencv.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>

// for timer 
#include <ctime>

// for playing sounds
#include <windows.h>
#include <mmsystem.h>

// defines
#define BLUE	cv::Scalar( 255, 0, 0 )
#define RED		cv::Scalar( 0, 0, 255 )
#define GREEN	cv::Scalar( 0, 255, 0 )
#define WHITE	cv::Scalar( 255, 255, 255 )
#define BLACK	cv::Scalar( 0, 0, 0 )
#define CYAN    cv::Scalar( 255, 255, 0 )

#define DEBUG_INFOS (0)

// paths
#define PATH_TO_IMAGES      std::string("D:\\Projects\\_images_\\")
#define PATH_TO_SOUNDS      std::string("D:\\Projects\\_sounds_\\")