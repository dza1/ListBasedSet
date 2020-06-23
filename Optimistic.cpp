/** @file Optimistic.cpp
 * @author Daniel Zainzinger
 * @date 2.6.2020
 *
 * @brief Optimistic list based set, where the datastructure search a element without locking.
 * If the node is found, the current and the previous node get locked and than the reachability of the two nodes from the head gets checked.
 * Without memory management  
 */
#include <iostream>
using namespace std;
#include "Optimistic.hpp"
#include "key.hpp"
#include "node.hpp"
#include <assert.h>
#include <omp.h>
#include <stdint.h>
#include "benchmark.hpp"
#include <chrono>

/**
 * @brief Constructor for the datastructure
 */
template <class T> Optimistic<T>::Optimistic() {
	head = new nodeFine<T>(0, INT32_MIN);
	head->next = new nodeFine<T>(0, INT32_MAX);
}

/**
 * @brief Destructor for the datastructure
 */
template <class T> Optimistic<T>::~Optimistic() {
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
template <class T> bool Optimistic<T>::add(T item,sub_benchMark_t *benchMark) {
	Window_t<nodeFine<T>> w;
	try {
		w = find(item,benchMark);
		int32_t key = key_calc<T>(item);

		// Item already in the set
		if (key == w.curr->key) {
			unlock(w);
			return false;
		}

		// Add item to the set
		nodeFine<T> *n = new nodeFine<T>(item);
		n->next = w.curr;
		w.pred->next = n;

		assert(w.pred->key < n->key);
		assert(n->key < w.curr->key);
		unlock(w);
		return true;
	}

	// Exception handling
	catch (exception &e) {
		unlock(w);
		cerr << "Error during add: " << item << std::endl;
		cerr << "Standard exception: " << e.what() << endl;
		return false;
	} catch (...) {
		unlock(w);
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
template <class T> bool Optimistic<T>::remove(T item, sub_benchMark_t *benchMark) {
	Window_t<nodeFine<T>> w;
	try {
		w = find(item,benchMark);
		int32_t key = key_calc<T>(item);

		if (key == w.curr->key) {
			w.pred->next = w.curr->next;
			unlock(w);
			// delete w.curr;
			return true;
		} else {
			unlock(w);
			return false;
		}
	}
	// Exception handling
	catch (exception &e) {
		unlock(w);
		cerr << "Error during remove: " << item << std::endl;
		cerr << "Standard exception: " << e.what() << endl;
		return false;
	} catch (...) {
		unlock(w);
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
template <class T> bool Optimistic<T>::contains(T item, sub_benchMark_t *benchMark) {
	Window_t<nodeFine<T>> w;
	try {
		w = find(item,benchMark);
		int32_t key = key_calc<T>(item);

		if (key == w.curr->key) {
			unlock(w);
			return true;
		} else {
			unlock(w);
			return false;
		}
	}
	// Exception handling
	catch (exception &e) {
		unlock(w);
		cerr << "Error during contains: " << item << std::endl;
		cerr << "Standard exception: " << e.what() << endl;
		return false;
	} catch (...) {
		unlock(w);
		cerr << "Error during contains: " << item << std::endl;
		return false;
	}
}

/**
 * @brief Function which returns a window, where the key is of the first element is smaller than the key of the item,
 * and the second key bigger or equal. 
 *
 * @param[in]  	item  		item, from which the key is calculated to search the window 
 * @param[out]  benchMark  	a struct, which stores information for benchmarking
 */
template <class T> Window_t<nodeFine<T>> Optimistic<T>::find(T item, sub_benchMark_t *benchMark) {
	nodeFine<T> *pred, *curr;
	std::chrono::_V2::system_clock::time_point resetTime;
	bool reset=false; //is true, if there was a reset and we have to start again from the beginning
	int32_t key = key_calc(item);
	// lock_guard<std::mutex> g(mtx);
	while (true) {
		pred = head;
		curr = head->next;

		while (curr->key < key) {
			assert(curr->next != NULL);
			pred = curr;
			curr = curr->next;
		}
		if(reset==true){
			auto finishTime = chrono::high_resolution_clock::now();
			chrono::duration<double> elapsed = finishTime - resetTime;
			uint32_t mus = chrono::duration_cast<chrono::microseconds>(elapsed).count();
			benchMark->lostTime+=mus;
			reset=false;
		}
		Window_t<nodeFine<T>> w{pred, curr};
		lock(w);
		assert(w.pred->key <= key);
		assert(w.curr->key >= key);
		if (validate(w) == true) {
			return w;
		} else { // not reachable
			unlock(w);
			benchMark->goToStart+=1;
			resetTime = chrono::high_resolution_clock::now();
			reset=true;
		}
	}
}

/**
 * @brief Function which checks if the window w is still reachable from the head
 *
 * @param[in]  	w	  		window, which includes two nodes
 * @return true, if both nodes are still reachable from the head, otherwise false
 */
template <class T> bool Optimistic<T>::validate(Window_t<nodeFine<T>> w) {
	nodeFine<T> *n = head;
	while (n->key <= w.pred->key) {
		assert(n->next != NULL);
		if (n == w.pred) {
			return n->next == w.curr;
		}
		n = n->next;
	}
	return false;
}

/**
 * @brief Function locks the window
 *
 * @param[in]  	w  		window, which includes two nodes
 */
template <class T> void Optimistic<T>::lock(Window_t<nodeFine<T>> w) {
	w.pred->lock();
	w.curr->lock();
}

/**
 * @brief Function unlock the window
 *
 * @param[in]  	w  		window, which includes two nodes
 */
template <class T> void Optimistic<T>::unlock(Window_t<nodeFine<T>> w) {
	w.pred->unlock();
	w.curr->unlock();
}

template class Optimistic<int>;
