#include <iomanip>
#include <iostream>
#include <cassert>

#include "the_stack.h"


// ==============================================================================
// ACCESSORS

// A helper function for printing.  Returns the label (if any) for
// that address on the stack.
std::string TheStack::get_label(intptr_t* address) const {
  for (unsigned int i = 0; i < labeled_addresses.size(); i++) {
    if (address == labeled_addresses[i]) {
      return labels[i];
    }
  }
  return "";
}

// A helper function for printing.  Returns true if this address on
// the stack has been tagged as a pointer to code (not stored near the
// valid stack addresses).
bool TheStack::is_return_address(intptr_t* address) const {
  for (unsigned int i = 0; i < return_addresses.size(); i++) {
    if (address == return_addresses[i]) {
      return true;
    }
  }
  return false;
}

// ==============================================================================
// ACCESSORS


// clear all previous labels and tags
void TheStack::clear_labels() {
  labeled_addresses.clear();
  labels.clear();
  return_addresses.clear();
}


// add a label to a particular address in the stack
void TheStack::set_label(intptr_t* address, const std::string& label) {
  for (unsigned int i = 0; i < labeled_addresses.size(); i++) {
    // check to see if that address is already labeled
    if (address == labeled_addresses[i]) {
      // add additional labels
      labels[i] += ", " + label;      
      return;
    }
  }
  labeled_addresses.push_back(address);
  labels.push_back(label);
}


// tag this address as a pointer to the code (not stored near the
// valid stack addresses).
void TheStack::tag_return_address(intptr_t* address) {
  set_label(address,"return address");
  return_addresses.push_back(address);
}



// ==============================================================================
//
// This function does a limited print of the data stored on the stack
// in the range of the addresses currently "labeled" and stored in the
// global variables above.
//
// To make the output more readable, the function assumes that integer
// "values" will be between -1000 & 1000 and addresses will be within
// +/- 1000*4 (or *8) bytes from any of the labeled addresses.
// Anything else is assumed to be "garbage"
//
void TheStack::print() const { 

  assert (labeled_addresses.size() == labels.size());
  if (labeled_addresses.size() == 0) {
    std::cerr <<  "No labeled addresses to print" << std::endl;
    return;
  }

  // Identify the range of stack data to print
  intptr_t* min_address = labeled_addresses[0];
  intptr_t* max_address = labeled_addresses[0];
  unsigned int max_label_width = labels[0].size();
  for (unsigned int i = 1; i < labeled_addresses.size(); i++) {
    if (labeled_addresses[i] > max_address) { max_address = labeled_addresses[i]; }
    if (labeled_addresses[i] < min_address) { min_address = labeled_addresses[i]; }
    if (labels[i].size() > max_label_width) { max_label_width = labels[i].size(); }
  }
  for (unsigned int i = 1; i < return_addresses.size(); i++) {
    if (return_addresses[i] > max_address) { max_address = return_addresses[i]; }
    if (return_addresses[i] < min_address) { min_address = return_addresses[i]; }
    if (labels[i].size() > max_label_width) { max_label_width = labels[i].size(); }
  }

  // double check that my intptr_t is actually the size of a pointer
  assert (sizeof(intptr_t) == sizeof(void*));
  // double check that the start and end address are aligned 
  assert ((((intptr_t)min_address) & (sizeof(intptr_t)-1)) == 0);
  assert ((((intptr_t)max_address) & (sizeof(intptr_t)-1)) == 0);

  // double check that the range of address is reasonable 
  if (max_address-min_address > 10000) {
    std::cerr << "WARNING: trying to print too much" << std::endl;
    return;
  }

  // loop over all addresses in the range (plus some above & below)
  std::cout << "-----------------------------------------" << std::endl;
  for (intptr_t* address = max_address+5; address >= min_address-5; address--) {
    std::string label = get_label(address);
    std::cout << std::setw(max_label_width) << label << " location: " << address;

    // this location might store an integer value (we only attempt to
    // display integer values)
    int an_int = (int)(*address);
    // or it might store an address (so we cast it to a pointer)
    intptr_t* an_address = (intptr_t*)(*address);

    // if it's a value
    if (an_int > -10000 && an_int < 10000) {
      std::cout << "  VALUE:   " << an_int << std::endl;
    } 
    // if it's an address
    else if (an_address >= min_address-1000 && an_address <= max_address+1000) {
      std::string ptr_to_what = get_label(an_address);
      if (ptr_to_what != "") { ptr_to_what = "  ----> " + ptr_to_what; }
      std::cout << "  POINTER: " << an_address << ptr_to_what << std::endl;
    }
    // if it's a return address (not stored near the valid stack
    // addresses)
    else if (is_return_address(address)) {
      std::cout << "  CODE: " << an_address << std::endl;
    }
    // otherwise it's probably garbage or a floating point number
    else { 
      std::cout << "  garbage or float? " << std::endl;
    }
  }
  std::cout << "-----------------------------------------" << std::endl;
}

// ==============================================================================
