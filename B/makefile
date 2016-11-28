openmp: b_openmp.c
	gcc $^ -ggdb3 -fopenmp -lm -Wall -o $@.out

pthreads: b_pthreads.c
	gcc $^ -ggdb3 -pthread -lm -Wall -o $@.out

pthreads-run: pthreads
	./$^.out

openmp-run: openmp
	./$^.out

clean:
	@rm -f *.out
