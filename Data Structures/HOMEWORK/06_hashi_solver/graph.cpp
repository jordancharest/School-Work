#include <string>
#include <iostream>
#include <cassert>
#include "graph.h"


// POINT ===============================================================================

Point::Point(int x_, int y_) : x(x_),y(y_) {
  // Points must have positive coordinates
  assert (x_ >= 0);
  assert (y_ >= 0);
}

// helper function for printing Points
std::ostream& operator<<(std::ostream &ostr, const Point &p) {
  ostr << "(" << p.get_x() << "," << p.get_y() << ")";
  return ostr;
}

// EDGE ===============================================================================

Edge::Edge(const Point &a_, const Point &b_) : a(a_),b(b_) {
  // Edges must be horizontal or vertical
  assert ((a_.get_x() == b_.get_x() && a_.get_y() != b_.get_y()) ||
          (a_.get_x() != b_.get_x() && a_.get_y() == b_.get_y()));
}

// NODE ================================================================================

Node::Node(const Point &p, int n) : pt(p),num_paths(n), num_bridges(0) {
  // Nodes/islands must have at least one Edge/bridge.  At most 2
  // Edges/bridges can connect a pair of Nodes/islands.  Edges/bridges
  // may only run horizontal or vertical and cannot cross other
  // Edges/bridges.  Thus a Node/island can support at most 8 bridges.
  assert (n >= 0 && n <= 8);
}

// helper function for printing Edges
std::ostream& operator<<(std::ostream &ostr, const Edge &e) {
  ostr << e.get_a() << "," << e.get_b();
  return ostr;
}

// GRAPH ===============================================================================

Graph::Graph() {
  // We track the maximum dimensions of the graph.
  max_x = 0;
  max_y = 0;
  min_x = 0;
  min_y = 0;
}

// ADD NODE TO GRAPH =====================================================================
void Graph::addNode(const Point &p, int n) {
  // Incrementally track the maximum dimensions of the graph.
  max_x = std::max(p.get_x(), max_x);
  max_y = std::max(p.get_y(), max_y);
  min_x = std::min(p.get_x(), 0);
  min_y = std::min(p.get_y(), 0);
  nodes.push_back(Node(p,n));
}

// ADD EDGE TO GRAPH =====================================================================
void Graph::addEdge(const Point &a, const Point &b) {
  assert (a.get_x() <= max_x);
  assert (a.get_y() <= max_y);
  assert (b.get_x() <= max_x);
  assert (b.get_y() <= max_y);
  edges.push_back(Edge(a,b));
}

// DRAW EDGE ON BOARD ====================================================================
void Graph::drawEdge(std::vector<std::vector<Node> > &board, const Point &a, const Point &b){

    // determine the edge/edge direction
    int diff_x = b.get_x()-a.get_x();
    int diff_y = b.get_y()-a.get_y();
    if (diff_x > 0) diff_x = 1;
    if (diff_x < 0) diff_x = -1;
    if (diff_y > 0) diff_y = 1;
    if (diff_y < 0) diff_y = -1;

    // walk between the islands to draw the bridge
    int x = a.get_x()+diff_x;
    int y = a.get_y()+diff_y;
    while (x != b.get_x() || y != b.get_y()) {
      if (diff_x == 0) {
        // vertical bridge
        assert (diff_y != 0);
        if (board[x][y].get_bridge() == "|") {
          // if we already have a bridge, draw a double vertical bridge
          board[x][y].setBridgeType("#");
        } else {
          assert (board[x][y].get_bridge() == "");
          // otherwise draw a single vertical bridge
          board[x][y].setBridgeType("|");
        }
      } else {
        assert (diff_y == 0);
        // horizontal bridge
        if (board[x][y].get_bridge() == "-") {
          // if we already have a bridge, draw a double horizontal bridge
          board[x][y].setBridgeType("=");
        } else {
          //assert (board[x][y].get_bridge() == "");
          // otherwise draw a single horizontal bridge
          board[x][y].setBridgeType("-");
        }
      }
      x+=diff_x;
      y+=diff_y;
    }


}

// IS CORNER =============================================================================
bool Graph::isCorner4(Node &n){
    Point p = n.get_pt();
    return (((p.get_x() == min_x)  &&  (p.get_y() == min_y))  ||  // lower left corner
            ((p.get_x() == min_x)  &&  (p.get_y() == max_y))  ||  // upper left corner
            ((p.get_x() == max_x)  &&  (p.get_y() == min_y))  ||  // lower right corner
            ((p.get_x() == max_x)  &&  (p.get_y() == max_y))) &&  // upper right corner
            (n.get_num_paths() == 4);
}

// IS EDGE 6 =============================================================================
bool Graph::isEdge6(Node &n){
    Point p = n.get_pt();
    return (p.get_x() == min_x  || // left edge
            p.get_x() == min_y  || // bottom edge
            p.get_x() == max_x  || // right edge
            p.get_x() == max_y)  && // top edge
            (n.get_num_paths() == 6);
}


// IS ISLAND 8 ===================================================================================
bool Graph::isIsland8(Node &n){
    Point p = n.get_pt();
    return(p.get_x() != min_x  &&
           p.get_y() != min_y  &&
           n.get_num_paths() == 8);
}



// SORT TO SOLVE THE REQUIRED BRIDGES ============================================================
void Graph::SolveRequired(std::vector<std::vector<Node>> &board){

    int x, y;

    for (int i = 0; i < nodes.size(); i++){

        if (this->isCorner4(nodes[i])  ||  this->isEdge6(nodes[i])  ||  this->isIsland8(nodes[i])){

            x = nodes[i].get_pt().get_x();
            y = nodes[i].get_pt().get_y();

            // we can automatically add bridges for these three scenarios without verifying
            while (board[x][y].num_available_paths() > 0)
                MakeRequiredConnection(board[x][y], board);
        }
    }
}

// MAKE REQUIRED CONNECTION =======================================================================
// This function searches  in all four directions for connections to the node and automatically
// generates the bridge without confirming the validity of that bridge, used for guaranteed bridges
// (e.g. corner 4's, edge 6's, island 8's)
void Graph::MakeRequiredConnection(Node &n, std::vector<std::vector<Node>> &board){

    int x = n.get_pt().get_x();
    int y = n.get_pt().get_y();
    bool found = false;

    // Search up, add bridge to first node found
    while(y <= max_y && !found){

        // If the node has a path available and we are not pointing at the starting node, make a bridge
        if (board[x][y] != n  &&  board[x][y].num_available_paths() > 0){

            std::cout << "\nAdding Edge\n";
            this->addEdge(n.get_pt(), board[x][y].get_pt());
            this->drawEdge(board, board[x][y].get_pt(), n.get_pt());

            n.addBridge();
            board[x][y].addBridge();
            found = true;

        // a node is found, but it doesn't have available paths, we don't want to search past it
        } else if (board[x][y].get_num_paths() > 0 && board[x][y].num_available_paths() <= 0) {
            found = true;

        // if we encounter a bridge, stop searching
        } else if (board[x][y].get_bridge() == "-"  ||  board[x][y].get_bridge() == "#"  || board[x][y].get_bridge() == "="){
            found = true;
        }
        y++;
    }



    y = n.get_pt().get_y();
    found = false;

    // Search down, add bridge to first node found
    while(y >= min_y && !found){

        // If the node has a path available and we are not pointing at the starting node, make a bridge
        if (board[x][y] != n  &&  board[x][y].num_available_paths() > 0){

            std::cout << "\nAdding Edge\n";
            this->addEdge(n.get_pt(), board[x][y].get_pt());
            this->drawEdge(board, board[x][y].get_pt(), n.get_pt());

            n.addBridge();
            board[x][y].addBridge();
            found = true;

        // a node is found, but it doesn't have available paths, we don't want to search past it
        } else if (board[x][y].get_num_paths() > 0 && board[x][y].num_available_paths() <= 0) {
            found = true;

        // if we encounter a bridge, stop searching
        } else if (board[x][y].get_bridge() == "-"  ||  board[x][y].get_bridge() == "#"  || board[x][y].get_bridge() == "="){
            found = true;
        }

        y--;
    }



    y = n.get_pt().get_y();
    found = false;

    // Search left, add bridge to first node found
    while(x >= min_x && !found){

        // If the node has a path available and we are not pointing at the starting node, make a bridge
        if (board[x][y] != n  &&  board[x][y].num_available_paths() > 0){

            std::cout << "\nAdding Edge\n";
            this->addEdge(n.get_pt(), board[x][y].get_pt());
            this->drawEdge(board, board[x][y].get_pt(), n.get_pt());

            n.addBridge();
            board[x][y].addBridge();
            found = true;

        // a node is found, but it doesn't have available paths, we don't want to search past it
        } else if (board[x][y].get_num_paths() > 0 && board[x][y].num_available_paths() <= 0) {
            found = true;

        // if we encounter a bridge, stop searching
        } else if (board[x][y].get_bridge() == "|"  ||  board[x][y].get_bridge() == "#"  || board[x][y].get_bridge() == "="){
            found = true;
        }

        x--;
    }



    x = n.get_pt().get_x();
    found = false;

    // Search right, add bridge to first node found
    while(x <= max_x && !found){

        // If the node has a path available and we are not pointing at the starting node, make a bridge
        if (board[x][y] != n  &&  board[x][y].num_available_paths() > 0){

            std::cout << "\nAdding Edge\n";
            this->addEdge(n.get_pt(), board[x][y].get_pt());
            this->drawEdge(board, board[x][y].get_pt(), n.get_pt());

            n.addBridge();
            board[x][y].addBridge();
            found = true;

        // a node is found, but it doesn't have available paths, we don't want to search past it
        } else if (board[x][y].get_num_paths() > 0 && board[x][y].num_available_paths() <= 0) {
            found = true;

        // if we encounter a bridge, stop searching
        } else if (board[x][y].get_bridge() == "|"  ||  board[x][y].get_bridge() == "#"  || board[x][y].get_bridge() == "="){
            found = true;
        }

        x++;
    }
}


// ONE PATH PER BRIDGE ========================================================================
void Graph::OnePathPerBridge(std::vector<std::vector<Node>> &board){
    bool connection = false;
    int neighbors = 0;

    std::cout << "\n\nAttempt OnePathPerBridge:";
    for (int y = max_y; y >=0; y--){
        for(int x = 0; x <= max_x; x++){

            if (board[x][y].num_available_paths() > 0){

                // Searches for a connection in all four directions
                neighbors = this->Search(board[x][y], board);
                std::cout << "\n" << "Point: " << board[x][y].get_pt()
                          << "  Neighbors: " << neighbors << "  Paths: " << board[x][y].num_available_paths();

                if (board[x][y].num_available_paths() == neighbors){
                    connection = true;
                    while (board[x][y].num_available_paths() > 0)
                        MakeRequiredConnection(board[x][y], board);
                }
            }
        }
    }

    // recursive call to recheck nodes if a connection was made in this run
    if (connection)
        this->OnePathPerBridge(board);
}

// SEARCH ====================================================================================
int Graph::Search(Node &n, std::vector<std::vector<Node>> &board){

    int x = n.get_pt().get_x();
    int y = n.get_pt().get_y();
    int numPaths = 0;
    bool found = false;

    // Search up, track number of available paths
    while(y <= max_y && !found){

        if (board[x][y] != n  &&  board[x][y].num_available_paths() > 0){
            found = true;

            // can only add a max of two bridges between each node
            if (board[x][y].num_available_paths() >= 2 && n.num_available_paths() >=2)
                numPaths += 2;
            else
                numPaths++;


        // a node is found, but it doesn't have available paths, we don't want to search past it
        } else if (board[x][y].get_num_paths() > 0 && board[x][y].num_available_paths() <= 0) {
            found = true;

        // bridge encountered, stop searching
        } else if (board[x][y].get_bridge() == "#"  ||  board[x][y].get_bridge() == "="  ||  board[x][y].get_bridge() == "-"){
            found = true;
        }

        y++;
    }



    y = n.get_pt().get_y();
    found = false;

    // Search down, add bridge to first node found
    while(y >= min_y && !found){

        if (board[x][y] != n  &&  board[x][y].num_available_paths() > 0){
            found = true;

            if (board[x][y].num_available_paths() >= 2 && n.num_available_paths() >=2)
                numPaths += 2;
            else
                numPaths++;

        // a node is found, but it doesn't have available paths, we don't want to search past it
        } else if (board[x][y].get_num_paths() > 0 && board[x][y].num_available_paths() <= 0) {
            found = true;

        // bridge encountered, stop searching
        } else if (board[x][y].get_bridge() == "#"  ||  board[x][y].get_bridge() == "="  ||  board[x][y].get_bridge() == "-"){
            found = true;
        }

        y--;
    }



    y = n.get_pt().get_y();
    found = false;

    // Search left, add bridge to first node found
    while(x >= min_x && !found){

        if (board[x][y] != n  &&  board[x][y].num_available_paths() > 0){
            found = true;

            if (board[x][y].num_available_paths() >= 2 && n.num_available_paths() >=2)
                numPaths += 2;
            else
                numPaths++;

        // a node is found, but it doesn't have available paths, we don't want to search past it
        } else if (board[x][y].get_num_paths() > 0 && board[x][y].num_available_paths() <= 0) {
            found = true;

        // bridge encountered, stop searching
        } else if (board[x][y].get_bridge() == "#"  ||  board[x][y].get_bridge() == "="  ||  board[x][y].get_bridge() == "|"){
            found = true;
        }

        x--;
    }



    x = n.get_pt().get_x();
    found = false;

    // Search right, add bridge to first node found
    while(x <= max_x && !found){

        if (board[x][y] != n  &&  board[x][y].num_available_paths() > 0){
            found = true;

            if (board[x][y].num_available_paths() >= 2 && n.num_available_paths() >=2)
                numPaths += 2;
            else
                numPaths++;

        // a node is found, but it doesn't have available paths, we don't want to search past it
        } else if (board[x][y].get_num_paths() > 0 && board[x][y].num_available_paths() <= 0) {
            found = true;

        // bridge encountered, stop searching
        } else if (board[x][y].get_bridge() == "#"  ||  board[x][y].get_bridge() == "="  ||  board[x][y].get_bridge() == "|"){
            found = true;
        }

        x++;
    }

    return numPaths;
}

// DEPTH FIRST SEARCH ========================================================================
void Graph::DepthFirstSearch(std::vector<std::vector<Node>> &board, Node &n){



}

// TEST CONNECTION ===========================================================================
// Uses Breadth First Search to determine connectivity of the graph
bool Graph::TestConnection(std::vector<std::vector<Node> > &board){
    bool connected = false;

    // TO DO: breadth first search

    return connected;
}


// PRINT AVAILABLE NODES =====================================================================
int Graph::AvailableNodes(std::vector<std::vector<Node>> &board) const {
    std::cout << "\nAll Nodes: ";

    int remainingPaths = 0;

    for (int row = max_y; row >=0; row--){
        for(int col = 0; col <= max_x; col++)
            if(board[col][row].get_num_paths()){
                std::cout << "\nPoint: " << board[col][row].get_pt() <<  " Available Paths: "
                          << board[col][row].num_available_paths();
                remainingPaths++;
            }
    }

    return remainingPaths;
}



// PRINT SOLUTION ========================================================================
void Graph::printSolution() const {
  std::cout << "\n\nSolution:" << std::endl;
  for (int i = 0; i < edges.size(); i++) {
    std::cout << edges[i] << std::endl;
  }
}


// PRINT GRAPH ===========================================================================
void Graph::printGraph() const {
  // a graph should have at least one node/island
  assert (nodes.size() > 1);
  // the dimensions of the board should be positive
  assert (max_x >= 0);
  assert (max_y >= 0);

  // allocate an empty/blank board, twice as big in each dimension so
  // we can draw lines between the nodes/islands.
  std::vector<std::string> board(2*max_y+1,std::string(2*max_x+1,' '));
  // mark all of the islands with their bridge count
  for (int i = 0; i < nodes.size(); i++) {
    const Point &pt = nodes[i].get_pt();
    std::string value = std::to_string(nodes[i].get_num_paths());
    board[2*pt.get_y()][2*pt.get_x()] = value[0];
  }

  // loop over all of the edges/bridges
  for (int i = 0; i < edges.size(); i++) {
    Point a = edges[i].get_a();
    Point b = edges[i].get_b();
    // determine the edge/edge direction
    int diff_x = b.get_x()-a.get_x();
    int diff_y = b.get_y()-a.get_y();
    if (diff_x > 0) diff_x = 1;
    if (diff_x < 0) diff_x = -1;
    if (diff_y > 0) diff_y = 1;
    if (diff_y < 0) diff_y = -1;

    // walk between the islands to draw the bridge
    int x = a.get_x()*2+diff_x;
    int y = a.get_y()*2+diff_y;
    while (x != b.get_x()*2 || y != b.get_y()*2) {
      if (diff_x == 0) {
        // vertical bridge
        assert (diff_y != 0);
        if (board[y][x] == '|') {
          // if we already have a bridge, draw a double vertical bridge
          board[y][x] = '#';
        } else {
          assert (board[y][x] == ' ');
          // otherwise draw a single vertical bridge
          board[y][x] = '|';
        }
      } else {
        assert (diff_y == 0);
        // horizontal bridge
        if (board[y][x] == '-') {
          // if we already have a bridge, draw a double horizontal bridge
          board[y][x] = '=';
        } else {
          // otherwise draw a single horizontal bridge
          board[y][x] = '-';
        }
      }
      x+=diff_x;
      y+=diff_y;
    }
  }

  // surround the board with a border of spaces
  char border = ' ';
  std::cout << std::string(board[0].size()+2,border) << std::endl;
  for (int i = board.size()-1; i >= 0; i--) {
    std::cout << border << board[i] << border << std::endl;
  }
  std::cout << std::string(board[0].size()+2,border) << std::endl;
}

// PRINT EDGES - DEBUGGING =====================================================================
void Graph::printEdges(std::vector<std::vector<Node>> &board) const {
    std::cout << "\nAll Edges:\n";

    for (int row = max_y; row >= 0; row--){
        for(int col = 0; col <= max_x; col++){
            if(board[col][row].get_bridge() != "")
                std::cout << board[col][row].get_bridge();
            else
                std::cout << "*";
        }
        std::cout << "\n";
    }
}
