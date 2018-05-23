#include <fstream>
#include <iostream>
#include <list>
#include <vector>


// CLASS: Point ============================================================
class Point {
public:
  Point(int x0, int y0) : x(x0), y(y0) {}
  int x,y;
};


// FIND PATH ===============================================================
long long FindPaths(Point coordinate){

    // found the origin
    if (coordinate.x == 0 && coordinate.y == 0)
        return 1;

    else if (coordinate.x > 0 && coordinate.y > 0)
        return FindPaths(Point(coordinate.x-1, coordinate.y)) + FindPaths(Point(coordinate.x, coordinate.y-1));

    // on the y-axis
    else if (coordinate.x > 0 && coordinate.y == 0)
        return FindPaths(Point(coordinate.x-1, coordinate.y));

    // on the x-axis
    else if (coordinate.x == 0 && coordinate.y > 0)
        return FindPaths(Point(coordinate.x, coordinate.y-1));

    else return 0;

}


// MAIN ====================================================================
int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cerr << "ERROR: Need an x and y coordinate";
    return 1;
  }

    int x = atoi(argv[1]);
    int y = atoi(argv[2]);


    long long numPaths = FindPaths(Point(x,y));
    std::cout << "There are  " << numPaths << " paths from (" << x << "," << y << ") to the origin";

    return 0;
}

