#include <iostream>
#include <vector>

#include "robot.hpp"

std::default_random_engine generator;
std::uniform_real_distribution<double> distribution(0.0, 1.0);


// MAIN ==========================================================================================
int main(int argc, char** argv) {

    // seed for random particle generation
    int seed;
    if (argc == 2) seed = atoi(argv[1]);
    else seed = 42;

    generator.seed(seed);



    int N = 10;    // number of particles
    int world_size = 10;   // circular world
    std::vector<Point> landmarks;

    // Initialize N random particles
    std::vector<Robot> particles;
    for (int i = 0; i < N; i++) {
        particles.push_back(Robot(world_size, landmarks));
        //std::cout << "Particle initialized to: (" << particles[i].x() << ", " << particles[i].y() << ")\n";
    }



    return EXIT_SUCCESS;
}
