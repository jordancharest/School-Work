HOMEWORK 8: QUAD TREES AND TREE ITERATION


NAME:  < Jordan Charest >


COLLABORATORS AND OTHER RESOURCES:
List the names of everyone you talked to about this assignment
(classmates, TAs, ALAC tutors, upperclassmen, students/instructor via
LMS, etc.), and all of the resources (books, online reference
material, etc.) you consulted in completing this assignment.

< Lecture Notes, LMS discussion board, C++ reference, Stack Overflow >

Remember: Your implementation for this assignment must be done on your
own, as described in "Academic Integrity for Homework" handout.



ESTIMATE OF # OF HOURS SPENT ON THIS ASSIGNMENT:  < 14 >



ORDER NOTATION ANALYSIS: 
Give the big O notation of each of the QuadTree operations and justify
your answer for the non trivial operations (please be concise!)
Analyze both the running time and the additional memory usage needed
(beyond the space allocated for the existing tree structure).  You may
assume that the tree is reasonably well balanced for all operations.


n = the number of elements in the tree


size()
  running time:	O(1)
  memory usage:	O(1)

insert()
  running time:	O(log n) -  reduces possible insertion location by 75% each recursive call
  memory usage:	O(log n) - 	stores stack frames for each recursive call, O(log n) recursive calls for a well-balanced tree

find()
  running time:	O(log n) -  reduces possible find location by 75% each recursive call
  memory usage:	O(log n) - 	stores stack frames for each recursive call, O(log n) recursive calls for a well-balanced tree


height()
  running time:	O(n) - must traverse entire tree to find the deepest node
  memory usage:	O(n) - store stack frames for O(n) recursive calls

begin()
  running time:	O(1)
  memory usage:	O(1)

end()
  running time:	O(1)
  memory usage:	O(1)

bf_begin()
  running time:	O(1)
  memory usage:	O(1)

bf_end()
  running time:	O(1)
  memory usage:	O(1)

operator++()
  running time:	O(n) - performs depth first or breadth first search and returns the next node after finding the current node
  memory usage:	O(n) -  queue size for BFS,  O(maximum depth) - recursive stack frames for DFS

operator*()
  running time:	O(1)
  memory usage:	O(1)

getLabel()
  running time: O(1)
  memory usage:	O(1)

getDepth()
  running time: O(maximum depth) - 
  memory usage:	O(1)

copy constructor
  running time: O(n) - uses Breadth First Search to iterate over and copy every node of the old tree
  memory usage: O(n) - queue size for BFS

assignment operator
  running time:	O(n) - O(n) to destroy the tree and O(n) to copy the new one
  memory usage:	O(n) - queue size for BFS (to copy)

destructor
  running time:	O(n) - iterate over every node
  memory usage:	O(-n) - destroying memory, this function appears to create infinite memory...just kidding O(1)



EXTRA CREDIT:  TREE BALANCING
How does the point insertion order affect the shape of the resulting
QuadTree object? What are specific examples of the worst case and best
case?  Discuss your stratgy for reordering the input to rebalance the
tree.  Paste example output with and without your rebalancing.






MISC. COMMENTS TO GRADER:  
(optional, please be concise!)


