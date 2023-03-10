/** @file Lock_free_impr_mem.cpp
 * @author Daniel Zainzinger
 * @date 2.6.2020
 *
 * @brief Lockfree list based set, with improvement for linkout nodes with memory management.
 */
#include <iostream>
using namespace std;
#include "Lock_free_impr_mem.hpp"
#include "benchmark.hpp"
#include "key.hpp"
#include "node.hpp"
#include <assert.h>
#include <cstring>
#include <omp.h>
#include <queue>
#include <stdint.h>

#define FLAG_POS 63
#define MASK 1ULL << FLAG_POS

// queue to store the candidats for delete
static thread_local queue<void *> deleteQueue;

/**
 * @brief Constructor for the datastructure
 */
template <class T> LockFree_impr_mem<T>::LockFree_impr_mem(size_t Tmax) {
	head = new nodeAtom<T>(0, INT32_MIN);
	head->next = new nodeAtom<T>(0, INT32_MAX);

	this->Tmax =Tmax;
	snap = new std::atomic<uint32_t>[Tmax];
	active = new std::atomic<uint16_t>[Tmax];
	for (size_t i = 0; i < Tmax; i++) {
		snap[i].store(0);
		active[i].store(0);
	}
}

/**
 * @brief Destructor for the datastructure
 */
template <class T> LockFree_impr_mem<T>::~LockFree_impr_mem() {
	while (head != NULL) {
		nodeAtom<T> *oldHead = head;
		head = getPointer(head->next);
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
template <class T> bool LockFree_impr_mem<T>::add(T item, sub_benchMark_t *benchMark) {
	int tid = omp_get_thread_num();
	active[tid].store(1);
	Window_t<nodeAtom<T>> w;
	std::chrono::_V2::system_clock::time_point resetTime;
	bool reset = false; // is true, if there was a reset and we have to start again from the beginning
	int32_t key = key_calc<T>(item);
	try {
		while (true) {
			w = find(item, benchMark);
			nodeAtom<T> *pred = w.pred;
			nodeAtom<T> *curr = w.curr;

			if (reset == true) { // messure the lost time because of a reset
				auto finishTime = chrono::high_resolution_clock::now();
				chrono::duration<double> elapsed = finishTime - resetTime;
				uint32_t mus = chrono::duration_cast<chrono::microseconds>(elapsed).count();
				benchMark->lostTime += mus;
				reset = false;
			}

			// Item already in the set
			if (curr->key == key) {
				emptyQueue(false);
				return false;
			}
			assert(pred->key < key && curr->key > key);

			// Add item to the set
			nodeAtom<T> *n = new nodeAtom<T>(item);
			n->next.store(w.curr);
			resetFlag(&n->next);
			if (atomic_compare_exchange_strong(&pred->next, &curr, n) == false) { // include node
				// delete the new node, if it was not possible to add
				delete n;
				benchMark->goToStart += 1;
				resetTime = chrono::high_resolution_clock::now();
				reset = true;
				continue;
			}
			emptyQueue(false);
			return true;
		}
	}

	// Exception handling
	catch (exception &e) {
		cerr << "Error during add: " << item << std::endl;
		cerr << "Standard exception: " << e.what() << endl;
		return false;
	} catch (...) {
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
template <class T> bool LockFree_impr_mem<T>::remove(T item, sub_benchMark_t *benchMark) {
	int tid = omp_get_thread_num();
	active[tid].store(1);
	Window_t<nodeAtom<T>> w;
	std::chrono::_V2::system_clock::time_point resetTime;
	bool reset = false; // is true, if there was a reset and we have to start again from the beginning
	int32_t key = key_calc<T>(item);
	try {
		while (true) {

			w = find(item, benchMark);

			if (reset == true) { // messure the lost time because of a reset
				auto finishTime = chrono::high_resolution_clock::now();
				chrono::duration<double> elapsed = finishTime - resetTime;
				uint32_t mus = chrono::duration_cast<chrono::microseconds>(elapsed).count();
				benchMark->lostTime += mus;
				reset = false;
			}

			if (key == w.curr->key) {
				nodeAtom<T> *succ = w.curr->next.load();
				nodeAtom<T> *markedsucc = succ;

				setFlag(&markedsucc);
				resetFlag(&succ);

				// mark as deleted
				if (atomic_compare_exchange_strong(&w.curr->next, &succ, markedsucc) == false) {
					benchMark->goToStart += 1;
					resetTime = chrono::high_resolution_clock::now();
					reset = true;
					continue;
				}

				// try to unlink
				if (atomic_compare_exchange_strong(&w.pred->next, &w.curr, succ) == true) {

					deleteQueue.push(new node_del<nodeAtom<T>>(w.curr, this->snap, this->Tmax));
				}
				emptyQueue(false);
				return true;
			} else {
				emptyQueue(false);
				return false;
			}
		}
	}
	// Exception handling
	catch (exception &e) {
		cerr << "Error during remove: " << item << std::endl;
		cerr << "Standard exception: " << e.what() << endl;
		return false;
	} catch (...) {

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
template <class T> bool LockFree_impr_mem<T>::contains(T item, sub_benchMark_t *benchMark) {
	int tid = omp_get_thread_num();
	active[tid].store(1);
	nodeAtom<T> *n = head;

	int32_t key = key_calc<T>(item);
	while (n->key < key) {
		n = getPointer(n->next.load());
	}
	bool result = n->key == key && !getFlag(n->next.load());
	emptyQueue(false);
	return result;
}

/**
 * @brief Function which returns a window, where the key is of the first element is smaller than the key of the item,
 * and the second key bigger or equal.
 *
 * @param[in]  	item  		item, from which the key is calculated to search the window
 * @param[out]  benchMark  	a struct, which stores information for benchmarking
 */
template <class T> Window_t<nodeAtom<T>> LockFree_impr_mem<T>::find(T item, sub_benchMark_t *benchMark) {

	nodeAtom<T> *pred, *curr, *old;
	;
	int32_t key = key_calc<T>(item);
	old = head;
retry:
	while (true) {
		pred = head;
		curr = getPointer(pred->next.load());
		auto resetTime = chrono::high_resolution_clock::now();

		// get through the list
		while (true) {
			assert(pred->next != NULL);
			nodeAtom<T> *succ = getPointer(curr->next.load());

			// unlink marked items
			while (getFlag(curr->next.load())) {
				resetFlag(&curr);
				resetFlag(&succ);

				//  link out curr, not possible when pred is flaged
				if (atomic_compare_exchange_strong(&pred->next, &curr, succ) == false) {
					if (getFlag(old->next) == false && pred != old) { // improvement
						pred = old;
						curr = getPointer(pred->next);
						succ = getPointer(curr->next);
						continue;
					}
					benchMark->goToStart += 1;
					// messure the time thats lost because of the reset
					auto finishTime = chrono::high_resolution_clock::now();
					chrono::duration<double> elapsed = finishTime - resetTime;
					uint32_t mus = chrono::duration_cast<chrono::microseconds>(elapsed).count();
					benchMark->lostTime += mus;
					goto retry;
				}
				resetTime = chrono::high_resolution_clock::now(); // set time, because we did something useful

				deleteQueue.push(
					new node_del<nodeAtom<T>>(curr, this->snap, this->Tmax)); // put in de que for delete candidats
				curr = succ;
				succ = getPointer(succ->next.load());
			}
			if (curr->key >= key) {
				assert(pred->key < key && curr->key >= key);
				Window_t<nodeAtom<T>> w{pred, curr};
				return w;
			}
			old = pred;
			pred = curr;
			curr = succ;
		}
	}
}

/**
 * @brief Function return pointer without a flag
 *
 * @param	[in]  pointer  	pointer to the node with flag included
 * @return 	pointer without flag
 */
template <class T> nodeAtom<T> *LockFree_impr_mem<T>::getPointer(nodeAtom<T> *pointer) {
	uint64_t u64_ptr = (uint64_t)pointer;
	return (nodeAtom<T> *)(u64_ptr &= ~(MASK));
}

/**
 * @brief Function set flag, which mark the node for delete (overloaded)
 *
 * @param	[in,out]  pointer  	pointer to the node
 */
template <class T> void LockFree_impr_mem<T>::setFlag(nodeAtom<T> **pointer) {
	uint64_t u64_ptr = (uint64_t)*pointer;
	*pointer = (nodeAtom<T> *)(u64_ptr |= MASK);
}

/**
 * @brief Function reset flag, which mark the node for delete (overloaded)
 *
 * @param	[in,out]  pointer  	pointer to the node
 */
template <class T> void LockFree_impr_mem<T>::resetFlag(nodeAtom<T> **pointer) {
	uint64_t u64_ptr = (uint64_t)*pointer;
	*pointer = (nodeAtom<T> *)(u64_ptr &= ~(MASK));
}

/**
 * @brief Function reset flag, which mark the node for delete (overloaded)
 *
 * @param	[in,out]  pointer  	pointer to the node
 */
template <class T> void LockFree_impr_mem<T>::resetFlag(atomic<nodeAtom<T> *> *pointer) {
	uint64_t u64_ptr = (uint64_t)pointer->load();
	pointer->store((nodeAtom<T> *)(u64_ptr &= ~(MASK)));
}

/**
 * @brief Function return if node is marked for delete
 *
 * @param	[in]  pointer  	pointer to the node
 * @return 	flag, if node is marded for delete
 */
template <class T> bool LockFree_impr_mem<T>::getFlag(nodeAtom<T> *pointer) {
	return (nodeAtom<int> *)((((uint64_t)pointer) >> FLAG_POS) & 1U);
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
template <class T> void LockFree_impr_mem<T>::emptyQueue(bool final) {
	int tid = omp_get_thread_num();
	active[tid].store(0);
	/* get the total number of threads available in this parallel region */
	size_t NPR = omp_get_num_threads();
	snap[tid]++;
	while (deleteQueue.empty() == false) {
		node_del<nodeAtom<T>> *curr = static_cast<node_del<nodeAtom<T>> *>(deleteQueue.front());
		if (final == false) {
			for (size_t i = 0; i < NPR; i++) {
				if (curr->snap[i] == this->snap[i].load() && active[i] == true) {
					return;
				}
			}
		}
		deleteQueue.pop();
		delete (curr);
	}
	return;
}

template class LockFree_impr_mem<int>;
