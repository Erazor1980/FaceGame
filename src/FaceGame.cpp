#include "..\include\FaceGame.h"

#include "opencv2/imgproc/imgproc.hpp"

const int g_camID = 0;

FaceGame::FaceGame( const std::string pathToOpenCV_haarcascades )
    :
    m_videoCap( g_camID )
{
    m_bInitialized = true;
    printf( "#######################################\n" );
    printf( "#         F A C E    G A M E          #\n\ninitializing...\n" );

    std::string cascadeFace = "haarcascade_frontalface_alt.xml";
    if( m_faceCascade.load( pathToOpenCV_haarcascades + cascadeFace ) )
    {
        printf( "\t-'%s' loaded successfully.\n", cascadeFace.c_str() );
    }
    else
    {
        printf( "\t- Could not load '%s' in '%s'!\n", cascadeFace.c_str(), pathToOpenCV_haarcascades.c_str() );
        m_bInitialized = false;
    }

    std::string cascadeEyes = "haarcascade_eye.xml";
    if( m_eyesCascade.load( pathToOpenCV_haarcascades + cascadeEyes ) )
    {
        printf( "\t-'%s' loaded successfully.\n", cascadeEyes.c_str() );
    }
    else
    {
        printf( "\t- Could not load '%s' in '%s'!\n", cascadeEyes.c_str(), pathToOpenCV_haarcascades.c_str() );
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

        m_eyesParams.minSize        = cv::Size( 10, 10 );
        m_eyesParams.maxSize        = cv::Size( 50, 50 );
        m_eyesParams.minNeighbors   = 2;
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
    cv::destroyAllWindows();
}

void FaceGame::update()
{
    // get next frame
    if( m_videoCap.read( m_img ) )
    {
        // detect face
        detectFace();

        // display image
        display();
    }

    // key handling
    keyHandling();
}

void FaceGame::keyHandling()
{
    int key = cv::waitKey( 30 );

    if( 27 == key )     // esc -> end game
    {
        m_bEndGame = true;
    }
    else if( 'e' == key )
    {
        m_options.showEyes = !m_options.showEyes;
    }
    else if( 'f' == key )
    {
        m_options.showFace = !m_options.showFace;
    }
    else if( 'w' == key )
    {
        m_options.showFaceWnd = !m_options.showFaceWnd;
    }
}

void FaceGame::display()
{
    if( !m_options.showFaceWnd )
    {
        cv::destroyWindow( "face" );
    }
    if( m_options.showFaceWnd && m_vFaceDetResults.size() )
    {
        // calc size of the allFaces image
        int width_allFaces = 0, height_allFaces = 0;
        for( int i = 0; i < m_vFaceDetResults.size(); ++i )
        {
            if( m_vFaceDetResults[ i ].face.height > height_allFaces )
                height_allFaces = m_vFaceDetResults[ i ].face.height;

            width_allFaces += m_vFaceDetResults[ i ].face.width;
        }
        cv::Mat allFaces( height_allFaces, width_allFaces, CV_8UC3 );

        // draw all faces into the allFaces image
        int diffX = 0;
        for( int i = 0; i < m_vFaceDetResults.size(); ++i )
        {
            cv::Mat faceImg = m_img( m_vFaceDetResults[ i ].face );
            faceImg.copyTo( allFaces( cv::Rect( diffX, 0, faceImg.cols, faceImg.rows ) ) );
            diffX += faceImg.cols;
        }
        cv::imshow( "face", allFaces );
    }

    if( m_options.showEyes || m_options.showFace )
    {
        for( int i = 0; i < m_vFaceDetResults.size(); ++i )
        {
            cv::Rect currFace = m_vFaceDetResults[ i ].face;
            cv::Scalar colorFace;
            if( m_vFaceDetResults[ i ].bFaceDetSuccessfull )
                colorFace = GREEN;
            else
                colorFace = CYAN;

            cv::Point center( currFace.x + currFace.width / 2, currFace.y + currFace.height / 2 );

            if( m_vFaceDetResults[ i ].vEyes.size() > 0 )
            {
                if( m_options.showFace )
                    cv::ellipse( m_img, center, cv::Size( currFace.width / 2, currFace.height / 2 ), 0, 0, 360, colorFace, 4 );

                if( m_options.showEyes )
                {
                    for( int j = 0; j < m_vFaceDetResults[ i ].vEyes.size(); ++j )
                    {
                        cv::Rect eye = m_vFaceDetResults[ i ].vEyes[ j ];
                        cv::Point centerEye( currFace.x + eye.x + eye.width / 2, currFace.y + eye.y + eye.height / 2 );
                        cv::ellipse( m_img, centerEye, cv::Size( eye.width / 2, eye.height / 2 ), 0, 0, 360, BLUE, 3 );
                    }
                }
            }
            else
            {
                if( m_options.showFace )
                    cv::ellipse( m_img, center, cv::Size( currFace.width / 2, currFace.height / 2 ), 0, 0, 360, RED, 4 );
            }
        }
    }
    

    cv::imshow( m_wndName, m_img );

}

void FaceGame::detectFace()
{
    // for searching a face using eyes in last "rect"
    cv::Rect lastFace;
    if( m_vFaceDetResults.size() )
    {
        lastFace = m_vFaceDetResults[ 0 ].face;
    }
    

    m_vFaceDetResults.clear();
    std::vector< cv::Rect > vTmpFaces;

    cv::Mat frame_gray;
    cv::cvtColor( m_img, frame_gray, CV_BGR2GRAY );

    // The algorithm normalizes the brightness and increases the contrast of the image.
    cv::equalizeHist( frame_gray, frame_gray );

    m_faceCascade.detectMultiScale( frame_gray, 
                                    vTmpFaces,
                                    m_faceParams.scaleFactor, 
                                    m_faceParams.minNeighbors,
                                    m_faceParams.flags, 
                                    m_faceParams.minSize,
                                    m_faceParams.maxSize );

    for( size_t i = 0; i < vTmpFaces.size(); i++ )
    {
        FaceDetectionResults tmpResults;
        tmpResults.face = vTmpFaces[ i ];

        cv::Mat face = frame_gray( tmpResults.face );
        
        m_eyesCascade.detectMultiScale( face,
                                        tmpResults.vEyes,
                                        m_eyesParams.scaleFactor,
                                        m_eyesParams.minNeighbors,
                                        m_eyesParams.flags,
                                        m_eyesParams.minSize );

        tmpResults.bFaceDetSuccessfull = true;
        m_vFaceDetResults.push_back( tmpResults );
    }

    // no face found, use "eyes" search in the last face area
    if( vTmpFaces.size() == 0 && lastFace.width > 0 )
    {
        int avg_x = 0;
        int avg_y = 0;

        cv::Point centerFace;

        FaceDetectionResults tmpResults;
        tmpResults.bFaceDetSuccessfull = false;
        tmpResults.face = lastFace; // widht and height remain, x and y will be adapted
        cv::Mat face = frame_gray( lastFace );

        m_eyesCascade.detectMultiScale( face,
                                        tmpResults.vEyes,
                                        m_eyesParams.scaleFactor,
                                        m_eyesParams.minNeighbors,
                                        m_eyesParams.flags,
                                        m_eyesParams.minSize );

        for( size_t i = 0; i < tmpResults.vEyes.size(); i++ )
        {

            // centerpoint of eyes                                                              
            cv::Point eye_center( lastFace.x + tmpResults.vEyes[ i ].x + tmpResults.vEyes[ i ].width / 2,
                                  lastFace.y + tmpResults.vEyes[ i ].y + tmpResults.vEyes[ i ].height / 2 );

            // Average center of eyes                                                           
            avg_x += eye_center.x;
            avg_y += eye_center.y;
        }

        // Use average location of eyes                                                       
        if( tmpResults.vEyes.size() > 0 )
        {
            centerFace.x = avg_x / tmpResults.vEyes.size();
            centerFace.y = avg_y / tmpResults.vEyes.size();

            tmpResults.face.x = centerFace.x - lastFace.width / 2;
            tmpResults.face.y = centerFace.y - lastFace.height / 2;
            m_vFaceDetResults.push_back( tmpResults );
        }
    }
}

void FaceGame::processResults()
{
}
