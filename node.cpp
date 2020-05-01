#include "node.h"
#include <iostream>
using namespace std;

node::node(int item) {
	this->key = item;
	this->next = NULL;
	this->item = item;
}
