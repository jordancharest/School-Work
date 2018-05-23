// ===================================================================
//
// We provide the Point class and the implementation of several
// QuadTree member functions for printing.
//
// IMPORTANT: You should modify this file to add all of the necessary
// functionality for the QuadTree class and its helper classes: Node,
// DepthIterator, and BreadthIterator.
//
// ===================================================================

#ifndef quad_tree_h_
#define quad_tree_h_

#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <cassert>
#include <utility>
#include <queue>


// ==============================================================
// ==============================================================
// A tiny templated class to store 2D coordinates.  This class works
// with number_type = int, float, double, unsigned char (a 1 byte=8
// bit integer), short (a 2 byte=16 bit integer).

template <class number_type>
class Point {
public:
    // Constructors
    Point(){}
    Point(const number_type& x_, const number_type& y_) : x(x_),y(y_) {}

    // REPRESENTATION
    number_type x;
    number_type y;
};

// a helper function to print Points to STL output stream
template <class number_type>
inline std::ostream& operator<<(std::ostream &ostr, const Point<number_type> &pt) {
  ostr << "(" << pt.x << "," << pt.y << ")";
  return ostr;
}

// NODE IMPLEMENTATION =============================================================
template <class number_type, class label_type>
class Node {
public:
    // Constructor
    Node(const Point<number_type> &pt_, const label_type &label_) {
        for (int i = 0; i < 4; i++)
            children[i] = nullptr;
        parent = nullptr;
        pt = Point<number_type>(pt_.x, pt_.y);
        label = label_;
    }


    label_type label;
    Point<number_type> pt;
    Node<number_type, label_type>* children[4];
    Node<number_type, label_type>* parent;
};

// =================================================================================
// NODE BREADTH ITERATOR CLASS
// Iterator that traverses the QuadTree in a breadth-first manner
template <class number_type, class label_type>
class BreadthIterator {
public:
    // Constructors, Destructor, Assignment Operator
    BreadthIterator() : ptr_(NULL) {}
    BreadthIterator(Node<number_type, label_type>* p) : ptr_(p) {}
    BreadthIterator(const BreadthIterator& old) : ptr_(old.ptr_) {}
    ~BreadthIterator() {}
    BreadthIterator& operator=(const BreadthIterator& old) { ptr_ = old.ptr_;  return *this; }

    // operator* gives constant access to the point at the node pointer
    const Point<number_type>& operator*() const { return ptr_->pt; }

    // Comparison operators
    bool operator== (const BreadthIterator& rgt) { return ptr_ == rgt.ptr_; }
    bool operator!= (const BreadthIterator& rgt) { return ptr_ != rgt.ptr_; }

    // Operators for increment and decrement
    BreadthIterator& operator++() { // pre-increment, e.g., ++iter
        Node<number_type, label_type>* root = ptr_;

        while (root->parent != nullptr) { root = root->parent; }
        ptr_ = increment(root);

        return *this;
    }

    BreadthIterator operator++(int) { // post-increment, e.g., iter++
        BreadthIterator temp(*this);
        Node<number_type, label_type>* root = ptr_;

        while (root->parent != nullptr) { root = root->parent; }
        ptr_ = increment(root);

        return temp;
    }

    BreadthIterator& operator--() { // pre-decrement, e.g., --iter
        Node<number_type, label_type>* root = ptr_;

        while (root->parent != nullptr) { root = root->parent; }
        ptr_ = decrement(root);

        return *this;
    }

    BreadthIterator operator--(int) { // post-decrement, e.g., iter--
        BreadthIterator temp(*this);
        Node<number_type, label_type>* root = ptr_;

        while (root->parent != nullptr) { root = root->parent; }
        ptr_ = decrement(root);

        return temp;
    }

    // INCREMENT (used for operator++)
    Node<number_type, label_type>* increment(Node<number_type, label_type>* root) {
        std::queue<Node<number_type, label_type>* > Q;
        Node<number_type, label_type>* current_node = root;
        bool return_next = false;

        // Breadth First Search
        Q.push(current_node);       // start at root
        while (!Q.empty()) {
            current_node = Q.front();
            Q.pop();

            if (return_next) return current_node;
            else if (current_node == ptr_) return_next = true;  // if true, will return the next node (on the next loop)

            for (int i = 0; i < 4; ++i) {
                if (current_node->children[i])
                    Q.push(current_node->children[i]);
            }
        }

        return nullptr;     // reaching this point means we incremented to the end of the tree
    }

    // DECREMENT (used for operator--)
    Node<number_type, label_type>* decrement(Node<number_type, label_type>* root) {
        std::queue<Node<number_type, label_type>* > Q;
        Node<number_type, label_type>* current_node = root;
        Node<number_type, label_type>* previous_node = root;

        // Breadth-First Search
        Q.push(current_node);       // start at root
        while (!Q.empty()) {
            previous_node = current_node;
            current_node = Q.front();
            Q.pop();

            if (current_node == ptr_) return previous_node; // want the node previous to the one we already have

            for (int i = 0; i < 4; ++i) {
                if (current_node->children[i])
                    Q.push(current_node->children[i]);
            }
        }

        return nullptr;
    }


    // ACCESSORS
    label_type getLabel() const { return ptr_->label; }

    unsigned int getDepth() const {
        Node<number_type, label_type>* root = ptr_;
        int i = 0;
        while (root->parent != nullptr) {   // count how many steps up we take take to reach root
                root = root->parent;
                ++i;
        }
        return i;
    }

private:
    // REPRESENTATION
    Node<number_type, label_type>* ptr_;
};





// =================================================================================
// NODE DEPTH ITERATOR CLASS
template <class number_type, class label_type>
class DepthIterator {
public:
    // Constructors, Destructor, Assignment Operator
    DepthIterator() : ptr_(NULL) {}
    DepthIterator(Node<number_type, label_type>* p) : ptr_(p) {}
    DepthIterator(const DepthIterator& old) : ptr_(old.ptr_) {}
    ~DepthIterator() {}
    DepthIterator& operator=(const DepthIterator& old) { ptr_ = old.ptr_;  return *this; }

    // operator* gives constant access to the point at the node pointer
    const Point<number_type>& operator*() const { return ptr_->pt; }

    // Comparison operators are straightforward
    bool operator== (const DepthIterator& rgt) { return ptr_ == rgt.ptr_; }
    bool operator!= (const DepthIterator& rgt) { return ptr_ != rgt.ptr_; }

    // Operators for increment and decrement
    DepthIterator& operator++() { // pre-increment, e.g., ++iter
        Node<number_type, label_type>* root = ptr_;

        while (root->parent != nullptr) { root = root->parent; }
        bool return_next = false;

        increment(root, return_next);

        return *this;
    }

    DepthIterator operator++(int) { // post-increment, e.g., iter++
        DepthIterator temp(*this);
        Node<number_type, label_type>* root = ptr_;

        while (root->parent != nullptr) { root = root->parent; }
        bool return_next = false;

        increment(root, return_next);

        return temp;
    }

    DepthIterator& operator--() { // pre-decrement, e.g., --iter
        if (ptr_->parent == nullptr) ptr_ = nullptr;
        else {
            Node<number_type, label_type>* root = ptr_;
            Node<number_type, label_type>* previous_node = ptr_;

            while (root->parent != nullptr) { root = root->parent; }
            bool found = false;
            decrement(root, previous_node, found);
        }
        return *this;
    }

    DepthIterator operator--(int) { // post-decrement, e.g., iter--
        DepthIterator temp(*this);

        if (ptr_->parent == nullptr) ptr_ = nullptr;
        else {

            Node<number_type, label_type>* root = ptr_;
            Node<number_type, label_type>* previous_node = ptr_;

            while (root->parent != nullptr) { root = root->parent; }
            bool found = false;
            decrement(root, previous_node,found);
        }

        return temp;
    }

    // INCREMENT (used by operator++)
    void increment(Node<number_type, label_type>* current_node, bool &return_next) { // in first call of increment, current_node is the root

        if (return_next) {
            ptr_ = current_node;
            return_next = false;
            return;
        }

        if (current_node == ptr_)
            return_next = true;

        // Depth First Search
        for (int i = 0; i < 4; ++i) {
            if (current_node->children[i])
                increment(current_node->children[i], return_next);
        }

        if (return_next) ptr_ = nullptr;    // if we make it this far then we are at the end of the tree
    }

    // DECREMENT (used by operator--)
    void decrement(Node<number_type, label_type>* &current_node, Node<number_type, label_type>* &previous_node, bool &found) {
        //std::cout << "\nCurrently Decrementing " << current_node->label << "  Previous: " << previous_node->label;
        if (found) return;
        if (current_node == ptr_){ ptr_ = previous_node;
            found = true;
        }

        previous_node = current_node;
        // Depth First Search
        for (int i = 0; i < 4; ++i) {
            if (current_node->children[i]) {
                decrement(current_node->children[i], previous_node, found);
            }
        }
    }


    // ACCESSORS
    label_type getLabel() const { return ptr_->label; }
    unsigned int getDepth() const {
        Node<number_type, label_type>* root = ptr_;
        int i = 0;
        while (root->parent != nullptr) {
                root = root->parent;
                ++i;
        }
        return i;
    }


// PRIVATE ---------------------------------------------------------------------------------
private:
    // REPRESENTATION
    Node<number_type, label_type>* ptr_;
};




// QUAD TREE DECLARATION ====================================================================================================================
template<class number_type, class label_type>
class QuadTree {
public:
    // Constructors, destructors, assignment operator
    QuadTree() : root_(nullptr), size_(0) {}
    QuadTree(const QuadTree& Q) { copy(Q); }
    ~QuadTree() { this->destroy(root_);  root_ = NULL; }
    QuadTree& operator=(const QuadTree<number_type, label_type>& Q) {
        if (&Q != this) {
          this->destroy(root_);
          this->copy(Q);
        }
        return *this;
    }

    // Iterators
    typedef DepthIterator<number_type, label_type> iterator;
    iterator begin() const { return iterator(root_); }
    iterator end() const { return iterator(nullptr); }

    typedef BreadthIterator<number_type, label_type> bf_iterator;
    bf_iterator bf_begin() const { return bf_iterator(root_); }
    bf_iterator bf_end() const { return bf_iterator(nullptr); }

    // Visualization
    void plot(int max_x, int max_y, bool draw_lines=true) const;
    void print_sideways() const { print_sideways(root_,""); }


    // Accessors and Calculations
    unsigned int size() { return size_; }
    int height() { if (!root_) return -1;  else return height(root_); };

    // Insert and Find
    std::pair<iterator,bool> insert(const Point<number_type>& pt, label_type label) { return insert(pt, label, root_, nullptr); }
    iterator find(const number_type& x, const number_type& y) { return find(x, y, root_); }


// PRIVATE ----------------------------------------------------------------------------------------------------------------------
private:

    // Visualization
    void print_sideways(Node<number_type,label_type>* p, const std::string &indent) const;
    void plot(Node<number_type,label_type> *p, std::vector<std::string> &board,
          int x_min, int x_max, int y_min, int y_max, bool draw_lines=true) const;

    // Constructors
    void copy(const QuadTree<number_type, label_type>& Q);
    void destroy(Node<number_type,label_type>* node);


    // INSERT ------------------------------------------------------------------------------------------------------------------
    std::pair<iterator,bool> insert(const Point<number_type>& point, label_type label,
                                    Node<number_type, label_type>*& p,  Node<number_type, label_type>* the_parent) {

        // Uncomment the outputs to better visualize the insert process

        //if(p) std::cout << "\nInserting (" << point.x << "," << point.y << ")  Comparing to (" << p->pt.x << "," << p->pt.y << ")";

        if (!p) {
            //std::cout << "\nMaking new child\n";
            p = new Node<number_type, label_type>(point, label);
            p->parent = the_parent;
            this->size_++;
            return std::pair<iterator,bool>(iterator(p), true);

        } else if (point.x < p->pt.x  &&  point.y < p->pt.y) {
            //std::cout << "\nTop left quadrant - children[0]\n";
            return insert(point, label, p->children[0], p);

        } else if (point.x > p->pt.x  &&  point.y < p->pt.y) {
            //std::cout << "\nTop right quadrant - children[1]\n";
            return insert(point, label, p->children[1], p);

        } else if (point.x < p->pt.x  &&  point.y > p->pt.y) {
            //std::cout << "\nLower left quadrant - children[2]\n";
            return insert(point, label, p->children[2], p);

        } else if (point.x > p->pt.x  &&  point.y > p->pt.y) {
            //std::cout << "\nLower right quadrant - children[3]\n";
            return insert(point, label, p->children[3], p);

        } else {
            //std::cout << "\nCould not insert\n";
            return std::pair<iterator,bool>(iterator(p), false);
        }
    }

    // FIND ------------------------------------------------------------------------------------------
    iterator find(const number_type& x, const number_type& y, Node<number_type, label_type>*& p) {
        // empty tree or the point does not exist in the tree
        if (!p) return iterator(nullptr);

        // for all four quadrants, follow the tree down until you find the point or hit nullptr
        if (x < p->pt.x  &&  y < p->pt.y)
            return find(x, y, p->children[0]);

        else if (x > p->pt.x  &&  y < p->pt.y)
            return find(x, y, p->children[1]);

        else if (x < p->pt.x  &&  y > p->pt.y)
            return find(x, y, p->children[2]);

        else if (x > p->pt.x  &&  y > p->pt.y)
            return find(x, y, p->children[3]);


        // found the point
        else return iterator(p);
    }

    // HEIGHT -----------------------------------------------------------------------------------------------------------------------
    int height(Node<number_type, label_type>*& p) {
        if (!p) return -1;
        else return std::max( {height(p->children[0]), height(p->children[1]), height(p->children[2]), height(p->children[3])} ) + 1;
    }

    // MEMBER REPRESENTATION --------------------------------
    Node<number_type, label_type>* root_;
    int size_;

};

// =====================================================================================
// QUAD TREE IMPLEMENTATION
// =====================================================================================

// COPY --------------------------------------------------------------------------------
template <class number_type, class label_type>
void QuadTree<number_type, label_type>::copy(const QuadTree<number_type, label_type>& Q) {

    this->root_ = new Node<number_type, label_type>(Q.root_->pt, Q.root_->label);
    this->size_ = 1;

    std::queue<Node<number_type, label_type>* > nodes;
    Node<number_type, label_type>* current_node;

    // Breadth First Search
    nodes.push(Q.root_);       // start at root
    while (!nodes.empty()) {
        current_node = nodes.front();
        nodes.pop();

        this->insert(current_node->pt, current_node->label);

        for (int i = 0; i < 4; ++i) {
            if (current_node->children[i])
                nodes.push(current_node->children[i]);
        }
    }
}

// DELETE ------------------------------------------------------------------------------
template <class number_type, class label_type>
void QuadTree<number_type, label_type>::destroy(Node<number_type,label_type>* root) {

    std::queue<Node<number_type, label_type>* > nodes;
    Node<number_type, label_type>* current_node = root;

    // Breadth First Search
    nodes.push(current_node);       // start at root
    while (!nodes.empty()) {
        current_node = nodes.front();
        nodes.pop();

        for (int i = 0; i < 4; ++i) {
            if (current_node->children[i])
                nodes.push(current_node->children[i]);
        }
        delete current_node;
    }
    this->root_ = nullptr;
}




// NOTE: this function only works for quad trees with non negative
// integer coordinates and char labels

// NOTE2: this function assumes that no two points have the same x
// coordinate or the same y coordinate.

// plot driver function
// takes in the maximum x and y coordinates for these data points
// the optional argument draw_lines defaults to true
template<class number_type, class label_type>
void QuadTree<number_type, label_type>::plot(int max_x, int max_y, bool draw_lines) const {
  // allocate blank space for the center of the board
  std::vector<std::string> board(max_y+1,std::string(max_x+1,' '));
  // edit the board to add the point labels and draw vertical and
  // horizontal subdivisions
  plot(root_,board,0,max_x,0,max_y,draw_lines);
  // print the top border of the plot
  std::cout << "+" << std::string(max_x+1,'-') << "+" << std::endl;
  for (int i = 0; i <= max_y; i++) {
    // print each row of the board between vertical border bars
    std::cout << "|" << board[i] << "|" << std::endl;
  }
  // print the top border of the plot
  std::cout << "+" << std::string(max_x+1,'-') << "+" << std::endl;
}

// actual recursive function for plotting
template<class number_type, class label_type>
void QuadTree<number_type, label_type>::plot(Node<number_type,label_type> *p, std::vector<std::string> &board,
          int x_min, int x_max, int y_min, int y_max, bool draw_lines) const {
  // base case, draw nothing if this node is NULL
  if (p == NULL) return;
  // check that the dimensions range of this node make sense
  assert (x_min >= 0 && x_min <= x_max);
  assert (y_min >= 0 && y_min <= y_max);
  assert (board.size() >= y_max);
  assert (board[0].size() >= x_max);
  // verify that the point stored at this node fits on the board
  assert (p->pt.y >= 0 && p->pt.y < board.size());
  assert (p->pt.x >= 0 && p->pt.x < board[0].size());
  // draw the vertical and horizontal bars extending across the
  // range of this node
  if (draw_lines) {
    for (int x = x_min; x <= x_max; x++) {
      board[p->pt.y][x] = '-';
    }
    for (int y = y_min; y <= y_max; y++) {
      board[y][p->pt.x] = '|';
    }
  }
  // draw this label
  board[p->pt.y][p->pt.x] = p->label;
  // recurse on the 4 children
  plot(p->children[0],board,x_min ,p->pt.x-1,y_min ,p->pt.y-1,draw_lines);
  plot(p->children[1],board,p->pt.x+1,x_max ,y_min ,p->pt.y-1,draw_lines);
  plot(p->children[2],board,x_min ,p->pt.x-1,p->pt.y+1,y_max ,draw_lines);
  plot(p->children[3],board,p->pt.x+1,x_max ,p->pt.y+1,y_max ,draw_lines);
}


// ==============================================================

// prints all of the tree data with a pre-order (node first, then
// children) traversal of the tree structure

// actual recursive function
template<class number_type, class label_type>
void QuadTree<number_type, label_type>::print_sideways(Node<number_type,label_type>* p, const std::string &indent) const {
  // base case
  if (p == NULL) return;
  // print out this node
  std::cout << indent << p->label << " (" << p->pt.x << "," << p->pt.y << ")" << std::endl;
  // recurse on each of the children trees
  // increasing the indentation
  print_sideways(p->children[0],indent+"  ");
  print_sideways(p->children[1],indent+"  ");
  print_sideways(p->children[2],indent+"  ");
  print_sideways(p->children[3],indent+"  ");
}

// ==============================================================
// ==============================================================


#endif
