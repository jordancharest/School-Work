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


// SEARCH MEM POOL ===============================================================================
/*  Search the mem pool for the requested space according to the algorithm provided
    return -1 if it is only possible with defragmentation
    return -2 if it is not possible
    othrewise return the starting frame for the available space                                 */
int search_mem_pool(std::vector<char> &mem_pool, int frames_needed, std::string algorithm) {

    int total = 0;
    int smallest = MEM_POOL_SIZE+1;
    int largest = 0;

    int free_counter = 0;
    int start_frame = -1;

    int i = 0;
    while (i < MEM_POOL_SIZE) {

        // if we find a free frame
        if (mem_pool[i] == '.') {

            // count the size of the free block associated with that frame
            while (i < MEM_POOL_SIZE  &&  mem_pool[i] == '.') {
                total++;
                free_counter++;
                i++;
            }

            std::cout << "Found " << free_counter << " available frames in this block\n";

            // if we found a new best fit block
            if (algorithm == "best"  &&  free_counter < smallest  &&  free_counter >= frames_needed) {
                start_frame = i - free_counter;
                smallest = free_counter;

            // or we found a new worst fit block
            } else if (algorithm == "worst"  && free_counter > largest  && free_counter >= frames_needed) {
                start_frame = i - free_counter;
                largest = free_counter;
            }

            free_counter = 0;


        } else {
            free_counter = 0;
            i++;
        }
    }

    // defragmentation
    if (start_frame == -1  &&  total >= frames_needed)
        return -1;

    // not enough space
    else if (start_frame == -1  &&  total < frames_needed)
        return -2;

    // a suitable block was found
    return start_frame;
}


// PROCESS REMOVAL ===============================================================================
void process_removal(std::vector<char> &mem_pool, Process &proc, int t) {

    int start = proc.getStartingFrame();
    int frames = proc.getNumFrames();

    std::cout << "Removing " << frames << " starting from " << start << " for process " << proc.getPID() << "\n";
    for (int i = start; i < start + frames; i++ )
        mem_pool[i] = '.';

    std::cout << "time " << t << "ms: Process " << proc.getPID() << " removed:\n";
    display_mem_pool(mem_pool);
}


// NEXT FIT ======================================================================================
bool next_fit(std::vector<char> &mem_pool, Process &proc, int t, int* start_frame) {

    static int current_frame = 0;
    int frames_needed = proc.getNumFrames();
    char pid = proc.getPID();

    std::cout << "time " << t << "ms: Process " << pid << " arrived (requires " << frames_needed;
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
                    *start_frame = j;

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

    std::cout << "time " << t << "ms: Placed process " << pid << "\n";
    display_mem_pool(mem_pool);
    return true;
}


// BEST FIT ======================================================================================
bool best_fit(std::vector<char> &mem_pool, Process &proc, int t, int* start_frame) {

    char pid = proc.getPID();
    int frames_needed = proc.getNumFrames();

    std::cout << "time " << t << "ms: Process " << pid << " arrived (requires " << frames_needed;
    if (frames_needed > 1)  std::cout << " frames)\n";
    else                    std::cout << " frame)\n";

    // search the mem pool for <frames_needed> frames according to the best-fit paradigm
    *start_frame = search_mem_pool(mem_pool, frames_needed, "best");

    // search_mem_pool returns -1 if the mem_pool needs defragmentation
    if (*start_frame == -1) {
        defragmentation(mem_pool);
        return true;

    // -2 if the process cannot be placed
    } else if (*start_frame == -2) {
        std::cout << "time " << t << "ms: Cannot place process " << pid << " -- skipped!\n";
        return false;

    // else it returns the frame to start at
    } else {
        for (int i = *start_frame; i < *start_frame + frames_needed; i++) {
            mem_pool[i] = pid;
        }

        std::cout << "time " << t << "ms: Placed process " << pid << "\n";
        display_mem_pool(mem_pool);
        return true;
    }



}


// WORST FIT =====================================================================================
bool worst_fit(std::vector<char> &mem_pool, Process &proc, int t, int* start_frame) {





    return true;
}


// SIMULATOR =====================================================================================
void simulator(std::vector<Process> &processes, std::string algorithm) {

    int t = 0;
    int remaining_processes = processes.size();

    std::vector<char> mem_pool(MEM_POOL_SIZE, '.');

    // temporary buffers for when multiple processes are arriving/leaving at the same time
    std::vector<Process> arriving;
    std::vector<Process> leaving;

    std::cout << "time " << t << "ms: Simulator started (Contiguous -- " << algorithm << ")\n";

    while (remaining_processes > 0) {

        // check if any processes need to be removed (removal is the same for all algorithms)
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

                bool success;
                int start_frame;

                // attempt to place it; start_frame is guaranteed to be set if success == true
                if (algorithm == "Next-Fit")
                    success = next_fit(mem_pool, proc, t, &start_frame);
                else if (algorithm == "Best-Fit")
                    success = best_fit(mem_pool, proc, t, &start_frame);
                else if (algorithm == "Worst-Fit")
                    success = worst_fit(mem_pool, proc, t, &start_frame);

                if (success) {
                    proc.placed(t, start_frame);
                    std::cout << "Process " << proc.getPID() << " will finish at " << proc.getEndTime() << "\n";
                } else {
                    proc.skipped();
                    if (proc.finished())
                        remaining_processes--;
                }
            }
        }





        t++;
        if (t > 3000) {
            std::cerr << "Preventing infinite loop\n";
            break;
        }
    }

    std::cout << "time " << (t-1) << "ms: Simulator ended (Contiguous -- " << algorithm << ")\n";
}




// CONTIGUOUS MEMORY ALLOCATION ==================================================================
void contiguous_memory_allocation(std::vector<Process> &processes) {
    //simulator(processes, "Next-Fit");
    simulator(processes, "Best-Fit");
    //simulator(processes, "Worst-Fit");
}





