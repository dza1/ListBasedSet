#include "key.h"
#include "node.h"
#include <iostream>
#include <stdint.h>
using namespace std;

template <class T> node_virt<T>::node_virt(T item) {
	this->key = key_calc(item);
	this->item = item;
}

template <class T> node_virt<T>::node_virt(T item, int32_t key) {
	this->key = key;
	this->item = item;
}


// node Lock for fine Grained

template <class T> void nodeFine<T>::lock() {
	mtx.lock();
	return;
}

template <class T> void nodeFine<T>::unlock() {
	mtx.unlock();
	return;
}

template <class T> nodeFine_mem<T>::nodeFine_mem(T item):nodeFine<T>::nodeFine(item)  {
	// this->key = (uint32_t)item;
	// this->item = item;
	this->hash_mem.store( (uint64_t)this->key << 32);
}

template <class T> nodeFine_mem<T>::nodeFine_mem(T item, int32_t key):nodeFine<T>::nodeFine(item,key) {
	this->key = key;
	this->item = item;
	this->hash_mem.store( (uint64_t)this->key << 32);
}




template class node_virt<int>;
//template class node_virt<float>;

// template class node<float>;

template class nodeFine<int>;
//template class nodeFine<float>;
template class nodeFine_mem<int>;
// template class nodeFine<float>;