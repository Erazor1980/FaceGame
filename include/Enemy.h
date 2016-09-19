#pragma once
#include "defines.h"

#define ENEMY_SIZE      (30)    //TODO if needed, make parameter
#define NUMBER_ENEMIES  (10)     //TODO if needed, make parameter
#define MAX_SPEED       (10)     // max speed of an enemy in px diff

class Enemy
{
public:
    Enemy( const cv::Mat img, cv::Mat* const gameImg, const int size = ENEMY_SIZE );

    void update();
    void display();

    bool collisionTest( const cv::Rect otherBB );
private:
    cv::Point   m_pos;      // top left pixel position in gameImg

    cv::Point   m_speed;    // in x and y direction

    cv::Mat     m_img;
    cv::Mat*    mp_gameImg;
};