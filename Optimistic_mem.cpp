/** @file Optimistic_mem.cpp
 * @author Daniel Zainzinger
 * @date 2.6.2020
 *
 * @brief Optimistic list based set, where the datastructure search a element without locking.
 * If the node is found, the current and the previous node get locked and than the reachability of the two nodes from the head gets checked  
 * With memory management  
 */
#include <iostream>
using namespace std;
#include "Optimistic_mem.hpp"
#include "key.hpp"
#include "node.hpp"
#include <assert.h>
#include <omp.h>
#include <stdint.h>
#include "benchmark.hpp"
#include <queue>
#include <unordered_map>


//queue to store the candidats for delete
static thread_local queue<void*> deleteQueue;


/**
 * @brief Constructor for the datastructure
 */
template <class T> Optimistic_mem<T>::Optimistic_mem() {
	head = new nodeFine<T>(0, INT32_MIN);
	head->next = new nodeFine<T>(0, INT32_MAX);

	Tmax = omp_get_max_threads();
	snap = new std::atomic<uint32_t>[Tmax];
	for (size_t i = 0; i < Tmax; i++) {
		snap[i].store(0);
	}
}

/**
 * @brief Destructor for the datastructure
 */
template <class T> Optimistic_mem<T>::~Optimistic_mem() {
	while (head != NULL) {
		nodeFine<T> *oldHead = head;
		head = head->next;
		delete oldHead;
	}
	delete[] snap;
}

/**
 * @brief Function which adds one item from the datastructure
 *
 * @param[in]  	item  		item, which should be add to the datastructure
 * @param[out]  benchMark  	a struct, which stores information for benchmarking
 * @return true, if it was succeccfully added, false otherwise
 */
template <class T> bool Optimistic_mem<T>::add(T item,sub_benchMark_t *benchMark) {
	Window_t<nodeFine<T>> w;
	try {
		w = find(item,benchMark);
		int32_t key = key_calc<T>(item);

		// Item already in the set
		if (key == w.curr->key) {
			unlock(w);
			emptyQueue(false);
			return false;
		}

		// Add item to the set
		nodeFine<T> *n = new nodeFine<T>(item);
		if (w.pred->key >= n->key) {
			printf("Error");
		}
		n->next = w.curr;
		w.pred->next = n;

		assert(w.pred->key < n->key);
		assert(n->key < w.curr->key);
		unlock(w);
		emptyQueue(false);
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
template <class T> bool Optimistic_mem<T>::remove(T item, sub_benchMark_t *benchMark) {
	Window_t<nodeFine<T>> w;
	try {
		w = find(item,benchMark);
		int32_t key = key_calc<T>(item);

		if (key == w.curr->key) {
			w.pred->next = w.curr->next;
			unlock(w);
			//put th unlinked node in the queue to delete
			deleteQueue.push(new node_del<nodeFine<T>>(w.curr, this->snap, this->Tmax));
			emptyQueue(false);
			return true;
		} else {
			unlock(w);
			emptyQueue(false);
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
template <class T> bool Optimistic_mem<T>::contains(T item, sub_benchMark_t *benchMark) {
	Window_t<nodeFine<T>> w;
	try {
		w = find(item,benchMark);
		int32_t key = key_calc<T>(item);

		if (key == w.curr->key) {
			unlock(w);
			emptyQueue(false);
			return true;
		} else {
			unlock(w);
			emptyQueue(false);
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
template <class T> Window_t<nodeFine<T>> Optimistic_mem<T>::find(T item, sub_benchMark_t *benchMark) {
	nodeFine<T> *pred, *curr;
	int32_t key = key_calc(item);
	while (true) {
		pred = head;
		curr = head->next;

		while (curr->key < key) {
			assert(curr->next != NULL);
			pred = curr;
			curr = curr->next;
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
		}
	}
}

/**
 * @brief Function which checks if the window w is still reachable from the head
 *
 * @param[in]  	w	  		window, which includes two nodes
 * @return true, if both nodes are still reachable from the head, otherwise false
 */
template <class T> bool Optimistic_mem<T>::validate(Window_t<nodeFine<T>> w) {
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
 * @param[in]  	w 		window, which includes two nodes
 */
template <class T> void Optimistic_mem<T>::lock(Window_t<nodeFine<T>> w) {
	w.pred->lock();
	w.curr->lock();
}

/**
 * @brief Function unlock the window
 *
 * @param[in]  	w  		window, which includes two nodes
 */
template <class T> void Optimistic_mem<T>::unlock(Window_t<nodeFine<T>> w) {
	w.pred->unlock();
	w.curr->unlock();
}


/**
 * @brief Function to empty and delete que of candidats for delete
 *
 * @details emptyQueue is always called before return from this object. It checks, if
 * each thread increased allready the timestamp, to be sure, that no one reads the
 * node during delete
 *
 * @param[in]  final  	if it is true, the function doesn't check if snap changed
 */
template <class T> void Optimistic_mem<T>::emptyQueue(bool final) {
	int tid = omp_get_thread_num();
	/* get the total number of threads available in this parallel region */
	size_t NPR = omp_get_num_threads();
	while (deleteQueue.empty() == false) {
		node_del<nodeFine<T>> *curr = static_cast<node_del<nodeFine<T>>*>(deleteQueue.front());
		snap[tid]++;
		if (final == false) {
			for (size_t i = 0; i < NPR; i++) {
				if (curr->snap[i] == this->snap[i].load()) {
					return;
				}
			}
		}
		deleteQueue.pop();
		delete (curr);
	}
	return;
}


template class Optimistic_mem<int>;