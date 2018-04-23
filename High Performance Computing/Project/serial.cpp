#include <stdio.h>
#include <stdlib.h>

#include "maze.hpp"

extern struct Node* nodes;  // maze; contained in one dimensional array

// SERIAL ========================================================================================
void serial_BFS(int width, int height) {

    int x = 1;
    int y = 1;
    int x_goal = width - 2;
    int y_goal = height - 2;

    nodes[x + y * width].c = 'S';
    nodes[x_goal + y_goal * width].c = 'G';
    draw();






}

