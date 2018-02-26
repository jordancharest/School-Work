/* rlimit.c */

/* To run this and have it actually take effect, you need to be root:

   bash$ sudo ./a.out
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>

int main()
{
  struct rlimit rl;
  getrlimit( RLIMIT_NPROC, &rl );
  printf( "RLIMIT_NPROC soft limit: %d\n", (int)rl.rlim_cur );
  printf( "RLIMIT_NPROC max limit: %d\n", (int)rl.rlim_max );

  rl.rlim_cur = rl.rlim_max = 10000;
  setrlimit( RLIMIT_NPROC, &rl );

  getrlimit( RLIMIT_NPROC, &rl );
  printf( "RLIMIT_NPROC soft limit: %d\n", (int)rl.rlim_cur );
  printf( "RLIMIT_NPROC max limit: %d\n", (int)rl.rlim_max );

  return EXIT_SUCCESS;
}
