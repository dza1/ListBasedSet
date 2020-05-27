#include <iostream>
using namespace std;
#include "Coarse_Grained.h"
#include "key.h"
#include "node.h"
#include <assert.h>
#include <mutex> // std::mutex, std::lock_guard
#include <omp.h>
#include <stdint.h>

template <class T> CoarseList<T>::CoarseList() {
	head = new node<T>(0, INT32_MIN);
	head->next = new node<T>(0, INT32_MAX);
}

template <class T> CoarseList<T>::~CoarseList() {
	while (head != NULL) {
		node<T>* oldHead = head;
		head=head->next;
		delete oldHead;
	} 
}

template <class T> bool CoarseList<T>::add(T item,int *benchMark) {
	try {
		node<T> *pred, *curr;
		// lock_guard<std::mutex> g(mtx);
		mtx.lock();
		pred = head;
		curr = pred->next;

		int32_t key = key_calc<T>(item);

		while (curr->key < key) {
			assert(curr->next != NULL);
			pred = curr;
			curr = curr->next;
		}

		// Item already in the set
		if (key == curr->key) {
			mtx.unlock();
			return false;
		}

		// Add item to the set
		node<T> *n = new node<T>(item);
		n->next = curr;
		pred->next = n;
		mtx.unlock();
		return true;
	}

	// Exception handling
	catch (exception &e) {
		mtx.unlock();
		cerr << "Error during add the item: " << item << std::endl;
		cerr << "Standard exception: " << e.what() << endl;
		return false;
	} catch (...) {
		mtx.unlock();
		cerr << "Error during add the item: " << item << std::endl;
		return false;
	}
}

template <class T> bool CoarseList<T>::remove(T item, int *benchMark) {
	node<T> *pred, *curr;
	int32_t key = key_calc<T>(item);
	mtx.lock();

	try {
		pred = head;
		curr = pred->next;
		while (curr->key < key) {
			assert(curr->next != NULL);
			pred = curr;
			curr = curr->next;
		}
		if (key == curr->key) {
			pred->next = curr->next;
			delete curr;
			mtx.unlock();
			return true;
		} else {
			mtx.unlock();
			return false;
		}
	}
	// Exception handling
	catch (exception &e) {
		mtx.unlock();
		cerr << "Error during remove the item: " << item << std::endl;
		cerr << "Standard exception: " << e.what() << endl;
		return false;
	} catch (...) {
		mtx.unlock();
		cerr << "Error during remove the item: " << item << std::endl;
		return false;
	}
}

template <class T> bool CoarseList<T>::contains(T item, int *benchMark) {
	node<T> *pred, *curr;
	int32_t key = key_calc<T>(item);
	mtx.lock();

	try {
		pred = head;
		curr = pred->next;
		while (curr->key < key) {
			assert(curr->next != NULL);
			pred = curr;
			curr = curr->next;
		}
		if (key == curr->key) {
			mtx.unlock();
			return true;
		} else {
			mtx.unlock();
			return false;
		}
	}
	// Exception handling
	catch (exception &e) {
		mtx.unlock();
		cerr << "Error during remove the item: " << item << std::endl;
		cerr << "Standard exception: " << e.what() << endl;
		return false;
	} catch (...) {
		mtx.unlock();
		cerr << "Error during remove the item: " << item << std::endl;
		return false;
	}
}

template class CoarseList<int>;
// template class CoarseList<float>;