#ifndef _EDGE_H_
#define _EDGE_H_
#include <iostream>

#include "vertex.h"


// "prototype" of the Mesh class (defined in mesh.h) so we can store
// a pointer inside the Edge
class Mesh;


// =================================================================

class Edge {

public:

  // CONSTRUCTOR
  Edge(Vertex *a, Vertex *b, Mesh *m);

  // ACCESSORS
  Vertex* getV1() const { return v1; }
  Vertex* getV2() const { return v2; }
  bool isLegal() const { return legal; }
  double getLength() const { return length; }
  double getPriorityValue() const;

  bool CheckValue() const;

  // MODIFIER
  void ReCalculateValue();
  void ShuffleVertices();

private:
  // REPRESENTATION
  Vertex *v1;
  Vertex *v2;
  // For efficiency, an edge stores its length and whether it's legal
  // to collapse.  We need to update these values if something in the
  // neighborhood changes.
  bool legal;
  double length;
  Mesh *mesh;
};

// =================================================================

// Allows easy printing of edges & edge pointers
std::ostream& operator<<(std::ostream &ostr, const Edge& e);
std::ostream& operator<<(std::ostream &ostr, Edge* e);


// A more intuitive ordering function for a pair of Vertex pointers,
// so we can store this thing in a map.  We will sort them by ID.
// (Without this they would be sorted by pointer value/memory
// location, which is not intuitive and could change on subsequent
// program runs or on another computer.)
class VertexPairByID {
public:
  bool operator()(std::pair<Vertex*,Vertex*> a,
                  std::pair<Vertex*,Vertex*> b) const {
    // Each edge must have its smaller vertex id first
    assert (a.first->getID() < a.second->getID());
    assert (b.first->getID() < b.second->getID());
    // And we sort edges by the first vertex id,
    // and break ties by looking at the second vertex id
    return (a.first->getID() < b.first->getID() ||
            (a.first->getID() == b.first->getID() &&
             a.second->getID() < b.second->getID()));
  }
};

// =================================================================

#endif
