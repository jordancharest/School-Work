#include <iostream>
#include <vector>
#include <thread>

#include "robot.hpp"

// random generator for particle initialization
std::default_random_engine generator;
std::uniform_real_distribution<double> distribution(0.0, 1.0);


// GLOBAL
int N = 10;    // number of particles

// PARTICLE FILTER ===============================================================================
void particle_filter(Robot &robot, std::vector<Robot> &particles, double sensor_noise) {

    int t = 0;
    double allowable = 0.33 * sensor_noise;
    double mean_error = 100.0;

    // simulate the robot moving about its environment until the solution converges
    while (mean_error > allowable) {




    }





}

// MAIN ==========================================================================================
int main(int argc, char** argv) {

    // seed for random particle generation
    int seed;
    if (argc == 2) seed = atoi(argv[1]);
    else seed = 42;

    generator.seed(seed);


    int world_size = 10;   // circular world
    double sensor_noise = 3.0;
    std::vector<Point> landmarks;

    Robot robot(world_size, landmarks);
    robot.setNoise(0.0, 0.0, sensor_noise);

    // Initialize N random particles
    std::vector<Robot> particles;
    for (int i = 0; i < N; i++) {
        Robot particle(world_size, landmarks);
        particle.setNoise(0.0, 0.0, sensor_noise);
        particles.push_back(Robot(world_size, landmarks));
    }

    #ifdef DEBUG
        for (auto &particle : particles) {
            std::cout << "Particle initialized to: (" << particle.x() << ", " << particle.y() << ")\n";
        }
    #endif

    particle_filter(robot, particles, sensor_noise);



    return EXIT_SUCCESS;
}
