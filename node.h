#ifndef NODE_H__
#define NODE_H__
#include <stdint.h>

template<typename T>
class node {
  public:
	uint32_t key;
	node *next;
	T item;
	node(T item);
	node(T item,uint32_t key);
	uint32_t hash();
};
#endif