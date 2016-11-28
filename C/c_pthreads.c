#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>

// Number of threads
#define NUM_THREADS 32

// Number of iterations
#define TIMES 1000

// Input Size
// #define NSIZE 7
// #define NMAX 262144
// int Ns[NSIZE] = {4096, 8192, 16384, 32768, 65536, 131072, 262144};
#define NSIZE 1
#define NMAX 16
int Ns[NSIZE] = {16};  // here change the input size

typedef struct __ThreadArg {
	int id;
	int nrT;
	int n;
} tThreadArg;


pthread_t callThd[NUM_THREADS];
pthread_mutex_t mutexpm;
pthread_barrier_t barr, internal_barr;

// Seed Input
// int P[NMAX];

/* example */
int P[] = {0,14,13,5,16,11,10,9,12,0,8,7,15,4,3,2,1};

int S[NMAX+1] = {0};
int R[NMAX+1] = {0};

void printResult(int length){
	int i;
	printf("\n    S     :  ");
	for (i = 1; i <= length; i++){
		printf("%d %s",S[i],(S[i]>9)?" ":"  ");
	}
	printf("\n    R     :  ");
	for (i = 1; i <= length; i++){
		printf("%d %s",R[i],(R[i]>9)?" ":"  ");
	}
	printf("\n");
}

void seq_function(int length){
	/* The code for sequential algorithm */
	// Perform operations on B

	int i;
	
	for (i=0; i<=length; i++) {
		S[i] = P[i];

		if (i != S[i])
			R[i] = 1;
		else
			R[i] = 0;

		if (S[i] != S[S[i]]) {
			R[i] = R[i] + R[S[i]];
			S[i] = S[S[i]];
		}
	}
}

void* par_function(void* threadArg){
	/* The code for threaded computation */
	// Perform operations on B
	
	tThreadArg *data = (tThreadArg *) threadArg;
	int id = data->id; // thread id
	int nrT = data->nrT; // number of threads
	int n = data->n; //input size


	int i;
	int i_max = id*(n/nrT);
	int i_min = (id-1)*(n/nrT)+1;

	for (i=i_min; i<=i_max; i++) {
		S[i] = P[i];

		if (i != S[i])
			R[i] = 1;
		else
			R[i] = 0;

		if (S[i] != S[S[i]]){
			R[i] = R[i] + R[S[i]];
			S[i] = S[S[i]];
		}
	}
	
	pthread_barrier_wait(&barr);

	//pthread_exit(NULL);

}

int main (int argc, char *argv[])
{
	struct timeval startt, endt, result;
	int j, nt, t, n, c;
	void *status;
 	pthread_attr_t attr;
	tThreadArg x[NUM_THREADS];
	
 	result.tv_sec = 0;
 	result.tv_usec= 0;

 	/* Initialize and set thread detached attribute */
 	pthread_attr_init(&attr);
 	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	// for each input size
	for(c=0; c<NSIZE; c++){
		n=Ns[c];
		printf("\n   NSize  :  %d\nIterations:  %d",n,TIMES);

		/* Run sequential algorithm */
		result.tv_usec=0;
		gettimeofday (&startt, NULL);
		for (t=0; t<TIMES; t++) {
			seq_function(n);
		}
		gettimeofday (&endt, NULL);
		result.tv_usec = (endt.tv_sec*1000000+endt.tv_usec) - (startt.tv_sec*1000000+startt.tv_usec);
		printf("\n   Seq    :  %ld.%06ld", result.tv_usec/1000000, result.tv_usec%1000000);	
		printResult(n); //print the result of sequential algorithm

		/* Run threaded algorithm(s) */
		for(nt=1; nt<NUM_THREADS; nt=nt<<1){
			if(pthread_barrier_init(&barr, NULL, nt+1))
			{
				printf("Could not create a barrier\n");
				return -1;
			}

		/*	if(pthread_barrier_init(&internal_barr, NULL, nt))
			{
				printf("Could not create a barrier\n");
				return -1;
			}*/

			result.tv_sec=0; 
			result.tv_usec=0;
			gettimeofday (&startt, NULL);
			for (t=0; t<TIMES; t++)
			{			
				for (j=1; j<=/*NUMTHRDS*/nt; j++)
				{
					x[j-1].id = j; 
					x[j-1].nrT=nt;
					x[j-1].n=n; 
					pthread_create(&callThd[j-1], &attr, par_function, (void *)&x[j-1]);
				}
				pthread_barrier_wait(&barr);
	
				/* Wait on the other threads */
				for(j=0; j</*NUMTHRDS*/nt; j++)
				{
					pthread_join(callThd[j], &status);
				}

			}
			gettimeofday (&endt, NULL);

			if (pthread_barrier_destroy(&barr)) {
					printf("Could not destroy the barrier\n");
					return -1;
			}
		/*	if (pthread_barrier_destroy(&internal_barr)) {
					printf("Could not destroy the barrier\n");
					return -1;
			}*/
 			result.tv_usec += (endt.tv_sec*1000000+endt.tv_usec) - (startt.tv_sec*1000000+startt.tv_usec);
			printf("\n   Th%02d   :  %ld.%06ld ", nt,result.tv_usec/1000000, result.tv_usec%1000000);
			printResult(n); //print the result of pthreads algorithm
		}
		printf("\n");
	}
	pthread_exit(NULL);
}