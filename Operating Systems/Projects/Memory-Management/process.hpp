#ifndef PROCESS_HPP_INCLUDED
#define PROCESS_HPP_INCLUDED

#include <vector>




class Process {
public:
    // Constructor
    Process();
    Process(char _pid, int _frames, int _bursts, std::vector<int> AT, std::vector<int> RT)
            : pid(_pid), frames(_frames), bursts(_bursts), arrival_times(AT), run_times(RT) {}

    // Accessors
    const char getPID() const { return pid; }
    const unsigned int getNumFrames() const { return frames; }
    const unsigned int getNumBursts() const { return bursts; }
    const int getArrTime(unsigned int index) const { return arrival_times.at(index); }
    const int getRunTime(unsigned int index) const { return run_times.at(index); }

private:
    char pid;
    unsigned int frames;
    unsigned int bursts;
    std::vector<int> arrival_times;
    std::vector<int> run_times;
    int current_burst = 0;


};



#endif // PROCESS_HPP_INCLUDED
