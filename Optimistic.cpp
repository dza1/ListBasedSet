#include <iostream>
using namespace std;
#include "Optimistic.hpp"
#include "key.hpp"
#include "node.hpp"
#include <assert.h>
#include <omp.h>
#include <stdint.h>
#include "benchmark.hpp"

template <class T> Optimistic<T>::Optimistic() {
	head = new nodeFine<T>(0, INT32_MIN);
	head->next = new nodeFine<T>(0, INT32_MAX);
}

template <class T> Optimistic<T>::~Optimistic() {
	while (head != NULL) {
		nodeFine<T> *oldHead = head;
		head = head->next;
		delete oldHead;
	}
}

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
		if (w.pred->key >= n->key) {
			printf("Error");
		}
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
		cerr << "Error during add the item: " << item << std::endl;
		cerr << "Standard exception: " << e.what() << endl;
		return false;
	} catch (...) {
		unlock(w);
		cerr << "Error during add the item: " << item << std::endl;
		return false;
	}
}

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
		cerr << "Error during add the item: " << item << std::endl;
		cerr << "Standard exception: " << e.what() << endl;
		return false;
	} catch (...) {
		unlock(w);
		cerr << "Error during add the item: " << item << std::endl;
		return false;
	}
}

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
		cerr << "Error during add the item: " << item << std::endl;
		cerr << "Standard exception: " << e.what() << endl;
		return false;
	} catch (...) {
		unlock(w);
		cerr << "Error during add the item: " << item << std::endl;
		return false;
	}
}

template <class T> Window_t<nodeFine<T>> Optimistic<T>::find(T item, sub_benchMark_t *benchMark) {
	nodeFine<T> *pred, *curr;
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

template <class T> void Optimistic<T>::lock(Window_t<nodeFine<T>> w) {
	w.pred->lock();
	w.curr->lock();
}

template <class T> void Optimistic<T>::unlock(Window_t<nodeFine<T>> w) {
	w.pred->unlock();
	w.curr->unlock();
}

template class Optimistic<int>;
// template class Optimistic<float>;