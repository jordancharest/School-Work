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

