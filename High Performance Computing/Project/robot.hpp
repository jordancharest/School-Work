#ifndef ROBOT_HPP_INCLUDED
#define ROBOT_HPP_INCLUDED

#include <vector>
#include <random>
#include <cmath>
#include <functional>

// random generator for particle initialization
extern std::default_random_engine generator;
extern std::uniform_real_distribution<double> uniform;
extern std::normal_distribution<double> gaussian;

// GLOBAL
typedef struct {
    double x, y;
} Point;

extern std::vector<Point> landmarks;


// CLASS ROBOT ===================================================================================
class Robot {
public:

    // Constructor
    Robot(int _size) : world_size(_size) {}

    // Accessors
    const double x() const { return X; }
    const double y() const { return Y; }
    const std::vector<double> location() const {
        std::vector<double> location = {X, Y, orientation};
        return location;
    }

    // Calculations
    void sense(std::vector<double> &measurements);

    // Modifiers
    void setNoise(double FN, double TN, double SN);
    void move(double forward_cmd, double turn_cmd);



private:
    int world_size;
    double X = world_size * uniform(generator);
    double Y = world_size * uniform(generator);
    double orientation = 2 * M_PI * uniform(generator);
    double forward_noise = 0.0;
    double turn_noise = 0.0;
    double sense_noise = 0.0;



};


#endif // ROBOT_HPP_INCLUDED
