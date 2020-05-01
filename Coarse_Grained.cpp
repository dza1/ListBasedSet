#include <iostream>
using namespace std;
#include "Coarse_Grained.h"
#include "node.h"
#include <mutex> // std::mutex, std::lock_guard
#include <omp.h>
#include <stdint.h>

template <class T>
CoarseList<T>::CoarseList() {
	head = new node(0);
	head->next = new node(UINT32_MAX);
}

template <class T>
bool CoarseList<T>::add(T item) {
	try {
		node *pred, *curr;
		// lock_guard<std::mutex> g(mtx);
		mtx.lock();
		pred = head;
		curr = pred->next;
		uint32_t key = (uint32_t)item; // Need to be changed to hash

		while (curr->key < key) {
			pred = curr;
			curr = curr->next;
		}

		// Item already in the set
		if (key == curr->key) {
			mtx.unlock();
			return false;
		}

		// Add item to the set
		node *n = new node(item);
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

template class CoarseList<int>;
template class CoarseList<float>;