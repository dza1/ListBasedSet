#include "node.h"
#include <iostream>
#include <stdint.h>
#include "key.h"
using namespace std;

template <class T> node_virt<T>::node_virt(T item) {
	this->key = (uint32_t)item;
	this->item = item;
}

template <class T> node_virt<T>::node_virt(T item, int32_t key) {
	this->key = key;
	this->item = item;
}

template <class T> int32_t node_virt<T>::hash() { 
	return this->key; 
	}


//node Lock for fine Grained

template <class T> void nodeFine<T>::lock() {
	mtx.lock();
	return;
}

template <class T> void nodeFine<T>::unlock() {
	mtx.unlock();
	return;
}

template class node_virt<int>;
template class node<int>;
//template class node<float>;

template class nodeFine<int>;
//template class nodeFine<float>;