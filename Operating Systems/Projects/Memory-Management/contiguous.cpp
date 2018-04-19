#include <iostream>
#include <vector>
#include <algorithm>


#include "process.hpp"

// ARRIVAL SORT ==================================================================================
bool arrival_sort(Process &a, Process &b) {
    return a.getPID() < b.getPID();
}

// DEFRAGMENTATION ===============================================================================
void defragmentation(std::vector<char> &mem_pool) {
    std::cout << "DEFRAGMENTATION\n";


}


// NEXT FIT PLACEMENT ============================================================================
void next_fit_placement(std::vector<char> &mem_pool, Process &proc, int t) {
    int frames_needed = proc.getNumFrames();
    char pid = proc.getPID();

    std::cout << "Process " << pid << " arrived (requires " << frames_needed;
    if (frames_needed > 1)  std::cout << " frames)\n";
    else                    std::cout << " frame)\n";

    bool placed = false;
    int free_counter = 0;
    int total = 0;


    // search for free space
    for (int i = 0; i < MEM_POOL_SIZE; i++) {

        // check if the frame is free
        if (mem_pool[i] == '.') {
           free_counter++;
           total++;

           // if we find a block large enough, place the process in that block
            if (free_counter == frames_needed) {
                int j = i + 1 - free_counter;

                // fill in from the starting point to the current position
                for (; j <= i; j++) {
                    mem_pool[j] = pid;
                }

                placed = true;
            }

        } else
            free_counter = 0;

    }

    if (!placed) {
        if (total >= frames_needed)
            defragmentation(mem_pool);
        else
            std::cout << "time " << t << "ms: Cannot place process " << pid << " -- skipped!\n";
            //proc.skip();
    } else {
        std::cout << "time " << t << "ms: Placed process " << proc.getPID() << "\n";
        display_mem_pool(mem_pool);
    }
}


// NEXT FIT ALGORITHM ============================================================================
void next_fit(std::vector<Process> &processes) {

    int t = 0;
    int remaining_processes = processes.size();

    std::vector<char> mem_pool(MEM_POOL_SIZE, '.');
    display_mem_pool(mem_pool);

    std::vector<Process> arriving;

    std::cout << "time " << t << "ms: Simulator started (Contiguous -- Next-Fit)\n";
    std::cout << "Last process: " << processes[processes.size()-1].getPID() << "\n";

    while (remaining_processes > 0) {

        // TODO: check if any processes need to be removed

        // check for arrival of any processes
        for (auto &proc : processes) {

            // if a process is arriving at the current time
            if (proc.getArrTime(proc.getCurrentBurst()) == t) {
                std::cout << "Process " << proc.getPID() << " is arriving\n";
                arriving.push_back(proc);
            }

            // if the last process has been checked, sort the processes by tiebreaker rules and place them in memory
            // this is safe because process PIDs are always unique
            if (proc.getPID() == processes[processes.size()-1].getPID()) {
                std::sort(arriving.begin(), arriving.end(), arrival_sort);

                for (auto &proc : arriving) {
                    next_fit_placement(mem_pool, proc, t);
                }

                arriving.clear();
            }


        }






        t++;
        if (t > 2000) {
            std::cerr << "Preventing infinite loop\n";
            break;
        }
    }


}




// CONTIGUOUS MEMORY ALLOCATION ==================================================================
void contiguous_memory_allocation(std::vector<Process> &processes) {

    next_fit(processes);






}





