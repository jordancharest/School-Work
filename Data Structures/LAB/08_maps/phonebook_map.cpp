
#include <map>
#include <iostream>
#include <vector>
#include <string>
using namespace std;


// given a phone number, determine who is calling
void identify(map<int, string> &phonebook, int number) {
  if (!phonebook.count(number))
    cout << "unknown caller!" << endl;
  else
    cout << phonebook[number] << " is calling!" << endl;
}



// MAIN ====================================================================
int main() {
  // create the phonebook; initially all numbers are unassigned
  map<int, string> phonebook;

  // add several names to the phonebook
  phonebook[1111] = "fred";
  phonebook[2222] = "sally";
  phonebook[3333] = "george";

  // test the phonebook
  identify(phonebook, 2222);
  identify(phonebook, 4444);
  return 0;
}
