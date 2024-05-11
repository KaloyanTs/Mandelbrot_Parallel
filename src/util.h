#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <algorithm>
#include <cmath>

#define PINK 0x00ff80d5

const int WIDTH = 512;
const int HEIGHT = 270;
const float ratioHW = (float)HEIGHT / WIDTH;
const float centerX = -1.30f;
const float centerY = 0.072f;
const float ZOOM = 50;
const int ITER = 10000;

float boundX = std::max(1.0f, 1 / ratioHW);
float boundY = std::max(1.0f, ratioHW);

std::mutex mtx;
std::atomic<double> progress(0);

inline int getRed(int color)
{
    return color & (((1ll << 8) - 1ll) << 16);
}

inline int getGreen(int color)
{
    return color & (((1ll << 8) - 1ll) << 8);
}

inline int getBlue(int color)
{
    return color & ((1ll << 8) - 1ll);
}

int inSet(float x, float y)
{
    float xx = x, yy = y;
    float tmpx, tmpy;
    int i;
    for (i = 0; i < ITER && xx * xx + yy * yy < 4.0f; ++i)
    {
        tmpx = xx * xx - yy * yy + x;
        tmpy = 2 * xx * yy + y;
        xx = tmpx;
        yy = tmpy;
    }
    return i;
}

void setPink(std::vector<unsigned char> &pixels, int x, int y, float interpolate)
{
    int index = (y * WIDTH + x) * 3;
    pixels[index] = static_cast<unsigned char>(getRed(PINK) * interpolate);
    pixels[index + 1] = static_cast<unsigned char>(getGreen(PINK) * interpolate);
    pixels[index + 2] = static_cast<unsigned char>(getBlue(PINK) * interpolate);
}

void setChecked(std::vector<unsigned char> &pixels, int x, int y, float coeff, int color1, int color2)
{
    // int index1 = (y * WIDTH + x) * 3;
    // pixels[index1] = static_cast<unsigned char>(255 * coeff);
    // pixels[index1 + 1] = static_cast<unsigned char>(255 * coeff);
    // pixels[index1 + 2] = static_cast<unsigned char>(255 * coeff);
    // return;
    int color;
    if (coeff < 1.0f)
    {
        color = INT16_MAX;
    }
    else
    {
        int xfloor = x - x % 7;
        int yfloor = y - y % 7;
        color = ((xfloor + yfloor) % 2 ? color1 : color2);
    }
    int index = (y * WIDTH + x) * 3;
    pixels[index] = static_cast<unsigned char>(getRed(color));
    pixels[index + 1] = static_cast<unsigned char>(getGreen(color));
    pixels[index + 2] = static_cast<unsigned char>(getBlue(color));
}