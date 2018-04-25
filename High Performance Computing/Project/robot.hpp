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
    const double x() const { return _x; }
    const double y() const { return _y; }
    const double weight() const { return _weight; }
    const std::vector<double> location() const {
        std::vector<double> location = {_x, _y, _orientation};
        return location;
    }

    // Calculations
    void sense(std::vector<double> &measurements);

    // Modifiers
    void setNoise(double FN, double TN, double SN);
    void move(double forward_cmd, double turn_cmd);
    void measurement_prob(const std::vector<double> &measurements);



private:
    int world_size;
    double _x = world_size * uniform(generator);
    double _y = world_size * uniform(generator);
    double _orientation = 2 * M_PI * uniform(generator);
    double _forward_noise = 0.0;
    double _turn_noise = 0.0;
    double _sense_noise = 0.0;
    double _weight = 0.0;    // importance weight; calculated after robot sensor updates
};


#endif // ROBOT_HPP_INCLUDED
