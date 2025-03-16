#include <Windows.h>
#include <gl/gl.h>

#include "camera.h"
#include "defines.h"


void applyCamera(Camera* cam)
{
    glRotatef(-cam->angleX, 1, 0 ,0);
    glRotatef(-cam->angleZ, 0, 0 ,1);
    glTranslatef(-cam->x, -cam->y, -cam->z);
}


void rotateCamera(Camera* cam, float dx, float dz)
{
    cam->angleZ += dz;

    if(cam->angleZ < 0) cam->angleZ += 360;
    if(cam->angleZ > 360) cam->angleZ -= 360;

    cam->angleX += dx;

    if(cam->angleX < 0) cam->angleX = 0;
    if(cam->angleX > 180) cam->angleX = 180;
}


void rotateByMouseCamera(Camera* cam, int x, int y, float speed)
{
    POINT cursor_pos = {0, 0};
    POINT base = {x, y};

    GetCursorPos(&cursor_pos);
    rotateCamera(cam, (base.y - cursor_pos.y) * speed, (base.x - cursor_pos.x) * speed);

    SetCursorPos(base.x, base.y);
}


void moveCamera(Camera* cam, int forward, int right, float speed)
{
    float angle = -cam->angleZ * DEGTORAD;

    if(forward > 0)
        angle += right > 0 ? M_PI_4 : (right < 0 ? -M_PI_4 : 0);

    if(forward < 0)
        angle += M_PI + (right > 0 ? -M_PI_4 : (right < 0 ? M_PI_4 : 0));

    if(forward == 0)
    {
        angle += right > 0 ? M_PI_2 : -M_PI_2;

        if(right == 0) speed = 0;
    }

    if(speed != 0)
    {
        cam->x += sin(angle) * speed;
        cam->y += cos(angle) * speed;
    }
}
