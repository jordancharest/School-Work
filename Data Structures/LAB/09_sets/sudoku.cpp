#include <vector>
#include <set>
#include <iostream>
#include <fstream>
#include <math.h>
#include <cassert>
#include "sudoku.h"

using std::set;
using std::vector;
using std::cout;
using std::cin;
using std::endl;

// ======================================================
// A program to solve very simple Sudoku puzzles
// ======================================================

int main(int argc, char* argv[]) {

  std::ifstream InputStream(argv[1]);
  if (!InputStream.good()){
    std::cerr << "Can't open" << argv[1] << ". Exiting program.\n";
    exit(1);
  }


  int size;
  while (InputStream >> size) {
    // Read in the board size and create an empty board
    Sudoku board(size);

    // Read in and set the known positions in the board
    char c;
    for (int i = 0; i < size; i++) {
      for (int j = 0; j < size; j++) {
        InputStream >> c;
        if (c != '.') {
          board.Set(i,j,c-'0');
        }
      }
    }

    // The iterative solving routine
    while (1) {

      // Is the puzzle is already solved?
      if (board.IsSolved()) {
        cout << "\nPuzzle is solved:" << endl;
        board.Print();
        break;
      }

      // Is the puzzle impossible?
      if (board.Impossible()) {
        cout << "\nCannot solve this puzzle:" << endl;
        board.Print();
        break;
      }

      // Try to reduce the number of choices in one or more board cells
      // by propagating known values within each row, column, and quadrant
      int changed = 0;
      for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
          if (board.KnownValue(i,j))
            changed += board.Propagate(i,j);
        }
      }

      // If nothing changed this iteration, give up
      if (changed == 0) {
        cout << "\nPuzzle is too difficult for this solver!" << endl;
        board.Print();
        break;
      }
    }
  }
}

// ======================================================

// Construct an empty board
Sudoku::Sudoku(int s) {
  // set the size
  size = s;
  quadrant_size = (int)sqrt(size);
  assert (quadrant_size*quadrant_size == s);


  std::vector<std::set<int> > column;
  for (int i = 0; i < size; i++)
    board.push_back(column);

  std::set<int> possible_values;
  for (int i = 1; i <= size; i++)
    possible_values.insert(i);



  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++)
      board[i].push_back(possible_values);
  }
}

// To construct the puzzle, set the value of a particular cell
void Sudoku::Set(int i, int j, int value) {
  // make sure the value is one of the legal values
  assert (board[i][j].find(value) != board[i][j].end());
  board[i][j].clear();
  board[i][j].insert(value);
}

// Does the cell have exactly one legal value?
bool Sudoku::KnownValue(int i, int j) const {
  return (board[i][j].size() == 1);
}

// If there is exactly one number in each cell of the grid, the puzzle is solved!
bool Sudoku::IsSolved() const {

  for (int i  = 0; i < size; i++)
  for (int j = 0; j < size; j++) {
    if (board[i][j].size() > 1)
      return false;
  }

  return true;
}

// If there are no legal choices in one or more cells of the grid, the puzzle is impossible
bool Sudoku::Impossible() const {
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      if (board[i][j].size() == 0)
        return true;
    }
  }
  return false;
}

// print out the solution
void Sudoku::Print() const {

  int solved = IsSolved();
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      int count = size+1;
      for (set<int>::iterator itr = board[i][j].begin(); itr != board[i][j].end(); itr++) {
        count--;
        cout << *itr;
      }
      if (!solved) {
        // if it's not solved, print out spaces to separate the values
        for (int k = 0; k < count; k++) {
          cout << " ";
        }
      }
    }
    cout << endl;
  }
}


// Propagate information from this cell along columns & rows &
// within quadrant. Return the number of cells that changed.
int Sudoku::Propagate(int i, int j) {

  int value = *board[i][j].begin();
  int changes = 0;

  // propagate through the rows
  for (int k = 0; k < size; k++) {
    if (k != i)
      changes += board[k][j].erase(value);
  }

  // propagate through the columns
  for (int k = 0; k < size; k++) {
    if (k != j)
      changes += board[i][k].erase(value);
  }

  // propagate through the blocks
  int row_block = i/quadrant_size;
  int col_block = j/quadrant_size;

  for (int m = row_block*quadrant_size; m < row_block*quadrant_size + quadrant_size; m++){
    for (int n = col_block*quadrant_size; n < col_block*quadrant_size + quadrant_size; n++)
      if (m != i  &&  n != j)
        changes += board[m][n].erase(value);
  }

  return changes;

}
