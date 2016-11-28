#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>

# define INT_MAX 32767

// Number of threads
#define NUM_THREADS 32

// Number of iterations
#define TIMES 1000

// Input Size
// #define NSIZE 7
// #define NMAX 262144
// int Ns[NSIZE] = {4096, 8192, 16384, 32768, 65536, 131072, 262144};
#define NSIZE 1
#define NMAX 32
int Ns[NSIZE] = {32}; //here set the input size


typedef struct __ThreadArg {
	int id;
	int nrT;
	int n;
} tThreadArg;


pthread_t callThd[NUM_THREADS];
pthread_mutex_t mutexpm;
pthread_barrier_t barr, internal_barr;

// Seed Input
//int I1[NMAX-1];
//int I2[NMAX];

//example
int I1[] = {28, 36, 40, 61, 68, 71,123, 149};
int I2[] = {2, 5, 18, 21, 24, 29, 31, 33, 34, 35, 47, 48, 49, 50, 52, 62, 66, 70, 73, 80, 88, 89, 114, 124, 125, 131, 143, 144, 145, 148, 155, 159};


int A[NMAX], B[NMAX]; //input
int C[2*NMAX]; //result
int CC[2*NMAX];
int a, b, sizeA, sizeB;
int AA[NMAX], BB[NMAX];

void printResult(length){
	int i;
	printf("\n    A     :  ");
	for (i = 0; i < sizeA; i++){
		printf("%d ",A[i]);
	}
	printf("\n    B     :  ");
	for (i = 0; i < sizeB; i++){
		printf("%d ",B[i]);
	}
	printf("\n    C     :  ");
	for (i = 0; i < (sizeA+sizeB); i++){
		printf("%d ",C[i]);
	}
	printf("\n");
}

int rank (int value, int* array, int sizeArray){
	int i = 0, cnt = 0;
	for (i=0; i<sizeArray; i++){
		if (*(array+i) <= value)
			cnt++;
	}
	return cnt;
}

void sort(int* array, int length){
	int i, flag, temp;

	do{
		flag = 0;  
		for (i = 0; i < length - 1; i++){
			if ((array[i] == -1 && array[i+1] != -1) || ((array[i] > array[i+1]) && array[i+1]!=-1)){ 
				flag = 1;
				temp = array[i];
				array[i] = array[i+1];
				array[i+1] = temp;
			}
		}
	} while (flag == 1);
}

void merge (int i, int j){

	int k, x = 0;
	int begin  = i + j;
	int end = begin;
	int tempC[2*NMAX] = {-1}; 
	
	while(1){
		if (i < sizeA)
			tempC[x++] = A[i++]; 

		if (j < sizeB)
			tempC[x++] = B[j++]; 

		end++;

		if(i >= sizeA && j >= sizeB)
			break;

		if (CC[end] != -1){
			end--;
			break;
		}
	}
	sort(tempC,x);

	for (k = begin; k <= end; k++)
		C[k] = tempC[k-begin];
}

void init(int length){
	/* Initialize the input for this iteration*/
	int i;

	//generate data for input A
/*	for(i = 0; i < NMAX-1; i++)
		I1[i] = 2*i + 1;

	//generate data for input B
	for(i = 0; i < NMAX; i++)
		I2[i] = 2*i + 2;
*/
	sizeA = sizeof(I1) / sizeof(I1[0]);
	sizeB = sizeof(I2) / sizeof(I2[0]);

	for (i = 0; i < NMAX; i++) {
		if (i < sizeA)
			A[i] = I1[i];
		else
			A[i] = -1;
		
		if (i < sizeB)
			B[i] = I2[i];
		else
			B[i] = -1;

		AA[i] = -1;
		BB[i] = -1;
		C[2*i] = -1; 
		C[2*i+1] = -1;
		CC[2*i] = -1;
		CC[2*i+1] = -1;		
	}

	a = floor(log2(sizeA));
	b = floor(log2(sizeB));
}

void seq_function(int length){          
	/* The code for sequential algorithm */
	// Perform operations on A and B
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

void* par_function(void* threadArg){
	/* The code for threaded computation */
	// Perform operations on B
	
	tThreadArg *data = (tThreadArg *) threadArg;  
	int id = data -> id; 
	int nrT = data -> nrT; 
	
	int i, j, i_minA, i_maxA, j_minB, j_maxB, delta, rd;

	// for input A
	rd = sizeA/(a*nrT);
	delta = sizeA/a - nrT*rd;
	if (id <= delta){
		i_minA = rd*(id-1)+id;
		i_maxA = rd + i_minA;
	} else {
		i_minA = rd*(id-1) + delta + 1;
		i_maxA = i_minA + rd - 1;
	}

	// for input B
	rd = sizeB/(b*nrT);
	delta = sizeB/b - nrT*rd;
	if (id <= delta){
		j_minB = rd*(id-1)+id;
		j_maxB = rd + j_minB;
	} else {
		j_minB = rd*(id-1) + delta + 1;
		j_maxB = j_minB + rd - 1;
	}

	//the second step
	for (i = i_minA; i <= i_maxA; i++){
		AA[i] = rank(A[a*i-1], B, sizeB);
	}
	for (j = j_minB; j <= j_maxB; j++){
		BB[j-1] = rank(B[b*j-1], A, sizeA);
	}

	//the third step
	for (i = i_minA; i <= i_maxA; i++){
		C[AA[i] + a*i - 1] = A[a*i-1];
		CC[AA[i] + a*i - 1] = A[a*i-1];
	}

	for (j = j_minB; j <= j_maxB; j++){
		C[BB[j-1] + b*j - 1] = B[b*j-1];
		CC[BB[j-1] + b*j - 1] = B[b*j-1];
	}

	//the fourth step
	for (i = i_minA - 1; i <= i_maxA; i++){
		merge(a*i , AA[i]);
	}

	for (j = j_minB; j <= j_maxB; j++){
		merge(BB[j-1], j*b);
	}

	pthread_barrier_wait(&barr);
	//pthread_exit(NULL);
}

int main ()
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

	for(c=0; c<NSIZE; c++){
		n=Ns[c];
		printf("\n   NSize  :  %d\nIterations:  %d",n,TIMES);

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
		printResult(n);   //print the result of sequential algorithm

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
				init(n);
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
			printResult(n);  //print the result of pthreads algorithm
		}
		printf("\n");
	}
	pthread_exit(NULL);
}