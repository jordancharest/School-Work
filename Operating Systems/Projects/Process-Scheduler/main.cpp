#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <list>
#include <cstdlib>
#include <cstdio>
#include "process.h"

int total_processes;


// PARSE INPUT ===================================================================================
/* Read all the processes from a formatted input file
    # Commented lines start with '#'
    # Processes are formatted as below (including delimiters):
    <proc-id>|<initial-arrival-time>|<cpu-burst-time>|<num-bursts>|<io-time>                    */

void parse_input(std::ifstream &InputStream, std::vector<Process> &processes) {

    char pid;
    int arrival_time;
    int burst_time;
    int num_bursts;
    int io_time;
    char delimiter;
    int i = 0;

    while (InputStream >> pid) {
        // line is commented, ignore until newline is found
        if (pid == '#') {
                InputStream.ignore(1000, '\n');
        } else {
            InputStream >> delimiter
                        >> arrival_time
                        >> delimiter
                        >> burst_time
                        >> delimiter
                        >> num_bursts
                        >> delimiter
                        >> io_time;

            processes.push_back( Process(pid, arrival_time, burst_time, num_bursts, io_time) );

            #ifdef DEBUG_MODE
                std::cout   << processes[i].getPID()
                            << delimiter
                            << processes[i].getArrivalTime()
                            << delimiter
                            << processes[i].getBurstTime()
                            << delimiter
                            << processes[i].getNumBursts()
                            << delimiter
                            << processes[i].getIOTime()
                            << std::endl;
            #endif // DEBUG_MODE
            i++;
        }
    }
    total_processes = i;
}

// MAIN ==========================================================================================
int main(int argc, char* argv[]){

    std::ifstream InputStream(argv[1]);
    if (!InputStream.good()){
        std::cerr << "Can't open " << argv[1] << " to read.\n";
        return EXIT_FAILURE;
    }

    std::vector<Process> processes;
    parse_input(InputStream, processes);
    InputStream.close();

    // Run three different scheduling simulations
    First_Come_First_Serve(processes);
    Shortest_Remaining_Time(processes);
    Round_Robin(processes);


    return EXIT_SUCCESS;
}
