HOMEWORK 4: JAGGED ARRAY


NAME:  < Jordan Charest >


COLLABORATORS AND OTHER RESOURCES:
List the names of everyone you talked to about this assignment
(classmates, TAs, ALAC tutors, upperclassmen, students/instructor via
LMS, etc.), and all of the resources (books, online reference
material, etc.) you consulted in completing this assignment.

< C++ Reference, Stack Overflow, vec.h from Lab 4 >

Remember: Your implementation for this assignment must be done on your
own, as described in "Academic Integrity for Homework" handout.



ESTIMATE OF # OF HOURS SPENT ON THIS ASSIGNMENT:  < 17 >



ORDER NOTATION:
For each function, for each version (packed vs. unpacked).  
b = the number of bins
e = the number of elements
k = the number of elements in the largest bin
m = number of elements in the bin being operated on

numElements
 unpacked: O(1) - Constant
 packed:   O(1) - Constant

numBins
 unpacked: O(1) - Constant
 packed:   O(1) - Constant

numElementsInBin
 unpacked: O(1) - Constant
 packed:   O(1) - Constant

getElement
 unpacked: O(1) - Constant
 packed:   O(1) - Constant

isPacked
 unpacked: O(1) - Constant
 packed:   O(1) - Constant

clear
 unpacked: O(b) - Linear in the number of bins
 packed:   Error: Cannot modify packed Jagged Array

addElement
 unpacked: O(m) - Linear in the number of elements in the bin being operated on
 packed:   Error: Cannot modify packed Jagged Array

removeElement
 unpacked: O(m) - Linear in the number of elements in the bin being operated on
 packed:   Error: cannot modify packed Jagged Array

pack
 unpacked: O(b*k)
 packed:   O(1) - Constant

unpack
 unpacked: O(1) - Constant
 packed:   O(b*k)

print 
 unpacked: O(e) - linear in the number of total elements
 packed:   O(b+e) - linear in the total elements and total bins

constructor w/ integer argument
 unpacked: O(b) - linear in the number of bins
 packed:   Jagged Array cannot be instantiated in packed configuration

copy constructor
 unpacked: O(b*k)
 packed:   O(b+e) - linear in the total elements and total bins

destructor
 unpacked: O(b) - linear in the number fo total bins
 packed:   O(1) - constant

assignment operator
 unpacked: O(b*k) - the copy constructor dominates runtime for the assignment operator
 packed:   O(b+e) - the copy constructor dominates runtime for the assignment operator




MISC. COMMENTS TO GRADER:  
I created a new variable for order notation, m, specifying the number of elements 
in the bin being operated on. Using any of the other specified variables in the
addElement and removeElement operations would be inaccurate for my code.


