#include <cstdio>
#include <iostream>
#include <cassert>
#include <cstdlib>
#include <stdint.h>

#include <vector>
#include <string>

// ==============================================================================
//
// This class supports a limited visualization / printing of the data
// stored on the stack in the range of the addresses currently
// "labeled" and stored in the member variables.
//
// To make the output more readable, this visualization assumes that
// integer "values" will be between -10000 & 10000 and addresses will
// be within +/- 1000*4 (or *8) bytes from any of the labeled
// addresses.  Anything else is assumed to be "garbage" (or floating
// point values).
//

class TheStack {

public:

  // ACCESSORS
  std::string get_label(intptr_t* address) const;
  bool is_return_address(intptr_t* address) const;
  
  // MODIFIERS
  void clear_labels();
  void set_label(intptr_t* address, const std::string& label);
  void tag_return_address(intptr_t* address);

  // PRINTING!
  void print() const;
  
 private:

  // PRIVATE REPRESENTATION
  std::vector<intptr_t*> labeled_addresses;
  std::vector<std::string> labels;
  std::vector<intptr_t*> return_addresses;
};

// ==============================================================================
