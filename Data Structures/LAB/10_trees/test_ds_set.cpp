#include <iostream>
#include <string>
#include <utility>
#include <cassert>
#include <vector>

#include "ds_set.h"

int main() {

  ds_set<std::string> set1;
  set1.insert("hello");
  set1.insert("good-bye");
  set1.insert("friend");
  set1.insert("abc");
  set1.insert("puppy");
  set1.insert("zebra");
  set1.insert("daddy");
  set1.insert("puppy");  // should recognize that this is a duplicate!

  assert(set1.sanity_check());
  assert (set1.size() == 7);

  ds_set<std::string>::iterator p = set1.begin();
  assert(p != set1.end() && *p == std::string("abc"));

  p = set1.find( "foo" );
  assert (p == set1.end());

  p = set1.find("puppy");
  assert (p != set1.end());
  assert (*p == "puppy");

  std::cout << "Here is the tree, printed sideways.\n"
	    << "The indentation is proportional to the depth of the node\n"
	    << "so that the value stored at the root is the only value printed\n"
	    << "without indentation.  Also, for each node, the right subtree\n"
	    << "can be found above where the node is printed and indented\n"
	    << "relative to it\n";
  set1.print_as_sideways_tree( std::cout );
  std::cout << std::endl;


  // copy the set
  ds_set<std::string> set2( set1 );
  assert(set1.sanity_check());
  assert(set2.sanity_check());
  assert(set1.size() == set2.size());


  //  Now add stuff to set2
  set2.insert( std::string("a") );
  set2.insert( std::string("b") );
  std::cout << "After inserting stuff:\n";
  set2.print_as_sideways_tree( std::cout );
  std::cout << std::endl;
  assert(set1.sanity_check());
  assert(set2.sanity_check());
  assert(set1.size() == set2.size() - 2);


  //  Now erase stuff from set2
  set2.erase("hello");
  set2.erase("a");
  set2.erase("hello"); // should recognize that it's not there anymore!
  set2.erase("abc");
  set2.erase("friend");
  std::cout << "After erasing stuff:\n";
  set2.print_as_sideways_tree( std::cout );
  std::cout << std::endl;
  assert(set1.sanity_check());
  assert(set2.sanity_check());
  assert(set1.size() == set2.size() + 2);


  // Test the iterators!
  std::cout << "\nTest iterators on balanced tree";
  ds_set<int> set3;
  std::vector<int> numbers = {8,4,12,2,6,10,14,1,3,5,7,9,11,13,15};

  for (int i = 0; i < numbers.size(); ++i) {
    set3.insert(numbers[i]);
  }

  std::cout << "\n\n";
  set3.print_as_sideways_tree(std::cout);


  ds_set<int>::iterator itr_tree = set3.begin();
  itr_tree = set3.end();

  std::cout << "\n";
  // print forwards
  for (itr_tree = set3.begin(); itr_tree != set3.end(); ++itr_tree) {
    std::cout << *itr_tree << " ";
  }

  std::cout << "\n";
  // print backwards
  for (itr_tree = set3.end(); itr_tree != set3.begin();) {
    itr_tree--;
    std::cout << *itr_tree << " ";
  }




  // Test the iterators!
  std::cout << "\n\n\nTest iterators on unbalanced tree";
  ds_set<int> set4;
  std::vector<int> numbers2 = {9,2,4,6,8,10,1,3,13,15,16,17,18,19,11,12,9,5,14,20,7};

  for (int i = 0; i < numbers2.size(); ++i) {
    set4.insert(numbers2[i]);
  }

  std::cout << "\n\n";
  set4.print_as_sideways_tree(std::cout);


  itr_tree = set4.begin();
  itr_tree = set4.end();

  std::cout << "\n";
  // print forwards
  for (itr_tree = set4.begin(); itr_tree != set4.end(); ++itr_tree) {
    std::cout << *itr_tree << " ";
  }

  std::cout << "\n";
  // print backwards
  for (itr_tree = set4.end(); itr_tree != set4.begin();) {
    itr_tree--;
    std::cout << *itr_tree << " ";
  }



  // test accumulating
  std::cout << "\n\nTest Accumulating\n";

  int initial_value = 0;
  std::cout << "Summing up all values in the tree:" << std::endl;
  set3.accumulate(initial_value);
  std::cout << "Sum of all elements is: " << initial_value << std::endl;

  std::string initial_string = std::string("");
  std::cout << "Concatenating all strings in the set:" << std::endl;
  set1.accumulate(initial_string);
  std::cout << "Concatenation of all the strings: " << initial_string << std::endl;















  return 0;
}
