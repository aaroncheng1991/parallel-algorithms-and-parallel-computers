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
#define NMAX 32
#define NSIZE 1
int Ns[NSIZE] = {32};

typedef struct __ThreadArg {
  int i_min;
  int i_max;
} tThreadArg;


pthread_t callThd[NUM_THREADS];
pthread_mutex_t mutexpm;
pthread_barrier_t barr, internal_barr;

// Seed Input
int A[] = {58,89,32,73,131,156,30,29,141,37,133,151,88,53,122,126,131,49,130,115,16,83,40,145,10,112,20,147,14,104,111,92};
//int A[4096];


// Subset
int P[NMAX]; //prefix minima
int S[NMAX]; //suffix minima

void init(int length){
	/* Initialize the input for this iteration*/
	int i;

/*	for (i = 0; i < length; i++){
		P[i] = i + 1;
		S[i] = i + 1;
	}*/	
	for(i=0;i<length;i++){
		P[i] = A[i];
		S[i] = A[i];
	}
}

void printResult(int length){
	int i;

	printf("\n      i      : ");	
	for (i = 1; i <= length; i++)
		printf("%3d",i);

	printf("\nPrefix Minima: ");
	for (i = 0; i < length; i++)
		printf("%3d",P[i]);

	printf("\nSuffix Minima: ");
	for (i = 0; i < length; i++)
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

void* par_function(void* threadArg){
	/* The code for threaded computation */
	tThreadArg *data = (tThreadArg *)threadArg;

	int i_min = data -> i_min;
	int i_max = data -> i_max;
	int min, i;
	int lastleft, firstright;
	int iLastleft = (i_max-i_min + 1)/2 + i_min - 1;
	int iFirstright = iLastleft + 1;

	// prefixMinima
	lastleft = P[iLastleft];
	firstright = P[iFirstright];
	if (lastleft < firstright){
		min = lastleft;
		for (i = iFirstright; i <= i_max; i++) {
			P[i] = min;
			if (P[i+1] <= min) break;
		}
	}

	// suffixMinima
	firstright = S[iFirstright];
	lastleft = S[iLastleft];
	if (lastleft > firstright) {
		min = firstright;
		for (i = iLastleft; i >= i_min; i--) {
			S[i] = min;
			if(S[i-1] <= min) break; 
		}
	}

	//pthread_barrier_wait(&barr);
	//pthread_exit(NULL);
}

//int main (int argc, char *argv[])
int main()
{
	struct timeval startt, endt, result;
	int j, nt, t, n, c;
	int task, tasksThisLevel, totalTasks, id, threadsThisLevel;
	int i_min, i_max, range;
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
		printf("\n     NSize   :  %d\n  Iterations :  %d",n,TIMES);

		/* Run sequential algorithm */
		result.tv_usec=0;
		gettimeofday (&startt, NULL);
		for (t=0; t<TIMES; t++) {
			init(n);
			seq_function(n);
		}
		gettimeofday (&endt, NULL);
		result.tv_usec = (endt.tv_sec*1000000+endt.tv_usec) - (startt.tv_sec*1000000+startt.tv_usec);
		printf("\n     Seq     :  %ld.%06ld", result.tv_usec/1000000, result.tv_usec%1000000);	
		printResult(n);

		/* Run threaded algorithm(s) */
		for(nt=1; nt<NUM_THREADS; nt=nt<<1){
			if(pthread_barrier_init(&barr, NULL, nt+1))
			{
				printf("Could not create a barrier\n");
				return -1;
			}

			if(pthread_barrier_init(&internal_barr, NULL, nt))
			{
				printf("Could not create a barrier\n");
				return -1;
			}

			result.tv_sec=0; result.tv_usec=0;
			gettimeofday (&startt, NULL);

			for (t=0; t<TIMES; t++) //threaded execution
			{			
				init(n);
				totalTasks = n;
				tasksThisLevel = n/2;
				task = 1;
				range = 2;
				
				while (task < totalTasks) {
					id = 1; 
					threadsThisLevel = 1; 
					i_min = 0;

					while(threadsThisLevel <= tasksThisLevel) { 

						i_max = i_min + range - 1;	
						x[id].i_min = i_min;
						x[id].i_max = i_max;
						pthread_create(&callThd[id], &attr, par_function, (void *)&x[id]);
						id++;
										
						if(id > nt) {
							for (id = 1; id <= nt; id++)
								pthread_join(callThd[id], &status);
							id = 1;
						}
						i_min = i_max + 1;
						task++;
						threadsThisLevel++;
					}

					/* Wait on the other threads */
					for (j = 1; j < id; j++)
						pthread_join(callThd[j], &status);

					range *= 2;
					tasksThisLevel /= 2;
				}
			}
			gettimeofday (&endt, NULL);
			if (pthread_barrier_destroy(&barr)) {
					printf("Could not destroy the barrier\n");
					return -1;
			}
			if (pthread_barrier_destroy(&internal_barr)) {
					printf("Could not destroy the barrier\n");
					return -1;
			}
 			result.tv_usec += (endt.tv_sec*1000000+endt.tv_usec) - (startt.tv_sec*1000000+startt.tv_usec);
			printf("\n     Th%02d    :  %ld.%06ld ", nt,result.tv_usec/1000000, result.tv_usec%1000000);
			printResult(n);
		}
		printf("\n");
	}
	pthread_exit(NULL);
}
