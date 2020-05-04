#include "Coarse_Grained.h"
#include <omp.h>
//#include <stdint.h>
#include "stdio.h"

int main(int argc, char *argv[]){
	CoarseList<int> *list; // Create an object of MyClass
	list = new CoarseList<int>();
	cout << "add 10: " << list->add(10) << endl;
	cout << "contains 11: " << list->contains(11) << endl;
	cout << "add 11: " << list->add(11) << endl;
	cout << "add 11: " << list->add(11) << endl;
	cout << "contains 11: " << list->contains(11) << endl;
	cout << "remove 10: " << list->remove(10) << endl;
	cout << "remove 10: " << list->remove(10) << endl;
	cout << "remove 11: " << list->remove(11) << endl;
	cout << "contain 11: " << list->contains(11) << endl;

#pragma omp parallel
	{
		printf("Key: %u\n", list->head->next->key);
		printf("item: %i\n", list->head->next->item);
		printf("Key: %u\n", list->head->key);

		int NCPU, tid, NPR, NTHR;
		/* get the total number of CPUs/cores available for OpenMP */
		NCPU = omp_get_num_procs();
		/* get the current thread ID in the parallel region */
		tid = omp_get_thread_num();
		/* get the total number of threads available in this parallel region */
		NPR = omp_get_num_threads();
		/* get the total number of threads requested */
		NTHR = omp_get_max_threads();
		/* only execute this on the master thread! */
		if (tid == 0) {
			printf("%i : NCPU\t= %i\n", tid, NCPU);
			printf("%i : NTHR\t= %i\n", tid, NTHR);
			printf("%i : NPR\t= %i\n", tid, NPR);
		}
		printf("%i : hello multicore user! I am thread %i out of %i\n", tid, tid, NPR);
	}
	return 0;
}