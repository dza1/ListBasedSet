#include <iostream>
using namespace std;
#include "Lock_free.hpp"
#include "key.hpp"
#include "node.hpp"
#include <assert.h>
#include <omp.h>
#include <stdint.h>
#include "benchmark.hpp"



template <class T> LockFree<T>::LockFree() {
	head = new nodeAtom<T>(0, INT32_MIN);
	head->next = new nodeAtom<T>(0, INT32_MAX);
}

template <class T> LockFree<T>::~LockFree() {
	while (head != NULL) {
		nodeAtom<T>* oldHead = head;
		head = getPointer(head->next);
		delete oldHead;
	} 
}

template <class T> bool LockFree<T>::add(T item,sub_benchMark_t *benchMark) {
	Window_t<nodeAtom<T>> w;
	int32_t key = key_calc<T>(item);
	try {
		while (true) {
			w = find(item,benchMark);
			nodeAtom<T> *pred = w.pred;
			nodeAtom<T> *curr = w.curr;

			// Item already in the set
			if (curr->key == key) {
				return false;
			}
			assert(pred->key<key && curr->key>key);
	
			// Add item to the set
			nodeAtom<T> *n = new nodeAtom<T>(item);
			n->next.store(w.curr);
			// n->next.store(resetFlag(&n->next.load()));
			resetFlag(&n->next);

			if (atomic_compare_exchange_weak(&pred->next, &curr, n) == true) {
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

template <class T> bool LockFree<T>::remove(T item, sub_benchMark_t *benchMark) {
	Window_t<nodeAtom<T>> w;
	try {
		while (true) {

			w = find(item,benchMark);
			int32_t key = key_calc<T>(item);

			if (key == w.curr->key) {
				nodeAtom<T> *succ = w.curr->next.load();
				nodeAtom<T> *markedsucc = succ;

				setFlag(&markedsucc);
				resetFlag(&succ);
				// delete w.curr;

				// mark as deleted
				if (atomic_compare_exchange_weak(&w.curr->next, &succ, markedsucc) == false) {
					benchMark->goToStart+=1;
					continue;
				}

				// try to unlink
				atomic_compare_exchange_weak(&w.pred->next, &w.curr, succ);

				return true;
			} else {

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

template <class T> bool LockFree<T>::contains(T item, sub_benchMark_t *benchMark) {
	nodeAtom<T> *n = head;

	int32_t key = key_calc<T>(item);
	while (n->key < key) {
		n = getPointer(n->next.load());
	}
	return n->key == key && !getFlag(n->next.load());
}



template <class T> Window_t<nodeAtom<T>> LockFree<T>::find(T item, sub_benchMark_t *benchMark) {

	nodeAtom<T> *pred, *curr;
	int32_t key = key_calc<T>(item);
retry:
	while (true) {
		pred = head;
		curr = getPointer(pred->next.load());

		while (true) {
			assert(pred->next != NULL);
			nodeAtom<T> *succ = getPointer(curr->next.load());

			// link out marked items
			while (getFlag(curr->next.load())) {
				resetFlag(&curr);
				resetFlag(&succ);

				//not possible to link out
				if (atomic_compare_exchange_weak(&pred->next, &curr, succ) == false) {
					benchMark->goToStart+=1;
					//cout<<"Error"<<endl;
					goto retry;
				}
				curr = succ;
				succ = getPointer(succ->next.load());
			}
			if (curr->key >= key) {
				Window_t<nodeAtom<T>> w{pred, curr};
				return w;
			}
			pred = curr;
			curr = succ;
		}
	}
}

template <class T> nodeAtom<T> *LockFree<T>::getPointer(nodeAtom<T> *pointer) {
	uint64_t u64_ptr = (uint64_t)pointer;
	return (nodeAtom<T> *)(u64_ptr &= ~(MASK));
}

template <class T> void LockFree<T>::setFlag(nodeAtom<T> **pointer) {
	// cout <<endl <<"Pointer: "<<*pointer <<" ";
	uint64_t u64_ptr = (uint64_t)*pointer;
	*pointer = (nodeAtom<T> *)(u64_ptr |= MASK);
	// cout <<*pointer <<" "<<u64_ptr << endl;
}

template <class T> void LockFree<T>::resetFlag(nodeAtom<T> **pointer) {
	uint64_t u64_ptr = (uint64_t)*pointer;
	*pointer = (nodeAtom<T> *)(u64_ptr &= ~(MASK));
}

template <class T> void LockFree<T>::resetFlag(atomic<nodeAtom<T> *> *pointer) {
	uint64_t u64_ptr = (uint64_t)pointer->load();
	pointer->store((nodeAtom<T> *)(u64_ptr &= ~(MASK)));
}

template <class T> bool LockFree<T>::getFlag(nodeAtom<T> *pointer) {
	return (nodeAtom<int> *)((((uint64_t)pointer) >> FLAG_POS) & 1U);
}

template class LockFree<int>;
// template class LockFree<float>;