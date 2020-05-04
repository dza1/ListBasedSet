#include "node.h"
#include <iostream>
#include <stdint.h>
using namespace std;

template <class T> node<T>::node(T item) {
	this->key = (uint32_t)std::hash<T>()(item);
	this->next = NULL;
	this->item = item;
}

template <class T> node<T>::node(T item, uint32_t key) {
	this->key = key;
	this->next = NULL;
	this->item = item;
}

template <class T> uint32_t node<T>::hash() { return (uint32_t)std::hash<T>()(this->item); }


//node Lock for fine Grained

template <class T> void nodeFine<T>::lock() {
	mtx.lock();
	return;
}

template <class T> void nodeFine<T>::unlock() {
	mtx.unlock();
	return;
}

template class node<int>;
template class node<float>;