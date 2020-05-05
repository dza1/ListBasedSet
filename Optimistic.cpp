#include <iostream>
using namespace std;
#include "Optimistic.h"
#include "key.h"
#include "node.h"
#include <assert.h>
#include <omp.h>
#include <stdint.h>

template <class T> Optimistic<T>::Optimistic() {
	head = new nodeFine<T>(0, INT32_MIN);
	head->next = new nodeFine<T>(0, INT32_MAX);
}

template <class T> bool Optimistic<T>::add(T item) {
	Window_t<T> w;
	try {
		while (true) {
			w = find(item);
			int32_t key = key_calc<T>(item);

			// Item already in the set
			if (key == w.curr->key) {
				unlock(w);
				return false;
			}

			// Item not reachable
			if (head == w.curr) {
				//printf("Item %i not reachable\n", item);
				continue;
			}

			// Add item to the set
			nodeFine<T> *n = new nodeFine<T>(item);
			n->next = w.curr;
			w.pred->next = n;
			unlock(w);
			return true;
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

template <class T> bool Optimistic<T>::remove(T item) {
	Window_t<T> w;
	try {
		w = find(item);
		int32_t key = key_calc<T>(item);

		// Item not reachable, unlocked in Window_t class
		if (head == w.curr) {
			return false;
		}
		if (key == w.curr->key) {
			w.pred->next = w.curr->next;
			unlock(w);
			delete w.curr;
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

template <class T> bool Optimistic<T>::contains(T item) {
	Window_t<T> w;
	try {
		w = find(item);
		int32_t key = key_calc<T>(item);

		// Item not reachable, unlocked in Window_t class
		if (head == w.curr) {
			return false;
		}
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

template <class T> Window_t<T> Optimistic<T>::find(T item) {
	nodeFine<T> *pred, *curr;
	int32_t key = key_calc<T>(item);
	// lock_guard<std::mutex> g(mtx);
	pred = head;
	curr = pred->next;

	while (curr->key < key) {
		assert(curr->next != NULL);
		pred = curr;
		curr = curr->next;
	}

	Window_t<T> w{pred, curr};
	lock(w);
	if (validate(w) == true) {
		return w;
	} else { // not reachable
		unlock(w);
		w.pred = head;
		w.curr = head;
		return w;
	}

	return w;
}

template <class T> bool Optimistic<T>::validate(Window_t<T> w) {
	nodeFine<T> *n = head;
	while (n->key <= w.pred->key) {
		assert(n->next != NULL);
		if (n == w.pred) {
			if (n->next != w.curr) {
				//printf("%i to %i not Reachable\n", w.pred->key, w.curr->key);
			}
			return n->next == w.curr;
		}
		n = n->next;
	}

	return false;
}

template <class T> void Optimistic<T>::lock(Window_t<T> w) {
	w.pred->lock();
	w.curr->lock();
}

template <class T> void Optimistic<T>::unlock(Window_t<T> w) {
	w.pred->unlock();
	w.curr->unlock();
}

template class Optimistic<int>;
// template class Optimistic<float>;