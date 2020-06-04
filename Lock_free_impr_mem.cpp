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

//queue to store the candidats for delete
static thread_local queue<void *> deleteQueue;

/**
 * @brief Constructor for the datastructure
 */
template <class T> LockFree_impr_mem<T>::LockFree_impr_mem() {
	head = new nodeAtom<T>(0, INT32_MIN);
	head->next = new nodeAtom<T>(0, INT32_MAX);

	Tmax = omp_get_max_threads();
	snap = new std::atomic<uint32_t>[Tmax];
	for (size_t i = 0; i < Tmax; i++) {
		snap[i].store(0);
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
	Window_t<nodeAtom<T>> w;
	int32_t key = key_calc<T>(item);
	try {
		while (true) {
			w = find(item, benchMark);
			nodeAtom<T> *pred = w.pred;
			nodeAtom<T> *curr = w.curr;

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
				//delete the new node, if it was not possible to add
				delete n;
				benchMark->goToStart += 1;
				continue;
			}
			emptyQueue(false);
			return true;
		}
	}

	// Exception handling
	catch (exception &e) {
		cerr << "Error during add the item: " << item << std::endl;
		cerr << "Standard exception: " << e.what() << endl;
		return false;
	} catch (...) {
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
template <class T> bool LockFree_impr_mem<T>::remove(T item, sub_benchMark_t *benchMark) {
	Window_t<nodeAtom<T>> w;
	try {
		while (true) {

			w = find(item, benchMark);
			int32_t key = key_calc<T>(item);

			if (key == w.curr->key) {
				nodeAtom<T> *succ = w.curr->next.load();
				nodeAtom<T> *markedsucc = succ;

				setFlag(&markedsucc);
				resetFlag(&succ);
				// delete w.curr;

				// mark as deleted
				if (atomic_compare_exchange_strong(&w.curr->next, &succ, markedsucc) == false) {
					benchMark->goToStart += 1;
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
		cerr << "Error during add the item: " << item << std::endl;
		cerr << "Standard exception: " << e.what() << endl;
		return false;
	} catch (...) {

		cerr << "Error during add the item: " << item << std::endl;
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

		// get through the list
		while (true) {
			assert(pred->next != NULL);
			nodeAtom<T> *succ = getPointer(curr->next.load());

			// unlink marked items
			while (getFlag(curr->next.load())) {
				resetFlag(&curr);
				resetFlag(&succ);

				//  link out curr, not possible when pred is flaged
				if (atomic_compare_exchange_weak(&pred->next, &curr, succ) == false) {
					if (getFlag(old->next) == false && pred != old) { // improvement
						pred = old;
						curr = getPointer(pred->next);
						succ = getPointer(curr->next);
						continue;
					}
					benchMark->goToStart += 1;
					goto retry;
				}
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
	// cout <<endl <<"Pointer: "<<*pointer <<" ";
	uint64_t u64_ptr = (uint64_t)*pointer;
	*pointer = (nodeAtom<T> *)(u64_ptr |= MASK);
	// cout <<*pointer <<" "<<u64_ptr << endl;
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
	/* get the total number of threads available in this parallel region */
	size_t NPR = omp_get_num_threads();
	while (deleteQueue.empty() == false) {
		node_del<nodeAtom<T>> *curr = static_cast<node_del<nodeAtom<T>>*> (deleteQueue.front());
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




template class LockFree_impr_mem<int>;
