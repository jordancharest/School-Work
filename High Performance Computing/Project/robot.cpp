//#include <boost/range/combine.hpp>    // Most people probably aren't using C++17
#include <stdexcept>
#include <cmath>

#include "robot.hpp"

// MODULO ========================================================================================
/* C++ '%' doesn't work the way I want with negative numbers                                    */
inline double modulo (double numerator, double denominator) {
	return (fmod((fmod(numerator, denominator) + denominator), denominator));
}


// SET NOISE =====================================================================================
void Robot::setNoise(double FN, double TN, double SN) {
    forward_noise = FN;
    turn_noise = TN;
    sense_noise = SN;
}


// MOVE ==========================================================================================
void Robot::move(double forward_cmd, double turn_cmd) {
    if (forward_cmd < 0)
        throw std::invalid_argument("Robot cannot move backwards");

    orientation += turn_cmd;
    orientation = modulo(orientation, 2 * M_PI);

    X += forward_cmd * std::cos(orientation);
    Y += forward_cmd * std::sin(orientation);
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
        measurements[i] = sqrt(pow(X-landmarks[i].x, 2) + pow(Y-landmarks[i].y, 2));
        measurements[i] += gaussian(generator);
    }
}
