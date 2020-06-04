/** @file Coarse_Grained.cpp
 * @author Daniel Zainzinger
 * @date 2.6.2020
 *
 * @brief list based set, which locks the whole datastructure, when it is reading or writing on it
 */
#include <iostream>
using namespace std;
#include "Coarse_Grained.hpp"
#include "key.hpp"
#include "node.hpp"
#include <assert.h>
#include <mutex> // std::mutex, std::lock_guard
#include <omp.h>
#include <stdint.h>

/**
 * @brief Constructor for the datastructure
 */
template <class T> CoarseList<T>::CoarseList() {
	head = new node<T>(0, INT32_MIN);
	head->next = new node<T>(0, INT32_MAX);
}

/**
 * @brief Destructor for the datastructure
 */
template <class T> CoarseList<T>::~CoarseList() {
	while (head != NULL) {
		node<T>* oldHead = head;
		head=head->next;
		delete oldHead;
	} 
}

/**
 * @brief Function which adds one item from the datastructure
 *
 * @param[in]  	item  		item, which should be add to the datastructure
 * @param[out]  benchMark  	a struct, which stores information for benchmarking
 * @return true, if it was succeccfully added, false otherwise
 */
template <class T> bool CoarseList<T>::add(T item,sub_benchMark_t *benchMark) {
	try {
		node<T> *pred, *curr;
		mtx.lock();
		pred = head;
		curr = pred->next;

		int32_t key = key_calc<T>(item);

		while (curr->key < key) {
			assert(curr->next != NULL);
			pred = curr;
			curr = curr->next;
		}

		// Item already in the set
		if (key == curr->key) {
			mtx.unlock();
			return false;
		}

		// Add item to the set
		node<T> *n = new node<T>(item);
		n->next = curr;
		pred->next = n;
		mtx.unlock();
		return true;
	}

	// Exception handling
	catch (exception &e) {
		mtx.unlock();
		cerr << "Error during add the item: " << item << std::endl;
		cerr << "Standard exception: " << e.what() << endl;
		return false;
	} catch (...) {
		mtx.unlock();
		cerr << "Error during add the item: " << item << std::endl;
		return false;
	}
}

/**
 * @brief Function which removes one item from the datastructure
 *
 * @param[in]  	item  		item, which should be removed from the datastructure
 * @param[out]  benchMark  	a struct, which stores information for benchmarking
 * @return true, if it was succeccfully removed, false otherwise
 */
template <class T> bool CoarseList<T>::remove(T item, sub_benchMark_t *benchMark) {
	node<T> *pred, *curr;
	int32_t key = key_calc<T>(item);
	mtx.lock();

	try {
		pred = head;
		curr = pred->next;
		while (curr->key < key) {
			assert(curr->next != NULL);
			pred = curr;
			curr = curr->next;
		}
		if (key == curr->key) {
			pred->next = curr->next;
			delete curr;
			mtx.unlock();
			return true;
		} else {
			mtx.unlock();
			return false;
		}
	}
	// Exception handling
	catch (exception &e) {
		mtx.unlock();
		cerr << "Error during remove the item: " << item << std::endl;
		cerr << "Standard exception: " << e.what() << endl;
		return false;
	} catch (...) {
		mtx.unlock();
		cerr << "Error during remove the item: " << item << std::endl;
		return false;
	}
}

/**
 * @brief Function which checks if the item is in the datastructure
 *
 * @param[in]  	item  		item for check if it is included
 * @param[out]  benchMark  	a struct, which stores information for benchmarking
 * @return true, if item is in the datastructure, false otherwise
 */
template <class T> bool CoarseList<T>::contains(T item, sub_benchMark_t *benchMark) {
	node<T> *pred, *curr;
	int32_t key = key_calc<T>(item);
	mtx.lock();

	try {
		pred = head;
		curr = pred->next;
		while (curr->key < key) {
			assert(curr->next != NULL);
			pred = curr;
			curr = curr->next;
		}
		if (key == curr->key) {
			mtx.unlock();
			return true;
		} else {
			mtx.unlock();
			return false;
		}
	}
	// Exception handling
	catch (exception &e) {
		mtx.unlock();
		cerr << "Error during remove the item: " << item << std::endl;
		cerr << "Standard exception: " << e.what() << endl;
		return false;
	} catch (...) {
		mtx.unlock();
		cerr << "Error during remove the item: " << item << std::endl;
		return false;
	}
}

template class CoarseList<int>;
