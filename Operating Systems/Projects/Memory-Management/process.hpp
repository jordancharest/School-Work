#ifndef PROCESS_HPP_INCLUDED
#define PROCESS_HPP_INCLUDED

#include <vector>

// Global ----------------------------------------------------------------------------------------
const int MEM_POOL_SIZE = 256;
const int T_MEMMOVE = 1;


// PROCESS CLASS =================================================================================
class Process {
public:
    // Constructor
    Process();
    Process(char _pid, int _frames, int _bursts, std::vector<int> AT, std::vector<int> RT)
            : pid(_pid), frames(_frames), bursts(_bursts), arrival_times(AT), run_times(RT) {}

    // Accessors
    const char getPID() const { return pid; }
    const int getNumFrames() const { return frames; }
    const int getNumBursts() const { return bursts; }
    const int getCurrentBurst() const { return current_burst; }
    const int getStartingFrame() const { return starting_frame; }
    const int getArrTime(size_t index) const { return arrival_times.at(index); }
    const int getRunTime(size_t index) const { return run_times.at(index); }
    const int getEndTime() const { return end_time; }
    const bool finished() const { return current_burst == bursts; }

    // Modifiers
    void setStartingFrame(int frame) { starting_frame = frame; }
    void skipped() { current_burst++; }
    void removed() { current_burst++; }

    void placed(int time, int frame) {
        end_time = time + run_times[current_burst];
        starting_frame = frame;
    }


private:
    char pid;
    int frames;
    int bursts;
    std::vector<int> arrival_times;
    std::vector<int> run_times;
    int current_burst = 0;
    int starting_frame = 0;
    int end_time = -1;
};



// Function Prototype ----------------------------------------------------------------------------
void display_mem_pool(std::vector<char> &mem_pool);
void contiguous_memory_allocation(std::vector<Process> &processes);



#endif // PROCESS_HPP_INCLUDED
