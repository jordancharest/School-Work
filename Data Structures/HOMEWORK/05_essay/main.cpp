#include <string>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <iomanip>
#include <cassert>

#include "essay.h"


// =======================================================================================
//
// NOTE: YOUR FINAL SUBMITTED VERSION SHOULD ONLY CONTAIN
//       CHANGES TO student_tests()
//
// =======================================================================================


// helper functions to convert an Essay to/from an STL string
Essay load(const std::string &s);
std::string print(Essay &e, bool backup);


// test cases
void test_basic_list_operations();
void test_backup_and_revert();
void test_splice();
void student_tests();


// =======================================================================================

int main() {
  test_basic_list_operations();
  test_backup_and_revert();
  test_splice();
  student_tests();
}

// =======================================================================================

//
// NOTE: YOUR FINAL SUBMITTED VERSION SHOULD CONTAIN NO CHANGES TO load or print
//

Essay load(const std::string &s) {
  Essay answer;
  // use an STL string stream to separate a string into words
  // separated by whitespace
  std::stringstream ss(s);
  std::string word;
  while (ss >> word) {
    // add each word to the Essay
    answer.push_back(word);
  }
  return answer;
}

std::string print(Essay &e, bool backup=false) {
  std::string answer = "";
  Essay::iterator begin;
  Essay::iterator end;
  // prepare iterators for the indicated traversal
  if (backup) {
    begin = e.backup_begin();
    end = e.backup_end();
  } else {
    begin = e.begin();
    end = e.end();
  }
  // loop over the list, concatenating the words of the essay
  for (Essay::iterator itr = begin; itr != end; itr++) {
    if (answer != "") answer += " ";
    answer += *itr;
  }
  return answer;
}

// =======================================================================================
// =======================================================================================

//
// NOTE: YOUR FINAL SUBMITTED VERSION SHOULD CONTAIN NO CHANGES TO either debug function
//

// NOTE2: Normally member functions for a class should go in the .cpp
// implementation file for that class.  However, because we provide
// the implementation for the Node and Essay debug member functions,
// please leave them in the main.cpp file.

void Node::debug() const {

  // print the string value, 4 pointers, and 2 booleans of this Node
  std::string s = "prev:";
  (prev_ == NULL) ? s+="NULL" : s+=prev_->value_;
  std::cout << "  " << std::left << std::setw(15) << s;
  s = "backup_prev:";
  (backup_prev_ == NULL) ? s+="NULL" : s+=backup_prev_->value_;
  std::cout << std::setw(22) << s;
  s="value:" + value_;
  assert (added_ == false || removed_ == false);
  if (added_) s += "(ADDED)";
  if (removed_) s += "(REMOVED)";
  std::cout << std::setw(25) << s;
  s = "next:";
  (next_ == NULL) ? s+="NULL" : s+=next_->value_;
  std::cout << std::setw(15) << s;
  s = "backup_next:";
  (backup_next_ == NULL) ? s+="NULL" : s+=backup_next_->value_;
  std::cout << std::setw(22) << s;
  std::cout << std::endl;

  // make sure the links for this node are consistent
  if (prev_ != NULL) assert (prev_->next_ == this);
  if (next_ != NULL) assert (next_->prev_ == this);
  if (backup_prev_ != NULL) assert (backup_prev_->backup_next_ == this);
  if (backup_next_ != NULL) assert (backup_next_->backup_prev_ == this);

  // a node cannot be both added & deleted since the backup
  assert (removed_ == false || added_ == false);
  // nodes added since the backup cannot be in the backup linked list chain
  if (added_ == true) {
    assert (backup_prev_ == NULL);
    assert (backup_next_ == NULL);
  }
  // nodes removed since the backup cannot be in the regular linked list chain
  if (removed_ == true) {
    assert (prev_ == NULL);
    assert (next_ == NULL);
  }
}

void Essay::debug() const {

  // print information about the head & tail pointers of the structure
  std::string s = "head:";
  (head_ == NULL) ? s+="NULL" : s += head_->value_;
  std::cout << "  " << std::setw(15) << std::left << s;
  s = "tail:";
  (tail_ == NULL) ? s+="NULL" : s += tail_->value_;
  std::cout << std::setw(15) << s;
  s = "backup_head:";
  (backup_head_ == NULL) ? s+="NULL" : s += backup_head_->value_;
  std::cout << std::setw(22) << s;
  s = "backup_tail:";
  (backup_tail_ == NULL) ? s+="NULL" : s += backup_tail_->value_;
  std::cout << std::setw(22) << s << std::endl;

  //  print the individual nodes in each traversal
  std::cout << "  -------------------------\n  current essay:" << std::endl;
  for (Node *n = head_; n != NULL; n = n->next_) {
    n->debug();
  }
  std::cout << "  -------------------------\n  backup essay:" << std::endl;
  for (Node *n = backup_head_; n != NULL; n = n->backup_next_) {
    n->debug();
  }
  std::cout << "  -------------------------\n" << std::endl;
}


// =======================================================================================
// =======================================================================================

//
// NOTE: YOUR FINAL SUBMITTED VERSION SHOULD CONTAIN NO CHANGES TO test_basic_list_operations
//       (EXCEPT TO UNCOMMENT THE TESTS AS YOU DEVELOP, TEST, & DEBUG)
//

void test_basic_list_operations() {

  std::cout << "in test_basic_list_operations()" << std::endl;

  // create an empty essay
  Essay my_essay;
  assert (my_essay.size() == 0);
  std::cout << "  empty: " << print(my_essay) << std::endl;
  assert (print(my_essay) == "");

  // add the first 3 words
  my_essay.push_back("apple");
  my_essay.push_back("banana");
  my_essay.push_back("carrot");
  assert (my_essay.size() == 3);
  std::cout << "  initial: " << print(my_essay) << std::endl;
  assert (print(my_essay) == "apple banana carrot");

  // basic operations: push_front
  my_essay.push_front("FRONT");
  assert (my_essay.size() == 4);
  std::cout << "  push_front:" << print(my_essay) << std::endl;
  assert (print(my_essay) == "FRONT apple banana carrot");


  // basic operations: insert
  Essay::iterator itr = my_essay.begin();
  assert (*itr == "FRONT");
  itr++;
  itr++;
  assert (*itr == "banana");
  itr = my_essay.insert(itr,"MIDDLE");
  assert (*itr == "MIDDLE");
  assert (my_essay.size() == 5);
  std::cout << "  insert:" << print(my_essay) << std::endl;
  assert (print(my_essay) == "FRONT apple MIDDLE banana carrot");

  // basic operations: push_back
  my_essay.push_back("BACK");
  assert (my_essay.size() == 6);
  std::cout << "  after:" << print(my_essay) << std::endl;
  assert (print(my_essay) == "FRONT apple MIDDLE banana carrot BACK");

  // basic operations: erase
  itr++;
  itr++;
  assert (*itr == "carrot");
  itr = my_essay.erase(itr);
  assert (*itr == "BACK");
  assert (my_essay.size() == 5);
  std::cout << "  erased:" << print(my_essay) << std::endl;
  assert (print(my_essay) == "FRONT apple MIDDLE banana BACK");

  // basic operations: pop_front & pop_pack
  my_essay.pop_back();
  my_essay.pop_front();
  assert (my_essay.size() == 3);
  std::cout << "  pop front&back:" << print(my_essay) << std::endl;
  assert (print(my_essay) == "apple MIDDLE banana");

  std::cout << "finished test_basic_list_operations()\n" << std::endl;

}

// =======================================================================================
// =======================================================================================

//
// NOTE: YOUR FINAL SUBMITTED VERSION SHOULD CONTAIN NO CHANGES TO test_backup_and_revert
//       (EXCEPT TO UNCOMMENT THE TESTS AS YOU DEVELOP, TEST, & DEBUG)
//

void test_backup_and_revert() {

/*
  std::cout << "in test_backup_and_revert()" << std::endl;

  // same commands as basic operations, but with backup & revert!

  // create an empty essay
  Essay my_essay;
  assert (my_essay.size() == 0);
  assert (my_essay.num_added() == 0);
  assert (my_essay.num_removed() == 0);
  std::cout << "  empty: " << print(my_essay) << std::endl;
  std::cout << "  empty: " << print(my_essay,true) << std::endl;
  assert (print(my_essay) == "");
  assert (print(my_essay,true) == "");

  // add the first 3 words
  my_essay.push_back("apple");
  my_essay.push_back("banana");
  my_essay.push_back("carrot");
  assert (my_essay.size() == 3);
  assert (my_essay.num_added() == 3);
  assert (my_essay.num_removed() == 0);
  std::cout << "  initial: " << print(my_essay) << std::endl;
  std::cout << "  initial: " << print(my_essay,true) << std::endl;
  assert (print(my_essay) == "apple banana carrot");
  assert (print(my_essay,true) == "");

  // backup the essay before making further edits
  my_essay.backup();
  assert (my_essay.size() == 3);
  assert (my_essay.num_added() == 0);
  assert (my_essay.num_removed() == 0);
  std::cout << "  backup: " << print(my_essay) << std::endl;
  std::cout << "  backup: " << print(my_essay,true) << std::endl;
  assert (print(my_essay) == "apple banana carrot");
  assert (print(my_essay,true) == "apple banana carrot");

  // print the full details of the data structure for debugging
  // (this should match the second diagram in the hw pdf handout)
  std::cout << "\n  FIRST DIAGRAM" << std::endl;
  my_essay.debug();

  // basic operations: push_front
  my_essay.push_front("FRONT");
  assert (my_essay.size() == 4);
  assert (my_essay.num_added() == 1);
  assert (my_essay.num_removed() == 0);
  std::cout << "  push_front:" << print(my_essay) << std::endl;
  std::cout << "  push_front:" << print(my_essay,true) << std::endl;
  assert (print(my_essay) == "FRONT apple banana carrot");
  assert (print(my_essay,true) == "apple banana carrot");

  // basic operations: insert
  Essay::iterator itr = my_essay.begin();
  assert (*itr == "FRONT");
  itr++;
  itr++;
  assert (*itr == "banana");
  itr = my_essay.insert(itr,"MIDDLE");
  assert (*itr == "MIDDLE");
  assert (my_essay.size() == 5);
  assert (my_essay.num_added() == 2);
  assert (my_essay.num_removed() == 0);
  std::cout << "  insert:" << print(my_essay) << std::endl;
  std::cout << "  insert:" << print(my_essay,true) << std::endl;
  assert (print(my_essay) == "FRONT apple MIDDLE banana carrot");
  assert (print(my_essay,true) == "apple banana carrot");

  // basic operations: push_back
  my_essay.push_back("BACK");
  assert (my_essay.size() == 6);
  assert (my_essay.num_added() == 3);
  assert (my_essay.num_removed() == 0);
  std::cout << "  after:" << print(my_essay) << std::endl;
  std::cout << "  after:" << print(my_essay,true) << std::endl;
  assert (print(my_essay) == "FRONT apple MIDDLE banana carrot BACK");
  assert (print(my_essay,true) == "apple banana carrot");

  // basic operations: erase
  itr++;
  itr++;
  assert (*itr == "carrot");
  itr = my_essay.erase(itr);
  assert (*itr == "BACK");
  assert (my_essay.size() == 5);
  assert (my_essay.num_added() == 3);
  assert (my_essay.num_removed() == 1);
  std::cout << "  erased:" << print(my_essay) << std::endl;
  std::cout << "  erased:" << print(my_essay,true) << std::endl;
  assert (print(my_essay) == "FRONT apple MIDDLE banana BACK");
  assert (print(my_essay,true) == "apple banana carrot");

  // print the full details of the data structure for debugging
  // (this should match the second diagram in the hw pdf handout)
  std::cout << "\n  SECOND DIAGRAM" << std::endl;
  my_essay.debug();

  // revert the essay to our last backup
  my_essay.revert();
  std::cout << "  revert:" << print(my_essay) << std::endl;
  std::cout << "  revert:" << print(my_essay,true) << std::endl;
  assert (my_essay.size() == 3);
  assert (my_essay.num_added() == 0);
  assert (my_essay.num_removed() == 0);
  assert (print(my_essay) == "apple banana carrot");
  assert (print(my_essay,true) == "apple banana carrot");

  // basic operations: pop_front & pop_pack
  my_essay.pop_back();
  my_essay.pop_front();
  assert (my_essay.size() == 1);
  assert (my_essay.num_added() == 0);
  assert (my_essay.num_removed() == 2);
  std::cout << "  pop front&back:" << print(my_essay) << std::endl;
  std::cout << "  pop front&back:" << print(my_essay,true) << std::endl;
  assert (print(my_essay) == "banana");
  assert (print(my_essay,true) == "apple banana carrot");

  std::cout << "finished test_backup_and_revert()\n" << std::endl;
*/
}

// =======================================================================================
// =======================================================================================

//
// NOTE: YOUR FINAL SUBMITTED VERSION SHOULD CONTAIN NO CHANGES TO test_splice
//       (EXCEPT TO UNCOMMENT THE TESTS AS YOU DEVELOP, TEST, & DEBUG)
//

void test_splice() {

  /*
  std::cout << "in test_splice()" << std::endl;

  // create a short sentence
  Essay my_essay = load("the quick brown fox jumped over the lazy dogs");
  my_essay.backup();
  std::cout << "  before splice:  " << print(my_essay) << std::endl;

  std::cout << "\n  BEFORE SPLICE" << std::endl;
  my_essay.debug();

  // position the splice iterators for the first splice operation
  Essay::iterator splice_begin = my_essay.begin();
  splice_begin++;
  Essay::iterator splice_end = splice_begin;
  splice_end++;
  splice_end++;
  Essay::iterator destination = splice_end;
  destination++;
  destination++;
  destination++;
  destination++;
  // verify the positioning
  assert (*splice_begin == "quick");
  assert (*splice_end == "fox");
  assert (*destination == "lazy");
  // check the results
  bool success = my_essay.splice(splice_begin,splice_end,destination);
  assert (success == true);
  std::cout << "  after splice A: " << print(my_essay) << std::endl;
  assert (print(my_essay) == "the fox jumped over the quick brown lazy dogs");
  assert (print(my_essay,true) == "the quick brown fox jumped over the lazy dogs");
  // No nodes are added or removed by splice.  Instead links between existing nodes are changed.
  assert (my_essay.num_added() == 0);
  assert (my_essay.num_removed() == 0);

  // position the splice iterators for the second splice operation
  destination = my_essay.begin();
  destination++;
  splice_begin = destination;
  splice_begin++;
  splice_begin++;
  splice_begin++;
  splice_begin++;
  splice_begin++;
  splice_begin++;
  splice_end = splice_begin;
  splice_end++;
  // verify the positioning
  assert (*splice_begin == "lazy");
  assert (*splice_end == "dogs");
  assert (*destination == "fox");
  // check the results
  success = my_essay.splice(splice_begin,splice_end,destination);
  assert (success == true);
  std::cout << "  after splice B: " << print(my_essay) << std::endl;
  assert (print(my_essay) == "the lazy fox jumped over the quick brown dogs");
  assert (print(my_essay,true) == "the quick brown fox jumped over the lazy dogs");
  assert (my_essay.num_added() == 0);
  assert (my_essay.num_removed() == 0);

  std::cout << "\n  AFTER SPLICE" << std::endl;
  my_essay.debug();

  // The splice should fail if either splice_begin does not precede
  // splice_end or destination is contained within the range of
  // slice_begin and splice end
  splice_begin = my_essay.begin();
  splice_begin++;
  destination = splice_begin;
  destination++;
  splice_end = destination;
  splice_end++;
  success = my_essay.splice(splice_begin,splice_end,destination);
  assert (success == false);
  splice_begin = splice_end;
  splice_begin++;
  success = my_essay.splice(splice_begin,splice_end,destination);
  assert (success == false);

  assert (print(my_essay) == "the lazy fox jumped over the quick brown dogs");
  assert (print(my_essay,true) == "the quick brown fox jumped over the lazy dogs");
  assert (my_essay.num_added() == 0);
  assert (my_essay.num_removed() == 0);

  std::cout << "finished test_splice()\n" << std::endl;
  */
}

// =======================================================================================
// =======================================================================================

//
// NOTE: ADD YOUR OWN TESTS TO THIS FUNCTION
//

void student_tests() {

  std::cout << "in student_tests()" << std::endl;

  //
  // You should add your own test cases here.
  // Test the corner cases of all required member functions.
  // Also test the copy constructor, assignment operator, and destructor.
  //

  std::cout << "finished student_tests()\n" << std::endl;

}

// =======================================================================================
// =======================================================================================
