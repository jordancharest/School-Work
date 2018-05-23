// time.cpp
// Implementation file for the Time class
// Stores the hour, minute, and second

#include <string>
#include<iostream>
#include "time.h"

// Default Constructor
Time::Time(){
    hour_ = 0;
    minute_ = 0;
    second_ = 0;
}

// Value Constructor
Time::Time(int Hour, int Minute, int Second){
    hour_ = Hour;
    minute_ = Minute;
    second_ = Second;
}


// PUBLIC METHODS -----------------------------------------------------
// Accessors
int Time::getHour() const{
    return hour_;
}

int Time::getMinute() const{
    return minute_;
}

int Time::getSecond() const {
    return second_;
}


// Modifiers
void Time::setHour(int Hour){
    hour_ = Hour;
};

void Time::setMinute(int Minute){
    minute_ = Minute;
};

void Time::setSecond(int Second){
    second_ = Second;
};


// Output
void Time::PrintAMPM(){
    std::string qualifier = " am";

    // check for AM or PM
    if (hour_ == 24){
        hour_ -= 12;
    } else if (hour_ > 12){
        hour_ -= 12;
        qualifier = " pm";
    } else if (hour_ == 12){
        qualifier = " pm";
    }

    // handle the case of midnight to 1 AM
    if (hour_ == 0)
        std::cout << "12" << ":";
    else
        std::cout << hour_ << ":";

    // print a leading zero in front of minutes and
    // seconds if they are only one digit
    if (minute_ < 10)
        std::cout << "0";

    std::cout << minute_ << ":";

    if (second_ < 10)
        std::cout << "0";

    std::cout << second_ << qualifier << "\n";

}
