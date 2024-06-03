#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <cmath>
#include <sstream>
#include <iomanip>

int WIDTH = 1000;
int HEIGHT = 1000;
float RATIO_HW = (float)HEIGHT / WIDTH;
// float CENTER_X = -1.30f;
// float CENTER_Y = 0.072f;
float CENTER_X = -0.4f;
float CENTER_Y = 0.0f;
// float CENTER_X = -0.3575f;
// float CENTER_Y = 0.6375f;
// float RADIUS = 0.02f;
float RADIUS = 1.2f;
// float RADIUS = 0.0475f;
int NUM_THREADS = std::thread::hardware_concurrency();
int ITER = 10000;
int GRANULARITY = 1;
float auxCoeff = 1.0f / ITER;

// MPI
std::atomic<int> rowIdx = 0;
const int STRIP_SIZE = 5;

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

const std::string SIZE_PARAM = "-s";
const std::string OUTPUT_PARAM = "-o";
const std::string BOUNDS_PARAM = "-c";
const std::string THREADS_PARAM = "-t";
const std::string GRANULARITY_PARAM = "-g";
const std::string COLOURS_PARAM = "-v";

struct RGB
{
    short red, green, blue;
    RGB(short r, short g, short b) : red(r), green(g), blue(b) {}
};

const RGB PINK(0xff, 80, 0xd5);
short RED = 0xff;
short GREEN = 127;
short BLUE = 127;

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
        coeff = pow(coeff, 0.3);
        color = RGB(0xff, 0xff, 0x00);
        int index = (y * WIDTH + x) * 3;
        pixels[index] = static_cast<unsigned char>(coeff * color.red);
        pixels[index + 1] = static_cast<unsigned char>(coeff * color.green);
        pixels[index + 2] = static_cast<unsigned char>(coeff * color.blue);
        return;
    }
    else
    {
        int xfloor = x - x % 19;
        int yfloor = y - y % 19;
        color = (xfloor + yfloor) % 2 ? RGB(0xff, 0x80, 0x0d) : RGB(0xe5, 0, 0);
    }
    int index = (y * WIDTH + x) * 3;
    pixels[index] = static_cast<unsigned char>(color.red);
    pixels[index + 1] = static_cast<unsigned char>(color.green);
    pixels[index + 2] = static_cast<unsigned char>(color.blue);
}