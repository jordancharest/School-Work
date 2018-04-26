#include <iostream>
#include <iomanip>
#include <vector>
#include <thread>
#include <atomic>
#include <shared_mutex>
#include <condition_variable>
#include <mutex>
#include <ctime>
#include <chrono>

#include "robot.hpp"

// GLOBAL - for access by all threads
static int world_size = 256;   // toroidal world
static int N = 1024;           // number of particles
static int L = 4;              // number of landmarks
static const double SENSOR_NOISE = 3.0;
static const double MOVE_NOISE = 0.08;
static const double ALLOWABLE = 0.4 * SENSOR_NOISE;
double mean_error = 100.0;
std::atomic<double> max_weight(-1.0);

static double forward_cmd;
static double turn_cmd;
static Robot parallel_robot(world_size);

std::vector<Point> landmarks(L);
static std::vector<Robot> particles(N);
static std::vector<double> measurements(L);

// random generator for particle initialization
std::default_random_engine generator;
std::uniform_real_distribution<double> uniform(0.0, 1.0);
std::normal_distribution<double> gaussian(0.0, SENSOR_NOISE);
std::normal_distribution<double> gaussian_move(0.0, MOVE_NOISE);

// to create a thread barrier for sychronization
pthread_barrier_t barrier;


// EVALUATE ======================================================================================
double evaluate(const auto &robot, const auto &particles, const int world_size) {
    double sum = 0.0;
    double dx, dy;

    for (auto &particle : particles) {
        dx = modulo((particle.x() - robot.x() + (world_size/2.0)), world_size) - (world_size/2.0);
        dy = modulo((particle.y() - robot.y() + (world_size/2.0)), world_size) - (world_size/2.0);
        sum += sqrt(dx*dx + dy*dy);
    }

    return sum / N;
}



// PARALLEL PARTICLE FILTER ======================================================================
double parallel_particle_filter(int num_threads, int index) {

    int particles_per_thread = N / num_threads;
    int particle_index = N / num_threads * index;

    std::cerr << "Thread " << index << " computing particles from " << particle_index << " to " << particle_index + particles_per_thread << "\n";
    #ifdef DEBUG
    if (index == 0) {
        for (auto &landmark : landmarks)
            std::cout << "Landmark: (" << landmark.x << ", " << landmark.y << ")\n";
    }
    #endif // DEBUG



    using namespace std::chrono;
    high_resolution_clock::time_point start;
    double total_time = 0.0;

    int t = 0;
    mean_error = 100.0;

    std::vector<double> loc;

    // simulate the robot moving about its environment until the solution converges
    while (mean_error > ALLOWABLE) {

        // only main will randomize the move command
        if (index == 0) {
            max_weight = -1.0;
            start = high_resolution_clock::now();

            // define a random movement
            forward_cmd = 1 + uniform(generator) * 5;
            if (uniform(generator) > 0.5)
                turn_cmd = uniform(generator) * 0.3;    // radians
            else
                turn_cmd = - uniform(generator) * 0.3;  // radians
        }

        // BARRIER
        pthread_barrier_wait(&barrier);

        // move the robot
        if (index == 0) {
            parallel_robot.move(forward_cmd, turn_cmd);
            parallel_robot.sense(measurements);
        }


        #ifdef DEBUG
        if (index == 0  &&  t < 3) {
            loc = parallel_robot.location();
            std::cout << "Robot is at (" << loc[0] << ", " << loc[1] << ", " << loc[2] << ")\n";
        }
        #endif // DEBUG

        #ifdef DEBUG
        if (index == 0  &&  t < 3) {
            std::cout << "\nLandmark sensor measurements:\n";
            for (auto measurement : measurements)
                std::cout << measurement << "\n";
        }
        #endif // DEBUG

        // simulate the same motion update for all particles
        for (int i = particle_index; i < particle_index + particles_per_thread; i++)
            particles[i].move(forward_cmd, turn_cmd);

        #if 0
        if (index == 0 && t == 0) {
            std::cout << "Particles are now at (x, y, theta):\n";
            for (auto &particle : particles) {
                std::cout << "(" << particle.x() << ", " << particle.y() << ")\n";
            }
        }
        #endif

        // calculate importance weights for all particles based on their locations, calculate the max on the way
        double p_max_weight = -1.0;
        for (int i = particle_index; i < particle_index + particles_per_thread; i++) {
            particles[i].measurement_prob(measurements);
            p_max_weight = std::max(p_max_weight, particles[i].weight());
        }

        if (p_max_weight > max_weight)
            max_weight = p_max_weight;

        #if 0
        if (index == 0) {
            std::cout << "\nImportance weights (max = " << max_weight << "):\n";
            for (auto &particle : particles)
                std::cout << particle.weight() << "\n";
        }
        #endif // DEBUG


        // BARRIER
        pthread_barrier_wait(&barrier);


        // NEEDS PARALLELIZATION
        if (index == 0) {
            // resample using the Resampling Wheel Algorithm
            double beta = 0.0;
            int index = uniform(generator) * N;
            std::vector<Robot> new_particles(particles.size());

            for (int i = 0; i < N; i++) {
                beta += uniform(generator) * 2.0 * max_weight;

                while (beta > particles[index].weight()) {
                    beta -= particles[index].weight();
                    index = (index + 1) % N;
                }

                new_particles[i] = particles[index];
            }

            particles.swap(new_particles);

            mean_error = evaluate(parallel_robot, particles, world_size);

            high_resolution_clock::time_point finish = high_resolution_clock::now();
            duration<double> span = duration_cast<duration<double> >(finish-start);
            total_time += span.count();
            std::cout << std::setw(6) << t << "  |     " << std::setw(10) << std::fixed << mean_error << "   |  " << span.count() << "\n";


        }

        // BARRIER
        pthread_barrier_wait(&barrier);

        t++;
    }

    std::cerr << "Thread " << index << " exiting\n";

    return total_time;
}


// PARALLEL INIT =================================================================================
void parallel(int num_threads) {

    using namespace std::chrono;
    //double total_time = 0.0;

    std::cout << "\nParallel Particle Filter - " << num_threads << " threads\n";
    std::cout << std::setw(6) << "time" << "  |  " << std::setw(12) << " Mean Error " << "  |  "
              << std::setw(10) << "Loop Time\n";


    std::vector<std::thread> threads(num_threads);
    for (int i = 1; i < num_threads; i++) {
        threads[i] = std::thread(parallel_particle_filter, num_threads, i);
    }

    parallel_particle_filter(num_threads, 0);


    for (int i = 1; i < num_threads; i++)
        threads[i].join();

    std::cout << "Solution Converged.\n";

}


// SERIAL PARTICLE FILTER ========================================================================
double serial_particle_filter(Robot &robot) {

    using namespace std::chrono;
    double total_time = 0.0;

    #ifdef DEBUG
    for (auto &landmark : landmarks)
        std::cout << "Landmark: (" << landmark.x << ", " << landmark.y << ")\n";
    #endif // DEBUG


    std::cout << "\nSerial Particle Filter\n";
    std::cout << std::setw(6) << "time" << "  |  " << std::setw(14) << " Mean Error " << "  |  "
              << std::setw(10) << "Loop Time\n";

    int t = 0;
    std::vector<double> loc;

    // simulate the robot moving about its environment until the solution converges
    while (mean_error > ALLOWABLE) {
        high_resolution_clock::time_point start = high_resolution_clock::now();

        // define a random movement
        forward_cmd = 1 + uniform(generator) * 5;
        if (uniform(generator) > 0.5)
            turn_cmd = uniform(generator) * 0.3;    // radians
        else
            turn_cmd = - uniform(generator) * 0.3;  // radians


        // move the robot
        robot.move(forward_cmd, turn_cmd);

        #ifdef DEBUG
        if (t < 3) {
            loc = robot.location();
            std::cout << "Robot is at (" << loc[0] << ", " << loc[1] << ", " << loc[2] << ")\n";
        }
        #endif // DEBUG



        // simulate the same motion update for all particles
        for (auto &particle : particles)
            particle.move(forward_cmd, turn_cmd);

        #if 0
        if (t == 0) {
            std::cout << "Particles are now at (x, y, theta):\n";
            for (auto &particle : particles) {
                std::cout << "(" << particle.x() << ", " << particle.y() << ")\n";
            }
        }
        #endif

        // take sensor measurements to all the landmarks
        robot.sense(measurements);

        #ifdef DEBUG
        if (t < 3) {
            std::cout << "\nLandmark sensor measurements:\n";
            for (auto measurement : measurements)
                std::cout << measurement << "\n";
        }
        #endif // DEBUG


        // calculate importance weights for all particles based on their locations, calculate the max on the way
        double max_weight = -1.0;
        for (auto &particle : particles) {
            particle.measurement_prob(measurements);
            max_weight = std::max(max_weight, particle.weight());
        }

        // resample using the Resampling Wheel Algorithm
        double beta = 0.0;
        int index = uniform(generator) * N;
        std::vector<Robot> new_particles(particles.size());

        for (int i = 0; i < N; i++) {
            beta += uniform(generator) * 2.0 * max_weight;

            while (beta > particles[index].weight()) {
                beta -= particles[index].weight();
                index = (index + 1) % N;
            }

            new_particles[i] = particles[index];
        }

        particles.swap(new_particles);

        // calculate the mean error
        mean_error = evaluate(robot, particles, world_size);

        // log
        high_resolution_clock::time_point finish = high_resolution_clock::now();
        duration<double> span = duration_cast<duration<double> >(finish-start);
        total_time += span.count();
        std::cout << std::setw(6) << t << "  |     " << std::setw(10) << std::fixed << mean_error << "   |  " << span.count() << "\n";

        t++;
    }

    std::cout << "Solution Converged. Average update time: " << total_time/t << std::endl;

    return total_time/t;
}



// INIT ==========================================================================================
void init(Robot &robot) {

    // Initialize L random landmarks
    Point landmark;
    for (int i = 0; i < L; i++) {
        landmark.x = uniform(generator) * world_size;
        landmark.y = uniform(generator) * world_size;
        landmarks[i] = landmark;
    }

    // Define noise levels on robot
    robot.setNoise(MOVE_NOISE, MOVE_NOISE, SENSOR_NOISE);

    // Initialize N random particles - particle locations are randomly generated in the constructor
    for (int i = 0; i < N; i++) {
        Robot particle(world_size);
        particle.setNoise(MOVE_NOISE, MOVE_NOISE, SENSOR_NOISE);
        particles[i] = particle;
    }
}


// MAIN ==========================================================================================
int main(int argc, char** argv) {

    if (argc != 2) {
        std::cerr << "ERROR: invalid argument(s)\n";
        std::cerr << "USAGE: " << argv[0] << " <num-threads>\n";
        exit(EXIT_FAILURE);
    }

    int num_threads = atoi(argv[1]);
    if (num_threads < 2 || num_threads > 32) {
        std::cerr << "ERROR: invalid argument(s)\n";
        std::cerr << "Number of threads must be between 2 and 32 inclusive\n";
        exit(EXIT_FAILURE);

    }

    // Initialize pthread barrier to block all threads in a process
	pthread_barrier_init(&barrier, NULL, num_threads);


    // seed for random particle generation
    int seed = time(0);
    std::vector<double> loc;

    // Initialize the robot to a random location and define noise levels; start the serial simulation
    generator.seed(seed);
    Robot serial_robot(world_size);
    loc = serial_robot.location();
    std::cout << "Robot starts at (" << loc[0] << ", " << loc[1] << ", " << loc[2] << ")\n";
    init(serial_robot);
    serial_particle_filter(serial_robot);

    // reset the world to the exact same configuration and rerun in parallel
    generator.seed(seed);
    Robot temp(world_size);
    parallel_robot = temp;
    loc = parallel_robot.location();
    std::cout << "Robot starts at (" << loc[0] << ", " << loc[1] << ", " << loc[2] << ")\n";
    init(parallel_robot);
    parallel(num_threads);



    return EXIT_SUCCESS;
}
