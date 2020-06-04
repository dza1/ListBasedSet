/** @file node.cpp
 * @author Daniel Zainzinger
 * @date 2.6.2020
 *
 * @brief nodes for the datastructure and the delete queue
 */
#include "key.hpp"
#include "node.hpp"
#include <iostream>
#include <stdint.h>
#include <omp.h>
using namespace std;

/////////////////////////node for inheritance/////////////////////////////

/**
 * @brief Constructor to create a node
 *
 * @param[in]  item  	item which should be stored in the node
 */
template <class T> node_virt<T>::node_virt(T item) {
	this->key = key_calc(item);
	this->item = item;
}

/**
 * @brief Constructor to create the head and the tail node
 *
 * @param[in]  item  	item which should be stored in the node
 * @param[in]  key  	key, with which the item should be stored
 */
template <class T> node_virt<T>::node_virt(T item, int32_t key) {
	this->key = key;
	this->item = item;
}

/////////////////////////nodeFine/////////////////////////////

/**
 * @brief Lock funktion for a node of 
 *
 * @param[in]  pointer  	Pointer to the node which should be deleted
 * @param[in]  snap  		snapshot of all timestamp, when the node was put in the queue
 * @param[in]  Tmax  		Amount of maximal threads
 */
template <class T> void nodeFine<T>::lock() {
	mtx.lock();
	return;
}

template <class T> void nodeFine<T>::unlock() {
	mtx.unlock();
	return;
}

/////////////////////////node_del/////////////////////////////

/**
 * @brief Constructor for create a node with a canidate to delete
 *
 * @param[in]  pointer  	Pointer to the node which should be deleted
 * @param[in]  snap  		snapshot of all timestamp, when the node was put in the queue
 * @param[in]  Tmax  		Amount of maximal threads
 */
template <class T> node_del<T>::node_del(T *pointer, std::atomic<uint32_t> *snap, size_t Tmax) {
	this->pointer = pointer;
	this->snap = new uint32_t[Tmax];
	for (size_t i = 0; i < Tmax; i++) {
		this->snap[i] = snap[i].load();
	}
}

/**
 * @brief Destructor for node with a canidate to delete
 */
template <class T> node_del<T>::~node_del() {
	delete pointer;
	delete[] snap;
}


////////////////////////////////////////////////////////////

template class node_virt<int>;
template class nodeFine<int>;
template class node_del<nodeFine<int>>;
template class node_del<nodeAtom<int>>;
template class nodeAtom<int>;
