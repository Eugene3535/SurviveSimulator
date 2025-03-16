#include "Camera.hpp"
#include "Defines.hpp"

#include <Windows.h>
#include <gl/gl.h>

void Camera::apply()
{
    glRotatef(-angleX, 1, 0 ,0);
    glRotatef(-angleZ, 0, 0 ,1);
    glTranslatef(-x, -y, -z);
}

void Camera::rotate(float dx, float dz)
{
    angleZ += dz;

    if(angleZ < 0) angleZ += 360;
    if(angleZ > 360) angleZ -= 360;

    angleX += dx;

    if(angleX < 0) angleX = 0;
    if(angleX > 180) angleX = 180;
}

void Camera::rotateByMouse(int x, int y, float speed)
{
    POINT cursor_pos{0, 0};
    POINT base = {x, y};

    GetCursorPos(&cursor_pos);
    rotate((base.y - cursor_pos.y) * speed, (base.x - cursor_pos.x) * speed);

    SetCursorPos(base.x, base.y);
}

void Camera::move(int forward, int right, float speed)
{
    float angle = -angleZ * DEGTORAD;

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
        x += sin(angle) * speed;
        y += cos(angle) * speed;
    }
}
