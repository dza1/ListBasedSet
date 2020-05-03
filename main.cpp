#include <stdint.h>
#include "Coarse_Grained.h"

int main() {
	CoarseList<int> *list;// Create an object of MyClass
	list = new CoarseList<int>();
	cout << list->add(10) << endl;
	cout << list->add(11) << endl;
	cout << list->add(11) << endl;

	//#pragma omp parallel
	{
		printf("Key: %u\n", list->head->next->key);
		printf("item: %i\n", list->head->next->item);
		printf("Key: %u\n", list->head->key);
	}
	return 0;
}