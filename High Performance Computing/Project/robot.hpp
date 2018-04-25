#ifndef ROBOT_HPP_INCLUDED
#define ROBOT_HPP_INCLUDED

#include <vector>
#include <random>
#include <cmath>

// random generator for particle initialization
extern std::default_random_engine generator;
extern std::uniform_real_distribution<double> distribution;


typedef struct {
    int x, y;
} Point;


class Robot {
public:

    // Constructor
    Robot(int _size, std::vector<Point> _landmarks) : world_size(_size), landmarks(_landmarks) {}

    // Accessors
    const double x() const { return X; }
    const double y() const { return Y; }
    const std::vector<double> location() const {
        std::vector<double> location = {X, Y, orientation};
        return location;
    }

    // Calculations
    void sense() {}

    // Modifiers
    void setNoise(double FN, double TN, double SN);
    void move(double forward_cmd, double turn_cmd);



private:
    int world_size;
    std::vector<Point> landmarks;
    //std::vector<double> distances;
    double X = world_size * distribution(generator);
    double Y = world_size * distribution(generator);
    double orientation = 2 * M_PI * distribution(generator);
    double forward_noise = 0.0;
    double turn_noise = 0.0;
    double sense_noise = 0.0;



};


#endif // ROBOT_HPP_INCLUDED
