#include <stdio.h>
#include <stdlib.h> 
#include <pthread.h> 


#define MAX_THREADS 16 

void *compute_pi (void *); 

pthread_mutex_t hits_lock;

unsigned long hits[MAX_THREADS];
unsigned long long total_hits=0;
unsigned long long total_fetch_and_adds=0;
unsigned long long total_compare_and_swaps=0;
unsigned int sample_points_per_thread=4*1024*1024;

int main() 
{ 
    unsigned long long i;
    pthread_t p_threads[MAX_THREADS]; 
    pthread_attr_t attr; 
    pthread_attr_init (&attr); 

    pthread_mutex_init( &hits_lock, NULL );

    for (i=0; i< MAX_THREADS; i++) 
    { 
	pthread_create(&p_threads[i], &attr, compute_pi, 
		       (void *) i); 
	printf("Create thread %lx \n", p_threads[i] );
    } 

    for (i=0; i< MAX_THREADS; i++)
    {
	pthread_join(p_threads[i], NULL);
	//total_hits += hits[i];
    }
#ifdef SYNC_MUTEX
    printf("Total MUTEX = %llu \n", total_hits );
#endif 
#ifdef SYNC_FAA
    printf("Total FAAs = %llu \n", total_fetch_and_adds );
#endif
#ifdef SYNC_CAS
    printf("Total CASs = %llu \n", total_compare_and_swaps );
#endif
    return( 0 );
}

void *compute_pi (void *s) 
{ 
  
  unsigned int i=0, sync_success=0;
  unsigned long long seed=0;
  unsigned long long *hit_pointer=NULL; 
  struct drand48_data seed_data;
  double rand_no_x=0.0, rand_no_y=0.0; 
  unsigned long long local_hits=0; 
  seed = (unsigned long long)s; 
  unsigned long long local_i = (unsigned long long)s;
  
  hits[local_i] = 0;

  srand48_r( seed, &seed_data );
  for (i = 0; i < sample_points_per_thread; i++) { 
    drand48_r(&seed_data, &rand_no_x ); 
    drand48_r(&seed_data, &rand_no_y); 
    if (((rand_no_x - 0.5) * (rand_no_x - 0.5) + 
	 (rand_no_y - 0.5) * (rand_no_y - 0.5)) < 0.25) 
      // local_hits ++; 
      hits[local_i]++;
      {
#ifdef SYNC_MUTEX
	pthread_mutex_lock( &hits_lock );
	total_hits++;
	pthread_mutex_unlock( &hits_lock );
#endif /* SYNCH_MUTEX */
	
#ifdef SYNC_FAA    
	__sync_fetch_and_add( &total_fetch_and_adds, (unsigned long long)1);
#endif /* SYNC_FAA */
	
#ifdef SYNC_CAS
	sync_success = __sync_bool_compare_and_swap( &total_compare_and_swaps, total_compare_and_swaps, total_compare_and_swaps+1);
	while( !sync_success )
	  sync_success = __sync_bool_compare_and_swap( &total_compare_and_swaps, total_compare_and_swaps, total_compare_and_swaps+1);
#endif /* SYNC_CAS */
      }

	seed *= i; 
    } 
    //*hit_pointer = local_hits; 
    pthread_exit(0); 
}
