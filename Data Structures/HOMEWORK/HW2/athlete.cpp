// Athlete.cpp
// Implementation file for the Athlete Class
// Stores an athlete's name, country, and scores for particular events

#include <iostream>
#include <iomanip>
#include <ios>
#include <fstream>
#include <unordered_map>
#include <cmath>
#include <string>
#include "athlete.h"

// Default Constructor
Athlete::Athlete(){
    firstName_ = "FirstName";
    lastName_ = "LastName";
    country_ = "Country";
}

// Input Argument Constructor
Athlete::Athlete(std::string FirstName,
                 std::string LastName,
                 std::string Country,
                 std::string event,
                 float score)   {

    firstName_ = FirstName;
    lastName_ = LastName;
    country_ = Country;
    eventScores_[event] = score;
    total_ = 0;
}


// Public Methods
// ADD EVENT SCORE--------------------------------------------------
void Athlete::AddEventScore(std::string &event, float &score){
    eventScores_[event] = score;
}

// PRINT NAME ------------------------------------------------------
std::ofstream& Athlete::PrintName(std::ofstream &OutputStream) const {
    OutputStream << std::setw(16) << std::left;
    OutputStream << firstName_;

    OutputStream << std::setw(16) << std::left;
    OutputStream << lastName_;

    OutputStream << std::setw(3) << std::right;
    OutputStream << country_;
    return OutputStream;
}

// PRINT SCORES ----------------------------------------------------
std::ofstream& Athlete::PrintScores(std::ofstream &OutputStream,
                                    std::string &event,
                                    const int &colWidth,
                                    std::unordered_map<std::string, std::string> &map1500m,
                                    const std::string &outputType) {



    OutputStream << std::setw(colWidth) << std::right;

    if (outputType == "scores")
        OutputStream << std::setprecision(2) << std::fixed;

    if (event == "1500" && outputType == "scores"){
        OutputStream << map1500m[lastName_];

    } else {    // print an empty string if the event score is 0
        if (eventScores_[event] != 0)
            OutputStream << eventScores_[event];

        else if (eventScores_[event] == 0 && outputType == "scores")
            OutputStream << "";

        else if (eventScores_[event] == 0 && outputType == "points")
            OutputStream << "0";
    }
    return OutputStream;
}

// PRINT TOTAL ---------------------------------------------------------
std::ofstream& Athlete::PrintTotal(std::ofstream &OutputStream, const int &colWidth){
    OutputStream << std::setw(colWidth) << std::right << std::setprecision(4);
    OutputStream << total_;

    return OutputStream;

}

// GET FUNCTIONS ------------------------------------------------------
std::string Athlete::GetCountry() const {
    return country_;
}

std::string Athlete::GetLastName() const {
    return lastName_;
}

float Athlete::GetTotal() const {
    return total_;
}

// CONVERT TO POINTS ---------------------------------------------------
void Athlete::ConvertToPoints(const std::string &event){
    std::string type;
    float A, B, C;
    float score = eventScores_[event];

    if(event == "PV"){
            A = 0.2797;
            B = 100;
            C = 1.35;
            type = "field";
            score *= 100.0; // convert meters to centimeters

    } else if (event == "100") {
            A = 25.4347;
            B = 18;
            C = 1.81;
            type = "track";

    } else if (event == "LJ") {
            A = 0.14354;
            B = 220;
            C = 1.4;
            type = "field";
            score *= 100.0; // convert meters to centimeters

    } else if (event == "SP") {
            A = 51.39;
            B = 1.5;
            C = 1.05;
            type = "field";

    } else if (event == "HJ") {
            A = 0.8465;
            B = 75;
            C = 1.42;
            type = "field";
            score *= 100.0; // convert meters to centimeters

    } else if (event == "400") {
            A = 1.53775;
            B = 82;
            C = 1.81;
            type = "track";

    } else if (event == "110H") {
            A = 5.74352;
            B = 28.5;
            C = 1.92;
            type = "track";

    } else if (event == "DT") {
            A = 12.91;
            B = 4;
            C = 1.1;
            type = "field";

    } else if (event == "JT") {
            A = 10.14;
            B = 7;
            C = 1.08;
            type = "field";

    } else if (event == "1500") {
            A = 0.03768;
            B = 480;
            C = 1.85;
            type = "track";
    }


    // formulas to convert scores into points
    if (type == "track" && score != 0) {
        score = int (A * pow((B - score), C));

    } else if (type == "field" && score != 0){
        score = int (A * pow((score - B), C));
    }

    eventScores_[event] = score;
    total_ += score;
}






