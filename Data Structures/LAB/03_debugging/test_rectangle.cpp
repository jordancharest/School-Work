#include <iostream>
#include <algorithm>
#include "Rectangle.h"
#include "Point2D.h"


// MAIN =======================================================
int main(){
    // Generate rectangle defining points
    Point2D lowerLeft(1,2);
    Point2D upperRight(4,6);
    std::cout << "x: " << lowerLeft.x() << "  y: " << lowerLeft.y();

    Point2D lowerLeft2(1,2);
    Point2D upperRight2(10,10);

    // generate rectangles
    Rectangle rect1(lowerLeft, upperRight);
    Rectangle rect2(lowerLeft2, upperRight2);
    Point2D UPC = rect1.upper_right_corner();
    Point2D LLC = rect1.lower_left_corner();
    std::cout << "\nURC: " << UPC.x() << "," << UPC.y();
    std::cout << "\nLLC: " << LLC.x() << "," << LLC.y();


    // generate a few other points to test
    Point2D testpoint0(0, 1);
    Point2D testPoint1(1,  4);
    Point2D testPoint5(1,  4);
    Point2D testPoint2(2,  4);
    Point2D testPoint3(1,  8);
    Point2D testPoint4(10,12);

    std::cout << "\nPoint Within(should be 0): " << rect1.is_point_within(testPoint4);

    // add them to the rectangles
    std::cout << "\nPoint Added? (should be 1): " << rect1.add_point(testPoint1);
    std::cout << "\nPoint Added? (should be 0): " << rect1.add_point(testPoint1);
    rect1.add_point(testPoint2);
    rect1.add_point(testPoint3);
    rect1.add_point(testPoint4);
    rect2.add_point(testPoint1);
    rect2.add_point(testPoint2);
    rect2.add_point(testPoint3);
    rect2.add_point(testPoint4);


    print_rectangle(rect1);
    print_rectangle(rect2);

    std::vector<Point2D> sharedPoints = points_in_both(rect1, rect2);
    for(int i=0; i<sharedPoints.size(); i++){
        std::cout << "\nPoint " << i+1 << "\n"
                  << sharedPoints[i].x() << ","
                  << sharedPoints[i].y() << "\n";
    }


    return 0;
}
