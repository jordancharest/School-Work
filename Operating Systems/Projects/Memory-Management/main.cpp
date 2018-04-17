#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>


#include "process.hpp"



// PARSE INPUT ===================================================================================
void parse_input(std::ifstream &InputStream) {

    char pid;
    int mem_frames;
    int arr_time;
    int run_time;
    char delimiter;
    std::vector<int> arrival_times;
    std::vector<int> run_times;
    std::string line;

    // pull a whole line from the input file
    while (std::getline(InputStream, line)) {

        // Ignore commented lines
        if (line[0] == '#') {
            continue;
        }

        std::stringstream ss(line);

        // first two values are the process ID and the memory frames needed
        ss >> pid >> mem_frames;

        // Then read an indeterminate number of arrival_time/run_time
        while (ss >> arr_time) {
            ss >> delimiter
               >> run_time;

            arrival_times.push_back(arr_time);
            run_times.push_back(run_time);
        }
    }
}


// MAIN ==========================================================================================
int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "ERROR: Invalid arguments\n"
                  << "USAGE: " << argv[0] << "<sim-input-file>\n";

        return EXIT_FAILURE;
    }

    std::ifstream InputStream(argv[1]);
    if (!InputStream.good()){
        std::cerr << "ERROR: Invalid input file format\n";
        return EXIT_FAILURE;
    }

    parse_input(InputStream);





    return EXIT_SUCCESS;
}
