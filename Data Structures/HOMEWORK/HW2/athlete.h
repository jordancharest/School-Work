// athlete.h
// Header file with the declaration of the Athlete class, including
// member functions and private member variables
#ifndef __athlete_h_
#define __athlete_h_


#include <unordered_map>
#include <string>
#include <fstream>
#include <vector>

class Athlete {
public:
    Athlete();
    Athlete(std::string FirstName, std::string LastName, std::string Country, std::string event, float score);

    // Accessors and Output
    std::ofstream& PrintName(std::ofstream &OutputStream) const;
    std::ofstream& PrintScores(std::ofstream &OutputStream, std::string &event, const int &colWidth, std::unordered_map<std::string, std::string> &map1500m, const std::string &outputType);
    std::ofstream& PrintTotal(std::ofstream &OutputStream, const int &colWidth);
    std::string GetCountry() const;
    std::string GetLastName() const;
    float GetTotal() const;

    // Modifiers
    void AddEventScore(std::string &event, float &score);
    void ConvertToPoints(const std::string &event);


    // Calculation Methods


private:
    std::string firstName_;
    std::string lastName_;
    std::string country_;
    std::string scoreString_;
    std::unordered_map<std::string, float> eventScores_;
    float total_;

};


#endif // __student_h_
