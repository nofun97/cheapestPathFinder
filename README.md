# cheapestPathFinder
This is a code for the second project of COMP10002 Foundation of Algorithm 2017 Semester 2 University of Melbourne

The code uses Dijkstra's algorithm to find a way from point A to point B in a city which is in the form of a grid

## Test Files
The test files' structure is, the first line is the row and column
The rest of the lines are the name of each point and the distances to other point.
The four numbers next to the name of the point shows the distance to the other point
The destination for each distance is in anti clockwise starting from the right.

For example

4b 5 2 999 3

This means from 4b to 4c costs 5, 4b to 3b costs 2, 4b to 4a costs 999, and 4b to 5b costs 3.

999 means the path should not be taken.
