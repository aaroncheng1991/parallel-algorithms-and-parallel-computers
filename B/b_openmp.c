#include <stdio.h>
#include <string.h>
#include <omp.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>

# define INT_MAX 32767

// Number of threads
#define NUM_THREADS 32

// Number of iterations
#define TIMES 1000

// Input Size
#define NSIZE 1
#define NMAX 32
int Ns[NSIZE] = {32};  //set the input size

// example
int S1[] = {28, 36, 40, 61, 68, 71,123, 149};
int S2[] = {2, 5, 18, 21, 24, 29, 31, 33, 34, 35, 47, 48, 49, 50, 52, 62, 66, 70, 73, 80, 88, 89, 114, 124, 125, 131, 143, 144, 145, 148, 155, 159};

//int S1[NMAX], S2[NMAX];

int A[NMAX], B[NMAX]; //input 
int C[2*NMAX]; //output

int sizeA,sizeB;
int AA[NMAX], BB[NMAX];

void init(int n){
	/* Initialize the input for this iteration*/
	int i;

/*	for(i = 0; i < NMAX; i++){
		S1[i] = 2*i;
		S2[i] = 2*i + 1;
	}*/

	sizeA = (sizeof(S1) / sizeof(S1[0]));
	sizeB = (sizeof(S2) / sizeof(S2[0]));

	for (i = 0; i < NMAX; i++) {
		if (i < sizeA)
			A[i] = S1[i];
		
		if (i < sizeB)
			B[i] = S2[i];
	}        
}

void printResult(int m, int n){
	int i;
	printf("\n    A     :  ");
	for (i = 0; i < m; i++){
		printf("%d ",A[i]);
	}
	printf("\n    B     :  ");
	for (i = 0; i < n; i++){
		printf("%d ",B[i]);
	}
	printf("\n    C     :  ");
	for (i = 0; i < m+n; i++){
		printf("%d ",C[i]);
	}
	printf("\n");
}

int rank (int value, int* array, int size_array){
	int i, ranking = 0;
	for (i=0; i<size_array; i++){
		if (*(array+i) <= value)
			ranking++;
	}
	return ranking;
}

void seq_function(int length){
	/* The code for sequential algorithm */
	
	int i,j;

	j = 0;
	for (i = 0; i < length;i++){
		if (i < sizeA)
			AA[i] = rank(A[i], B, sizeB);
		if (j < sizeB)
			BB[j] = rank(B[j], A, sizeA);
		j++;
	}

	j = 0;
	for (i = 0; i < length; i++){
		if (i < sizeA)
			C[AA[i] + i] = A[i];
		if (j < sizeB) 
			C[BB[j] + j] = B[j];
		j++;
	}
}

void omp_function(int length, int nthreads){
	/* The code for threaded computation */

	int i,j,chunk;

	if (sizeA > sizeB)
		chunk = ceil((float)sizeA/nthreads);
	else
		chunk = ceil((float)sizeB/nthreads);

	#pragma omp parallel num_threads(nthreads) shared(A,AA,B,BB,chunk) private(i,j)
	{
		j = 0;
		#pragma omp for schedule(dynamic,chunk) nowait
		for (i = 0; i < length;i++){
			if (i < sizeA)
				AA[i] = rank(A[i], B, sizeB);
			if (j < sizeB)
				BB[j] = rank(B[j], A, sizeA);
			j++;
		}

		j = 0;
		#pragma omp for schedule(dynamic,chunk) nowait
		for (i = 0; i < length; i++){
			if (i < sizeA)
				C[AA[i] + i] = A[i];
			if (j < sizeB) 
				C[BB[j] + j] = B[j];
			j++;
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
		printf("   NSize  :  %d\nIterations:  %d",n,TIMES);

		/* Run sequential algorithm */
		result.tv_usec=0;
		gettimeofday (&startt, NULL);
		for (t=0; t<TIMES; t++) {
			init(n);
			seq_function(n);
		}
		gettimeofday (&endt, NULL);
		result.tv_usec = (endt.tv_sec*1000000+endt.tv_usec) - (startt.tv_sec*1000000+startt.tv_usec);
		printf("\n   Seq    :  %ld.%06ld", result.tv_usec/1000000, result.tv_usec%1000000);	
		printResult(sizeA,sizeB);  //print the result of sequential implementation

		/* Run threaded algorithm(s) */
		for(nt=1; nt<NUM_THREADS; nt=nt<<1){

			result.tv_sec=0; result.tv_usec=0;
			gettimeofday (&startt, NULL);
			for (t=0; t<TIMES; t++)
			{			
				init(n);
				omp_function(n,nt);
			}
			gettimeofday (&endt, NULL);
 			result.tv_usec += (endt.tv_sec*1000000+endt.tv_usec) - (startt.tv_sec*1000000+startt.tv_usec);
			printf("\n   Th%02d   :  %ld.%06ld ", nt,result.tv_usec/1000000, result.tv_usec%1000000);
			printResult(sizeA,sizeB); //print the result of OpenMP implementation
		}
		printf("\n");
	}
	return 0;
}