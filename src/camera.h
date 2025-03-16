#ifndef CAMERA_H
#define CAMERA_H

typedef struct
{
    float x;
    float y;
    float z;
    float angleX;
    float angleZ;
} Camera;

void applyCamera(Camera* cam);
void rotateCamera(Camera* cam, float dx, float dz);
void rotateByMouseCamera(Camera* cam, int x, int y, float speed);
void moveCamera(Camera* cam, int forward, int right, float speed);

#endif // CAMERA_H
