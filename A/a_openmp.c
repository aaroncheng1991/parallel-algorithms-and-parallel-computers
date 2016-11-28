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
#define NMAX 32
#define NSIZE 1
int Ns[NSIZE] = {32};

// Seed Input
int A[] = {58,89,32,73,131,156,30,29,141,37,133,151,88,53,122,126,131,49,130,115,16,83,40,145,10,112,20,147,14,104,111,92}; //example

// Subset
int P[NMAX]; //prefix minima
int S[NMAX]; //suffix minima

void init(int n){
	/* Initialize the input for this iteration*/
	int i;
	for(i=0;i<n;i++){
		P[i] = A[i];
		S[i] = A[i];
		//P[i] = i;
		//S[i] = i;
	}
}

void printResult(int n){
	int i;

	printf("\n      i      : ");	
	for (i = 1; i <= n; i++)
		printf("%3d",i);

	printf("\nPrefix Minima: ");
	for (i = 0; i < n; i++)
		printf("%3d",P[i]);

	printf("\nSuffix Minima: ");
	for (i = 0; i < n; i++)
		printf("%3d",S[i]);

	printf("\n");

}

void seq_function(int length){
	/* The code for sequential algorithm */
	int i,j;
	int P_curmin,S_curmin;

 	for(i=0; i<2; i++){
 		if(i==0){
 			P_curmin = P[0];
 			for (j = 0; j < length; j++){
				if(P[j] < P_curmin)
					P_curmin = P[j];
				else
					P[j] = P_curmin;
			}
 		}

 	if(i==1){
 		S_curmin = S[length-1];
 		for (j = length-1; j >= 0; j--){
			if(S[j] < S_curmin)
				S_curmin = S[j];
			else 
				S[j] = S_curmin;
			}
 		}
 	}
}

void omp_function(int length, int nthreads){
	/* The code for threaded computation */

	int i,j,chunk;
	int P_curmin,S_curmin;

	chunk = ceil((float)length/nthreads);

	#pragma omp parallel num_threads(nthreads) shared(P,S,chunk) private(j,P_curmin, S_curmin)

 	for(i=0; i<2; i++){
 		if(i==0){
 			P_curmin = P[0];
 			#pragma omp for schedule(dynamic,chunk) nowait
 			for (j = 0; j < length; j++){
				if(P[j] < P_curmin)
					P_curmin = P[j];
				else
					P[j] = P_curmin;
			}
 		}

 		if(i==1){
 			S_curmin = S[length-1];
 		  	#pragma omp for schedule(dynamic,chunk) nowait
 			for (j = length-1; j >= 0; j--){
				if(S[j] < S_curmin)					
					S_curmin = S[j];
				else 
					S[j] = S_curmin;
			}
 		}
 	}
}

int main (int argc, char *argv[]){
  	struct timeval startt, endt, result;
	int nt, t, n, c;
	
  	result.tv_sec = 0;
  	result.tv_usec= 0;

	// for each input size
	for(c=0; c<NSIZE; c++){
		n=Ns[c];
		printf("\n    NSize    :  %d\n  Iterations :  %d",n,TIMES);

		/* Run sequential algorithm */
		result.tv_usec=0;
		gettimeofday (&startt, NULL);
		for (t=0; t<TIMES; t++) {
			init(n);
			seq_function(n);
		}
		gettimeofday (&endt, NULL);
		result.tv_usec = (endt.tv_sec*1000000+endt.tv_usec) - (startt.tv_sec*1000000+startt.tv_usec);
		printf("\n     Seq     :  %ld.%06ld  ", result.tv_usec/1000000, result.tv_usec%1000000);
		//printResult(n); //print the result of sequential implementation

		/* Run threaded algorithm(s) */
		for(nt=1; nt<NUM_THREADS; nt=nt<<1){

			result.tv_sec=0; result.tv_usec=0;
			gettimeofday (&startt, NULL);

			for (t=0; t<TIMES; t++) //threaded execution
			{			
				init(n);
				omp_function(n,nt);
			}
			gettimeofday (&endt, NULL);
   			result.tv_usec += (endt.tv_sec*1000000+endt.tv_usec) - (startt.tv_sec*1000000+startt.tv_usec);
			printf("\n    Th%02d     :  %ld.%06ld  ", nt,result.tv_usec/1000000, result.tv_usec%1000000);
			//printResult(n); //print the result of OpenMP implementation
		}
		printf("\n");
	}
	return 0;
}
