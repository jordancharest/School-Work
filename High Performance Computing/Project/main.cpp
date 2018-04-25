#include <iostream>
#include <vector>
#include <thread>

#include "robot.hpp"

// GLOBAL
int N = 10;    // number of particles
int L = 4;     // number of landmarks
double sensor_noise = 1.0;

// random generator for particle initialization
std::default_random_engine generator;
std::uniform_real_distribution<double> uniform(0.0, 1.0);
std::normal_distribution<double> gaussian(0.0, sensor_noise);

std::vector<Point> landmarks(L);



// PARTICLE FILTER ===============================================================================
void particle_filter(Robot &robot, std::vector<Robot> &particles, std::vector<Point> &landmarks, double sensor_noise) {

    int t = 0;
    double allowable = 0.33 * sensor_noise;
    double mean_error = 100.0;

    double forward_cmd;
    double turn_cmd;

    std::vector<double> loc;
    std::vector<double> measurements(landmarks.size());

    // simulate the robot moving about its environment until the solution converges
    while (mean_error > allowable) {

        loc = robot.location();

        // define a random movement
        forward_cmd = 1 + uniform(generator) * 5;
        if (uniform(generator) > 0.5)
            turn_cmd = uniform(generator) * 0.3;    // radians
        else
            turn_cmd = - uniform(generator) * 0.3;  // radians


        // move the robot
        robot.move(forward_cmd, turn_cmd);
        loc = robot.location();
        std::cout << "(x, y, theta) --> (" << loc[0] << ", " << loc[1] << ", " << loc[2] << ")\n";

        // simulate the same motion update for all particles
        for (auto particle : particles)
            particle.move(forward_cmd, turn_cmd);

        // take sensor measurements to all the landmarks
        robot.sense(measurements);

        // calculate importance weights for all particles based on the accuracy of their measurements
        for (auto particle : particles) {



        }

        #ifdef DEBUG
            for (auto measurement : measurements)
                std::cout << measurement << "\n";
        #endif // DEBUG


        t++;
        break;
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

    // Initialize L random landmarks
    Point landmark;
    for (int i = 0; i < L; i++) {
        landmark.x = uniform(generator) * world_size;
        landmark.y = uniform(generator) * world_size;
        landmarks[i] = landmark;
    }
    #ifdef DEBUG
        for (auto &landmark : landmarks) {
            std::cout << "Landmark: (" << landmark.x << ", " << landmark.y << ")\n";
        }
    #endif



    Robot robot(world_size);
    robot.setNoise(0.0, 0.0, sensor_noise);

    // Initialize N random particles
    std::vector<Robot> particles;
    for (int i = 0; i < N; i++) {
        Robot particle(world_size);
        particle.setNoise(0.0, 0.0, sensor_noise);
        particles.push_back(particle);
    }

    #ifdef DEBUG
        for (auto &particle : particles) {
            std::cout << "Particle initialized to: (" << particle.x() << ", " << particle.y() << ")\n";
        }
    #endif

    particle_filter(robot, particles, landmarks, sensor_noise);



    return EXIT_SUCCESS;
}
