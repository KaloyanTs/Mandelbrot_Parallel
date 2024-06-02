#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <cmath>
#include <sstream>
#include <iomanip>

int WIDTH = 512;
int HEIGHT = 270;
float RATIO_HW = (float)HEIGHT / WIDTH;
float MIN_X = -2.0f;
float MAX_X = 2.0f;
float MIN_Y = -2.0f;
float MAX_Y = 2.0f;
float CENTER_X = -1.30f;
float CENTER_Y = 0.072f;
float RADIUS = 0.02f;
int NUM_THREADS = std::thread::hardware_concurrency();
int ITER = 10000;

float boundX = std::max(1.0f, 1 / RATIO_HW);
float boundY = std::max(1.0f, RATIO_HW);

std::string getCurrentDayMonthString()
{

    std::time_t now = std::time(nullptr);
    std::tm *localTime = std::localtime(&now);
    std::ostringstream dateStream;
    dateStream << std::put_time(localTime, "%d.%m");
    return dateStream.str();
}
std::string OUTPUT_DEFAULT_FILE = "../results/default.ppm";
std::string STATS_DEFAULT_FILE = "../stats/" + getCurrentDayMonthString() + ".csv";
std::ofstream out(OUTPUT_DEFAULT_FILE, std::ofstream::out);

std::mutex mtx;
std::atomic<double> progress(0);

struct RGB
{
    short red, green, blue;
    RGB(short r, short g, short b) : red(r), green(g), blue(b) {}
};

const RGB PINK(0xff, 80, 0xd5);

int inSet(float x, float y)
{
    double xx = x, yy = y;
    double tmpx, tmpy;
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

void setInterpolate(std::vector<unsigned char> &pixels, int x, int y, float interpolate, RGB c1, RGB c2)
{
    int index = (y * WIDTH + x) * 3;
    pixels[index] = static_cast<unsigned char>((float)c1.red * interpolate + (float)c2.red * (1.0f - interpolate));
    pixels[index] = static_cast<unsigned char>((float)c1.green * interpolate + (float)c2.green * (1.0f - interpolate));
    pixels[index] = static_cast<unsigned char>((float)c1.blue * interpolate + (float)c2.blue * (1.0f - interpolate));
}

void setPink(std::vector<unsigned char> &pixels, int x, int y, float interpolate)
{
    int index = (y * WIDTH + x) * 3;
    pixels[index] = static_cast<unsigned char>(PINK.red * interpolate);
    pixels[index + 1] = static_cast<unsigned char>(PINK.green * interpolate);
    pixels[index + 2] = static_cast<unsigned char>(PINK.blue * interpolate);
}

void setChecked(std::vector<unsigned char> &pixels, int x, int y, float coeff, RGB c1, RGB c2)
{
    // int index1 = (y * WIDTH + x) * 3;
    // pixels[index1] = static_cast<unsigned char>(255 * coeff);
    // pixels[index1 + 1] = static_cast<unsigned char>(255 * coeff);
    // pixels[index1 + 2] = static_cast<unsigned char>(255 * coeff);
    // return;
    RGB color(0, 0, 0);
    if (coeff < 1.0f)
    {
        color = RGB(0xff, 0xff, 0xff);
    }
    else
    {
        int xfloor = x - x % 7;
        int yfloor = y - y % 7;
        color = (xfloor + yfloor) % 2 ? c1 : c2;
    }
    int index = (y * WIDTH + x) * 3;
    pixels[index] = static_cast<unsigned char>(color.red);
    pixels[index + 1] = static_cast<unsigned char>(color.green);
    pixels[index + 2] = static_cast<unsigned char>(color.blue);
}