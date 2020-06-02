#include <iostream>
using namespace std;
#include "Optimistic_mem.hpp"
#include "key.hpp"
#include "node.hpp"
#include <assert.h>
#include <omp.h>
#include <stdint.h>
#include "benchmark.hpp"
#include <queue>

static thread_local queue<node_fine_del<int> *> deleteQueue;


template <class T> Optimistic_mem<T>::Optimistic_mem() {
	head = new nodeFine<T>(0, INT32_MIN);
	head->next = new nodeFine<T>(0, INT32_MAX);

	Tmax = omp_get_max_threads();
	snap = new std::atomic<uint32_t>[Tmax];
	for (size_t i = 0; i < Tmax; i++) {
		snap[i].store(0);
	}
}

template <class T> Optimistic_mem<T>::~Optimistic_mem() {
	while (head != NULL) {
		nodeFine<T> *oldHead = head;
		head = head->next;
		delete oldHead;
	}
	delete[] snap;
}

template <class T> bool Optimistic_mem<T>::add(T item,sub_benchMark_t *benchMark) {
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

template <class T> bool Optimistic_mem<T>::remove(T item, sub_benchMark_t *benchMark) {
	Window_t<nodeFine<T>> w;
	try {
		w = find(item,benchMark);
		int32_t key = key_calc<T>(item);

		if (key == w.curr->key) {
			w.pred->next = w.curr->next;
			unlock(w);
			//delete
			deleteQueue.push(new node_fine_del<T>(w.curr, this->snap, this->Tmax));
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

template <class T> bool Optimistic_mem<T>::contains(T item, sub_benchMark_t *benchMark) {
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

template <class T> Window_t<nodeFine<T>> Optimistic_mem<T>::find(T item, sub_benchMark_t *benchMark) {
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

template <class T> bool Optimistic_mem<T>::validate(Window_t<nodeFine<T>> w) {
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

template <class T> void Optimistic_mem<T>::lock(Window_t<nodeFine<T>> w) {
	w.pred->lock();
	w.curr->lock();
}

template <class T> void Optimistic_mem<T>::unlock(Window_t<nodeFine<T>> w) {
	w.pred->unlock();
	w.curr->unlock();
}


/**
 * @brief Function to empty and delete que of candidats for delete
 *
 * @details emptyQueue is always called before return from this object. It checks, if
 * each thread increased allready the timestamp, to be sure, that no one reads the
 * node during delete
 *
 * @param[in]  final  	if it is true, the function doesn't check if snap changed
 */
template <class T> void Optimistic_mem<T>::emptyQueue(bool final) {
	int tid = omp_get_thread_num();
	while (deleteQueue.empty() == false) {
		node_fine_del<T> *curr = deleteQueue.front();
		snap[tid]++;
		if (final == false) {
			for (size_t i = 0; i < this->Tmax; i++) {
				if (curr->snap[i] == this->snap[i].load()) {
					return;
				}
			}
		}
		deleteQueue.pop();
		delete (curr);
	}
	return;
}

/**
 * @brief Constructor for create a node with a canidate for delete
 *
 * @param[in]  pointer  	Pointer to the node which should be deleted
 * @param[in]  snap  		snapshot of all timestamp, when the node was put in the queue
 * @param[in]  Tmax  		Amount of maximal threads
 */
template <class T> node_fine_del<T>::node_fine_del(nodeFine<T> *pointer, std::atomic<uint32_t> *snap, size_t Tmax) {
	this->pointer = pointer;
	this->snap = new uint32_t[Tmax];
	for (size_t i = 0; i < Tmax; i++) {
		this->snap[i] = snap[i].load();
	}
}

/**
 * @brief Destructor for node with a canidate for delete
 *
 */
template <class T> node_fine_del<T>::~node_fine_del() {
	delete pointer;
	delete[] snap;
}

template class Optimistic_mem<int>;
// template class Optimistic_mem<float>;