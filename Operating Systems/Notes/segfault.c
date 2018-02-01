/* segfault.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
  char * c = NULL;
  *c = 'A';

  return EXIT_SUCCESS;
}
