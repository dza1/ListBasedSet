#include <iostream>
using namespace std;
#include "Optimistic_mem.h"
#include "key.h"
#include "node.h"
#include <assert.h>
#include <omp.h>
#include <queue>
#include <stdint.h>

#define COUTNMASK 0x00000000FFFFFF // mask for counter, which shows how many threads read this item
thread_local static queue<nodeFine_mem<int> *> deleteQueue;

template <class T> Optimistic_mem<T>::Optimistic_mem() {
	head = new nodeFine_mem<T>(0, INT32_MIN);
	head->next = new nodeFine_mem<T>(0, INT32_MAX);
}

template <class T> Optimistic_mem<T>::~Optimistic_mem() {
	while (head != NULL) {
		nodeFine_mem<T> *oldHead = head;
		head = head->next;
		delete oldHead;
	}
}

template <class T> bool Optimistic_mem<T>::add(T item,int *benchMark) {
	Window_at_t<nodeFine_mem<T>> w;
	try {
		w = find(item);
		int32_t key = key_calc<T>(item);

		// Item already in the set
		if (key == w.curr->key) {
			unlock(w);
			deleteNodes();
			return false;
		}

		// Add item to the set
		nodeFine_mem<T> *n = new nodeFine_mem<T>(item);
		n->next = w.curr;
		w.pred->next = n;
		unlock(w);
		deleteNodes();
		return true;
	}

	// Exception handling
	catch (exception &e) {
		unlock(w);
		cerr << "Error during add the item: " << item << std::endl;
		cerr << "Standard exception: " << e.what() << endl;
		return false;
	} catch (...) {
		unlock(w);
		cerr << "Error during add the item: " << item << std::endl;
		return false;
	}
}

template <class T> bool Optimistic_mem<T>::remove(T item, int *benchMark) {
	Window_at_t<nodeFine_mem<T>> w;
	try {
		w = find(item);
		int32_t key = key_calc<T>(item);

		if (key == w.curr->key) {
			w.pred->next = w.curr->next;

			unlock(w);

			if ((w.curr->hash_mem & COUTNMASK) == 0) {
				w.curr->lock();
				w.curr->hash_mem = 0;
				w.curr->next = nullptr;
				delete w.curr;
			} else {
				deleteQueue.push(w.curr);
			}

			deleteNodes();
			return true;
		} else {
			unlock(w);
			return false;
		}
	}
	// Exception handling
	catch (exception &e) {
		unlock(w);
		cerr << "Error during add the item: " << item << std::endl;
		cerr << "Standard exception: " << e.what() << endl;
		return false;
	} catch (...) {
		unlock(w);
		cerr << "Error during add the item: " << item << std::endl;
		return false;
	}
}

template <class T> bool Optimistic_mem<T>::contains(T item, int *benchMark) {
	Window_at_t<nodeFine_mem<T>> w;
	try {
		w = find(item);
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
		cerr << "Error during add the item: " << item << std::endl;
		cerr << "Standard exception: " << e.what() << endl;
		return false;
	} catch (...) {
		unlock(w);
		cerr << "Error during add the item: " << item << std::endl;
		return false;
	}
}

template <class T> Window_at_t<nodeFine_mem<T>> Optimistic_mem<T>::find(T item) {
	nodeFine_mem<T> *pred, *curr;
	int32_t key = key_calc<T>(item);
	// lock_guard<std::mutex> g(mtx);

	while (true) {
		pred = head;
		head->hash_mem++;

		curr = pred->next;
		int64_t hash = curr->hash_mem.load();

		if (hash == 0) { // Check if hash was deledet
			cerr << "Error with hash" << endl;
			cout << __LINE__ << endl;
			pred->hash_mem--;
			continue;
		}
		if (atomic_compare_exchange_strong(&pred->next->hash_mem, &hash, hash + 1) == false) {
			pred->hash_mem--;
			continue;
		}
		while (curr->key < key) {

			nodeFine_mem<T> *old = pred;

			pred = curr;
			hash = curr->next->hash_mem.load();
			curr = curr->next;
			if (hash == 0) { // Check if hash was deleded, shouldn't happen
				cerr << "Error with hash" << endl;
				cout << __LINE__ << endl;
				curr = pred;
				pred = old;
				continue;
			}
			if (atomic_compare_exchange_strong(&curr->hash_mem, &hash, hash + 1) == false) {
				// cout<<tid<<":"<<__LINE__<<endl;
				curr = pred;
				pred = old;
				continue;
			}
			assert(curr != NULL);
			// printf("%i: Ittem %i add hash: %i Line: %i, addr: %p  \n", tid,
			// (int)curr->key,(int)hash+1,__LINE__,(void*)curr);
			old->hash_mem--;
			// printf("%i: Ittem %i rem hash: %i Line: %i, addr: %p\n", tid,
			// old->key,(int)old->hash_mem,__LINE__,(void*)old);
		}

		Window_at_t<nodeFine_mem<T>> w{pred, curr};
		lock(w);
		pred->hash_mem--;
		curr->hash_mem--;
		// printf("%i: Ittem %i rem hash: %i Line: %i, addr: %p \n", tid,
		// pred->key,(int)pred->hash_mem,__LINE__,(void*)pred); printf("%i: Ittem %i rem hash: %i Line: %i, addr: %p
		// \n", tid, curr->key,(int)curr->hash_mem,__LINE__,(void*)curr);
		if (validate(w) == true) {
			deleteNodes();
			return w;
		} else { // not reachable
			deleteNodes();
			unlock(w);
		}
	}
}

template <class T> bool Optimistic_mem<T>::validate(Window_at_t<nodeFine_mem<T>> w) {
	nodeFine_mem<T> *n = head;
	n->hash_mem++;
	int64_t hash;

	nodeFine_mem<T> *curr;

	while (n->key <= w.pred->key) {

		hash = n->next->hash_mem;
		curr = n->next;
		if (hash == 0) { // Check if hash was deledet, shouldn't happen
			cerr << "Error with hash" << endl;
			cout << __LINE__ << endl;
			continue;
		}
		if (atomic_compare_exchange_weak(&curr->hash_mem, &hash, hash + 1) == false) {
			// cout << __LINE__ << endl;
			continue;
		}
		assert(curr != NULL);
		////printf("%i: Ittem %i add hash: %i Line: %i \n", tid, curr->key, ((int)hash + 1));

		if (n == w.pred) {
			if (curr != w.curr) {
				// //printf("%i to %i not reachable\n", w.pred->key, w.curr->key);
			}
			n->hash_mem--;
			curr->hash_mem--;
			return curr == w.curr;
		}

		n->hash_mem--;
		////printf("%i: Ittem %i rem hash: %i Line: %i \n", tid, curr->key, ((int)n->hash_mem));
		n = curr;
	}
	//cout << "not found" << endl;
	curr->hash_mem--;
	return false;
}

template <class T> void Optimistic_mem<T>::lock(Window_at_t<nodeFine_mem<T>> w) {
	w.pred->lock();
	w.curr->lock();
}

template <class T> void Optimistic_mem<T>::unlock(Window_at_t<nodeFine_mem<T>> w) {
	w.pred->unlock();
	w.curr->unlock();
}

template <class T> void Optimistic_mem<T>::deleteNodes() {
	size_t size = deleteQueue.size();
	for (size_t i = 0; i < size; i++) {
		nodeFine_mem<T> *curr = deleteQueue.front();
		deleteQueue.pop();
		if ((curr->hash_mem & COUTNMASK) == 0) {
			delete (curr);
		} else {
			deleteQueue.push(curr); // Put again on the end, if it was not possible to delete
		}
	}
	return;
}

template class Optimistic_mem<int>;
// template class Optimistic_mem<float>;