#pragma once
#include "defines.h"
#include "Enemy.h"

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
    // display options
    bool showInfoWnd = true;
    bool showFace = true;
    bool showEyes = false;    
    bool showFaceWnd = false;
    bool showDebugInfos = false;

    // face detection options
    int maxPixDiff = 150;       // maximal number of pixel (in x and y direction) between 2 frames from player center
    int minPixDiff = 7;         // minimal movement of face center to change its position

    // game options
    int sizePlayer = 40;        // size of player image in game window (radius*2 and width/height respectively)
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
    void adjustBoundaries();    // after detectFace or processResults some Rects can partially lie outside the image
    void createEnemies();

    void showDebugInfos();

    // captured image widht and height
    int m_sxImg;
    int m_syImg;

    // current coordinates of the detected face (center)
    cv::Point               m_facePos = cv::Point( 0, 0 );

    bool                    m_bEndGame = false;
    bool                    m_bInitialized = false;

    bool                    m_bPayerFaceSet = false;

    Options                 m_options;

    // enemies
    std::vector< Enemy >    m_vEnemies;

    // openCV params
    cv::Mat                 m_img;          /* camera live image with additional infos */
    cv::Mat                 m_gameImg;      /* game image */
    cv::Mat                 m_playerImg;    /* player image */

    std::string             m_wndName = "Infos";
    std::string             m_wndGameName = "F A C E    G A M E";
    cv::VideoCapture        m_videoCap;

    cv::CascadeClassifier   m_faceCascade;
    cv::CascadeClassifier   m_eyesCascade;
    CascadeParams           m_faceParams;
    CascadeParams           m_eyesParams;

    std::vector< FaceDetectionResults > m_vFaceDetResults;   /* detected faces (and corresponding eyes) in the current image */
};