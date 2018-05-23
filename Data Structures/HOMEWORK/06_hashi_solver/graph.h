#include <vector>

// ====================================================================================
// A Point is just a 2D coordinate.

class Point {
public:
  // constructor
  Point(int x_, int y_);
  // accessors
  int get_x() const { return x; }
  int get_y() const { return y; }
private:
  // representation
  int x;
  int y;
};

// helper function for printing Points
std::ostream& operator<<(std::ostream &ostr, const Point &p);

// ====================================================================================
// An Edge is a vertical or horizontal line segment connecting 2 points.

class Edge {
public:
  // constructor
  Edge(const Point &a_, const Point &b_);
  // accessors
  const Point& get_a() const { return a; }
  const Point& get_b() const { return b; }
private:
  // representation
  Point a;
  Point b;
};

// helper function for printing Edges
std::ostream& operator<<(std::ostream &ostr, const Edge &e);

// ====================================================================================
// A Node is a Point with a count of the number of Edges that should touch it.

class Node {
public:
  Node(const Point &p, int n);

  // accessors
  const Point& get_pt() const { return pt; }
  int get_num_paths() const { return num_paths; }
  int num_available_paths() const { return num_paths - num_bridges;}
  std::string get_bridge() const { return bridge; }

  // Modifiers
  void addBridge() {num_bridges++;}
  void setBridgeType(std::string type) { bridge = type; }

  // Operators
  bool operator!=(const Node& n) const {
    return pt.get_x() != n.get_pt().get_x()  ||
           pt.get_y() != n.get_pt().get_y(); }

private:
  Point pt;
  int num_paths;
  int num_bridges;
  std::string bridge;
};

// ====================================================================================
// A Graph is a collection of Nodes and Edges that can be printed.

class Graph {
public:
  // constructor
  Graph();

  // Accessors
  int getMax_x() const { return max_x; };
  int getMax_y() const { return max_y; };

  // modifiers
  void addNode(const Point &p, int n);
  void addEdge(const Point &a, const Point &b);
  void drawEdge(std::vector<std::vector<Node> > &board, const Point &a, const Point &b);

  // Validation
  bool isCorner4(Node &n);
  bool isEdge6(Node &n);
  bool isIsland8(Node &n);
  bool TestConnection(std::vector<std::vector<Node> > &board);

  // Searching and Solving
  void SolveRequired(std::vector<std::vector<Node>> &board);
  void MakeRequiredConnection(Node &n,  std::vector<std::vector<Node>> &board);
  void OnePathPerBridge(std::vector<std::vector<Node>> &board);
  int Search(Node &n, std::vector<std::vector<Node>> &board);

  void DepthFirstSearch(std::vector<std::vector<Node>> &board, Node &n);

  void FindPossibleConnection(Node &n);

  // print helper functions
  int AvailableNodes(std::vector<std::vector<Node>> &board) const;
  void printEdges(std::vector<std::vector<Node>> &board) const;
  void printSolution() const;
  void printGraph() const;


private:
  // representation
  int max_x;
  int max_y;
  int min_x;
  int min_y;
  std::vector<Node> nodes;
  std::vector<Edge> edges;
};

// ====================================================================================
