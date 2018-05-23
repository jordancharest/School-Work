#include <cmath>
#include <iostream>
#include <cassert>

#include "triangle.h"
#include "mesh.h"

// =================================================================
// CONSTRUCTOR

Triangle::Triangle(Vertex* a, Vertex* b, Vertex* c, Mesh *m) {
  static int counter = 0;
  id = counter;
  counter++;
  // Order the vertices so the smallest ID # is first.
  // NOTE: The order of the 2nd & 3rd is not necessarily sorted.  The
  // vertices should be in clockwise order so it is "right side up".
  if (a->getID() < b->getID() && a->getID() < c->getID()) {
    pts[0]=a;
    pts[1]=b;
    pts[2]=c;
  } else if (b->getID() < a->getID() && b->getID() < c->getID()) {
    pts[0]=b;
    pts[1]=c;
    pts[2]=a;
  } else {
    assert (c->getID() < a->getID() && c->getID() < b->getID());
    pts[0]=c;
    pts[1]=a;
    pts[2]=b;
  }
  mesh = m;
  // make sure the initial triangle is oriented correctly
  assert (RightSideUp());
}

// =================================================================
// ACCESSORS

Vertex* Triangle::getVertex(int i) const {
  assert (i >= 0 && i < 3);
  return pts[i];
}

bool Triangle::HasVertex (Vertex *pt) const {
  if (pts[0] == pt || pts[1] == pt || pts[2] == pt) return true;
  return false;
}

double Triangle::getArea() const {
  return ::getArea(pts[0]->getPoint(),pts[1]->getPoint(),pts[2]->getPoint());
}

// If we replace one of the vertices of this triangle with a new
// coordinate, what will the area be?
double Triangle::getAreaAfterReplacement(Vertex *v, const Point& p) const {
  if (pts[0] == v)
    return ::getArea(p,pts[1]->getPoint(),pts[2]->getPoint());
  if (pts[1] == v)
    return ::getArea(pts[0]->getPoint(),p,pts[2]->getPoint());
  assert (pts[2] == v);
  return ::getArea(pts[0]->getPoint(),pts[1]->getPoint(),p);
}


bool Triangle::RightSideUp() const {
  if (mesh->PreserveArea()) {
    return ::RightSideUp(pts[0]->getPoint(),pts[1]->getPoint(),pts[2]->getPoint());
  }
  // if we aren't trying to preserve area, just skip this winding order test
  return true;
}

// If we replace one of the vertices of this triangle with a new
// coordinate, will the triangle still be upside down?
bool Triangle::RightSideUpAfterReplacement(Vertex *v, const Point& p) const {
  assert(HasVertex(v));
  if (pts[0] == v)
    return ::RightSideUp(p,pts[1]->getPoint(),pts[2]->getPoint());
  if (pts[1] == v)
    return ::RightSideUp(pts[0]->getPoint(),p,pts[2]->getPoint());
  assert (pts[2] == v);
  return ::RightSideUp(pts[0]->getPoint(),pts[1]->getPoint(),p);
}

// MODIFIERS
void Triangle::ReplaceVertex(Vertex *replacement, Vertex* garbage) {
    for (int i = 0; i < 3; ++i) {
        if (pts[i] == garbage)
            pts[i] = replacement;
    }
}



// =================================================================
// HELPER FUNCTIONS

double getArea(const Point& a, const Point& b, const Point& c) {
  // Heron's formula for the area of a triangle
  double ab = DistanceBetween(a,b);
  double bc = DistanceBetween(b,c);
  double ca = DistanceBetween(c,a);
  double s = (ab+bc+ca) / 2.0;
  return sqrt( s * (s-ab) * (s-bc) * (s-ca));
}

// Calculate the angle at vertex a, between two vectors a->b and a->c
double AngleBetween(const Point& a, const Point& b, const Point& c) {
  // unit vector from a->b
  double v1x = b.x - a.x;
  double v1y = b.y - a.y;
  double len1 = sqrt(v1x*v1x + v1y*v1y);
  v1x /= len1;
  v1y /= len1;
  // unit vector from a->c
  double v2x = c.x - a.x;
  double v2y = c.y - a.y;
  double len2 = sqrt(v2x*v2x + v2y*v2y);
  v2x /= len2;
  v2y /= len2;
  // calculate the angle between those vectors
  double angle = atan2(v2y,v2x) - atan2(v1y,v1x);
  if (angle < 0)
    angle += 2 * M_PI;
  return angle;
}

// We can detect an "upside down" triangle by summing the angles
bool RightSideUp(const Point& a, const Point& b, const Point& c) {
  double angle_a = AngleBetween(a,b,c);
  double angle_b = AngleBetween(b,c,a);
  double angle_c = AngleBetween(c,a,b);
  double sum =
    angle_a*180/M_PI +
    angle_b*180/M_PI +
    angle_c*180/M_PI;
  return fabs(sum-180.0) < 0.01;
}

// =================================================================

// stream printing function for Triangle
std::ostream& operator<<(std::ostream& ostr, const Triangle& t) {
  ostr << " TRIANGLE " << t.getID() << " ";
  ostr << "[ "
       << t.getVertex(0)->getID() << " "
       << t.getVertex(1)->getID() << " "
       << t.getVertex(2)->getID() << " ]";
  return ostr;
}

// =================================================================


