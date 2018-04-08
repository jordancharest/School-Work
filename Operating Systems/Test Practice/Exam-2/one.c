#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

void * omg( void * arg ) {
    fprintf(stderr, "Made it\n");
    int * x = (int *)arg;
    int rc = pthread_detach( pthread_self() );
    printf( "%u lucky %d %d\n",
    (unsigned int)pthread_self(), *x, rc );
    return NULL;
}

int main() {
    pthread_t tid;
    int x = 7;
    printf( "%u %d\n", (unsigned int)pthread_self(), x );
    int rc = pthread_create( &tid, NULL, omg, &x );
    printf( "%u %d\n", (unsigned int)pthread_self(), rc );
    rc = pthread_create( &tid, NULL, omg, &x );
    //sleep( 0.3 ); /* wait for child threads to end ... */
    return EXIT_SUCCESS;
}
