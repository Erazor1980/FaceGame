#include "Enemy.h"

Enemy::Enemy( const cv::Mat img, cv::Mat* const gameImg, const int size )
    :
    mp_gameImg( gameImg )
{
    cv::resize( img, m_img, cv::Size( size, size ) );

    m_pos.x     = rand() % ( gameImg->cols - size );
    m_pos.y     = rand() % ( gameImg->rows - size );

    m_speed.x   = rand() % ( MAX_SPEED * 2 + 1 ) - MAX_SPEED;   // from -MAX_SPEED to MAX_SPEED
    m_speed.y   = rand() % ( MAX_SPEED * 2 + 1 ) - MAX_SPEED;   // from -MAX_SPEED to MAX_SPEED
}

void Enemy::update()
{
    m_pos += m_speed;

    if( m_pos.x < 0 )
    {
        m_pos.x = 0;
        m_speed.x = -m_speed.x;
    }
    if( m_pos.x > mp_gameImg->cols - m_img.cols - 1 )
    {
        m_pos.x = mp_gameImg->cols - m_img.cols - 1;
        m_speed.x = -m_speed.x;
    }
    if( m_pos.y < 0 )
    {
        m_pos.y = 0;
        m_speed.y = -m_speed.y;
    }
    if( m_pos.y > mp_gameImg->rows - m_img.rows - 1 )
    {
        m_pos.y = mp_gameImg->rows - m_img.rows - 1;
        m_speed.y = -m_speed.y;
    }
}

void Enemy::display()
{
    m_img.copyTo( ( *mp_gameImg )( cv::Rect( m_pos, cv::Point( m_pos.x + m_img.cols, m_pos.y + m_img.rows ) ) ) );
}

bool Enemy::collisionTest( const cv::Rect otherBB )
{
    const int right0    = m_pos.x + m_img.cols;
    const int bottom0   = m_pos.y + m_img.rows;
    const int right1    = otherBB.x + otherBB.width;
    const int bottom1   = otherBB.y + otherBB.height;

    return  ( right0 >= otherBB.x 
              && m_pos.x <= right1 
              && bottom0 >= otherBB.y 
              && m_pos.y <= bottom1 );
}
