#include "FaceGame.h"

#include "opencv2/imgproc/imgproc.hpp"

// global params
const int g_camID = 0;
std::string g_cascadeFace = "haarcascade_frontalface_alt.xml";
//std::string g_cascadeFace = "haarcascade_frontalface_default.xml";


FaceGame::FaceGame( const std::string pathToOpenCV_haarcascades )
    :
    m_videoCap( g_camID )
{
    std::srand( ( unsigned int )std::time( 0 ) );

    m_bInitialized = true;
    printf( "#######################################\n" );
    printf( "#         F A C E    G A M E          #\n\ninitializing...\n" );

    if( m_faceCascade.load( pathToOpenCV_haarcascades + g_cascadeFace ) )
    {
        printf( "\t-'%s' loaded successfully.\n", g_cascadeFace.c_str() );
    }
    else
    {
        printf( "\t- Could not load '%s' in '%s'!\n", g_cascadeFace.c_str(), pathToOpenCV_haarcascades.c_str() );
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
        cv::namedWindow( m_wndGameName, cv::WINDOW_NORMAL );

        m_eyesParams.minSize        = cv::Size( 10, 10 );
        m_eyesParams.maxSize        = cv::Size( 50, 50 );
        m_eyesParams.minNeighbors   = 2;

        // create game image
        m_gameImg.create( m_img.size(), CV_8UC3 );
        m_gameImg.setTo( 0 );

        // create player
        m_playerInfo = PlayerInfo( cv::Size( m_options.sizePlayer, m_options.sizePlayer ) );
        
        // add enemies
        createEnemies();
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
    if( !m_bInitialized )
    {
        printf( "\nGame not initialized!\n" );
        m_bEndGame = true;
        return;
    }

    if( m_vFaceDetResults.size() )
        printf( "\rface center at: %d, %d", m_facePos.x, m_facePos.y );

    // get next frame
    if( m_videoCap.read( m_img ) )
    {
        // detect face
        detectFace();

        // process detection results
        processResults();

        // update enemies
        {
            auto it = m_vEnemies.begin();
            while( it != m_vEnemies.end() )
            {
                // collision test with player bb
                if( m_bPayerFaceSet )
                {
                    bool isBad = false;
                    if( it->collisionTest( m_playerInfo.getBB(), isBad ) )
                    {
                        it = m_vEnemies.erase( it );
                        if( m_playerInfo.update( isBad ) )
                        {
                            m_bGameOver = true;
                        }
                        if( isBad )
                        {
                            m_badEnemiesCounter--;
                        }
                        else
                        {
                            m_goodEnemiesCounter--;
                        }
                    }
                    else
                        it++;
                }
                else
                    it++;
            }
            for( int i = 0; i < m_vEnemies.size(); ++i )
            {
                m_vEnemies[ i ].update();
            }
        }

        // display image
        display();
    }

    // key handling
    keyHandling();
}

void FaceGame::display()
{
    // FACES IMAGE
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

    ////////////////////////////////////////
    //////////// INFO WINDOW ///////////////
    ////////////////////////////////////////
    // DEBUG DRAWINGS -> faces, eyes, etc
    if( m_options.showInfoWnd )
    {
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

        // show current player position
        if( m_bPayerFaceSet )
        {
            cv::circle( m_img, m_facePos, 40, WHITE, 2 );
        }

        if( m_options.showDebugInfos )
        {
            showDebugInfos();
        }
        cv::imshow( m_wndName, m_img );
    }
    else
    {
        cv::destroyWindow( m_wndName );
    }


    ////////////////////////////////////////
    //////////// GAME WINDOW ///////////////
    ////////////////////////////////////////
    {
        m_gameImg.setTo( 0 );
        // ENEMIES
        for( int i = 0; i < m_vEnemies.size(); ++i )
        {
            m_vEnemies[ i ].display();
        }

        if( m_bPayerFaceSet )
        {
            m_playerInfo.getImg().copyTo( m_gameImg( m_playerInfo.getBB() ) );
        }
    }
    
    // GAME OVER
    if( m_bGameOver )
    {
        PlaySound( "D:\\Projects\\_sounds_\\gameover2.wav", NULL, SND_ASYNC );
        gameOverScreen();
    }

    // VICTORY
    if( m_goodEnemiesCounter == 0 )
    {
        PlaySound( "D:\\Projects\\_sounds_\\victory.wav", NULL, SND_ASYNC );
        victoryScreen();
    }

    cv::imshow( m_wndGameName, m_gameImg );
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

    adjustBoundaries();

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
            centerFace.x = avg_x / ( int )tmpResults.vEyes.size();
            centerFace.y = avg_y / ( int )tmpResults.vEyes.size();

            tmpResults.face.x = centerFace.x - lastFace.width / 2;
            tmpResults.face.y = centerFace.y - lastFace.height / 2;
            m_vFaceDetResults.push_back( tmpResults );
        }

        adjustBoundaries();
    }
}

void FaceGame::processResults()
{
    if( !m_bPayerFaceSet )
    {
        return;
    }

    for( int i = 0; i < m_vFaceDetResults.size(); ++i )
    {
        cv::Rect currBB = m_vFaceDetResults[ i ].face;

        cv::Point center = cv::Point( currBB.x + currBB.width / 2, currBB.y + currBB.height / 2 );

        const int xDiff = abs( center.x - m_facePos.x );
        const int yDiff = abs( center.y - m_facePos.y );

        // change center position only, if detected face center moved less than m_options.maxPixDiff
        if( xDiff < m_options.maxPixDiff && yDiff < m_options.maxPixDiff )
        {
            // Check to see if the user moved enough to update position
            if( xDiff < m_options.minPixDiff && yDiff < m_options.minPixDiff )
            {
                // do not adjust position due to too less movement
            }
            else
            {
                m_facePos = center;
                m_playerInfo.adjustBB( m_facePos );
                m_playerInfo.addImg( m_img( currBB ) );
            }
        }
    }
}

void FaceGame::adjustBoundaries()
{
    for( int i = 0; i < m_vFaceDetResults.size(); ++i )
    {
        cv::Rect* curr = &(m_vFaceDetResults[ i ].face);
        if( curr->x + curr->width > m_sxImg )
        {
            curr->width -= abs( curr->x + curr->width - m_sxImg );
        }
        if( curr->x < 0 )
        {
            curr->width -= abs( curr->x );
            curr->x = 0;
        }
        if( curr->y + curr->height > m_syImg )
        {
            curr->height -= abs( curr->y + curr->height - m_syImg );
        }
        if( curr->y < 0 )
        {
            curr->height -= abs( curr->y );
            curr->y = 0;
        }
    }
}

void FaceGame::createEnemies()
{
    m_vEnemies.clear();

    // good enemies
    cv::Mat enemy = cv::imread( "D:/Projects/_images_/bvb.png", 1 );
     for( int i = 0; i < 5; ++i )
    {
        Enemy newEnemy( enemy, &m_gameImg, false, ENEMY_SIZE );
        m_vEnemies.push_back( newEnemy );
        m_goodEnemiesCounter++;
    }

    // bad enemies
    enemy = cv::imread( "D:/Projects/_images_/evil.png", 1 );
    for( int i = 0; i < 5; ++i )
    {
        Enemy newEnemy( enemy, &m_gameImg, true, ENEMY_SIZE );
        m_vEnemies.push_back( newEnemy );
        m_badEnemiesCounter++;
    }
}

void FaceGame::gameOverScreen()
{
    int fontFace = cv::FONT_HERSHEY_PLAIN;
    int fontScale = 4;
    int fontThickness = 3;

    const int x = 120;
    const int y = 170;

    cv::putText( m_gameImg, "GAME OVER", cv::Point( x, y ), fontFace, fontScale, GREEN, fontThickness );
    cv::putText( m_gameImg, "Close game -> press ESC", cv::Point( x, y + 40 ), fontFace, 1.5, RED, 2 );
    cv::putText( m_gameImg, "New game   -> press Enter", cv::Point( x, y + 70 ), fontFace, 1.5, RED, 2 );
    /*cv::putText( m_gameImg, "Start menu -> press 's'", cv::Point( x, y + 100 ), fontFace, 1.5, RED, 2 );
    cv::putText( m_gameImg, "High Score -> press 'h'", cv::Point( x, y + 130 ), fontFace, 1.5, RED, 2 );*/

    cv::imshow( m_wndGameName, m_gameImg );

    int key = cv::waitKey( 0 );
    if( 27 == key )
    {
        m_bEndGame = true;
    }
    else if( 13 == key )
    {
        resetGame();
    }
}

void FaceGame::victoryScreen()
{
    int fontFace = cv::FONT_HERSHEY_PLAIN;
    int fontScale = 4;
    int fontThickness = 3;

    const int x = 120;
    const int y = 170;

    cv::putText( m_gameImg, "VICTORY ! ! ! !", cv::Point( x, y ), fontFace, fontScale, BLUE, fontThickness );
    cv::putText( m_gameImg, "Close game -> press ESC", cv::Point( x, y + 40 ), fontFace, 1.5, RED, 2 );
    cv::putText( m_gameImg, "New game   -> press Enter", cv::Point( x, y + 70 ), fontFace, 1.5, RED, 2 );
    /*cv::putText( m_gameImg, "Start menu -> press 's'", cv::Point( x, y + 100 ), fontFace, 1.5, RED, 2 );
    cv::putText( m_gameImg, "High Score -> press 'h'", cv::Point( x, y + 130 ), fontFace, 1.5, RED, 2 );*/

    cv::imshow( m_wndGameName, m_gameImg );

    int key = cv::waitKey( 0 );
    if( 27 == key )
    {
        m_bEndGame = true;
    }
    else if( 13 == key )
    {
        resetGame();
    }
}

void FaceGame::showDebugInfos()
{
    int fontFace = cv::FONT_HERSHEY_PLAIN;
    double fontScale = 0.7;
    int fontThickness = 1;

    const int x = 10;
    int y = 20;
    const int yDiff = 15;

    char text[ 100 ];

    sprintf_s( text, "minPxDiff: %d", m_options.minPixDiff );
    cv::putText( m_img, text, cv::Point( x, y ), fontFace, fontScale, GREEN, fontThickness );

    sprintf_s( text, "maxPxDiff: %d", m_options.maxPixDiff ); y+= yDiff;
    cv::putText( m_img, text, cv::Point( x, y ), fontFace, fontScale, GREEN, fontThickness );
}

void FaceGame::resetGame()
{
    createEnemies();
    m_playerInfo.reset();
    m_bGameOver     = false;
    m_bPayerFaceSet = false;
}

void FaceGame::keyHandling()
{
    int key = cv::waitKey( 30 );

    if( 27 == key )     // esc -> end game
    {
        m_bEndGame = true;
    }
    else if( 13 == key )        // enter -> save currPos of face
    {
        if( m_vFaceDetResults.size() == 1 )
        {
            cv::Rect curr = m_vFaceDetResults[ 0 ].face;
            int x = curr.x + curr.width / 2;
            int y = curr.y + curr.height / 2;
            m_facePos = cv::Point( x, y );

            m_playerInfo.adjustBB( m_facePos );
            m_playerInfo.addImg( m_img( cv::Rect( curr.x, curr.y, curr.width, curr.height ) ) );
            
            printf( "\nFace coordinates successfully saved at %d, %d.\n", x, y );
            m_bPayerFaceSet = true;
        }
        else
        {
            printf( "\nNo face or too many faces detected!\n" );
            m_bPayerFaceSet = false;
        }
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
    else if( 'd' == key )
    {
        m_options.showDebugInfos = !m_options.showDebugInfos;
    }
    else if( 'o' == key )
    {
        m_options.showInfoWnd = !m_options.showInfoWnd;
    }
    else if( '+' == key )
    {
        m_options.minPixDiff++;
        m_options.minPixDiff = MIN( 20, m_options.minPixDiff );
    }
    else if( '-' == key )
    {
        m_options.minPixDiff--;
        m_options.minPixDiff = MAX( 0, m_options.minPixDiff );
    }
    else if( '*' == key )
    {
        m_options.maxPixDiff += 10;
        m_options.maxPixDiff = MIN( 300, m_options.maxPixDiff );
    }
    else if( '_' == key )
    {
        m_options.maxPixDiff -= 10;
        m_options.maxPixDiff = MAX( 0, m_options.maxPixDiff );
    }
    else if( ' ' == key )
    {
        createEnemies();
    }
}