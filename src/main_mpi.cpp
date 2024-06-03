#include <mpi.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <complex>
#include <chrono>

using namespace std;
using namespace chrono;

const int WIDTH = 800;
const int HEIGHT = 600;
const double RATIO_HW = (double)HEIGHT / WIDTH;
const int MAX_ITER = 1000;

int mandelbrot(int x, int y)
{
    double scaled_x = (x - WIDTH / 1.5) * 3.0 / WIDTH;
    double scaled_y = (y - HEIGHT / 1.5 / RATIO_HW) * 3.0 * RATIO_HW / HEIGHT + 0.84;
    complex<double> c(scaled_x, scaled_y);
    complex<double> z = c;
    int iter = 0;
    while (abs(z) < 2.0 && iter < MAX_ITER)
    {
        z = z * z + c;
        iter++;
    }
    return iter;
}

void save_image(const vector<int> &image, const string &filename)
{
    ofstream ofs(filename, ios::binary);
    ofs << "P6\n"
        << WIDTH << " " << HEIGHT << "\n255\n";
    for (int i = 0; i < WIDTH * HEIGHT; ++i)
    {
        int value = image[i];
        char r, g, b;
        if (value < 10)
        {
            r = 255;
            g = 0;
            b = 0;
        }
        else
        {
            char color = static_cast<char>(255 - (value * 255 / MAX_ITER));
            r = color;
            g = color;
            b = color;
        }
        ofs << r << g << b;
    }
    ofs.close();
}

void master(int num_procs)
{
    vector<int> image(WIDTH * HEIGHT, 0);
    int next_task = 0;
    MPI_Status status;
    high_resolution_clock::time_point master_start_time = high_resolution_clock::now();

    for (int rank = 1; rank < num_procs; ++rank)
    {
        MPI_Send(&next_task, 1, MPI_INT, rank, 0, MPI_COMM_WORLD);
        next_task++;
    }

    for (int task = 0; task < WIDTH * HEIGHT; ++task)
    {
        int result[2];
        MPI_Recv(result, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        int rank = status.MPI_SOURCE;
        image[result[0]] = result[1];
        if (next_task < WIDTH * HEIGHT)
        {
            MPI_Send(&next_task, 1, MPI_INT, rank, 0, MPI_COMM_WORLD);
            next_task++;
        }
        else
        {
            int done_signal = -1;
            MPI_Send(&done_signal, 1, MPI_INT, rank, 0, MPI_COMM_WORLD);
        }
    }

    high_resolution_clock::time_point master_end_time = high_resolution_clock::now();
    duration<double> master_elapsed_time = duration_cast<duration<double>>(master_end_time - master_start_time);
    cout << "Total time taken by master: " << master_elapsed_time.count() << " seconds" << endl;

    save_image(image, "/home/kaloyants/Documents/spo/results/mandelbrot.ppm");
}

void worker(int rank)
{
    int task;
    int result[2];
    MPI_Status status;
    int task_count = 0;
    high_resolution_clock::time_point worker_start_time, worker_end_time;
    duration<double> worker_elapsed_time;
    double total_worker_time = 0;

    while (true)
    {
        worker_start_time = high_resolution_clock::now();
        MPI_Recv(&task, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        if (task == -1)
            break;
        task_count++;
        int x = task % WIDTH;
        int y = task / WIDTH;
        result[0] = task;
        result[1] = mandelbrot(x, y);
        MPI_Send(result, 2, MPI_INT, 0, 0, MPI_COMM_WORLD);
        worker_end_time = high_resolution_clock::now();
        worker_elapsed_time = duration_cast<duration<double>>(worker_end_time - worker_start_time);
        total_worker_time += worker_elapsed_time.count();
    }
    cout << "Worker " << rank << " solved " << (float)task_count/5/HEIGHT << " tasks." << endl;
    cout << "Total time taken by Worker " << rank << ": " << total_worker_time << " seconds" << endl;
}

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    high_resolution_clock::time_point start_time, end_time;
    duration<double> elapsed_time;

    start_time = high_resolution_clock::now();

    int rank, num_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    if (rank == 0)
        master(num_procs);
    else
        worker(rank);

    MPI_Finalize();

    end_time = high_resolution_clock::now();
    elapsed_time = duration_cast<duration<double>>(end_time - start_time);
    cout << "Total time taken by the program: " << elapsed_time.count() << " seconds" << endl;

    return 0;
}

