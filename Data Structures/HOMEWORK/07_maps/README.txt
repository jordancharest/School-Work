HOMEWORK 7: HALLOWEEN COSTUME MAPS


NAME:  < Jordan Charest >


COLLABORATORS AND OTHER RESOURCES:
List the names of everyone you talked to about this assignment
(classmates, TAs, ALAC tutors, upperclassmen, students/instructor via
LMS, etc.), and all of the resources (books, online reference
material, etc.) you consulted in completing this assignment.

< C++ reference, class notes >

Remember: Your implementation for this assignment must be done on your
own, as described in "Academic Integrity for Homework" handout.



ESTIMATE OF # OF HOURS SPENT ON THIS ASSIGNMENT:  < 9 >



ORDER NOTATION ANALYSIS (ORIGINAL VERSION W/MAPS):
of each of the costume shop operations (please be concise!)

add:	O(log n) - when adding a costume, the map must be binary searched to find the correct location for the new costume

rent:	O(log n) + O(log c) - searches the inventory once to find the wanted costume and ensure it is in stock. If it is, searches the
								customers once to return their current costume (if they have one) and give them the one they want.
								Searches the inventory again to increment the number of costumes for the costume the customer returned.
								
lookup:	O(log n) + O(m) - searches the inventory for the costume, prints the number of available copies, then searches the rental_history
							for the costume and prints every person that is currently renting the costume (a max of m people).



EXTRA CREDIT (W/O MAPS)
What alternate data structures did you use?  What is the order
notation of each operation without maps?  What is the expected
performance difference?  Summarize the results of your testing.
N/A



MISC. COMMENTS TO GRADER:  
(optional, please be concise!)






