#include "the_stack.h"

// NOTE: For Part 3 of this homework we'll use "intptr_t", a specially
// defined integer type that is the same size in memory as a pointer.
// We'll use this type everywhere, instead of "int".  On 32 bit
// machines/compilers, this type is probably 4 bytes, and on 64 bit
// machines/compilers, this is probably 8 bytes.


// Normally, global variables are forbidden in Data Structures.  For
// this homework we allow one global variable, which is used to record
// and visualize the memory usage of our program.  We prefer to use a
// global variable so we don't need to modify each function with an
// extra argument to pass this variable around.
TheStack thestack;


// ================================================================================

void helper(intptr_t param1, intptr_t param2) {

  // miscellaneous prints in helper
  intptr_t x = 72;
  intptr_t a[5] = {10, 11, 12, 13, 14};
  intptr_t *y = &x;
  intptr_t z = 98;
  std::cout << "x address: " << &x << std::endl;
  std::cout << "a address: " << &a << std::endl;
  std::cout << "y address: " << &y << std::endl;
  std::cout << "z address: " << &z << std::endl;

  // label the frame pointer and the return address for the stack frame for this function
  thestack.set_label((intptr_t*)__builtin_frame_address(0),"FUNCTION HELPER");
  thestack.tag_return_address((intptr_t*)(__builtin_frame_address(0))+1);
    
  // label the addresses we want to examine on the stack
  thestack.set_label(&param1,"param1");
  thestack.set_label(&param2,"param2");
  thestack.set_label(&x,"x");
  thestack.set_label(&a[0],"a[0]");
  thestack.set_label(&a[4],"a[4]");
  thestack.set_label((intptr_t*)&y,"y");
  thestack.set_label(&z,"z");

  // print the range of the stack containing these addresses
  thestack.print();

}


// ================================================================================

int main() {

  // label the frame pointer and the return address for the stack frame for the main function
  thestack.set_label((intptr_t*)__builtin_frame_address(0),"FUNCTION MAIN");
  thestack.tag_return_address((intptr_t*)(__builtin_frame_address(0))+1);

  // a local variable
  intptr_t localvar = 2017;
  thestack.set_label((intptr_t*)&localvar, "localvar");

  // miscellaneous prints in main
  std::cout << "size of intptr_t: " << sizeof(intptr_t) << std::endl;
  std::cout << "localvar address: " << &localvar << std::endl;
  
  // call the helper function
  helper(1776,1863);

}

// ================================================================================
