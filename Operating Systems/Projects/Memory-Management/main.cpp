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
    std::vector<int> arrival_times;
    std::vector<int> run_times;
    std::string line;


    int i = 0;
    char delimiter;

    while (std::getline(InputStream, line)) {
        std::cout << line << "\n";

        std::stringstream ss(line);

        i = 0;
        while (ss >> pid) {

            // commented line
            if (pid == '#')
                break;

            // first loop read the frames needed, after, it's just a loop of arr_time/run_time until newline
            if (i == 0) {
                ss >> mem_frames;
                std::cout << mem_frames;

            } else {
                ss >> arr_time
                   >> delimiter
                   >> run_time;

                std::cout << " " << arr_time << delimiter << run_time;

                arrival_times.push_back(arr_time);
                run_times.push_back(run_time);

            }
        }

        std::cout << "\n";

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
