#include <iostream>
#include <vector>
#include <list>
#include <climits>
#include <algorithm>
#include "process.h"


// FIRST COME FIRST SERVE ========================================================================
/* Control function for first come first serve simulator                                        */
void First_Come_First_Serve(std::vector<Process> &processes) {

    std::list<Process> ready_queue;
    std::cout <<  "time 0ms: Simulator started for FCFS " << queue_contents(ready_queue) << "\n";

    // Build the initial ready queue, processes are ordered by arrival time
    long time = processes[0].getArrivalTime();
    int next = 0;

    // check for all processes that will arrive before the first process can start running
    for (int i = 0; i < (T_CS/2); i++) {
        while (processes[next].getArrivalTime() == time + i ) {
            process_arrival(ready_queue, processes[next], time+i);
            next++;
        }
    }

    // start the first process
    Process running = ready_queue.front();
    process_start(ready_queue, running, time+T_CS/2);   // allow time for the hald context switch on the first process

    long soonest_event;
    int context_counter = 0;
    bool context_switch = false;
    std::list<Process> IO_blocked;

    while (next < total_processes || ready_queue.size() > 0 || IO_blocked.size() > 0) {
        soonest_event = LONG_MAX;
        //std::cout << "\ntime " << time << "ms\n";
        //std::cout << processes[next].getPID();

        if (context_switch) {
            context_counter++;

            // once the context switch is completed, start the next process
            if (context_counter == T_CS) {
                context_switch = false;
                context_counter = 0;
                running = ready_queue.front();
                process_start(ready_queue, running, time);
            }
        }

        // check if any processes are arriving or when the next will arrive
        if (processes[next].getArrivalTime() == time) {
            process_arrival(ready_queue, processes[next], time);
            next++;
        }
        if (next < total_processes) {
            soonest_event = std::min(soonest_event, processes[next].getArrivalTime());
        }

        // check if the current running process is done using the CPU
        if (running.endBurstTime() == time) {
            running.decrementNumBursts();

            if (running.getNumBursts() == 0) {
                process_termination(ready_queue, running, time);
                context_switch = true;
            } else {
                process_block(ready_queue, IO_blocked, running, time);
            }
        } else {
            soonest_event = std::min(soonest_event, running.endBurstTime());
        }

        // check if any process is done IO
        if (IO_blocked.front().endIOTime() == time) {
            process_finished_IO(ready_queue, IO_blocked, time);
        }
        soonest_event = std::min(soonest_event, IO_blocked.front().endIOTime());


        // during a context switch, only increment by 1ms
        if (context_switch)
            time++;
        // else skip ahead to the next event
        else
            time = soonest_event;
    }




    std::cout << std::endl;
}
