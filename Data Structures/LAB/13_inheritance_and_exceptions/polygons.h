#ifndef POLYGONS_H_INCLUDED
#define POLYGONS_H_INCLUDED

#include <vector>
#include <string>
#include <cmath>

#include "utilities.h"


#define ANGLE_EPSILON 0.1
#define DISTANCE_EPSILON 0.0001



// POLYGON ==================================================
class Polygon {
public:
    // Constructor
    Polygon(std::string _name, std::vector<Point> _pts) : name(_name), pts(_pts) {
        int i = 0;

        for (; i < pts.size()-1; ++i) {
            sides.push_back(Vector(pts[i], pts[i+1]));
        }
        sides.push_back( Vector(pts[i], pts[0]));


        for (i = 0; i < pts.size()-2; ++i) {
            angles.push_back( Angle(pts[i], pts[i+1], pts[i+2]));
        }
        angles.push_back( Angle(pts[i], pts[i+1], pts[0]));
        angles.push_back( Angle(pts[i+1], pts[0], pts[1]));


        /*// DEBUGGING
        std::cout << "\n\n" << name << "\n";
        for (int j = 0; j < angles.size(); ++j) {
            std::cout << angles[j] << "\n";
        }*/
    }

    // Destructor
    virtual ~Polygon() {}

    // ACCESSORS
    std::string getName() const { return name; }

    // VERIFICATION
    bool HasAllEqualSides() {
        for (int i = 0; i < sides.size()-1; ++i) {
            if ( !EqualSides(sides[0].Length(), sides[i+1].Length())) return false;
        }
        return true;
    }

    bool HasAllEqualAngles() {
        for (int i = 0; i < angles.size()-1; ++i) {
            if (!EqualAngles(angles[i], angles[i+1])) return false;
        }
        return true;
    }

    bool HasARightAngle() {
        for (int i = 0; i < angles.size(); ++i) {
            if ( RightAngle(angles[i])) return true;
        }
        return false;
    }


protected:
    std::string name;
    std::vector<Point> pts;
    std::vector<Vector> sides;
    std::vector<double> angles;
};


// TRIANGLE AND CHILDREN ====================================
class Triangle : public Polygon {
public:
    // Constructor
    Triangle(std::string _name, std::vector<Point> _pts) : Polygon(_name, _pts) {
        if (sides.size() != 3) throw 3;
    }


};

// ----------------------------------------
class IsoscelesTriangle : virtual public Triangle {
public:
    // Constructor
    IsoscelesTriangle(std::string _name, std::vector<Point> _pts) : Triangle(_name, _pts) {
        bool valid = false;
        // check that at least two of the sides are equal
        for (int i = 0; i < sides.size(); ++i) {
            for (int j = 0; j < sides.size(); ++j) {
                if (i == j) continue;
                if (EqualSides(sides[i].Length(), sides[j].Length())) valid = true;
            }
        }
        if (!valid) throw 3;
    }

};

// ----------------------------------------
class EquilateralTriangle : public IsoscelesTriangle {
public:
    // Constructor
    EquilateralTriangle(std::string _name, std::vector<Point> _pts) : Triangle(_name, _pts), IsoscelesTriangle(_name, _pts) {
        if (!this->HasAllEqualSides()) throw 3;
    }

};

// ----------------------------------------
class RightTriangle : virtual public Triangle {
public:
    RightTriangle(std::string _name, std::vector<Point> _pts) : Triangle(_name, _pts) {
        if (!this->HasARightAngle()) throw 3;


    }




};




// ----------------------------------------
class IsoscelesRightTriangle : public IsoscelesTriangle, public RightTriangle {
public:
    IsoscelesRightTriangle(std::string _name, std::vector<Point> _pts) :
                            Triangle(_name, _pts),
                            IsoscelesTriangle(_name, _pts),
                            RightTriangle(_name, _pts)      {}







};


// QUADRILATERAL AND CHILDREN ===============================
class Quadrilateral : public Polygon {
public:
    // Constructor
    Quadrilateral(std::string _name, std::vector<Point> _pts) : Polygon(_name, _pts) {
        if (sides.size() != 4) throw 4;
    }

};


// ---------------------------------------
class Rectangle : public Quadrilateral {
public:
    // Constructor
    Rectangle(std::string _name, std::vector<Point> _pts) : Quadrilateral(_name, _pts) {
        // check for all right angles
        if      ( !RightAngle( Angle(pts[0], pts[1], pts[2]) ) ) throw 4;
        else if ( !RightAngle( Angle(pts[1], pts[2], pts[3]) ) ) throw 4;
        else if ( !RightAngle( Angle(pts[2], pts[3], pts[0]) ) ) throw 4;
        else if ( !RightAngle( Angle(pts[3], pts[0], pts[1]) ) ) throw 4;
    }

};


// ---------------------------------------
class Square : public Rectangle {
public:
    // Constructor
    Square(std::string _name, std::vector<Point> _pts) : Rectangle(_name, _pts) {

        /*std::cout << "\n\nSide Lengths:";
        for (int i = 0; i < sides.size(); ++i) std::cout << "\n " << sides[i].Length();
        */
        if (!(this->HasAllEqualSides())) throw 1;

    }

};





#endif // POLYGONS_H_INCLUDED
