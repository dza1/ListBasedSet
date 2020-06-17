/** @file Optimistic_mem.hpp
 * @author Daniel Zainzinger
 * @date 2.6.2020
 *
 * @brief Optimistic list based set, where the datastructure search a element without locking.
 * If the node is found, the current and the previous node get locked and than the reachability of the two nodes from the head gets checked  
 * With memory management  
 */
#ifndef OPTIMISTIC_MEM_H__
#define OPTIMISTIC_MEM_H__
#include <iostream>
using namespace std;
#include "node.hpp"
#include <mutex> // std::mutex, std::lock_guard
#include <omp.h>
#include <stdint.h>
#include <queue>
#include "Window.hpp"
#include "setlist.hpp"



template <typename T> class Optimistic_mem : public SetList<T> { // The class
  private:
	T item;
	std::mutex mtx;
	Window_t<nodeFine<T>> find(T item, sub_benchMark_t *benchMark);
	std::atomic<uint32_t> *snap;
	size_t Tmax;

	void lock(Window_t<nodeFine<T>> w);
	void unlock(Window_t<nodeFine<T>> w);
	bool validate(Window_t<nodeFine<T>> w);

  public:
	nodeFine<T> *head;
	Optimistic_mem(size_t Tmax);
	~Optimistic_mem();
	bool add(T item, sub_benchMark_t *benchMark);
	bool remove(T item, sub_benchMark_t *benchMark);
	bool contains(T item, sub_benchMark_t *benchMark);
	void emptyQueue(bool final); // not used
};



#endif
