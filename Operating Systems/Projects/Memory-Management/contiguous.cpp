#include <iostream>
#include <vector>
#include <algorithm>


#include "process.hpp"

int defrag_time = 0;


// PID SORT ======================================================================================
bool pid_sort(Process &a, Process &b) {
    return a.getPID() < b.getPID();
}


// DEFRAG UPDATE =================================================================================
/* Update the process parameters (end times, start frames, etc.) to account for
    the changes made during defragmentation                                                     */
void defrag_update(std::vector<char> &mem_pool, std::vector<Process> &processes) {

    char prev_frame = '.';
    for (int i = 0; i < MEM_POOL_SIZE; i++) {

        // a new process start was detected
        if (mem_pool[i] != prev_frame) {
            for (auto &proc : processes) {

                // update the start frame of that process
                if (proc.getPID() == mem_pool[i]) {
                    proc.updateStartFrame(i);
                }
            }
        }

        prev_frame = mem_pool[i];
    }


}
// DEFRAGMENTATION ===============================================================================
/* Push all processes to the top of memory to make room at the bottom                           */
void defragmentation(std::vector<char> &mem_pool, Process &proc, int &t, int &start_frame) {
    char pid = proc.getPID();
    std::cout << "time " << t << "ms: Cannot place process " << pid << " -- starting defragmentation\n";

    int total_frames = 0;
    //int total_processes = 0;
    //std::vector<char> processes_moved;
    int start_move = 0;
    int return_loc = 0;
    int free_counter = 0;
    char cache;

    int i = 0;
    while (i < MEM_POOL_SIZE) {
        if (mem_pool[i] == '.') {
            // restart the search from here after moving some frames
            return_loc = i;

            // count the size of the free block
            while (i < MEM_POOL_SIZE  &&  mem_pool[i] == '.') {
                free_counter++;
                i++;
            }

            // no more frames to move
            if (i == MEM_POOL_SIZE){
                i = return_loc;
                break;
            }

            start_move = i;

            // move the memory
            while (i < MEM_POOL_SIZE  &&  i < (start_move+free_counter-1)) {
                cache = mem_pool[i];   // cache the memory in that frame
                mem_pool[i] = '.';      // erase the frame
                mem_pool[i-free_counter] = cache;   // write the cached frame to the new location
                i++;

                // sometimes we may be 'moving' empty frames; don't count them in the total frames moved
                if (cache != '.')
                    total_frames++;
            }

            i = return_loc;
            free_counter = 0;

        } else
            i++;
    }

    start_frame = i;

    // place the new process in memory
    for (; i < start_frame + proc.getNumFrames(); i++)
        mem_pool[i] = pid;

    // update the global clock
    t += (total_frames * T_MEMMOVE);
    defrag_time += (total_frames * T_MEMMOVE);
    std::cout << "time " << t << "ms: Defragmentation complete (moved " << total_frames << " frames\n";

    std::cout << "time " << t << "ms: Placed process " << pid << ":\n";
    display_mem_pool(mem_pool);
}


// SEARCH MEM POOL ===============================================================================
/*  Search the mem pool for the requested space according to the algorithm provided
    return -1 if it is only possible with defragmentation
    return -2 if it is not possible
    otherwise return the starting frame for the available space                                 */
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


        } else
            i++;
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

    for (int i = start; i < start + frames; i++ )
        mem_pool[i] = '.';

    std::cout << "time " << t << "ms: Process " << proc.getPID() << " removed:\n";
    display_mem_pool(mem_pool);
}


// NEXT FIT ======================================================================================
bool next_fit(std::vector<char> &mem_pool, std::vector<Process> &processes, Process &proc, int &t, int &start_frame) {

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
                start_frame = j;

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
                    start_frame = j;

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
            defragmentation(mem_pool, proc, t, start_frame);
            defrag_update(mem_pool, processes);
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
bool best_or_worst_fit(std::string algorithm, std::vector<char> &mem_pool, std::vector<Process> &processes,
                       Process &proc, int &t, int &start_frame) {

    char pid = proc.getPID();
    int frames_needed = proc.getNumFrames();

    std::cout << "time " << t << "ms: Process " << pid << " arrived (requires " << frames_needed;
    if (frames_needed > 1)  std::cout << " frames)\n";
    else                    std::cout << " frame)\n";

    // search the mem pool for <frames_needed> frames according to the best-fit paradigm
    start_frame = search_mem_pool(mem_pool, frames_needed, algorithm);

    // search_mem_pool returns -1 if the mem_pool needs defragmentation
    if (start_frame == -1) {
        defragmentation(mem_pool, proc, t, start_frame);
        defrag_update(mem_pool, processes);
        return true;

    // -2 if the process cannot be placed
    } else if (start_frame == -2) {
        std::cout << "time " << t << "ms: Cannot place process " << pid << " -- skipped!\n";
        return false;

    // else it returns the frame to start at
    } else {
        for (int i = start_frame; i < start_frame + frames_needed; i++) {
            mem_pool[i] = pid;
        }

        std::cout << "time " << t << "ms: Placed process " << pid << "\n";
        display_mem_pool(mem_pool);
        return true;
    }
}

// SIMULATOR =====================================================================================
void simulator(std::vector<Process> processes, std::string algorithm) {

    int t = 0;
    defrag_time = 0;
    int remaining_processes = processes.size();

    std::vector<char> mem_pool(MEM_POOL_SIZE, '.');

    // temporary buffers for when multiple processes are arriving/leaving at the same time
    std::vector<Process> arriving;
    std::vector<Process> leaving;

    std::cout << "time " << t << "ms: Simulator started (Contiguous -- " << algorithm << ")\n";

    while (remaining_processes > 0) {

        // check if any processes need to be removed (removal is the same for all algorithms)
        for (auto &proc : processes) {

            if (!proc.finished()  &&  proc.getEndTime() == (t - defrag_time)) {
                process_removal(mem_pool, proc, t);
                proc.removed();

                if (proc.finished())
                    remaining_processes--;
            }

        }


        // check for arrival of any processes
        for (auto &proc : processes) {

            // if a process is arriving at the current time
            if (!proc.finished()  &&  proc.getArrTime( proc.getCurrentBurst() ) == (t - defrag_time)) {

                bool success;
                int start_frame;

                // attempt to place it; start_frame is guaranteed to be set if success == true
                // t may be changed if defragmentation occurred
                if (algorithm == "Next-Fit")
                    success = next_fit(mem_pool, processes, proc, t, start_frame);

                else if (algorithm == "Best-Fit")
                    success = best_or_worst_fit("best", mem_pool, processes, proc, t, start_frame);

                else if (algorithm == "Worst-Fit")
                    success = best_or_worst_fit("worst", mem_pool, processes, proc, t, start_frame);


                if (success) {
                    proc.placed(t, start_frame);
                } else {
                    proc.skipped();
                    if (proc.finished())
                        remaining_processes--;
                }
            }
        }





        t++;
        if (t > 10000) {
            std::cerr << "Preventing infinite loop\n";
            break;
        }
    }

    std::cout << "time " << (t-1) << "ms: Simulator ended (Contiguous -- " << algorithm << ")\n\n";
}




// CONTIGUOUS MEMORY ALLOCATION ==================================================================
void contiguous_memory_allocation(std::vector<Process> &processes) {
    simulator(processes, "Next-Fit");
    simulator(processes, "Best-Fit");
    simulator(processes, "Worst-Fit");
}





