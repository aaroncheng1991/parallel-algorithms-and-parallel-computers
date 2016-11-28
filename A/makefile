openmp: a_openmp.c
	gcc $^ -ggdb3 -fopenmp -lm -Wall -o $@.out

pthreads: a_pthreads.c
	gcc $^ -ggdb3 -pthread -lm -Wall -o $@.out

openmp-run: openmp 
	./$^.out

pthreads-run: pthreads
	./$^.out

clean:
	@rm -f *.out
