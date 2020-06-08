/** @file Fine_Grained.cpp
 * @author Daniel Zainzinger
 * @date 2.6.2020
 *
 * @brief list based set, which locks node, when it is reading or writing on it
 */
#include <iostream>
using namespace std;
#include "Fine_Grained.hpp"
#include "benchmark.hpp"
#include "key.hpp"
#include "node.hpp"
#include <assert.h>
#include <omp.h>
#include <stdint.h>

/**
 * @brief Constructor for the datastructure
 */
template <class T> FineList<T>::FineList() {
	head = new nodeFine<T>(0, INT32_MIN);
	head->next = new nodeFine<T>(0, INT32_MAX);
}

/**
 * @brief Destructor for the datastructure
 */
template <class T> FineList<T>::~FineList() {
	while (head != NULL) {
		nodeFine<T> *oldHead = head;
		head = head->next;
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
template <class T> bool FineList<T>::add(T item, sub_benchMark_t *benchMark) {
	nodeFine<T> *pred = NULL, *curr = NULL;
	try {
		int32_t key = key_calc<T>(item);
		head->lock();
		pred = head;
		curr = pred->next;
		curr->lock();

		while (curr->key < key) {
			assert(curr->next != NULL);
			pred->unlock();
			pred = curr;
			curr = curr->next;
			curr->lock();
		}

		// Item already in the set
		if (key == curr->key) {
			pred->unlock();
			curr->unlock();
			return false;
		}

		// Add item to the set
		nodeFine<T> *n = new nodeFine<T>(item);
		n->next = curr;
		pred->next = n;
		pred->unlock();
		curr->unlock();
		return true;
	}

	// Exception handling
	catch (exception &e) {
		pred->unlock();
		curr->unlock();
		cerr << "Error during add: " << item << std::endl;
		cerr << "Standard exception: " << e.what() << endl;
		return false;
	} catch (...) {
		pred->unlock();
		curr->unlock();
		cerr << "Error during add: " << item << std::endl;
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
template <class T> bool FineList<T>::remove(T item, sub_benchMark_t *benchMark) {
	nodeFine<T> *pred, *curr;
	int32_t key = key_calc<T>(item);
	head->lock();

	try {
		pred = head;
		curr = pred->next;
		curr->lock();
		while (curr->key < key) {
			assert(curr->next != NULL);
			pred->unlock();
			pred = curr;
			curr = curr->next;
			curr->lock();
		}
		if (key == curr->key) {
			pred->next = curr->next;
			pred->unlock();
			curr->unlock();
			delete curr;
			return true;
		} else {
			pred->unlock();
			curr->unlock();
			return false;
		}
	}
	// Exception handling
	catch (exception &e) {
		pred->unlock();
		curr->unlock();
		cerr << "Error during remove: " << item << std::endl;
		cerr << "Standard exception: " << e.what() << endl;
		return false;
	} catch (...) {
		pred->unlock();
		curr->unlock();
		cerr << "Error during remove: " << item << std::endl;
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
template <class T> bool FineList<T>::contains(T item, sub_benchMark_t *benchMark) {
	nodeFine<T> *pred, *curr;
	int32_t key = key_calc<T>(item);
	head->lock();

	try {
		pred = head;
		curr = pred->next;
		curr->lock();
		while (curr->key < key) {
			assert(curr->next != NULL);
			pred->unlock();
			pred = curr;
			curr = curr->next;
			curr->lock();
		}
		if (key == curr->key) {
			pred->unlock();
			curr->unlock();
			return true;
		} else {
			pred->unlock();
			curr->unlock();
			return false;
		}
	}
	// Exception handling
	catch (exception &e) {
		pred->unlock();
		curr->unlock();
		cerr << "Error during remove: " << item << std::endl;
		cerr << "Standard exception: " << e.what() << endl;
		return false;
	} catch (...) {
		pred->unlock();
		curr->unlock();
		cerr << "Error during remove: " << item << std::endl;
		return false;
	}
}

template class FineList<int>;
