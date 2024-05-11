#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <thread>
#include "util.h"

void renderChunk(std::vector<unsigned char> &pixels,
                 int number,
                 int startX, int endX,
                 int startY, int endY)
{
    double step = (double)100 / ((endX - startX)) / ((endY - startY));
    float p = 0;

    for (int x = startX; x < endX; x++)
    {
        for (int y = startY; y < endY; y++)
        {
            float newX = (float)(x) / (WIDTH) * 2 * boundX - boundX;
            float newY = (float)(y) / (HEIGHT) * 2 * boundY - boundY;
            int reps = inSet(newX / ZOOM + centerX, newY / ZOOM + centerY);
            float coeff = (float)reps / ITER;
            coeff = pow(coeff, 0.2);
            if (coeff < 0.4)
                coeff = pow(coeff, 6 - 12.5 * coeff);
            p += step;
            std::cerr << number << ":\t" << p << "%\n";
            // {
            //     std::lock_guard<std::mutex> lock(mtx);
            //     progress.store(p, std::memory_order_relaxed);
            // }

            // todo repair...not working properly!!!!!
            // todo use struct
            // setPink(pixels, x, y, coeff);
            setChecked(pixels, x, y, coeff, 0x00000000, 0x00ffffff);
        }
    }
}

void display(GLFWwindow *window)
{
    // Clear the color buffer
    glClear(GL_COLOR_BUFFER_BIT);

    std::vector<unsigned char> pixels(WIDTH * HEIGHT * 3);

    int numThreads = std::thread::hardware_concurrency();
    // int numThreads = 2;
    std::vector<std::thread> threads;

    float chunkSizeY = HEIGHT / numThreads;

    auto timeBegin = std::chrono::steady_clock::now();

    for (int i = 0; i < numThreads; ++i)
    {
        int startY = i * chunkSizeY;
        int endY = (i + 1) * chunkSizeY;
        threads.emplace_back(renderChunk, std::ref(pixels), i, 0, WIDTH, startY, endY);
    }

    for (auto &thread : threads)
    {
        thread.join();
    }

    auto timeEnd = std::chrono::steady_clock::now();
    std::chrono::duration<double> duration = timeEnd - timeBegin;
    std::clog << "Execution time: " << duration.count() << " seconds\n";

    glDrawPixels(WIDTH, HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    // Swap front and back buffers
    glfwSwapBuffers(window);

    // Poll for and process events
    glfwPollEvents();
}

void saveToPPM()
{
    int width, height;
    glfwGetFramebufferSize(glfwGetCurrentContext(), &width, &height);

    // Print the PPM header
    std::cout << "P6\n"
              << width << " " << height << "\n255\n";

    for (int y = height - 1; y >= 0; y--)
    {
        for (int x = 0; x < width; x++)
        {
            unsigned char pixel[3];
            glReadPixels(x, y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);
            std::cout.write(reinterpret_cast<const char *>(pixel), 3);
        }
    }
}

int main()
{
    if (!glfwInit())
        return -1;

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Red Square", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    display(window);

    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window))
    {

        // Poll for and process events
        glfwPollEvents();
    }

    saveToPPM();

    glfwTerminate();
    return 0;
}
