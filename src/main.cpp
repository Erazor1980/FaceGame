#include "FaceGame.h"

int main()
{
    FaceGame fg;

    while( !fg.isGameEnded() )
    {
        fg.update();
    }

    return 0;
}