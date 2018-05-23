#include <iostream>
#include <ios>
#include <iomanip>
#include <fstream>
#include <cstdlib>
#include <algorithm>
#include <vector>
#include <string>
#include <unordered_map>
#include "athlete.h"

// ABBREVIATE EVENTS ---------------------------------------------------------------
void Abbreviate (std::string &event, std::vector<std::string> &eventList){

    // This entire function shouldn't exist, but the autograder gives me a 0 without it. yay.
    int index = 10;

    if (event == "POLE_VAULT"){
        event = "PV";
        index = 7;

    } else if (event == "100_METERS"){
        event = "100";
        index = 0;

    } else if (event == "LONG_JUMP"){
        event = "LJ";
        index = 1;

    } else if (event == "HIGH_JUMP"){
        event = "HJ";
        index = 3;

    } else if (event == "400_METERS"){
        event = "400";
        index = 4;

    } else if (event == "110_METERS_HURDLES"){
        event = "110H";
        index = 5;

    } else if (event == "DISCUS_THROW"){
        event = "DT";
        index = 6;

    } else if (event == "JAVELIN_THROW"){
        event = "JT";
        index = 8;

    } else if (event == "SHOT_PUT"){
        event = "SP";
        index = 2;

    } else if (event == "1500_METERS"){
        event = "1500";
        index = 9;

    }

    eventList[index] = event;

}



// CONVERT TIME --------------------------------------------------------------------
float convertTime (const std::string &unformattedTime, float &score){
    // parse the string and convert to seconds
    int minutes = 0;
    int i = 0;
    std::string number;

    // check the string one character at a time until ":" to find the total minutes
    while (unformattedTime.substr(i,1) != ":"){

        number = unformattedTime.substr(i,1);
        minutes = minutes*10 + std::stof(number);
        i++;
    }

    // convert the minutes to seconds;
    // take the rest of the string following the ":" and add to the total
    // score is now in seconds
    std::string seconds = unformattedTime.substr(i+1);
    score = minutes*60 + std::stof(seconds);

    return score;
}

// READ FILE ----------------------------------------------------------------------
void ReadFile (std::string &file,
               std::vector<Athlete> &athleteList,
               std::vector<std::string> &eventList,
               std::string outputType,
               std::unordered_map<std::string, std::string> &map1500m){

    std::ifstream InputStream(file.c_str());

    // ensure the stream was opened correctly
    if (!InputStream.good()){
        std::cerr << "Can't open" << file << ". Exiting program.\n";
        exit(1);
    }

    std::string word;
    std::string event;
    std::string firstName;
    std::string lastName;
    std::string name;
    std::string country;
    std::unordered_map<std::string, int> nameMap;
    std::string scoreString;
    float score;
    int arrayIndex = 0;

    //start reading the file
    while (InputStream >> word){

        if (word == "event"){   // if "event" is read, then the next word is the name of the event
            InputStream >> event;

            // Must output events in stupid abbreviated format and in pointless order
            Abbreviate(event, eventList);

        } else {       // read in the athlete first and last names and concatenate to make one name
            firstName = word;
            InputStream >> lastName;
            name = firstName + " " + lastName;

            InputStream >> country >> scoreString;

            // save the 1500 meters time separately because it can't be converted to a float
            if (outputType == "scores" && event == "1500")
                map1500m[lastName] = scoreString;


            if ((outputType == "points" || outputType == "custom" )&& event == "1500")
                score = convertTime(scoreString, score);

            else
                score = std::stof(scoreString);



            // place the names in a map so we can quickly check whether we have already encountered the name
            if (!nameMap.count(name)){

                // assign the value of the name to the array index that the athlete object will be placed at
                nameMap[name] = arrayIndex;
                arrayIndex++;

                Athlete tempAthlete(firstName, lastName, country, event, score);
                athleteList.push_back(tempAthlete);


            } else {    // else the athlete already exists in our vector, find the index and add event score to the athlete at that index
                athleteList[nameMap[name]].AddEventScore(event, score);
            }
        }
    }
}


// PRINT EVENTS --------------------------------------------------------------------------------
std::ofstream& PrintEvents (const std::vector<std::string> &eventList, std::ofstream &OutputStream, const int &colWidth){

    for (const std::string &event : eventList){

        OutputStream << std::setw(colWidth) << std::right;
        OutputStream << event;
    }

    return OutputStream;
}

// WRITE FILE -----------------------------------------------------------------------------------
void WriteFile(std::string &file,
               std::vector<Athlete> &athleteList,
               std::vector<std::string> &eventList,
               const std::string &outputType,
               std::unordered_map<std::string, std::string> map1500m,
               std::unordered_map<std::string, float> countryTotals){

    std::ofstream OutputStream(file.c_str());

    // ensure the stream was opened correctly
    if (!OutputStream.good()){
        std::cerr << "Can't open" << file << ". Exiting program.\n";
        exit(1);
    }

    OutputStream << std::setw(35) << std::left;
    int colWidth = 10;

    // print the headers
    if (outputType == "scores"){
        OutputStream << "DECATHLETE SCORES";
        PrintEvents(eventList, OutputStream, colWidth);
        OutputStream << "\n";


    } else if (outputType == "points" || outputType == "custom"){
        OutputStream << "DECATHLETE POINTS";
        PrintEvents(eventList, OutputStream, colWidth);
        OutputStream << std::setw(colWidth) << std::right;
        OutputStream << "TOTAL" << "\n";
    }

    // print the scores for each athlete, for each event
    for (Athlete &person : athleteList){
        person.PrintName(OutputStream);
        for (std::string &event : eventList){
            person.PrintScores(OutputStream, event, colWidth, map1500m, outputType);
        }

        // don't print for "scores" because there is no total
        if (outputType == "points" || outputType == "custom")
            person.PrintTotal(OutputStream, colWidth);

        OutputStream << "\n";
    }

    // print the total score for each country
    if (outputType == "custom"){
        OutputStream << "\nCOUNTRY TOTALS" << "\n";

        typedef std::unordered_map <std::string, float>::iterator mapIterator;
        for (mapIterator it = countryTotals.begin(); it != countryTotals.end(); it++){
            OutputStream << it->first << "  " << std::setprecision(5) << it->second;
            OutputStream << "\n";
        }
    }
}

// COUNTRY COMPARATOR ---------------------------------------------------------
bool CountryComparator (const Athlete &a, const Athlete &b){
    return ((a.GetCountry() < b.GetCountry()) ||
            ((a.GetCountry() == b.GetCountry()) && a.GetLastName() < b.GetLastName()));
}

// POINTS COMPARATOR ----------------------------------------------------------
bool PointsComparator (const Athlete &a, const Athlete &b){
    return (a.GetTotal() > b.GetTotal());
}


// MAIN -----------------------------------------------------------------------
int main (int argc, char *argv[]){

    // ensure the program was called with the correct number of arguments
    if (argc != 4){
        std::cerr << "ERROR: Incorrect number of arguments";
        return 1;
    }

    std::string outputType = argv[3];

    std::vector<Athlete> athleteList;           // all info for athletes
    std::string inputFile = argv[1];
    std::vector<std::string> eventList (10);
    std::unordered_map<std::string, std::string> map1500m;
    std::unordered_map<std::string, float> countryTotals;


    ReadFile(inputFile, athleteList, eventList, outputType, map1500m);


    // sort the athletes alphabetically by country, then last name
    if (outputType == "scores"){
        std::sort(athleteList.begin(), athleteList.end(), CountryComparator);

    // sort the athletes by total points
    } else if (outputType == "points" || outputType == "custom"){

        // first need to convert the scores to points based on the given formula
        for (Athlete &person : athleteList){
            for (std::string &event : eventList){
                person.ConvertToPoints(event);
            }
        }

        std::sort(athleteList.begin(), athleteList.end(), PointsComparator);
    }

    if (outputType == "custom") {
        std::string country;
        for (Athlete &person : athleteList){
            country = person.GetCountry();

            // for each country, add the athlete's total to the country total
            if (!countryTotals.count(country)){
                countryTotals[country] = person.GetTotal();
            } else {
                countryTotals[country] += person.GetTotal();
            }
        }
    }

    std::string outputFile = argv[2];

    WriteFile(outputFile, athleteList, eventList, outputType, map1500m, countryTotals);
}
