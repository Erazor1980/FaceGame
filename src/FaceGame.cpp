#include "..\include\FaceGame.h"

const int g_camID = 0;

FaceGame::FaceGame( const std::string pathToOpenCV_haarcascades )
    :
    m_videoCap( g_camID )
{
    m_bInitialized = true;
    printf( "#######################################\n" );
    printf( "#         F A C E    G A M E          #\n\ninitializing...\n" );

    std::string cascade = "haarcascade_frontalface_alt.xml";
    if( m_faceCascade.load( pathToOpenCV_haarcascades + cascade ) )
    {
        printf( "\t-'%s' loaded successfully.\n", cascade.c_str() );
    }
    else
    {
        printf( "\t- Could not load '%s' in '%s'!\n", cascade.c_str(), pathToOpenCV_haarcascades.c_str() );
        m_bInitialized = false;
    }

    if( m_videoCap.isOpened() )
    {
        printf( "\t- Video camera nr. %d opened successfully.\n", g_camID );
        if( m_videoCap.read( m_img ) )
        {
            m_sxImg = m_img.cols;
            m_syImg = m_img.rows;
            printf( "\t- Image size: %d x %d.\n", m_sxImg, m_syImg );
        }
        else
        {
            printf( "\t- Cannot read a frame from video stream.\n" );
            m_bInitialized = false;
        }
    }
    else
    {
        printf( "\t- Coud not open camera nr. %d.\n", g_camID );
        m_bInitialized = false;
    }

    if( m_bInitialized )
    {
        printf( "... done.\n" );

        cv::namedWindow( m_wndName, cv::WINDOW_NORMAL );
    }
    else
    {
        printf( "... failed!\n" );
    }
    printf( "#                                     #\n" );
    printf( "#######################################\n" );

}

FaceGame::~FaceGame()
{
}

void FaceGame::update()
{
    // get next frame
    m_videoCap.read( m_img );

    // display image
    display();

    // key handling
    keyHandling();
}

void FaceGame::keyHandling()
{
    int key = cv::waitKey( 1 );

    if( 27 == key )     // esc -> end game
    {
        m_bEndGame = true;
    }
}

void FaceGame::display()
{
    cv::imshow( m_wndName, m_img );
}
