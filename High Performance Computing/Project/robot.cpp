//#include <boost/range/combine.hpp>    // Most people probably aren't using C++17
#include <stdexcept>
#include <cmath>

#include "robot.hpp"
// GAUSSIAN PROBABILITY ==========================================================================
inline double gaussian_probability(double mu, double sigma, double x) {
    return exp(- (pow(mu - x, 2)) / (pow(sigma, 2) * 2.0)) / sqrt(2.0 * M_PI * pow(sigma, 2));
}


// MODULO ========================================================================================
/* C++ '%' doesn't work the way I want with negative numbers                                    */
inline double modulo (double numerator, double denominator) {
	return (fmod((fmod(numerator, denominator) + denominator), denominator));
}


// SET NOISE =====================================================================================
void Robot::setNoise(double FN, double TN, double SN) {
    _forward_noise = FN;
    _turn_noise = TN;
    _sense_noise = SN;
}


// MOVE ==========================================================================================
void Robot::move(double forward_cmd, double turn_cmd) {
    if (forward_cmd < 0)
        throw std::invalid_argument("Robot cannot move backwards");

    _orientation += turn_cmd;
    _orientation = modulo(_orientation, 2 * M_PI);

    _x += modulo(forward_cmd * std::cos(_orientation), _world_size);    // cyclic truncation
    _y += modulo(forward_cmd * std::sin(_orientation), _world_size);    // cyclic truncation
}


// SENSE =========================================================================================
void Robot::sense(std::vector<double> &measurements) {

    /* Most people probably aren't using C++17
    for (auto const& [landmark, measurement] : boost::combine(landmarks, measurements)) {
        measurement = sqrt(pow(X-landmark.x, 2) + pow(Y-landmark.y, 2));
        measurement += gaussian(generator);
    }
    */

    for (size_t i = 0; i < landmarks.size(); i++) {
        measurements[i] = sqrt(pow(_x - landmarks[i].x, 2) + pow(_y - landmarks[i].y, 2));
        measurements[i] += gaussian(generator);
    }
}

// MEASUREMENT PROBABILITY =======================================================================
/* Calculate how likely a list of measurements are, given a particles current location
    and the vector of sensor measurements taken by the robot                                    */
void Robot::measurement_prob(std::vector<double> const& measurements) {
    _weight = 1.0;
    double distance;
    for (size_t i = 0; i < measurements.size(); i++) {
        distance = sqrt(pow(_x - landmarks[i].x, 2) + pow(_y - landmarks[i].y, 2));
        _weight *= gaussian_probability(distance, _sense_noise, measurements[i]);
    }
}
