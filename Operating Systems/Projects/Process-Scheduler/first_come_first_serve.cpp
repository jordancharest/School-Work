#include <iostream>
#include <vector>
#include <list>
#include <climits>
#include <algorithm>
#include "process.hpp"


// FIRST COME FIRST SERVE ========================================================================
/* Control function for first come first serve simulator                                        */
stat_t First_Come_First_Serve(std::vector<Process> &processes) {

    stat_t stats;
    stats.num_preemptions = 0;
    stats.algorithm = "FCFS";

    std::list<Process> ready_queue;
    std::cout <<  "time 0ms: Simulator started for FCFS " << queue_contents(ready_queue) << "\n";

    // Build the initial ready queue, processes are ordered by arrival time
    int time = processes[0].getArrivalTime();
    int next = 0;

    // check for all processes that will arrive before the first process can start running
    for (int i = 0; i < (T_CS/2); i++) {
        while (processes[next].getArrivalTime() == time + i  &&  next < total_processes) {
            process_arrival(ready_queue, processes[next], time+i);
            next++;
        }
    }

    // start the first process
    Process running = ready_queue.front();
    process_start(ready_queue, running, time+T_CS/2);   // allow time for the half context switch on the first process

    int context_counter = 0;
    int CPU_available = 0;
    int total_burst_time = 0;
    std::list<Process> IO_blocked;

    while (next < total_processes || ready_queue.size() > 0  || IO_blocked.size() > 0  ||  running.getStatus() == Status::RUNNING) {

        // check if a new process should be started
        if (running.getStatus() != Status::RUNNING  &&  ready_queue.size() > 0) {
            context_counter++;

            if (time >= CPU_available  && context_counter >= T_CS/2) {
                running = ready_queue.front();
                stats.avg_wait_time += (time - running.getReadyTime() - T_CS/2);
                context_counter = 0;

                process_start(ready_queue, running, time);
            }
        }

        // check if the current running process is done using the CPU
        if (running.getStatus() == Status::RUNNING  &&  running.endBurstTime() == time) {
            total_burst_time += (time - running.getStartTime());
            calculate_turnaround(&stats, running, time);
            process_finished_burst(ready_queue, IO_blocked, running, &CPU_available, &stats, time);
        }

        // check if any process is done IO
        if (IO_blocked.size() != 0) {
            if (IO_blocked.front().endIOTime() == time) {
                process_finished_IO(ready_queue, IO_blocked, time, &stats);
            }
        }

        // check if any processes are arriving
        if (next < total_processes  &&  processes[next].getArrivalTime() == time) {
            process_arrival(ready_queue, processes[next], time);
            next++;
        }

        time++;
    }

    // Calculate statistics
    int total_bursts = 0;
    for (auto &proc : processes) {
        stats.avg_burst_time += (proc.getTotalBursts() * proc.getBurstLength());
        total_bursts += proc.getTotalBursts();
    }
    stats.avg_burst_time /= total_bursts;
    stats.avg_turnaround_time /= total_bursts;
    stats.avg_turnaround_time += T_CS/2;
    stats.avg_wait_time /= total_bursts;

    time += (T_CS/2 - 1);   // allow time for context switch
    std::cout << "time " << time << "ms: Simulator ended for FCFS\n" << std::endl;

    return stats;
}