#include <iostream>
#include <vector>
#include <algorithm>


#include "process.hpp"

// PID SORT ======================================================================================
bool pid_sort(Process &a, Process &b) {
    return a.getPID() < b.getPID();
}

// DEFRAGMENTATION ===============================================================================
void defragmentation(std::vector<char> &mem_pool) {
    std::cout << "DEFRAGMENTATION\n";


}


// PROCESS REMOVAL ===============================================================================
void process_removal(std::vector<char> &mem_pool, Process &proc, int t) {

    int start = proc.getStartingFrame();
    int frames = proc.getNumFrames();

    for (int i = start; i < start + frames; i++ )
        mem_pool[i] = '.';

    std::cout << "time " << t << "ms: Process " << proc.getPID() << " removed:\n";
    display_mem_pool(mem_pool);
}

// NEXT FIT PLACEMENT ============================================================================
bool next_fit_placement(std::vector<char> &mem_pool, Process &proc, int t) {

    static int current_frame = 0;
    std::cout << "Current frame: " << current_frame << "\n";
    int frames_needed = proc.getNumFrames();
    char pid = proc.getPID();

    std::cerr << "time " << t << "ms: Process " << pid << " arrived (requires " << frames_needed;
    if (frames_needed > 1)  std::cout << " frames)\n";
    else                    std::cout << " frame)\n";

    bool placed = false;
    int free_counter = 0;
    int total = 0;


    // search for free space from the last placed frame to the end
    for (int i = current_frame; i < MEM_POOL_SIZE; i++) {

        // check if the frame is free
        if (mem_pool[i] == '.') {
           free_counter++;
           total++;

           // if we find a block large enough, place the process in that block
            if (free_counter == frames_needed) {
                int j = i + 1 - free_counter;
                proc.setStartingFrame(j);

                // fill in from the starting point to the current position
                for (; j <= i; j++)
                    mem_pool[j] = pid;

                current_frame = (i+1) % MEM_POOL_SIZE;
                placed = true;
                break;
            }

        } else
            free_counter = 0;
    }


    free_counter = 0;
    if (!placed) {
        // then search from the beginning of memory to the current frame
        for (int i = 0; i < current_frame; i++) {

            // check if the frame is free
            if (mem_pool[i] == '.') {
                free_counter++;
                total++;

                // if we find a block large enough, place the process in that block
                if (free_counter == frames_needed) {
                    int j = i + 1 - free_counter;
                    proc.setStartingFrame(j);

                    // fill in from the starting point to the current position
                    for (; j <= i; j++)
                        mem_pool[j] = pid;

                    current_frame = (i+1) % MEM_POOL_SIZE;
                    placed = true;
                    break;
                }

            } else
                free_counter = 0;
        }
    }

    if (!placed) {
        if (total >= frames_needed) {
            defragmentation(mem_pool);
            return true;

       } else {
            std::cout << "time " << t << "ms: Cannot place process " << pid << " -- skipped!\n";
            return false;
       }
    }

    std::cout << "time " << t << "ms: Placed process " << proc.getPID() << "\n";
    display_mem_pool(mem_pool);
    return true;
}


// NEXT FIT ALGORITHM ============================================================================
void next_fit(std::vector<Process> &processes) {

    int t = 0;
    int remaining_processes = processes.size();

    std::vector<char> mem_pool(MEM_POOL_SIZE, '.');
    display_mem_pool(mem_pool);

    // temporary buffers for when multiple processes are arriving/leaving at the same time
    std::vector<Process> arriving;
    std::vector<Process> leaving;

    std::cout << "time " << t << "ms: Simulator started (Contiguous -- Next-Fit)\n";

    while (remaining_processes > 0) {

        // check if any processes need to be removed
        for (auto &proc : processes) {

            if (!proc.finished()  &&  proc.getEndTime() == t) {
                process_removal(mem_pool, proc, t);
                proc.removed();

                if (proc.finished())
                    remaining_processes--;
            }

        }


        // check for arrival of any processes
        for (auto &proc : processes) {

            // if a process is arriving at the current time
            if (!proc.finished()  &&  proc.getArrTime( proc.getCurrentBurst() ) == t) {

                // attempt to place it
                if ( next_fit_placement(mem_pool, proc, t) ) {
                    proc.placed(t);
                    std::cout << "Process will finish at " << proc.getEndTime() << "\n";
                } else {
                    proc.skipped();
                    if (proc.finished())
                        remaining_processes--;
                }
            }
        }





        t++;
        if (t > 4000) {
            std::cerr << "Preventing infinite loop\n";
            break;
        }
    }

    std::cout << "Simulator finished\n";

}




// CONTIGUOUS MEMORY ALLOCATION ==================================================================
void contiguous_memory_allocation(std::vector<Process> &processes) {

    next_fit(processes);






}





