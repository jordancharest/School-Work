#include <fstream>
#include <cassert>
#include <cstdlib>
#include <iostream>

#include "graph.h"

const int BOARD_SIZE = 25;


// Function Declarations
void PrintBoard(const std::vector<std::vector<Node> > &board);



// MAIN ===================================================================================
int main(int argc, char* argv[] ) {

    // Parse the arguments
    if (argc < 2) {
    std::cout << "ERROR!  Must specify input file" << std::endl;
    exit(1);
    }
    std::ifstream istr(argv[1]);
    if (!istr.good()) {
    std::cout << "ERROR!  Could not open input file '" << argv[1] << "'" << std::endl;
    exit(1);
    }
    bool find_all_solutions = false;
    bool connected = false;
    for (int i = 2; i < argc; i++) {
    if (argv[i] == std::string("--find_all_solutions")) {
      find_all_solutions = true;
    } else if (argv[i] == std::string("--connected")) {
      connected = true;
    } else {
      std::cout << "ERROR!  Unknown argument '" << argv[i] << "'" << std::endl;
      exit(1);
    }
    }


    // Create a board with every node: for constant time checking of whether
    // a node exists at a given location; always use 25x25 boardsize, memory is cheap
    std::vector<std::vector<Node> > board(BOARD_SIZE);

    for (int row = 0; row < BOARD_SIZE; row++){
        for (int col = 1; col < BOARD_SIZE; col++){
            board[row].push_back(Node(Point(row,col), 0));
        }
    }



    // Create an empty graph object
    Graph g;
    // Read in the puzzle from the input file
    int x,y,n;
    std::vector<Point> pts;
    while (istr >> x >> y >> n) {
        pts.push_back(Point(x,y));
        g.addNode(Point(x,y),n);

    // populate the board
        board[x][y] = Node(Point(x,y),n);
    }

    //PrintBoard(board);






    /* BASIC TECHNIQUES
        These functions build bridges on paths that are unconditionally required.
        They do no verification, and assume that the board is solvable.
        Solves 4-bridge corners, 6-bridge edges, and 8-bridge islands.
    */
    g.SolveRequired(board);

    /* INTERMEDIATE TECHNIQUES
        Cycles through all nodes, making connections
        if the # of bridges matches the # of neighbors.
        Recursive calls if a cycle is made through the board
        and a connection is made
    */
    g.OnePathPerBridge(board);

    // DEBUGGING
    //g.printEdges(board);

    int remainingNodes = g.AvailableNodes(board);
    int solutions = 0;


    /* ADVANCED TECHNIQUES
        Uses a Depth First Search to recursively find possible solutions
    */
    if (!remainingNodes){
        for (int row = BOARD_SIZE-1; row >=0; row--){
            for(int col = 0; col < BOARD_SIZE; col++){
                if (board[col][row].num_available_paths())
                    g.DepthFirstSearch(board, board[col][row]);
            }
        }

    } else {
        solutions = 1;
    }


    if (find_all_solutions){
        std::cout << "\n\nFound " << solutions << " solution(s)";
        g.printSolution();
        g.printGraph();

    } else if (connected) {
        if (g.TestConnection(board)){
            g.printSolution();
            g.printGraph();

        } else {
            std::cout << "\n\nNo solutions";
        }

    } else {
        g.printSolution();
        g.printGraph();
    }
}






// PRINT BOARD =======================================================================================================
void PrintBoard(const std::vector<std::vector<Node> > &board){

    for (int row = BOARD_SIZE-1; row >=0; row--){
        for(int col = 0; col < BOARD_SIZE; col++)
            std::cout << board[col][row].get_num_paths() << " ";


        std::cout << "\n";
    }
}






