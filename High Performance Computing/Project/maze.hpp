#ifndef MAZE_H_INCLUDED
#define MAZE_H_INCLUDED

struct Node {
	int x, y; // Node position - little waste of memory, but it allows faster generation
	void *parent; // Pointer to parent node
	char c; // Character to be displayed
	char dirs; // Directions that still haven't been explored
};



// FUNCTION PROTOTYPES ===========================================================================
void draw();
int init();
struct Node* link(struct Node* n);
void generate_maze(int argc, char** argv);
void serial_BFS(int width, int height);

#endif // MAZE_H_INCLUDED
