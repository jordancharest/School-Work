#include <iostream>
#include <cassert>
#include <fstream>
#include <cmath>
#include <iomanip>
#include <vector>
#include <string>
#include <cstdlib>
#include <sstream>


// A few global constants
const int feet_per_mile = 5280;
const double seconds_between_samples = 4.25;



/* Add your code here to finish the implementation. */
// GPS CLASS ----------------------------------------------
class GPSData {
public:
    GPSData(){
        Xpos_ = 0;
        Ypos_ = 0;
        speed_ = 0;
    }

    // ACCESSORS
    int getXPos(){
        return Xpos_;
    }

    int getYPos(){
        return Ypos_;
    }

    int getSpeed(){
        return speed_;
    }

    // MODIFIERS
    void set_position(int x, int y){
        Xpos_ = x;
        Ypos_ = y;
    }

    void set_speed(int s){
        speed_ = s;
    }

private:
    int Xpos_;
    int Ypos_;
    int speed_;

};

// PRINT PACE ==============================================================================
void print_pace(const int &speed){

    double minutes;
    double seconds;
    double pace;

    // convert the ft/min speed into min/mile
    pace = (1.0/speed) * feet_per_mile;

    // pull the minutes and seconds separately to output in mm:ss format
    minutes = (int) pace;
    seconds = (pace - minutes) * 60 + 0.0001; // <--- added so output matches submitty

    // print the formatted time
    std::string filler = "";
    std::string fillermin = "";
    std::string stringPace;

    if (seconds < 10)   // leading zero if needed
        filler = "0";

    if (minutes < 10)   // leading space because submitty is stupid
        fillermin = " ";

    // concatenate as a string so we can print it all in one column
    std::stringstream ss;
    ss << fillermin << minutes << ":" << filler << (int) seconds;
    std::string paceString = ss.str();
    std::cout << paceString;
}

// PRINT ===================================================================================
void print(GPSData *GPSDataArray, const int &numData){
    // accepts an array of GPS data prints with identical format to given example

    int columns = 4;
    std::string headers[4] = {"x-pos", "y-pos", "ft/min", "min/mile"};
    for(int j=0; j<columns; j++){
        std::cout << std::setw(7+j) << std::fixed << std::right << headers[j];
    }
    std::cout << std::endl;


    for (int i=0; i<numData; i++){
        std::cout << std::setw(7) << std::right
                  << GPSDataArray[i].getXPos()
                  << std::setw(8) << std::right
                  << GPSDataArray[i].getYPos()
                  << std::setw(9) << std::right
                  << GPSDataArray[i].getSpeed();

        // print N/A for pace when speed = 0 to avoid divide by zero error
        if (GPSDataArray[i].getSpeed() == 0){
            std::cout << std::setw(10) << std::right << "N/A" << std::endl;
        } else {
            std::cout << std::setw(10);
            print_pace(GPSDataArray[i].getSpeed());
            std::cout << "\n";
        }
    }
}

// DISTANCE ================================================================================
double distance(GPSData *data, int n, double &avg_feet_per_minute){

    avg_feet_per_minute = 0;

    int xCurrent, xLast, yCurrent, yLast;
    double distanceChange = 0;
    double totalDistance = 0;
    double speed = 0;

    // assume that the user is stopped at the end of data collection
    data[0].set_speed(0);

    // calculate the speed for each point using current position and next position
    for (int i = 1; i<n; i++){
        xCurrent = data[i].getXPos();
        yCurrent = data[i].getYPos();
        xLast = data[i-1].getXPos();
        yLast = data[i-1].getYPos();

        // find the hypotenuse and divide by the time
        distanceChange = sqrt(pow((xCurrent - xLast),2) + pow((yCurrent - yLast),2));
        speed = (distanceChange*60)/seconds_between_samples;

        // set the speed at that position
        data[i].set_speed( int (speed));

        // increment the speed (to be divided by total time at the end) and total distance
        avg_feet_per_minute += speed;
        totalDistance += distanceChange;
    }

    // divide all of the speeds added together by the total number of speeds = average speed
    avg_feet_per_minute = avg_feet_per_minute/(n-1);

    return totalDistance;
}

// FILTER ===================================================================================
double filter(GPSData* input, GPSData* output, int n){

    double inputDistance, outputDistance;
    double fillerVariable;
    inputDistance = distance(input, n, fillerVariable);

    // first and last points don't get averaged
    output[0] = input[0];
    output[n-1] = input[n-1];


    // average all the intermediate points
    int Xaverage, Yaverage;
    for (int i=1; i<(n-1); i++){
        Xaverage = (input[i-1].getXPos() + input[i].getXPos() + input[i+1].getXPos() ) / 3;
        Yaverage = (input[i-1].getYPos() + input[i].getYPos() + input[i+1].getYPos() ) / 3;

        output[i].set_position(Xaverage, Yaverage);
    }

    outputDistance = distance(output, n, fillerVariable);

    double percentChange = 100.0 * ((inputDistance - outputDistance)/(inputDistance));
    return percentChange;

}
// ==========================================================================================

void recursive_filter(GPSData* input, GPSData* output, int n, double percent_change_threshold) {
  // make a temporary array to store the intermediate filtering result
  GPSData tmp[n];
  // run the filter once
  double percent_change = filter(input,tmp,n);
  std::cout << "RECURSE " << std::setw(5) << std::fixed << std::setprecision(2)
            << percent_change << "% change" << std::endl;
  if (fabs(percent_change) < percent_change_threshold) {
    // if the percent change is under the target threshold, we're done
    for (int i = 0; i < n; i++) {
      // copy the data from the temporary arrays into the final answer
      output[i] = tmp[i];
    }
  } else {
    // otherwise, print the intermediate result (for debugging)
    print(tmp,n);
    // and repeat!
    recursive_filter(tmp,output,n,percent_change_threshold);
  }
}

// ==========================================================================================

int main(int argc, char** argv) {

  // The mandatory first argument is the GPS data input file.
  if (argc < 2) {
    std::cerr << "ERROR!  This program must be called with at least one argument, the GPS data input file" << std::endl;
  }
  std::ifstream istr(argv[1]);
  assert (istr.good());


  // Load the GPS data file contents into a vector.
  // NOTE: this is the only place we use a vector in Part 1 of the homework.
  // This is just for convenience &/ flexibility for testing.
  std::vector<int> file_contents;
  int x;
  while (istr >> x) { file_contents.push_back(x); }


  // The optional second argument is the number of data points to work with.
  int count = -1;
  if (argc > 2) {
    count = atoi(argv[2]);
  }
  // If the argument is not provided, or if it is larger than the size
  // of the file, just use the whole file.
  if (count < 0 || file_contents.size() / 2 < count) count = file_contents.size() / 2;


  // The optional third argument is the distance percentage change stopping criterion
  double percent_change_threshold = 0.5;
  if (argc > 3) {
    percent_change_threshold = atof(argv[3]);
  }
  assert (percent_change_threshold > 0.0);


  // Prepare an array of the original data (the selected amount of data).
  GPSData input[count];
  for (int i = 0; i < count; i++) {
    input[i].set_position(file_contents[i*2],file_contents[i*2+1]);
  }
  std::cout << "-----------------------------------" << std::endl;
  double original_avg_speed;
  double original_distance = distance(input,count,original_avg_speed) / double (feet_per_mile);
  std::cout << "ORIGINAL" << std::endl;
  print(input,count);


  // Prepare arrays for the filter data.
  GPSData filtered[count];
  GPSData recursive_filtered[count];


  // Perform a single pass of filtering
  std::cout << "-----------------------------------" << std::endl;
  filter(input,filtered,count);
  double filtered_avg_speed;
  double filtered_distance = distance(filtered,count,filtered_avg_speed) / double (feet_per_mile);
  std::cout << "FILTERED" << std::endl;
  print(filtered,count);


  // Perform multiple passes of filtering (until the distance changes by less than the target percentage).
  std::cout << "-----------------------------------" << std::endl;
  recursive_filter(input,recursive_filtered,count,percent_change_threshold);
  double recursive_filtered_avg_speed;
  double recursive_filtered_distance = distance(recursive_filtered,count,recursive_filtered_avg_speed) / double (feet_per_mile);
  std::cout << "RECURSIVE FILTERED" << std::endl;
  print(recursive_filtered,count);


  // Output
  std::cout << "-----------------------------------" << std::endl;
  std::cout << "ORIGINAL:           "
            << std::fixed << std::setprecision(2) << std::setw(5) << original_distance
            << " miles, " << std::setw(4) << (int)original_avg_speed << " feet per minute, ";
  print_pace(original_avg_speed);
  std::cout << " minutes per mile" << std::endl;
  std::cout << "FILTERED:           "
            << std::fixed << std::setprecision(2) << std::setw(5) << filtered_distance
            << " miles, " << std::setw(4) << (int)filtered_avg_speed   << " feet per minute, ";
  print_pace(filtered_avg_speed);
  std::cout << " minutes per mile" << std::endl;
  std::cout << "RECURSIVE FILTERED: "
            << std::fixed << std::setprecision(2) << std::setw(5) << recursive_filtered_distance
            << " miles, " << std::setw(4) << (int)recursive_filtered_avg_speed << " feet per minute, ";
  print_pace(recursive_filtered_avg_speed);
  std::cout << " minutes per mile" << std::endl;
}

