
#include <iostream>
using namespace std;
#include "node.h"


int main() {
	node *oldtNode = new node();
	oldtNode->key = 10;
	node *newNode = new node();
	newNode->next = oldtNode;
	std::cout << "Key: " << newNode->next->key;

	return 0;
}