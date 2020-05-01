#ifndef NODE_H__
#define NODE_H__
#include <stdint.h>
class node {
  public:
	uint32_t key;
	node *next;
	int item;
	node(int item);
};
#endif