/** @file Lock_free_impr.cpp
 * @author Daniel Zainzinger
 * @date 2.6.2020
 *
 * @brief Lockfree list based set, with improvement for linkout nodes without memory management.
 */
#include <iostream>
using namespace std;
#include "Lock_free_impr.hpp"
#include "benchmark.hpp"
#include "key.hpp"
#include "node.hpp"
#include <assert.h>
#include <omp.h>
#include <stdint.h>

/**
 * @brief Constructor for the datastructure
 */
template <class T> LockFree_impr<T>::LockFree_impr() {
	head = new nodeAtom<T>(0, INT32_MIN);
	head->next = new nodeAtom<T>(0, INT32_MAX);
}

/**
 * @brief Destructor for the datastructure
 */
template <class T> LockFree_impr<T>::~LockFree_impr() {
	while (head != NULL) {
		nodeAtom<T> *oldHead = head;
		head = getPointer(head->next);
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
template <class T> bool LockFree_impr<T>::add(T item, sub_benchMark_t *benchMark) {
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
template <class T> bool LockFree_impr<T>::remove(T item, sub_benchMark_t *benchMark) {
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
				atomic_compare_exchange_strong(&w.pred->next, &w.curr, succ);
				return true;
			} else {
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
template <class T> bool LockFree_impr<T>::contains(T item, sub_benchMark_t *benchMark) {
	nodeAtom<T> *n = head;

	int32_t key = key_calc<T>(item);
	while (n->key < key) {
		n = getPointer(n->next.load());
	}
	return n->key == key && !getFlag(n->next.load());
}

/**
 * @brief Function which returns a window, where the key is of the first element is smaller than the key of the item,
 * and the second key bigger or equal.
 *
 * @param[in]  	item  		item, from which the key is calculated to search the window
 * @param[out]  benchMark  	a struct, which stores information for benchmarking
 */
template <class T> Window_t<nodeAtom<T>> LockFree_impr<T>::find(T item, sub_benchMark_t *benchMark) {

	nodeAtom<T> *pred, *curr, *old;

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

			// link out marked items
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
				curr = succ;
				succ = getPointer(succ->next.load());
			}
			if (curr->key >= key) {
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
template <class T> nodeAtom<T> *LockFree_impr<T>::getPointer(nodeAtom<T> *pointer) {
	uint64_t u64_ptr = (uint64_t)pointer;
	return (nodeAtom<T> *)(u64_ptr &= ~(MASK));
}

/**
 * @brief Function set flag, which mark the node for delete (overloaded)
 *
 * @param	[in,out]  pointer  	pointer to the node
 */
template <class T> void LockFree_impr<T>::setFlag(nodeAtom<T> **pointer) {
	uint64_t u64_ptr = (uint64_t)*pointer;
	*pointer = (nodeAtom<T> *)(u64_ptr |= MASK);
}

/**
 * @brief Function reset flag, which mark the node for delete (overloaded)
 *
 * @param	[in,out]  pointer  	pointer to the node
 */
template <class T> void LockFree_impr<T>::resetFlag(nodeAtom<T> **pointer) {
	uint64_t u64_ptr = (uint64_t)*pointer;
	*pointer = (nodeAtom<T> *)(u64_ptr &= ~(MASK));
}

/**
 * @brief Function reset flag, which mark the node for delete (overloaded)
 *
 * @param	[in,out]  pointer  	pointer to the node
 */
template <class T> void LockFree_impr<T>::resetFlag(atomic<nodeAtom<T> *> *pointer) {
	uint64_t u64_ptr = (uint64_t)pointer->load();
	pointer->store((nodeAtom<T> *)(u64_ptr &= ~(MASK)));
}

/**
 * @brief Function return if node is marked for delete
 *
 * @param	[in]  pointer  	pointer to the node
 * @return 	flag, if node is marded for delete
 */
template <class T> bool LockFree_impr<T>::getFlag(nodeAtom<T> *pointer) {
	return (nodeAtom<int> *)((((uint64_t)pointer) >> FLAG_POS) & 1U);
}

template class LockFree_impr<int>;
