#include "Enemy.h"

Enemy::Enemy( const cv::Mat img, cv::Mat* const gameImg )
    :
    mp_gameImg( gameImg )
{
    cv::resize( img, m_img, cv::Size( ENEMY_SIZE, ENEMY_SIZE ) );

    m_pos.x     = rand() % ( gameImg->cols - ENEMY_SIZE );
    m_pos.y     = rand() % ( gameImg->rows - ENEMY_SIZE );

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
    if( m_pos.x > mp_gameImg->cols - ENEMY_SIZE - 1 )
    {
        m_pos.x = mp_gameImg->cols - ENEMY_SIZE - 1;
        m_speed.x = -m_speed.x;
    }
    if( m_pos.y < 0 )
    {
        m_pos.y = 0;
        m_speed.y = -m_speed.y;
    }
    if( m_pos.y > mp_gameImg->rows - ENEMY_SIZE - 1 )
    {
        m_pos.y = mp_gameImg->rows - ENEMY_SIZE - 1;
        m_speed.y = -m_speed.y;
    }

    m_img.copyTo( (*mp_gameImg)( cv::Rect( m_pos, cv::Point( m_pos.x + ENEMY_SIZE, m_pos.y + ENEMY_SIZE ) ) ) );
}
