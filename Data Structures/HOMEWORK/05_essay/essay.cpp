// essay.cpp
// Implementation file for the Essay Class
// Stores an essay as a linked list of words
#include <cassert>
#include "essay.h"

void Essay::backup(){



    for (Node *n = head_; n != NULL; n = n->next_) {
        n->backup_prev_ = n->prev_;
        n->backup_next_ = n->next_;
        n->added_ = false;
        n->removed_ = false;
    }

    backup_head_ = head_;
    backup_tail_ = tail_;
    backup_dummytail_ = dummytail_;


    numAdded_ = 0;
    numRemoved_ = 0;
}


void Essay::revert(){



}


Essay& Essay::operator= (const Essay& old) {
  // check for self-assignment
  if (&old != this) {
    destroy_list();
    copy_list(old);
  }
  return *this;
}


void Essay::push_front(const std::string& v) {
  Node* newp = new Node(v);
  // special case: initially empty list
  if (!head_) {
    Node* newtail = new Node();
    dummytail_ = newtail;

    head_ = tail_ = newp;

    tail_->next_ = dummytail_;
    dummytail_->prev_ = tail_;
  } else {
    // normal case: at least one node already
    newp->next_ = head_;
    head_->prev_ = newp;
    head_ = newp;
    }
  ++size_;
  ++numAdded_;
}


void Essay::pop_front() {
    //empty list
    if (!size_){
        return;

    } else if (size_ == 1){
        delete head_;   // head_ and tail_ are the same
        delete dummytail_;

    /*  assign the tail to the second to last node,
        delete the former tail, and reatttach thedummytail  */
    } else {
        Node* temp = head_;
        head_ = head_->next_;
        head_->prev_ = 0;
        delete temp;
        --size_;
        ++numRemoved_;
    }

}


void Essay::push_back(const std::string& v) {
  Node* newp = new Node(v);
  // special case: initially empty list
  if (!tail_) {
    Node* newtail = new Node();
    dummytail_ = newtail;

    head_ = tail_ = newp;

    tail_->next_ = dummytail_;
    dummytail_->prev_ = tail_;
  } else {
    // normal case: at least one node already
    newp->prev_ = tail_;
    tail_->next_ = newp;
    tail_ = newp;
    tail_->next_ = dummytail_;
    dummytail_->prev_ = tail_;
    }
  ++size_;
  ++numAdded_;
}


void Essay::pop_back() {
    //empty list
    if (!size_){
        return;

    } else if (size_ == 1){
        delete head_;   // head_ and tail_ are the same
        delete dummytail_;

    /*  assign the tail to the second to last node,
        delete the former tail, and reatttach the dummytail  */
    } else {
        Node* temp = tail_;
        tail_ = tail_->prev_;
        tail_->next_ = dummytail_;
        dummytail_->prev_ = tail_;
        delete temp;
        --size_;
        ++numRemoved_;
    }

}

// do these lists look the same (length & contents)?

bool operator== (Essay& left, Essay& right) {
  if (left.size() != right.size()) return false;
  typename Essay::iterator left_itr = left.begin();
  typename Essay::iterator right_itr = right.begin();
  // walk over both lists, looking for a mismatched value
  while (left_itr != left.end()) {
    if (*left_itr != *right_itr) return false;
    left_itr++; right_itr++;
  }
  return true;
}


bool operator!= (Essay& left, Essay& right){ return !(left==right); }

typename Essay::iterator Essay::erase(iterator itr) {
  assert (size_ > 0);
  --size_;
  ++numRemoved_;

  iterator result(itr.ptr_->next_);
  // One node left in the list.
  if (itr.ptr_ == head_ && head_ == tail_) {
    head_ = tail_ = 0;
  }
  // Removing the head in a list with at least two nodes
  else if (itr.ptr_ == head_) {
    head_ = head_->next_;
    head_->prev_ = 0;
  }
  // Removing the tail in a list with at least two nodes
  else if (itr.ptr_ == tail_) {
    tail_ = tail_->prev_;
    tail_->next_ = 0;
  }
  // Normal remove
  else {
    itr.ptr_->backup_next_ = itr.ptr_->next_;
    itr.ptr_->backup_prev_ = itr.ptr_->prev_;

    itr.ptr_->prev_->next_ = itr.ptr_->next_;
    itr.ptr_->next_->prev_ = itr.ptr_->prev_;


  }
  if (itr.backup_)
    delete itr.ptr_;
  return result;
}



typename Essay::iterator Essay::insert(iterator itr, const std::string& v) {
  ++size_ ;
  ++numAdded_;

  // needed for the backup essay, need the current list before value is inserted
  Node* before = itr.ptr_->prev_;
  Node* after = itr.ptr_;

  Node* p = new Node(v);
  p->prev_ = itr.ptr_->prev_;
  p->next_ = itr.ptr_;
  itr.ptr_->prev_ = p;
  if (itr.ptr_ == head_)
    head_ = p;
  else
    p->prev_->next_ = p;


  // make sure the backed up essay skips over the new word
  if (backup_tail_){
    before->backup_next_ = after;
    after->backup_prev_ = before;
  }

  return iterator(p);
}



void Essay::copy_list(const Essay& old) {
  size_ = old.size_;
  // Handle the special case of an empty list.
  if (size_ == 0) {
    head_ = tail_ = 0;
    return;
  }
  // Create a new head node.
  head_ = new Node(old.head_->value_);
  // tail_ will point to the last node created and therefore will move
  // down the new list as it is built
  tail_ = head_;
  // old_p will point to the next node to be copied in the old list
  Node* old_p = old.head_->next_;
  // copy the remainder of the old list, one node at a time
  while (old_p) {
    tail_->next_ = new Node(old_p->value_);
    tail_->next_->prev_ = tail_;
    tail_ = tail_->next_;
    old_p = old_p->next_;
  }
}


void Essay::destroy_list() {

    // do nothing if the list is empty
    if (size_ == 0)
        return;

    // else the list has at least one element
    Node *current = head_;
    Node *nextNode = head_->next_;

    while (nextNode){
        delete current;
        current = nextNode;
        nextNode = nextNode->next_;
    }

    // current now points to the last node
    delete current;

    head_ = tail_ = NULL;
    size_ = 0;

}

