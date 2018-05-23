HOMEWORK 3: GPS TRACKING & STACK HACKING


NAME:  < Jordan Charest >


COLLABORATORS AND OTHER RESOURCES:
List the names of everyone you talked to about this assignment
(classmates, TAs, ALAC tutors, upperclassment, students/instructor via
LMS, etc.), and all of the resources (books, online reference
material, etc.) you consulted in completing this assignment.

< Lecture Notes, cplusplus.com >

Remember: Your implementation for this assignment must be done on your
own, as described in "Academic Integrity for Homework" handout.



ESTIMATE OF # OF HOURS SPENT ON THIS ASSIGNMENT:  < 13 >



PART 2: DIAGRAMMING MEMORY & MAKING HYPOTHESES
List the name of the images files of the diagrams you are uploading
with this portion of the assignment.  Describe the scenario shown in
each diagram, and your predictions for the errors that will arise from
incorrect use of your distance and filter functions.



PART 3: TESTING YOUR HYPOTHESES
Describe the testing of your hypotheses for part 2.  Paste small
examples of the stack visualization that support your investigation.
Describe the program behavior/errors that results when you exaggerate
the mis-use of memory with these functions.

Passing in a smaller number than count into the distance function allows
the program to operate normally, but misses some of the values. Passing
in a larger than expected count value results in a segfault when the for
loop attempts to access memory that was never allocated to the input array.

Stack Print
-----------------------------------------
            location: 0x28dce8  VALUE:   0
            location: 0x28dce4  VALUE:   38
            location: 0x28dce0  VALUE:   39
            location: 0x28dcdc  VALUE:   0
            location: 0x28dcd8  VALUE:   19
Input Array location: 0x28dcd4  VALUE:   0
            location: 0x28dcd0  VALUE:   2
            location: 0x28dccc  VALUE:   552
            location: 0x28dcc8  POINTER: 0x28e118
            location: 0x28dcc4  garbage or float?
            location: 0x28dcc0  garbage or float?
-----------------------------------------

Stack Print
-----------------------------------------
            location: 0x28dce8  VALUE:   612
            location: 0x28dce4  VALUE:   38
            location: 0x28dce0  VALUE:   39
            location: 0x28dcdc  VALUE:   0
            location: 0x28dcd8  VALUE:   19
Input Array location: 0x28dcd4  VALUE:   0
            location: 0x28dcd0  VALUE:   2
            location: 0x28dccc  VALUE:   552
            location: 0x28dcc8  garbage or float?
            location: 0x28dcc4  garbage or float?
            location: 0x28dcc0  garbage or float?
-----------------------------------------
ORIGINAL
     x-pos     y-pos    ft/min  min/mile
-----------------------------------
Segmentation fault


PART 4: PASS-BY-VALUE VS PASS-BY-REFERENCE
Paste the relevant portion of the visualization and describe in your
own words what the visualization demonstrates.

The output below demonstrates that 'avg_feet_per_minute' is being
passed as a pointer while the variable 'n' is being passed by value.

				location: 0x28fc98  POINTER: 0x28fd50  ----> AVG FEET PER MINUTE REF.
	 DATA ARRAY location: 0x28fc94  VALUE:   0
			  n location: 0x28fc90  VALUE:   0
 return address location: 0x28fc8c  VALUE:   0



PART 5: STACK FRAMES OF A RECURSIVE FUNCTION
Paste the relevant portion of the stck visualization (abbreviate as
appropriate) and your observations about the operation of recursive
functions on the stack.

Observations: That was interesting



           return address location: 0x28f38c  CODE: 0x402689
FUNCTION RECURSIVE FILTER location: 0x28f388  POINTER: 0x28f4b8  ----> FUNCTION RECURSIVE FILTER
                          location: 0x28f384  VALUE:   2
                          location: 0x28f380  VALUE:   96
                          location: 0x28f37c  VALUE:   0
                          location: 0x28f378  VALUE:   8
                          location: 0x28f374  garbage or float?
                          location: 0x28f370  garbage or float?
                          location: 0x28f36c  VALUE:   4
                          location: 0x28f368  VALUE:   5
                          location: 0x28f364  POINTER: 0x28f274  ----> TEMP Array
                          location: 0x28f360  VALUE:   647
						  
						 ... Intermediate Values ...
						 
                          location: 0x28f280  VALUE:   0
                          location: 0x28f27c  VALUE:   0
                          location: 0x28f278  VALUE:   0
               TEMP Array location: 0x28f274  VALUE:   0
                          location: 0x28f270  VALUE:   1
                          location: 0x28f26c  garbage or float?
                          location: 0x28f268  POINTER: 0x28f388  ----> FUNCTION RECURSIVE FILTER
                          location: 0x28f264  POINTER: 0x28f33c
                          location: 0x28f260  POINTER: 0x28f274  ----> TEMP Array








MISC. COMMENTS TO GRADER:  
Optional, please be concise!

