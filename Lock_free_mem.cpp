#include <iostream>
using namespace std;
#include "Lock_free_mem.h"
#include "key.h"
#include "node.h"
#include <assert.h>
#include <omp.h>
#include <queue>
#include <stdint.h>

#define COUTNMASK 0x00000000FFFFFF // mask for counter, which shows how many threads read this item

thread_local static queue<nodeAtom<int> *> deleteQueue;

template <class T> LockFree_mem<T>::LockFree_mem() {
	nodeAtom<T> *tmp;
	head = new nodeAtom<T>(0, INT32_MIN);
	tmp = head->next = new nodeAtom<T>(0, INT32_MAX);
	tmp->next = new nodeAtom<T>(0, INT32_MAX);
}

template <class T> LockFree_mem<T>::~LockFree_mem() {
	while (head != NULL) {
		nodeAtom<T> *oldHead = head;
		head = getPointer(head->next);
		delete oldHead;
	}
}

template <class T> bool LockFree_mem<T>::add(T item) {
	Window_at_t<nodeAtom<T>> w;
	int32_t key = key_calc<T>(item);
	try {
		while (true) {
			w = find(item);
			nodeAtom<T> *pred = w.pred;
			nodeAtom<T> *curr = w.curr;

			// Item already in the set
			if (curr->key == key) {
				deleteNodes();
				return false;
			}

			// Add item to the set
			nodeAtom<T> *n = new nodeAtom<T>(item);
			n->next.store(w.curr);
			// n->next.store(resetFlag(&n->next.load()));
			resetFlag(&n->next);

			if (atomic_compare_exchange_weak(&pred->next, &curr, n) == true) {
				deleteNodes();
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

template <class T> bool LockFree_mem<T>::remove(T item) {
	Window_at_t<nodeAtom<T>> w;
	try {
		while (true) {

			w = find(item);
			int32_t key = key_calc<T>(item);

			if (key == w.curr->key) {
				nodeAtom<T> *succ = w.curr->next.load();
				nodeAtom<T> *markedsucc = succ;

				setFlag(&markedsucc);
				resetFlag(&succ);
				// delete w.curr;

				// mark as deleted
				if (atomic_compare_exchange_weak(&w.curr->next, &succ, markedsucc) == false) {
					continue;
				}

				// try to unlink
				if (atomic_compare_exchange_weak(&w.pred->next, &w.curr, succ) == true) {
					if ((w.curr->hash_mem & COUTNMASK) == 0) {
						delete (w.curr);
					} else {
						deleteQueue.push(w.curr);
					}
				}

deleteNodes();
				return true;
			} else {
deleteNodes();
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

template <class T> bool LockFree_mem<T>::contains(T item) {
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
		if (atomic_compare_exchange_weak(&curr->hash_mem, &hash, hash + 1) == false) {
			continue;
		}
		n->hash_mem--;
		n = curr;
	}
	bool result = n->key == key && !getFlag(n->next.load());
	n->hash_mem--;
	return result;
}

template <class T> Window_at_t<nodeAtom<T>> LockFree_mem<T>::find(T item) {
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
			// printf("%i: Error  Line: %i  \n", tid, __LINE__);
			continue;
		}
		// increace hash for one
		if (atomic_compare_exchange_strong(&(getPointer(pred->next.load())->hash_mem), &hash, hash + 1) == false) {
			pred->hash_mem--;
			// printf("%i: Error  Line: %i  \n", tid, __LINE__);
			continue;
		}

		while (true) {

			assert(pred->next != NULL);
			nodeAtom<T> *succ;
			try {
				hash = getPointer(curr->next.load())->hash_mem.load();
				succ= getPointer(curr->next.load());
				// printf("%i: normal keypred: %i key curr %i key next: %i  Line: %i \n", tid,pred->key,curr->key,succ->key,
				// __LINE__);
				// cout << "Atomic " << curr->next << " " << curr->next.load() << endl;

				if (hash == 0) { // Check if hash was deleded
					cerr << "Error with hash" << endl;
					cout << __LINE__ << endl;
					// printf("%i: Error  Line: %i  \n", tid, __LINE__);
					continue;
				}
				if (atomic_compare_exchange_strong(&succ->hash_mem, &hash, hash + 1) == false) {
					// printf("%i: Error  Line: %i  \n", tid, __LINE__);
					continue;
			}
			}
			catch(...){
				cerr<<"Exeption"<<endl;
				goto retry;
			}

			// link out marked items
			while (getFlag(curr->next.load())) {
				// reset Flag at local pointer
				resetFlag(&curr);
				resetFlag(&succ);

				// link node out
				nodeAtom<T> *old = curr; // atomic_compare_exchange overwrite  curr, no idea why
				// printf("curr 1: %p next: %p\n", curr, &curr);
				if (atomic_compare_exchange_strong(&pred->next, &curr, succ) == false) {
					// printf("curr 2: %p next: %p\n", curr, &curr);
					// printf("%i: Error keypred: %i key curr %i key next: %i  Line: %i \n", tid, pred->key,
					// curr->key,succ->key, __LINE__);
					// printf("%i: Error pred: %p curr %p  next: %p  Line:
					// %i \n",tid,(void*)pred,(void*)curr,(void*)succ, __LINE__);
					pred->hash_mem--;
					old->hash_mem--;
					succ->hash_mem--;
					goto retry;
				}
				curr->hash_mem--;

				deleteQueue.push(curr);
				
				curr = succ;
				while (true) {
					hash = getPointer(succ->next.load())->hash_mem.load();
					succ = getPointer(succ->next.load());
					if (hash == 0) { // Check if hash was deleded
						printf("%i: Error  Line: %i  \n", tid, __LINE__);
						cerr << "Error with hash" << endl;
						cout << __LINE__ << endl;
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
				Window_at_t<nodeAtom<T>> w{pred, curr};
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

template <class T> void LockFree_mem<T>::resetFlag(atomic<nodeAtom<T> *> *pointer) {
	uint64_t u64_ptr = (uint64_t)pointer->load();
	pointer->store((nodeAtom<T> *)(u64_ptr &= ~(MASK)));
}

template <class T> bool LockFree_mem<T>::getFlag(nodeAtom<T> *pointer) {
	return (nodeAtom<int> *)((((uint64_t)pointer) >> FLAG_POS) & 1U);
}

// template <class T> void LockFree_mem<T>::deleteNodes() {
// 	size_t size = deleteQueue.size();
// 	for (size_t i = 0; i < size; i++) {
// 		nodeAtom<T> *curr = deleteQueue.front();
// 		deleteQueue.pop();
// 		if ((curr->hash_mem & COUTNMASK) == 0) {
// 			curr->hash_mem=0;
// 			delete (curr);
// 		} else {
// 			deleteQueue.push(curr); // Put again on the end, if it was not possible to delete
// 		}
// 	}
// 	return;
// }

template <class T> void LockFree_mem<T>::deleteNodes() {
	while (deleteQueue.empty() == false) {
		nodeAtom<T> *curr = deleteQueue.front();
		deleteQueue.pop();

		while ((curr->hash_mem & COUTNMASK) != 0) {
		}
		delete(curr);
	}
	return;
}
template class LockFree_mem<int>;
// template class LockFree<float>;