cmake_minimum_required(VERSION 3.12)
project(spo_mandelbrot VERSION 1.0)

# Find OpenGL
find_package(OpenGL REQUIRED)
if(NOT OPENGL_FOUND)
    message(FATAL_ERROR "OpenGL not found!")
endif()

# Enable OpenMP
find_package(OpenMP REQUIRED)
if(NOT OPENMP_FOUND)
    message(FATAL_ERROR "OpenMP not found!")
endif()

# Add your source files here
set(SOURCES 
    ./src/main_threaded.cpp
)

# Add executable
add_executable(mandelbrot ${SOURCES})

# Link libraries
target_link_libraries(mandelbrot PRIVATE OpenGL::GL glfw OpenMP::OpenMP_CXX)
