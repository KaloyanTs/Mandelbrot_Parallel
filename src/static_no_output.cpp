#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <thread>
#include <algorithm>
#include <cstring>
#include "util.h"

void display();

void renderChunk(std::vector<unsigned char> &pixels,
                 const int number,
                 const int startY, const int endY);

void renderTask(std::vector<unsigned char> &pixels,
                int number,
                const std::vector<int> &startY,
                const std::vector<int> &endY)
{

    auto startTime = std::chrono::steady_clock::now();
    for (int i = 0; i < startY.size(); ++i)
    {
        renderChunk(pixels, number, startY[i], endY[i]);
    }
    auto endTime = std::chrono::steady_clock::now();
    std::chrono::duration<double> duration = endTime - startTime;
    std::clog << "Thread " << number << " took "
              << duration.count() << " seconds.\n";
}

void renderChunk(std::vector<unsigned char> &pixels,
                 const int number,
                 const int startY, const int endY)
{
    int startX = 0, endX = WIDTH;
    double step = (double)100 / ((endX - startX)) / ((endY - startY));
    float p = 0;

    float auxX = 1.0f / WIDTH * 2.0f * boundX;
    float auxY = 1.0f / HEIGHT * 2.0f * boundY;
    float newX = (float)startX * auxX - boundX, newY;

    for (int x = startX; x < endX; x++)
    {
        newX += auxX;
        newY = (float)startY * auxY - boundY;
        for (int y = startY; y < endY; y++)
        {
            newY += auxY;
            int reps = inSet(newX * RADIUS + CENTER_X, newY * RADIUS + CENTER_Y);
            float coeff = (float)reps * auxCoeff;
            coeff = pow(coeff, 0.2);
            if (coeff < 0.4)
                coeff = pow(coeff, 6 - 12.5 * coeff);
            p += step;

            int index = (y * WIDTH + x) * 3;
            pixels[index] = static_cast<unsigned char>(RED * coeff + (1.0f - coeff) * 100);
            pixels[index + 1] = static_cast<unsigned char>(GREEN * coeff + (1.0f - coeff) * 100);
            pixels[index + 2] = static_cast<unsigned char>(BLUE * coeff + (1.0f - coeff) * 34);
            // setChecked(pixels, x, y, coeff, RGB(0, 0, 0), PINK);
        }
    }
}

void display()
{

    std::clog << "Displaying " << CENTER_X << ' ' << CENTER_Y << ' ' << RADIUS << '\n';

    std::vector<unsigned char> pixels(WIDTH * HEIGHT * 3);

    std::vector<std::thread> threads;

    float chunkSizeY = HEIGHT / NUM_THREADS / GRANULARITY;

    auto timeBegin = std::chrono::steady_clock::now();

    for (int i = 0; i < NUM_THREADS; ++i)
    {
        std::vector<int> startY, endY, startX, endX;
        for (int j = 0; j < GRANULARITY; ++j)
        {
            startY.push_back((j * NUM_THREADS + i) * chunkSizeY);
            endY.push_back((j * NUM_THREADS + i + 1) * chunkSizeY);
        }
        threads.emplace_back(renderTask,
                             std::ref(pixels),
                             i,
                             startY, endY);
    }

    for (auto &thread : threads)
        thread.join();

    auto timeEnd = std::chrono::steady_clock::now();
    std::chrono::duration<double> duration = timeEnd - timeBegin;
    std::clog << "Execution time: " << duration.count() << " seconds\n";
    std::ofstream stat(STATS_DEFAULT_FILE, std::ofstream::app);
    stat << NUM_THREADS << '|';
    stat << GRANULARITY << '|';
    stat << WIDTH << " x " << HEIGHT << '|';
    stat << duration.count() << '\n';
    stat.close();
}

int main(int argc, char **argv)
{
    for (int i = 1; i < argc - argc % 2; i += 2)
    {
        std::string s = argv[i];
        if (SIZE_PARAM == s)
        {
            char *mid = strchr(argv[i + 1], 'x');
            if (!mid)
                return 1;
            *mid = 0;
            WIDTH = atoi(argv[i + 1]);
            HEIGHT = atoi(mid + 1);
            RATIO_HW = HEIGHT / WIDTH;
        }
        else if (OUTPUT_PARAM == s)
        {
            out = std::ofstream(argv[i + 1], std::ios::out);
        }
        else if (s == BOUNDS_PARAM)
        {
            if (*argv[i + 1] != 'x')
                return 1;
            ++argv[i + 1];
            char *p = strchr(argv[i + 1], 'y'), *q = argv[i + 1];
            if (!p)
                return 1;
            *p = 0;
            CENTER_X = std::stof(q);
            q = p + 1;
            p = strchr(q, 'r');
            if (!p)
                return 1;
            *p = 0;
            CENTER_Y = std::stof(q);
            q = p + 1;
            RADIUS = std::stof(q);
        }
        else if (s == COLOURS_PARAM)
        {
            if (*argv[i + 1] != 'r')
                return 1;
            ++argv[i + 1];
            char *p = strchr(argv[i + 1], 'g'), *q = argv[i + 1];
            if (!p)
                return 1;
            *p = 0;
            RED = std::stoi(q);
            q = p + 1;
            p = strchr(q, 'b');
            if (!p)
                return 1;
            *p = 0;
            GREEN = std::stoi(q);
            q = p + 1;
            BLUE = std::stof(q);
        }
        else if (s == THREADS_PARAM)
        {
            NUM_THREADS = std::stoi(argv[i + 1]);
        }
        else if (s == GRANULARITY_PARAM)
        {
            GRANULARITY = std::stoi(argv[i + 1]);
        }
    }

    display();
    out.close();
    return 0;
}
