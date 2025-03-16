#ifndef CAMERA_HPP
#define CAMERA_HPP

struct Camera
{
    float x;
    float y;
    float z;
    float angleX;
    float angleZ;
};

void applyCamera(Camera* cam);
void rotateCamera(Camera* cam, float dx, float dz);
void rotateByMouseCamera(Camera* cam, int x, int y, float speed);
void moveCamera(Camera* cam, int forward, int right, float speed);

#endif // CAMERA_HPP
