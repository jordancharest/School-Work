#include <fstream>
#include <iostream>
#include <map>
#include <vector>




// MAIN ====================================================================
int main(int argc, char* argv[]) {

    std::vector<int> numbers;

    if (argc > 2){
            std::cerr << "USAGE: " << argv[0] << " optional-input-file\n";
            exit(1);

    } else if (argc == 2) {
        std::ifstream InputStream(argv[1]);
        // ensure the stream was opened correctly
        if (!InputStream.good()){
        std::cerr << "Can't open" << argv[1] << ". Exiting program.\n";
        exit(1);
        }

        int num;
        while (InputStream >> num)
            numbers.push_back(num);

    } else {
        numbers = {19,83,-12,83,65,19,45,-12,45,19,45};
    }


    std::map<int, int> frequency_map;

    // populate the map
    int most_frequent = 0;
    for (int i = 0; i < numbers.size(); i++){
        frequency_map[numbers[i]]++;

        if (frequency_map[numbers[i]] > most_frequent)
            most_frequent++;
    }

    std::cout << "\nMost frequent appearance: " << most_frequent << "\n\n";

    // verification
    std::map<int, int>::iterator itr_map;
    for (itr_map = frequency_map.begin(); itr_map != frequency_map.end(); itr_map++)
        std::cout << itr_map-> first << ": " << itr_map-> second << "\n";

    // CHECKPOINT 1: Using operator[]
    std::cout << "\nPrinting modes using operator[]:\n";
    for (itr_map = frequency_map.begin(); itr_map != frequency_map.end(); itr_map++){
        if (itr_map-> second == most_frequent)
            std::cout << itr_map-> first << ": " << itr_map-> second << "\n";
    }


    // CHECKPOINT 2: using find()
    std::map<int, int> modes;
    std::cout << "\nPrinting modes using find():\n";
    for (int i = 0; i <numbers.size(); i++){
        if (frequency_map.find(numbers[i])->second == most_frequent)
            modes.insert(std::pair<int,int>(numbers[i], frequency_map.find(numbers[i])->second));
    }

    for (itr_map = modes.begin(); itr_map != modes.end(); itr_map++)
        std::cout << itr_map-> first << ": " << itr_map-> second << "\n";



    return 0;
}

