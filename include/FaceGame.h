#pragma once
#include "defines.h"

struct CascadeParams
{
    double scaleFactor = 1.1;
    int minNeighbors = 3;
    int flags =  0 | CV_HAAR_SCALE_IMAGE;   //TODO nochmal checken, was das genau bedeutet!
    cv::Size minSize = cv::Size( 80, 80 );
    cv::Size maxSize = cv::Size( 250, 250 );
};

struct Options
{
    bool showFace = true;
    bool showEyes = false;    
    bool showFaceWnd = false;
};

struct FaceDetectionResults
{
    cv::Rect face;
    std::vector< cv::Rect > vEyes;
    bool bFaceDetSuccessfull;       // if false, face position was calculated based on last position and eye detection!
};

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
    void detectFace();
    void processResults();

    // captured image widht and height
    int m_sxImg;
    int m_syImg;

    // current coordinates of the detected face
    int m_faceX;
    int m_faceY;

    bool m_bEndGame = false;
    bool m_bInitialized = false;

    Options m_options;

    // openCV params
    cv::Mat                 m_img;
    std::string             m_wndName = "Face Game";
    cv::VideoCapture        m_videoCap;

    cv::CascadeClassifier   m_faceCascade;
    cv::CascadeClassifier   m_eyesCascade;
    CascadeParams           m_faceParams;
    CascadeParams           m_eyesParams;

    std::vector< FaceDetectionResults > m_vFaceDetResults;   /* detected faces (and corresponding eyes) in the current image */
};