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
struct PlayerInfo
{
public:
    PlayerInfo()
    {
        bb = cv::Rect( 0, 0, 10, 10 );
        img.create( 10, 10, CV_8UC3 );
    }
    PlayerInfo( const cv::Size size )
    {
        bb.width    = size.width;
        bb.height   = size.height;

        img.create( size, CV_8UC3 );
        img.setTo( 0 );
    }
    void addImg( const cv::Mat& newImg )
    {
        cv::resize( newImg, img, img.size() );
        drawLifeBar();
    }
    void adjustBB( const cv::Point& newCenter )
    {
        bb.x = newCenter.x - img.cols / 2;
        bb.y = newCenter.y - img.rows / 2;
    }

    // returns true, if player is dead
    bool update( bool aua )
    {
        if( aua )
        {
            life -= maxLife / 4;
            life = MAX( 0, life );
        }
        else
        {
            life += maxLife / 5;
            life = MIN( life, maxLife );
        }
        drawLifeBar();

        if( life == 0 )
            return true;
        else
            return false;
    }

    cv::Rect getBB() const
    {
        return bb;
    }
    cv::Mat getImg() const
    {
        return img;
    }
    void reset()
    {
        life = maxLife;
    }
private:
    void drawLifeBar()
    {
        double perc = (double)life / maxLife;
        int px = (int)( perc * img.cols );  /* length of green bar */
        cv::line( img, cv::Point( px, 0 ), cv::Point( img.cols - 1, 0 ), RED, 3 );
        if( px  )
            cv::line( img, cv::Point( 0, 0 ), cv::Point( px, 0 ), GREEN, 3 );
    }
    cv::Rect bb;    /* bounding box */
    cv::Mat img;

    int maxLife = 100;
    int life    = 100;
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
    void resetGame();
    void keyHandling();
    void display();
    void detectFace();
    void processResults();
    void adjustBoundaries();    // after detectFace or processResults some Rects can partially lie outside the image
    void createEnemies();
    void gameOverScreen();
    void victoryScreen();

    void showDebugInfos();

    // captured image widht and height
    int m_sxImg;
    int m_syImg;

    // current coordinates of the detected face (center)
    cv::Point               m_facePos = cv::Point( 0, 0 );

    // player info
    PlayerInfo              m_playerInfo;

    bool                    m_bEndGame = false;         // finish game bool
    bool                    m_bInitialized = false;
    bool                    m_bGameOver = false;        // game over bool

    bool                    m_bPayerFaceSet = false;

    Options                 m_options;

    // enemies
    std::vector< Enemy >    m_vEnemies;
    int                     m_badEnemiesCounter = 0;
    int                     m_goodEnemiesCounter = 0;

    // openCV params
    cv::Mat                 m_img;          /* camera live image with additional infos */
    cv::Mat                 m_gameImg;      /* game image */
    //cv::Mat                 m_playerImg;    /* player image */

    std::string             m_wndName = "Infos";
    std::string             m_wndGameName = "F A C E    G A M E";
    cv::VideoCapture        m_videoCap;

    cv::CascadeClassifier   m_faceCascade;
    cv::CascadeClassifier   m_eyesCascade;
    CascadeParams           m_faceParams;
    CascadeParams           m_eyesParams;

    std::vector< FaceDetectionResults > m_vFaceDetResults;   /* detected faces (and corresponding eyes) in the current image */
};