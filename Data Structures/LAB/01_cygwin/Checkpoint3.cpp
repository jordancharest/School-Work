#include <iostream>

int main(){

    std::cout << "How many numbers do you wish to enter? " ;
    int arraySize;
    std::cin >> arraySize;

    std::cout << "\nEnter your numbers:\n";
    float number;
    float myArray[arraySize];
    float sum = 0;

    // read every entered number into the float array;
    // computes the sum as the user is inputting numbers
    for(int i=0; i<arraySize; i++){
        std::cin >> number;
        sum += number;
        myArray[i] = number;
    }

    // compute the average
    float average = sum/arraySize;
    std::cout << "Average: " << average;

    // check for values less than the average and print them
    std::cout << "\nElements less than the average: ";
    for (float i : myArray){
        if (i < average)
            std::cout << i << ' ';
    }
}
