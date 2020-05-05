#include "node.h"
#include <iostream>
#include <stdint.h>
#include "key.h"
using namespace std;

template <class T> node<T>::node(T item) {
	this->key = key_calc<T>(item);
	this->next = NULL;
	this->item = item;
}

template <class T> node<T>::node(T item, int32_t key) {
	this->key = key;
	this->next = NULL;
	this->item = item;
}

template <class T> int32_t node<T>::hash() { 
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

template class node<int>;
//template class node<float>;

template class nodeFine<int>;
//template class nodeFine<float>;