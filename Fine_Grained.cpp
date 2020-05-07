#include <iostream>
using namespace std;
#include "Fine_Grained.h"
#include "key.h"
#include "node.h"
#include <assert.h>
#include <omp.h>
#include <stdint.h>

template <class T> FineList<T>::FineList() {
	head = new nodeFine<T>(0, INT32_MIN);
	head->next = new nodeFine<T>(0, INT32_MAX);
}

template <class T> FineList<T>::~FineList() {
	while (head != NULL) {
		nodeFine<T>* oldHead = head;
		head=head->next;
		delete oldHead;
	} 
}

template <class T> bool FineList<T>::add(T item) {
	nodeFine<T> *pred=NULL, *curr=NULL;
	try {
		int32_t key = key_calc<T>(item);
		// lock_guard<std::mutex> g(mtx);
		head->lock();
		pred = head;
		curr = pred->next;
		curr->lock();

		while (curr->key < key) {
			assert(curr->next != NULL);
			pred->unlock();
			pred = curr;
			curr = curr->next;
			curr->lock();
		}

		// Item already in the set
		if (key == curr->key) {
			pred->unlock();
			curr->unlock();
			return false;
		}

		// Add item to the set
		nodeFine<T> *n = new nodeFine<T>(item);
		n->next = curr;
		pred->next = n;
		pred->unlock();
		curr->unlock();
		return true;
	}

	// Exception handling
	catch (exception &e) {
		pred->unlock();
		curr->unlock();
		cerr << "Error during add the item: " << item << std::endl;
		cerr << "Standard exception: " << e.what() << endl;
		return false;
	} catch (...) {
		pred->unlock();
		curr->unlock();
		cerr << "Error during add the item: " << item << std::endl;
		return false;
	}
}

template <class T> bool FineList<T>::remove(T item) {
	nodeFine<T> *pred, *curr;
	int32_t key = key_calc<T>(item);
	head->lock();

	try {
		pred = head;
		curr = pred->next;
		curr->lock();
		while (curr->key < key) {
			assert(curr->next != NULL);
			pred->unlock();
			pred = curr;
			curr = curr->next;
			curr->lock();
		}
		if (key == curr->key) {
			pred->next = curr->next;
			pred->unlock();
			curr->unlock();
			delete curr;
			return true;
		} else {
			pred->unlock();
			curr->unlock();
			return false;
		}
	}
	// Exception handling
	catch (exception &e) {
		pred->unlock();
			curr->unlock();
		cerr << "Error during remove the item: " << item << std::endl;
		cerr << "Standard exception: " << e.what() << endl;
		return false;
	} catch (...) {
		pred->unlock();
			curr->unlock();
		cerr << "Error during remove the item: " << item << std::endl;
		return false;
	}
}

template <class T> bool FineList<T>::contains(T item) {
	nodeFine<T> *pred, *curr;
	int32_t key = key_calc<T>(item);
	head->lock();

	try {
		pred = head;
		curr = pred->next;
		curr->lock();
		while (curr->key < key) {
			assert(curr->next != NULL);
			pred->unlock();
			pred = curr;
			curr = curr->next;
			curr->lock();
		}
		if (key == curr->key) {
			pred->unlock();
			curr->unlock();
			return true;
		} else {
			pred->unlock();
			curr->unlock();
			return false;
		}
	}
	// Exception handling
	catch (exception &e) {
		pred->unlock();
			curr->unlock();
		cerr << "Error during remove the item: " << item << std::endl;
		cerr << "Standard exception: " << e.what() << endl;
		return false;
	} catch (...) {
		pred->unlock();
			curr->unlock();
		cerr << "Error during remove the item: " << item << std::endl;
		return false;
	}
}

template class FineList<int>;
// template class FineList<float>;