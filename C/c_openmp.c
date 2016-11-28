#include <stdio.h>
#include <string.h>
#include <omp.h>
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
int Ns[NSIZE] = {16}; //here change the input size


// Seed Input
//int A[NMAX];
/*example*/
int A[] = {0,14,13,5,16,11,10,9,12,0,8,7,15,4,3,2,1};

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
		S[i] = A[i];

		if (i != S[i])
			R[i] = 1;
		else
			R[i] = 0;

		while(S[i] != S[S[i]]){
			R[i] = R[i] + R[S[i]];
			S[i] = S[S[i]];
		}
	}
}

void omp_function(int length, int nthreads){
	/* The code for threaded computation */
	// Perform operations on B

	int i,chunk;

	chunk = ceil(length/nthreads);

	#pragma omp parallel num_threads(nthreads) shared(S,R) private(i)
	#pragma omp for schedule(dynamic,chunk)
	for (i=0; i<=length; i++) {
		S[i] = A[i];

		if (i != S[i])
			R[i] = 1;
		else
			R[i] = 0;

		while(S[i] != S[S[i]]){
			R[i] = R[i] + R[S[i]];
			S[i] = S[S[i]];
		}
	}
}

int main ()
{
	struct timeval startt, endt, result;
	int nt, t, n, c;
	
 	result.tv_sec = 0;
 	result.tv_usec= 0;

	// for each input size
	for(c=0; c<NSIZE; c++){
		n=Ns[c];
		printf("\n   NSize  :  %d\nIterations:  %d",n,TIMES);

		/* Run sequential algorithm */
		result.tv_usec=0;
		gettimeofday (&startt, NULL);
		for (t=0; t<TIMES; t++) {
			//init(n);
			seq_function(n);
		}
		gettimeofday (&endt, NULL);
		result.tv_usec = (endt.tv_sec*1000000+endt.tv_usec) - (startt.tv_sec*1000000+startt.tv_usec);
		printf("\n   Seq    :  %ld.%06ld", result.tv_usec/1000000, result.tv_usec%1000000);	
		printResult(n); //print the result of sequential algorithm

		/* Run threaded algorithm(s) */
		for(nt=1; nt<NUM_THREADS; nt=nt<<1){
			
			result.tv_sec=0; 
			result.tv_usec=0;
			gettimeofday (&startt, NULL);
			for (t=0; t<TIMES; t++)
			{			
				//init(n);
				omp_function(n,nt);

			}
			gettimeofday (&endt, NULL);
 			result.tv_usec += (endt.tv_sec*1000000+endt.tv_usec) - (startt.tv_sec*1000000+startt.tv_usec);
			printf("\n   Th%02d   :  %ld.%06ld ", nt,result.tv_usec/1000000, result.tv_usec%1000000);
			printResult(n);  //print the result of openmp algorithm
		}
		printf("\n");
	}
	return 0;
}