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
// EVALUATE ======================================================================================
double evaluate(const auto &robot, const auto &particles, const int world_size) {
    double sum = 0.0;

    for (auto &particle : particles) {
        double dx = modulo((particle.x() - robot.x() + (world_size/2.0)), world_size) - (world_size/2.0);
        double dy = modulo((particle.y() - robot.y() + (world_size/2.0)), world_size) - (world_size/2.0);
        sum += sqrt(dx*dx + dy*dy);
    }

    return sum / particles.size();
}


// PARTICLE FILTER ===============================================================================
void particle_filter(Robot &robot, std::vector<Robot> &particles, std::vector<Point> &landmarks,
                      const int world_size) {

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
        std::cout << "ROBOT: (x, y, theta) --> (" << loc[0] << ", " << loc[1] << ", " << loc[2] << ")\n";

        // simulate the same motion update for all particles
        for (auto &particle : particles)
            particle.move(forward_cmd, turn_cmd);

        #ifdef DEBUG
            std::cout << "Particles are now at (x, y, theta):\n";
            for (auto &particle : particles) {
                std::cout << "(" << particle.x() << ", " << particle.y() << ")\n";
            }
        #endif

        // take sensor measurements to all the landmarks
        robot.sense(measurements);

        #ifdef DEBUG
            std::cout << "\nLandmark sensor measurements:\n";
            for (auto measurement : measurements)
                std::cout << measurement << "\n";
        #endif // DEBUG

        // calculate importance weights for all particles based on their locations, calculate the max on the way
        double max_weight = -1.0;
        for (auto &particle : particles) {
            particle.measurement_prob(measurements);
            max_weight = std::max(max_weight, particle.weight());
        }

        #ifdef DEBUG
            std::cout << "\nImportance weights (max = " << max_weight << "):\n";
            for (auto particle : particles)
                std::cout << particle.weight() << "\n";
        #endif // DEBUG

        // resample using the Resampling Wheel Algorithm
        double beta = 0.0;
        int index = uniform(generator) * N;
        std::vector<Robot> new_particles(particles.size());

        for (int i = 0; i < N; i++) {
            beta += uniform(generator) * 2 * max_weight;

            while (beta > particles[index].weight()) {
                beta -= particles[index].weight();
                index = (index + 1) % N;
            }

            new_particles[i] = particles[index];
        }

        particles.swap(new_particles);

        #ifdef DEBUG
            std::cout << "\nParticles after resampling (x, y, theta):\n";
            for (auto &particle : particles)
                std::cout << "(" << particle.x() << ", " << particle.y() << ") -- size:" << particle.size() << "\n";
        #endif


        mean_error = evaluate(robot, particles, world_size);

        #ifdef DEBUG
            std::cout << "Mean Error: " << mean_error << "\n\n";
        #endif // DEBUG




        t++;
        //if (t == 10)
          //  break;
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


    // Initialize the robot to a random location
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
    std::cout << "Particles are initialized to (x, y, theta):\n";
        for (auto &particle : particles) {
            std::cout << "(" << particle.x() << ", " << particle.y() << ")\n";
        }
    #endif

    particle_filter(robot, particles, landmarks, world_size);



    return EXIT_SUCCESS;
}
