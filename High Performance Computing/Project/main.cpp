#include <iostream>
#include <iomanip>
#include <vector>
#include <thread>
#include <ctime>

#include "robot.hpp"

// GLOBAL
int N = 500;    // number of particles
int L = 4;     // number of landmarks
double sensor_noise = 3.0;
double move_noise = 0.08;

// random generator for particle initialization
std::default_random_engine generator;
std::uniform_real_distribution<double> uniform(0.0, 1.0);
std::normal_distribution<double> gaussian(0.0, sensor_noise);
std::normal_distribution<double> gaussian_move(0.0, move_noise);

std::vector<Point> landmarks(L);
// EVALUATE ======================================================================================
double evaluate(const auto &robot, const auto &particles, const int world_size) {
    double sum = 0.0;

    for (auto &particle : particles) {
        double dx = modulo((particle.x() - robot.x() + (world_size/2.0)), world_size) - (world_size/2.0);
        double dy = modulo((particle.y() - robot.y() + (world_size/2.0)), world_size) - (world_size/2.0);
        sum += sqrt(dx*dx + dy*dy);
    }

    return sum / N;
}


// PARTICLE FILTER ===============================================================================
void particle_filter(Robot &robot, std::vector<Robot> &particles, std::vector<Point> &landmarks,
                      const int world_size) {

    std::cout << "Serial Particle Filter\n";
    std::cout << std::setw(6) << "time" << "  |  " << std::setw(10) << "Mean Error\n";

    int t = 0;
    double allowable = 0.33 * sensor_noise;
    double mean_error = 100.0;

    double forward_cmd;
    double turn_cmd;

    std::vector<double> loc;
    std::vector<double> measurements(landmarks.size());

    // simulate the robot moving about its environment until the solution converges
    while (mean_error > allowable) {

        // define a random movement
        forward_cmd = 1 + uniform(generator) * 5;
        if (uniform(generator) > 0.5)
            turn_cmd = uniform(generator) * 0.3;    // radians
        else
            turn_cmd = - uniform(generator) * 0.3;  // radians


        // move the robot
        robot.move(forward_cmd, turn_cmd);
        loc = robot.location();
        //std::cout << "ROBOT(after): (x, y, theta) --> (" << loc[0] << ", " << loc[1] << ", " << loc[2] << ")\n";

        // simulate the same motion update for all particles
        for (auto &particle : particles)
            particle.move(forward_cmd, turn_cmd);

        #if 0
            std::cout << "Particles are now at (x, y, theta):\n";
            for (auto &particle : particles) {
                std::cout << "(" << particle.x() << ", " << particle.y() << ")\n";
            }
        #endif

        // take sensor measurements to all the landmarks
        robot.sense(measurements);

        #if 0
            std::cout << "\nLandmark sensor measurements:\n";
            for (auto measurement : measurements)
                std::cout << measurement << "\n";
        #endif // DEBUG

        // calculate importance weights for all particles based on their locations, calculate the max on the way
        double max_weight = -1.0;
        for (auto &particle : particles) {
            particle.measurement_prob(measurements);
            max_weight = std::max(max_weight, particle.weight());
            //std::cout << "Weight: " << particle.weight() << "\n";
        }

        #if 0
            std::cout << "\nImportance weights (max = " << max_weight << "):\n";
            for (auto &particle : particles)
                std::cout << particle.weight() << "\n";
        #endif // DEBUG

        // resample using the Resampling Wheel Algorithm
        double beta = 0.0;
        int index = uniform(generator) * N;
        std::vector<Robot> new_particles(particles.size());

        for (int i = 0; i < N; i++) {
            beta += uniform(generator) * 2.0 * max_weight;
            //std::cout << beta << "\n";

            while (beta > particles[index].weight()) {
                beta -= particles[index].weight();
                index = (index + 1) % N;
            }

            new_particles[i] = particles[index];
        }

        particles.swap(new_particles);

        #if 0
            std::cout << "\nParticles after resampling (x, y, theta):\n";
            for (auto &particle : particles)
                std::cout << "(" << particle.x() << ", " << particle.y() << ") -- size:" << particle.size() << "\n";
        #endif


        mean_error = evaluate(robot, particles, world_size);
        std::cout << std::setw(6) << t << "  |  " << mean_error << "\n";

        t++;
        //if (t == 100)
            //break;
    }





}

// MAIN ==========================================================================================
int main(int argc, char** argv) {

    // seed for random particle generation
    int seed;
    if (argc == 2) seed = atoi(argv[1]);
    else seed = time(0);

    generator.seed(seed);

    int world_size = 100;   // circular world

    // Initialize L random landmarks
    Point landmark;
    for (int i = 0; i < L; i++) {
        landmark.x = uniform(generator) * world_size;
        landmark.y = uniform(generator) * world_size;
        landmarks[i] = landmark;
    }
    #if 0
        for (auto &landmark : landmarks) {
            std::cout << "Landmark: (" << landmark.x << ", " << landmark.y << ")\n";
        }
    #endif


    // Initialize the robot to a random location
    Robot robot(world_size);
    robot.setNoise(move_noise, move_noise, sensor_noise);

    // Initialize N random particles
    std::vector<Robot> particles;
    for (int i = 0; i < N; i++) {
        Robot particle(world_size);
        particle.setNoise(move_noise, move_noise, sensor_noise);
        particles.push_back(particle);
    }

    #if 0
    std::cout << "Particles are initialized to (x, y, theta):\n";
        for (auto &particle : particles) {
            std::cout << "(" << particle.x() << ", " << particle.y() << ")\n";
        }
    #endif

    particle_filter(robot, particles, landmarks, world_size);



    return EXIT_SUCCESS;
}
