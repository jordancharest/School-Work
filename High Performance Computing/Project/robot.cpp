#include "robot.hpp"

// SET NOISE =====================================================================================
void Robot::setNoise(double FN, double TN, double SN) {
    forward_noise = FN;
    turn_noise = TN;
    sense_noise = SN;
}
