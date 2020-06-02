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

thread_local queue<node_atomic_del<int> *> deleteQueue;

template <class T> LockFree_impr_mem<T>::LockFree_impr_mem() {
	head = new nodeAtom<T>(0, INT32_MIN);
	head->next = new nodeAtom<T>(0, INT32_MAX);

	Tmax = omp_get_max_threads();
	snap = new std::atomic<uint32_t>[Tmax];
	// std::fill(snap, snap + Tmax * sizeof(uint32_t), 0);
}

template <class T> LockFree_impr_mem<T>::~LockFree_impr_mem() {
	while (head != NULL) {
		nodeAtom<T> *oldHead = head;
		head = getPointer(head->next);
		delete oldHead;
	}
	delete snap;
}

template <class T> bool LockFree_impr_mem<T>::add(T item, sub_benchMark_t *benchMark) {
	Window_at_t<nodeAtom<T>> w;
	int32_t key = key_calc<T>(item);
	int tid = omp_get_thread_num();
	try {
		while (true) {
			w = find(item, benchMark);
			nodeAtom<T> *pred = w.pred;
			nodeAtom<T> *curr = w.curr;

			// Item already in the set
			if (curr->key == key) {
				snap[tid]++;
				return false;
			}
			assert(pred->key < key && curr->key > key);

			// Add item to the set
			nodeAtom<T> *n = new nodeAtom<T>(item);
			n->next.store(w.curr);
			// n->next.store(resetFlag(&n->next.load()));
			resetFlag(&n->next);

			if (atomic_compare_exchange_strong(&pred->next, &curr, n) == true) {
				snap[tid]++;
				return true;
			}
			benchMark->goToStart += 1;
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

template <class T> bool LockFree_impr_mem<T>::remove(T item, sub_benchMark_t *benchMark) {
	Window_at_t<nodeAtom<T>> w;
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
				if (atomic_compare_exchange_weak(&w.curr->next, &succ, markedsucc) == false) {
					benchMark->goToStart += 1;
					continue;
				}

				// try to unlink
				if (atomic_compare_exchange_weak(&w.pred->next, &w.curr, succ) == true) {

					deleteQueue.push(new node_atomic_del<T>(w.curr, this->snap, this->Tmax));
				}
				emteyQueue(false);
				return true;
			} else {
				emteyQueue(false);
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

template <class T> bool LockFree_impr_mem<T>::contains(T item, sub_benchMark_t *benchMark) {
	nodeAtom<T> *n = head;

	int32_t key = key_calc<T>(item);
	while (n->key < key) {
		n = getPointer(n->next.load());
	}
	bool result = n->key == key && !getFlag(n->next.load());
	emteyQueue(false);
	return result;
}

template <class T> Window_at_t<nodeAtom<T>> LockFree_impr_mem<T>::find(T item, sub_benchMark_t *benchMark) {

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

			// link out marked items
			while (getFlag(curr->next.load())) {
				resetFlag(&curr);
				resetFlag(&succ);

				//  link out curr, not possible when pred is flaged
				if (atomic_compare_exchange_weak(&pred->next, &curr, succ) == false) {
					if (getFlag(old->next) == false && pred != old) { // impruvement
						pred = old;
						curr = getPointer(pred->next);
						succ = getPointer(curr->next);
						continue;
					}
					benchMark->goToStart += 1;
					goto retry;
				}
				curr = succ;
				succ = getPointer(succ->next.load());
			}
			if (curr->key >= key) {
				Window_at_t<nodeAtom<T>> w{pred, curr};
				return w;
			}
			old = pred;
			pred = curr;
			curr = succ;
		}
	}
}

template <class T> nodeAtom<T> *LockFree_impr_mem<T>::getPointer(nodeAtom<T> *pointer) {
	uint64_t u64_ptr = (uint64_t)pointer;
	return (nodeAtom<T> *)(u64_ptr &= ~(MASK));
}

template <class T> void LockFree_impr_mem<T>::setFlag(nodeAtom<T> **pointer) {
	// cout <<endl <<"Pointer: "<<*pointer <<" ";
	uint64_t u64_ptr = (uint64_t)*pointer;
	*pointer = (nodeAtom<T> *)(u64_ptr |= MASK);
	// cout <<*pointer <<" "<<u64_ptr << endl;
}

template <class T> void LockFree_impr_mem<T>::resetFlag(nodeAtom<T> **pointer) {
	uint64_t u64_ptr = (uint64_t)*pointer;
	*pointer = (nodeAtom<T> *)(u64_ptr &= ~(MASK));
}

template <class T> void LockFree_impr_mem<T>::resetFlag(atomic<nodeAtom<T> *> *pointer) {
	uint64_t u64_ptr = (uint64_t)pointer->load();
	pointer->store((nodeAtom<T> *)(u64_ptr &= ~(MASK)));
}

template <class T> bool LockFree_impr_mem<T>::getFlag(nodeAtom<T> *pointer) {
	return (nodeAtom<int> *)((((uint64_t)pointer) >> FLAG_POS) & 1U);
}

template <class T> void LockFree_impr_mem<T>::emteyQueue(bool final) {
	int tid = omp_get_thread_num();
	while (deleteQueue.empty() == false) {
		node_atomic_del<T> *curr = deleteQueue.front();
		snap[tid]++;
		if (final == false) {
			for (size_t i = 0; i < this->Tmax; i++) {
				if (curr->snap[i] == this->snap[i]) {
					return;
				}
			}
		}
		deleteQueue.pop();
		delete (curr);
	}
	return;
}

template <class T>
node_atomic_del<T>::node_atomic_del(nodeAtom<T> *pointer, std::atomic<uint32_t> *snap, size_t T_max) {
	this->pointer = pointer;
	this->snap = new std::atomic<uint32_t>[T_max];
	std::memcpy(this->snap, snap, T_max * sizeof(uint32_t));
}

template <class T> node_atomic_del<T>::~node_atomic_del() {
	delete pointer;
	delete snap;
}

template class LockFree_impr_mem<int>;
template class node_atomic_del<int>;