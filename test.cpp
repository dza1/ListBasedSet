
#include <iostream>
using namespace std;
#include "node.h"
#include  <omp.h>


int main() {
	node *oldtNode = new node();
	oldtNode->key = 10;
	node *newNode = new node();
	newNode->next = oldtNode;
	#pragma omp parallel
	{
		printf("Key: %i\n",newNode->next->key);
	}
	return 0;
}