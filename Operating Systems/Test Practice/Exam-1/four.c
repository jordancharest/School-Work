#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
int main(){
    int fd;
    close( 2 );
    close( 1 ); /* <== add this line later.... */
    printf( "HI\n" );
    fflush( stdout );
    fd = open( "newfile.txt", O_WRONLY | O_CREAT | O_TRUNC, 0600 );
    printf( "==> %d\n", fd );
    printf( "WHAT?\n" );
    fprintf( stderr, "ERROR\n" );
    fflush( stdout );
    close( fd );
return EXIT_SUCCESS;
}
