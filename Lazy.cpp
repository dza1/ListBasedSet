/** @file Lazy.cpp
 * @author Daniel Zainzinger and Noah Bruns
 * @date 2.6.2020
 *
 * @brief TODO 
 */
#include <iostream>
using namespace std;
#include "Lazy.hpp"
#include "key.hpp"
#include "node.hpp"
#include <assert.h>
#include <omp.h>
#include <stdint.h>
#include "benchmark.hpp"

/**
 * @brief Constructor for the datastructure
 */
template <class T> Lazy<T>::Lazy() {
	head = new nodeLazy<T>(0, INT32_MIN);
	head->next = new nodeLazy<T>(0, INT32_MAX);
}

/**
 * @brief Destructor for the datastructure
 */
template <class T> Lazy<T>::~Lazy() {
	while (head != NULL) {
		nodeLazy<T> *oldHead = head;
		head = head->next;
		delete oldHead;
	}
}

/**
 * @brief Function which adds one item from the datastructure
 *
 * @param[in]  	item  		item, which should be add to the datastructure
 * @param[out]  benchMark  	a struct, which stores information for benchmarking
 * @return true, if it was succeccfully added, false otherwise
 */
template <class T> bool Lazy<T>::add(T item,sub_benchMark_t *benchMark) {
	nodeLazy<T> *pred, *curr;
	Window_t<nodeLazy<T>> w;
	try {
		int32_t key = key_calc<T>(item);
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
		nodeLazy<T> *n = new nodeLazy<T>(item);
		n->next = curr;
		pred->next = n;
		pred->unlock();
		curr->unlock();
		return true;
	}

	// Exception handling
	catch (exception &e) {
		unlock(w);
		cerr << "Error during add: " << item << std::endl;
		cerr << "Standard exception: " << e.what() << endl;
		return false;
	} catch (...) {
		unlock(w);
		cerr << "Error during add: " << item << std::endl;
		return false;
	}
}

/**
 * @brief Function which removes one item from the datastructure
 *
 * @param[in]  	item  		item, which should be removed from the datastructure
 * @param[out]  benchMark  	a struct, which stores information for benchmarking
 * @return true, if it was succeccfully removed, false otherwise
 */
template <class T> bool Lazy<T>::remove(T item, sub_benchMark_t *benchMark) {
	Window_t<nodeLazy<T>> w;
	try {
		w = find(item,benchMark);
		int32_t key = key_calc<T>(item);

		if (key != w.curr->key) {
			unlock(w);
			return false;
		}

		w.curr->marked = true;
		w.pred->next = w.curr->next;
		unlock(w);

		return true;
	}
	// Exception handling
	catch (exception &e) {
		unlock(w);
		cerr << "Error during add: " << item << std::endl;
		cerr << "Standard exception: " << e.what() << endl;
		return false;
	} catch (...) {
		unlock(w);
		cerr << "Error during add: " << item << std::endl;
		return false;
	}
}

/**
 * @brief Function which checks if the item is in the datastructure 
 *
 * @param[in]  	item  		item for check if it is included
 * @param[out]  benchMark  	a struct, which stores information for benchmarking
 * @return true, if item is in the datastructure, false otherwise 
 */
template <class T> bool Lazy<T>::contains(T item, sub_benchMark_t *benchMark) {
	int32_t key = key_calc(item);

	try {
		nodeLazy<T> *n = head;

		while(n->key < key) {n = n->next;}

		return n->key == key && !n->marked;
	}
	// Exception handling
	catch (exception &e) {
		cerr << "Error during add: " << item << std::endl;
		cerr << "Standard exception: " << e.what() << endl;
		return false;
	} catch (...) {
		cerr << "Error during add: " << item << std::endl;
		return false;
	}
}

/**
 * @brief Function which returns a window, where the key is of the first element is smaller than the key of the item,
 * and the second key bigger or equal. 
 *
 * @param[in]  	item  		item, from which the key is calculated to search the window 
 * @param[out]  benchMark  	a struct, which stores information for benchmarking
 */
template <class T> Window_t<nodeLazy<T>> Lazy<T>::find(T item, sub_benchMark_t *benchMark) {
	nodeLazy<T> *pred, *curr;
	std::chrono::_V2::system_clock::time_point resetTime;
	bool reset=false;
	int32_t key = key_calc(item);
	
	while (true) {
		pred = head;
		curr = pred->next;

		while (curr->key < key)
		{
			pred = curr;
			curr = curr->next;
		}
		if(reset==true){
			auto finishTime = chrono::high_resolution_clock::now();
			chrono::duration<double> elapsed = finishTime - resetTime;
			uint32_t mus = chrono::duration_cast<chrono::microseconds>(elapsed).count();
			benchMark->lostTime+=mus;
			reset=false;
		}

		Window_t<nodeLazy<T>> w{pred, curr};
		lock(w);
		assert(w.pred->key <= key);
		assert(w.curr->key >= key);

		if (validate(w) == true) {
			return w;
		} else { // not reachable
			unlock(w);
			benchMark->goToStart+=1;
			resetTime = chrono::high_resolution_clock::now();
			reset=true;
		}
	}
}

/**
 * @brief Function which checks if the window w is still reachable from the head
 *
 * @param[in]  	w	  		window, which includes two nodes
 * @return true, if both nodes are still reachable from the head, otherwise false
 */
template <class T> bool Lazy<T>::validate(Window_t<nodeLazy<T>> w) {
	return !w.pred->marked && !w.curr->marked && w.pred->next == w.curr;
}

/**
 * @brief Function locks the window
 *
 * @param[in]  	w  		window, which includes two nodes
 */
template <class T> void Lazy<T>::lock(Window_t<nodeLazy<T>> w) {
	w.pred->lock();
	w.curr->lock();
}

/**
 * @brief Function unlock the window
 *
 * @param[in]  	w  		window, which includes two nodes
 */
template <class T> void Lazy<T>::unlock(Window_t<nodeLazy<T>> w) {
	w.pred->unlock();
	w.curr->unlock();
}

template class Lazy<int>;
