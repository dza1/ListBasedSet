#include <iostream>
using namespace std;
#include "Lock_free_mem.hpp"
#include "key.hpp"
#include "node.hpp"
#include <assert.h>
#include <omp.h>
#include <stdint.h>
#include "benchmark.hpp"

#define COUTNMASK 0x00000000FFFFFF // mask for counter, which shows how many threads read this item

thread_local queue<nodeAtom<int> *> deleteQueue;

template <class T> LockFree_mem<T>::LockFree_mem() {
	nodeAtom<T> *tmp;
	head = new nodeAtom<T>(0, INT32_MIN);
	tmp = head->next = new nodeAtom<T>(0, INT32_MAX);
	tmp->next = new nodeAtom<T>(0, INT32_MAX);
}

template <class T> LockFree_mem<T>::~LockFree_mem() {
	while (head != NULL) {
		nodeAtom<T> *oldHead = head;
		head = head->next;
		delete oldHead;
	}
}

template <class T> bool LockFree_mem<T>::add(T item,sub_benchMark_t *benchMark) {
	Window_t<nodeAtom<T>> w;
	int32_t key = key_calc<T>(item);
	try {
		while (true) {
			w = find(item);
			nodeAtom<T> *pred = w.pred;
			nodeAtom<T> *curr = w.curr;

			// Item already in the set
			if (curr->key == key) {
				//deleteNodes();
				return false;
			}

			// Add item to the set
			nodeAtom<T> *n = new nodeAtom<T>(item);
			n->next.store(w.curr);
			// n->next.store(resetFlag(&n->next.load()));
			resetFlag(&n->next);

			if (atomic_compare_exchange_strong(&pred->next, &curr, n) == true) {
				//deleteNodes();
				return true;
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

template <class T> bool LockFree_mem<T>::remove(T item, sub_benchMark_t *benchMark) {
	Window_t<nodeAtom<T>> w;
	try {
		while (true) {

			w = find(item);
			int32_t key = key_calc<T>(item);

			if (key == w.curr->key) {
				nodeAtom<T> *succ = w.curr->next.load();
				nodeAtom<T> *markedsucc = succ;

				setFlag(&markedsucc);
				resetFlag(&succ);

				// mark as deleted
				if (atomic_compare_exchange_strong(&w.curr->next, &succ, markedsucc) == false) {
					continue;
				}

				// try to unlink
				atomic_compare_exchange_weak(&w.pred->next, &w.curr, succ);

				//deleteNodes();
				return true;
			} else {
				//deleteNodes();
				return false;
			}
		}
	}
	// Exception handling
	catch (exception &e) {

		cerr << "Error during add the item: " << item << std::endl;
		cerr << "Standard exception: " << e.what() << endl;
		return false;
	}
	// catch (...) {

	// 	cerr << "Error during add the item: " << item << std::endl;
	// 	return false;
	// }
}

template <class T> bool LockFree_mem<T>::contains(T item, sub_benchMark_t *benchMark) {
	nodeAtom<T> *n = head;
	nodeAtom<T> *curr;
	head->hash_mem++;
	int32_t key = key_calc<T>(item);

	while (n->key < key) {
		int64_t hash = getPointer(n->next.load())->hash_mem.load();
		curr = getPointer(n->next.load());
		if (hash == 0) { // Check if hash was deledet, shouldn't happen
			cerr << "Error with hash" << endl;
			cout << __LINE__ << endl;
			continue;
		}
		if (atomic_compare_exchange_strong(&curr->hash_mem, &hash, hash + 1) == false) {
			continue;
		}
		n->hash_mem--;
		n = curr;
	}
	bool benchMark = n->key == key && !getFlag(n->next.load());
	n->hash_mem--;
	return benchMark;
}

template <class T> Window_t<nodeAtom<T>> LockFree_mem<T>::find(T item) {
	int tid = omp_get_thread_num();
	nodeAtom<T> *pred;
	nodeAtom<T> *curr;
	int32_t key = key_calc<T>(item);
retry:
	while (true) {
		pred = head;
		head->hash_mem++;
		curr = getPointer(pred->next.load());
		int64_t hash = curr->hash_mem.load();
		if (hash == 0) { // Check if hash was deledet
			cerr << "Error with hash" << endl;
			cout << __LINE__ << endl;
			pred->hash_mem--;
			continue;
		}
		// increace hash for one
		if (atomic_compare_exchange_strong(&(getPointer(pred->next.load())->hash_mem), &hash, hash + 1) == false) {
			pred->hash_mem--;
			continue;
		}

		while (true) {

			assert(pred->next != NULL);
			hash = getPointer(curr->next.load())->hash_mem.load();
			nodeAtom<T> *succ = getPointer(curr->next.load());

			if (hash == 0) { // Check if hash was deleded
				cerr << "Error with hash" << endl;
				cout << __LINE__ << endl;
				continue;
			}
			if (atomic_compare_exchange_strong(&succ->hash_mem, &hash, hash + 1) == false) {
				continue;
			}

			// link out marked items
			while (getFlag(curr->next.load())) {
				// reset Flag at local pointer
				resetFlag(&curr);
				resetFlag(&succ);

				// link node out
				printf("%i: Error  Line: %i  \n", tid, __LINE__);
				nodeAtom<T> *old = curr; // atomic_compare_exchange overwrite  curr if it return false
				// if (atomic_compare_exchange_strong(&pred->next, &curr, succ) == false) {
				// 	pred->hash_mem--;
				// 	old->hash_mem--;
				// 	succ->hash_mem--;
				// 	printf("%i: Error  Line: %i  \n", tid, __LINE__);
				// 	goto retry;
				// }
				printf("%i: Normal  Line: %i key: %i \n", tid, __LINE__,getPointer(pred->next)->key);
				curr->hash_mem--;
				// if ((curr->hash_mem & COUTNMASK) == 0) {
				// 	delete (curr);
				// } else {
				// 	deleteQueue.push(curr);
				// }

				curr = succ;
				while (true) {
					hash = getPointer(succ->next.load())->hash_mem.load();
					succ = getPointer(succ->next.load());
					if (hash == 0) { // Check if hash was deleded
						succ = curr;
						continue;
					}
					if (atomic_compare_exchange_strong(&succ->hash_mem, &hash, hash + 1) == false) {
						printf("%i: Error  Line: %i  \n", tid, __LINE__);
						succ = curr;
						continue;
					}
					break;
				}
			}
			if (curr->key >= key) {
				Window_t<nodeAtom<T>> w{pred, curr};
				pred->hash_mem--;
				curr->hash_mem--;
				succ->hash_mem--;
				return w;
			}
			pred->hash_mem--;
			pred = curr;
			curr = succ;
		}
	}
}

template <class T> nodeAtom<T> *LockFree_mem<T>::getPointer(nodeAtom<T> *pointer) {
	uint64_t u64_ptr = (uint64_t)pointer;
	return (nodeAtom<T> *)(u64_ptr &= ~(MASK));
}

template <class T> void LockFree_mem<T>::setFlag(nodeAtom<T> **pointer) {
	// cout <<endl <<"Pointer: "<<*pointer <<" ";
	uint64_t u64_ptr = (uint64_t)*pointer;
	*pointer = (nodeAtom<T> *)(u64_ptr |= MASK);
	// cout <<*pointer <<" "<<u64_ptr << endl;
}

template <class T> void LockFree_mem<T>::resetFlag(nodeAtom<T> **pointer) {
	uint64_t u64_ptr = (uint64_t)*pointer;
	*pointer = (nodeAtom<T> *)(u64_ptr &= ~(MASK));
}

template <class T> void LockFree_mem<T>::resetFlag(atomic<nodeAtom<int> *> *pointer) {
	uint64_t u64_ptr = (uint64_t)pointer->load();
	pointer->store((nodeAtom<T> *)(u64_ptr &= ~(MASK)));
}

template <class T> bool LockFree_mem<T>::getFlag(nodeAtom<T> *pointer) {
	return (nodeAtom<int> *)((((uint64_t)pointer) >> FLAG_POS) & 1U);
}
template <class T> void LockFree_mem<T>::deleteNodes() {
	int tid = omp_get_thread_num();
	while (deleteQueue.empty() == false) {
		nodeAtom<T> *curr = deleteQueue.front();
		deleteQueue.pop();
		int i = 0;
		int j = 0;
		while ((curr->hash_mem & COUTNMASK) != 0) {
			i++;
			if (i > 100) {
				if (j > 30)
					//throw "wait to much often";
				cout << tid << " wait " << curr->key << " hash: " << (curr->hash_mem & COUTNMASK) << endl;
				i = 0;
				j++;
			}
		}
	}
	return;
}

template class LockFree_mem<int>;
// template class LockFree<float>;