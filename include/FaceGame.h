#pragma once
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
//#include "opencv2/imgproc/imgproc.hpp"

class FaceGame
{
public:
    FaceGame( const std::string pathToOpenCV_haarcascades = "C:/opencv/build/etc/haarcascades/" );
    ~FaceGame();

    void update();
    bool isGameEnded() const
    {
        return m_bEndGame;
    }
private:
    void keyHandling();
    void display();

    // captured image widht and height
    int m_sxImg;
    int m_syImg;

    // current coordinates of the detected face
    int m_faceX;
    int m_faceY;

    bool m_bEndGame = false;
    bool m_bInitialized = false;

    // openCV params
    cv::Mat m_img;
    std::string m_wndName = "Face Game";
    cv::VideoCapture m_videoCap;

    cv::CascadeClassifier m_faceCascade;
};