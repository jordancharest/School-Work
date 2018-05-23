#ifndef ESSAY_H_INCLUDED
#define ESSAY_H_INCLUDED


#include <string>


// -----------------------------------------------------------------
// NODE CLASS
class Node {
public:
  Node() : next_(NULL), prev_(NULL) {}
  Node(const std::string& v) : value_(v), next_(NULL), backup_next_(NULL), prev_(NULL), backup_prev_(NULL), added_(false), removed_(false) {}

  // given in assignment (defined in main.cpp)
  void debug() const;

  // REPRESENTATION
  std::string value_;
  Node* next_;
  Node* backup_next_;
  Node* prev_;
  Node* backup_prev_;
  bool added_;
  bool removed_;
};


// A "forward declaration" of this class is needed
class Essay;

// ----------------------------------------------------------------------------
// LIST ITERATOR CLASS
class list_iterator {
public:
  // default constructor, copy constructor, assignment operator, & destructor
  list_iterator(Node* p=NULL, bool backup=false) : ptr_(p), backup_(backup) {}
  // NOTE: the implicit compiler definitions of the copy constructor,
  // assignment operator, and destructor are correct for this class

  // dereferencing operator gives access to the value at the pointer
  std::string& operator*()  { return ptr_->value_;  }

  // increment & decrement operators
  list_iterator& operator++() { // pre-increment, e.g., ++iter
    if (backup_)
      ptr_ = ptr_->backup_next_;
    else
      ptr_ = ptr_->next_;
    return *this;
  }
  list_iterator operator++(int) { // post-increment, e.g., iter++
    list_iterator temp(*this);
    if (backup_)
      ptr_ = ptr_->backup_next_;
    else
      ptr_ = ptr_->next_;
    return temp;
  }
  list_iterator& operator--() { // pre-decrement, e.g., --iter
    if (backup_)
      ptr_ = ptr_->backup_prev_;
    else
      ptr_ = ptr_->prev_;
    return *this;
  }
  list_iterator operator--(int) { // post-decrement, e.g., iter--
    list_iterator temp(*this);
    if (backup_)
      ptr_ = ptr_->backup_prev_;
    else
        ptr_ = ptr_->prev_;
    return temp;
  }
  // the Essay class needs access to the private ptr_ member variable
  friend class Essay;

  // Comparions operators are straightforward
  bool operator==(const list_iterator& r) const {
    return ptr_ == r.ptr_; }
  bool operator!=(const list_iterator& r) const {
    return ptr_ != r.ptr_; }

private:
  // REPRESENTATION
  Node* ptr_;    // ptr to node in the list
  bool backup_;
};







class Essay {
public:
  // default constructor, copy constructor, assignment operator, & destructor
  Essay() : head_(NULL), tail_(NULL), dummytail_(NULL), backup_head_(NULL), backup_tail_(NULL), backup_dummytail_(NULL), size_(0), numAdded_(0), numRemoved_(0) {}
  Essay(const Essay& old) { copy_list(old); }
  Essay& operator= (const Essay& old);
  ~Essay() { destroy_list(); }

  typedef list_iterator iterator;

  // given in assignment (defined in main.cpp)
  void debug() const;

  // simple accessors & modifiers
  unsigned int size() const { return size_; }
  bool empty() const { return head_ == NULL; }
  void clear() { destroy_list(); }
  unsigned int num_added() const {return numAdded_; }
  unsigned int num_removed() const {return numRemoved_; }

  // read/write access to contents
  const std::string& front() const { return head_->value_;  }
  std::string& front() { return head_->value_; }
  const std::string& back() const { return tail_->value_; }
  std::string& back() { return tail_->value_; }

  // modify the linked list structure
  void push_front(const std::string& v);
  void pop_front();
  void push_back(const std::string& v);
  void pop_back();

  void backup();
  void revert();

  iterator erase(iterator itr);
  iterator insert(iterator itr, const std::string& v);
  iterator begin() { return iterator(head_); }
  iterator end() { return iterator(dummytail_); }
  iterator backup_begin() { return iterator(backup_head_, true); }
  iterator backup_end() { return iterator(backup_dummytail_, true); }

  // list iterator needs access to tail_ and head_
  friend class list_iterator;

private:
  // private helper functions
  void copy_list(const Essay& old);
  void destroy_list();

  //REPRESENTATION
  Node* head_;
  Node* tail_;
  Node* dummytail_;
  Node* backup_head_;
  Node* backup_tail_;
  Node* backup_dummytail_;
  unsigned int size_;
  unsigned int numAdded_;
  unsigned int numRemoved_;
};









#endif // ESSAY_H_INCLUDED
