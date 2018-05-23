HOMEWORK 6: HASHI RECURSION


NAME:  Jordan Charest



COLLABORATORS AND OTHER RESOURCES:
List the names of everyone you talked to about this assignment
(classmates, TAs, ALAC tutors, upperclassmen, students/instructor via
LMS, etc.), and all of the resources (books, online reference
material, etc.) you consulted in completing this assignment.

C++ reference

Remember: Your implementation for this assignment must be done on your
own, as described in "Academic Integrity for Homework" handout.



ESTIMATE OF # OF HOURS SPENT ON THIS ASSIGNMENT:  25



ANALYSIS OF PERFORMANCE OF YOUR ALGORITHM:
(order notation & concise paragraph, < 200 words)
The dimensions of the board (w and h) 
The number of nodes/islands (n)? 
The number of edges/bridges (e)?
Etc. 

Algorithm first checks for special case nodes, corners with 4, edges with 6, or islands with 8,
and makes bridges. This runs  in O(s) with s being the number of special case nodes.

The next part searches for nodes that have the same number of paths available as possible bridges.
This is recursively called, checking every free node on the board each time. In a worst case scenario,
the algorithm only makes one bridge per recursive call, and solves the entire board through these
recursive calls, one by one. In that case, it would search n times on the first call, n-1 on the second,
then n-2, n-3, etc. Therefore the algorithm runs in O(n!). However, the time it takes to
build a bridge is much greater than the time it takes to search a node, so for small n, (such as the sizes
that would be encountered in this game) the run time is dominated by bridge building -> O(b).





SUMMARY OF PERFORMANCE OF YOUR PROGRAM ON THE PROVIDED PUZZLES:
# of solutions & approximate wall clock running time for different
puzzles for a single solution or all solutions.  Also, describe the
image in words.

Successfully solved all puzzles except for puzzles 3, 7, and 10. Didn't have enough time to
implement a depth first search technique to solve the more difficult problems (I'm a slow coder).
Approximate running time of solved puzzles: a couple hundred milliseconds; virtually no conceivable delay.

Describe the image in words: not sure what this is trying to say really but the image looks like a solved puzzle,
with bridges connecting the nodes. The unsolved puzzles show nodes without a full set of bridges.



MISC. COMMENTS TO GRADER:  
Really fun assignment, but I'm a little disappointed I didn't get to finish. For only one week of work, 25 hours
on one homework is a little ridiculous. I probably need to get faster.


