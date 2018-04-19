#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>


#include "process.hpp"

// DISPLAY MEM POOL ==============================================================================
void display_mem_pool(std::vector<char> &mem_pool) {
    int rows = 8;
    int elements_per_row = MEM_POOL_SIZE / rows;

    // display delineator
    for (int i = 0; i < elements_per_row; i++)
        std::cout << '=';

    // display memory pool
    int i = 0;
    for (auto &pid : mem_pool) {
        if (i % elements_per_row == 0)
            std::cout << '\n';

        std::cout << pid;
        i++;
    }
    std::cout << '\n';

    // display delineator
    for (int i = 0; i < elements_per_row; i++)
        std::cout << '=';

    std::cout << std::endl;
}

// PARSE INPUT ===================================================================================
void parse_input(std::ifstream &InputStream, std::vector<Process> &processes) {

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

        arrival_times.clear();
        run_times.clear();

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

        processes.push_back(Process(pid, mem_frames, arrival_times.size(), arrival_times, run_times));
    }

    #ifdef DEBUG
        std::cerr << "Read from input file:\n";

        for (auto &proc : processes) {
            std::cerr << proc.getPID() << " " << proc.getNumFrames();

            for (size_t i = 0; i < proc.getNumBursts(); i++) {
                std::cerr << " " << proc.getArrTime(i) << "/" << proc.getRunTime(i);
            }
            std::cerr << std::endl;
        }
        std::cerr << "\n\n";
    #endif // DEBUG
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

    std::vector<Process> processes;
    parse_input(InputStream, processes);

    contiguous_memory_allocation(processes);




    return EXIT_SUCCESS;
}
