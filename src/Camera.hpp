#ifndef CAMERA_HPP
#define CAMERA_HPP

struct Camera
{
    void apply();
    void rotate(float dx, float dz);
    void rotateByMouse(int x, int y, float speed);
    void move(int forward, int right, float speed);

    float x = 0;
    float y = 0;
    float z = 1.7;
    float angleX = 90;
    float angleZ = 0;
};

#endif // CAMERA_HPP
