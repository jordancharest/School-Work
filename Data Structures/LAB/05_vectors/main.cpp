#include <vector>
#include <iostream>
#include <cstdlib>
#include <list>


// PRINT VECTOR =============================================
void PrintVector(const std::vector<int> &myVector){

    std::cout << "Vector Size: " << myVector.size() << "\n";

    for (int i = 0; i < myVector.size(); i++)
        std::cout << myVector[i] << " ";

    std::cout << "\n\n";
}


// REVERSE VECTOR ==========================================
void ReverseVector(std::vector<int> &myVector){
    // can it be done without temp? In python it can!
    int temp;
    int last = myVector.size() - 1;

    // swap the positions of opposite elements
    for(int i = 0; i < myVector.size()/2; i++){
        temp = myVector[i];
        myVector[i] = myVector[last-i];
        myVector[last-i] = temp;
    }
}


// PRINT LIST =============================================
void PrintList(const std::list<int> &myList){
    std::list<int>::const_iterator it;
    std::cout << "List Size: " << myList.size() << "\n";

    for (it = myList.begin(); it != myList.end(); it++)
        std::cout << *it << " ";

    std::cout << "\n\n";
}


// REVERSE LIST ==========================================
void ReverseList(std::list<int> &myList){
    // can it be done without temp? In python it can!
    int temp;
    int index = 0;
    std::list<int>::iterator it;
    std::list<int>::reverse_iterator r_it = myList.rbegin();

    // swap the positions of opposite elements
    for(it = myList.begin(); index < myList.size()/2; it++){
        temp = *it;
        *it = *r_it;
        *r_it = temp;
        index++;
        r_it++;
    }
}





// MAIN ====================================================
int main () {

    int numElements = 17;
    std::vector<int> myVector (numElements);
    std::list<int> myList;
    float random;
    float measurementError;

    // VECTORS
    // Generate the "random" vector
    for (int i=0; i<numElements; i++){

        // random # between 0 and 1 determines positive or negative noise
        random = ((double) rand() / (RAND_MAX));
        measurementError = (numElements/2)*((double) rand() / RAND_MAX);

        if(random < 0.5){
            myVector[i] = i - measurementError;
            myList.push_back(i-measurementError);
        } else {
            myVector[i] = i + measurementError;
            myList.push_back(i+measurementError);
        }
    }

    // Random Vector
    std::cout << "-------------------------------------"
              << "VECTOR WITH ODD NUMBER OF ELEMENTS\n";
    PrintVector(myVector);
    ReverseVector(myVector);
    PrintVector(myVector);

    std::cout << "-------------------------------------\n"
              << "VECTOR WITH EVEN NUMBER OF ELEMENTS\n";
    myVector.push_back(( (double)rand() / RAND_MAX) * 10);
    PrintVector(myVector);
    ReverseVector(myVector);
    PrintVector(myVector);

    std::cout << "-------------------------------------\n"
              << "EMPTY VECTOR\n";
    myVector.clear();
    PrintVector(myVector);
    ReverseVector(myVector);
    PrintVector(myVector);

    std::cout << "-------------------------------------\n"
              << "ONE ELEMENT VECTOR\n";
    myVector.push_back(( (double)rand() / RAND_MAX) * 10);
    PrintVector(myVector);
    ReverseVector(myVector);
    PrintVector(myVector);

    std::cout << "-------------------------------------\n"
              << "TWO ELEMENT VECTOR\n";
    myVector.push_back(( (double)rand() / RAND_MAX)  * 10);
    PrintVector(myVector);
    ReverseVector(myVector);
    PrintVector(myVector);

    std::cout << "-------------------------------------\n"
              << "THREE ELEMENT VECTOR\n";
    myVector.push_back(( (double)rand() / RAND_MAX)  * 10);
    PrintVector(myVector);
    ReverseVector(myVector);
    PrintVector(myVector);


    // LISTS
    std::cout << "\n\n\n-------------------------------------\n"
              << "LIST WITH ODD NUMBER OF ELEMENTS\n";
    PrintList(myList);
    ReverseList(myList);
    PrintList(myList);


    std::cout << "-------------------------------------\n"
              << "LIST WITH ODD NUMBER OF ELEMENTS\n";
    myList.push_back(( (double)rand() / RAND_MAX) * 10);
    PrintList(myList);
    ReverseList(myList);
    PrintList(myList);

    std::cout << "-------------------------------------\n"
              << "EMPTY LIST\n";
    myList.clear();
    PrintList(myList);
    ReverseList(myList);
    PrintList(myList);

    std::cout << "-------------------------------------\n"
              << "ONE ELEMENT LIST\n";
    myList.push_back(( (double)rand() / RAND_MAX) * 10);
    PrintList(myList);
    ReverseList(myList);
    PrintList(myList);

    std::cout << "-------------------------------------\n"
              << "TWO ELEMENT LIST\n";
    myList.push_back(( (double)rand() / RAND_MAX) * 10);
    PrintList(myList);
    ReverseList(myList);
    PrintList(myList);

    std::cout << "-------------------------------------\n"
              << "THREE ELEMENT LIST\n";
    myList.push_back(( (double)rand() / RAND_MAX) * 10);
    PrintList(myList);
    ReverseList(myList);
    PrintList(myList);




    return 0;
}
