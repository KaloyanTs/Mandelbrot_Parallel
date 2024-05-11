#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <algorithm>
#include <cmath>

const int WIDTH = 800;
const int HEIGHT = 600;
const double ratioHW = (double)HEIGHT / WIDTH;
const double centerX = -1.25f;
const double centerY = 0.05f;
const int PRECISION = 1e4;
const double ZOOM = 18;
const int ITERATIONS = 200;

int inSet(double x, double y)
{
    double xx = x, yy = y;
    double tmpx, tmpy;
    int i;
    for (i = 0; i < ITERATIONS && xx * xx + yy * yy < 4.0f; ++i)
    {
        tmpx = xx * xx - yy * yy + x;
        tmpy = 2 * xx * yy + y;
        xx = tmpx;
        yy = tmpy;
    }
    return i;
}

void display(GLFWwindow *window)
{
    // Clear the color buffer
    glClear(GL_COLOR_BUFFER_BIT);

    glColor3f(1.0f, 0.0f, 0.0f);

    double boundX = std::max(1.0, 1 / ratioHW);
    double boundY = std::max(1.0, ratioHW);
    double stepX = boundX / PRECISION;
    double stepY = boundY / PRECISION;

    double step = (double)25 / PRECISION / PRECISION;
    double p = 0;
    double show = 1;

    for (double x = 1 / ratioHW; x >= -1 / ratioHW; x -= stepX)
        for (double y = boundY; y >= -boundY; y -= stepY)
        {
            int reps = inSet((x / ZOOM) + centerX, y / ZOOM + centerY);
            double blue = (double)reps / ITERATIONS;
            blue = pow(blue, 0.2);
            // std::cerr << "calc: " << x + centerX << " " << y + centerY << "\t\t\t\t" << blue << std::endl;
            //  std::cerr << "Placing: " << x << " " << y << std::endl;
            if (p > show)
            {
                std::cerr << p << "%\n";
                show += 1;
            }
            p += step;
            // glColor3f(1.0f - blue, 1.0f, 1.0f - blue * blue);
            glColor3f(blue, blue, blue);
            glBegin(GL_POINTS);
            glVertex2f(x * ratioHW, y);
            glEnd();
        }

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
    {
        return -1;
    }

    // Create a windowed mode window and its OpenGL context
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
