#include <iostream>
#include <list>

void reverse_splice(std::list<int> &numbers, int s, int e){


    std::list<int>::iterator start = numbers.begin();
    std::list<int>::iterator end = numbers.begin();

    advance(start, s);
    advance(end, e);

    std::cout << "Start: " << *start
              << "  End: " << *end;


    // reverse splice
    int temp;
    while (start != end){
        temp = *start;
        start = numbers.erase(start);
        numbers.insert(start, *end);

        end = numbers.erase(end);
        numbers.insert(end, temp);

        /*
        for (itr_list = numbers.begin(); itr_list != numbers.end(); itr_list++)
            std::cout << *itr_list << " ";
        std::cout << "\n";
        */

        end--;
        end--;
    }

    start = numbers.begin();
    end = numbers.begin();

    advance(start, e);
    advance(end, s);

    // verification
    std::cout << "\n\nAFTER:\n";
    std::list<int>::iterator itr_list;
    for (itr_list = numbers.begin(); itr_list != numbers.end(); itr_list++)
        std::cout << *itr_list << " ";

    std::cout << "\nStart: " << *start
              << "  End: " << *end;

}




// MAIN ============================================================================
int main() {

    int start = 3;
    int end = 7;

    std::list<int> numbers = {0,1,2,3,4,5,6,7,8,9};
    std::cout << "BEFORE:\n";

    std::list<int>::iterator itr_list;
    for (itr_list = numbers.begin(); itr_list != numbers.end(); itr_list++)
        std::cout << *itr_list << " ";
    std::cout << "\n";



    reverse_splice(numbers, start, end);


    return 0;
}
