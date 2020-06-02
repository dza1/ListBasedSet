#ifndef OPTIMISTIC_MEM_H__
#define OPTIMISTIC_MEM_H__
#include <iostream>
using namespace std;
#include "node.hpp"
#include <mutex> // std::mutex, std::lock_guard
#include <omp.h>
#include <stdint.h>

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
	Optimistic_mem();
	~Optimistic_mem();
	bool add(T item, sub_benchMark_t *benchMark);
	bool remove(T item, sub_benchMark_t *benchMark);
	bool contains(T item, sub_benchMark_t *benchMark);
	void emptyQueue(bool final); // not used
};

template <typename T> class node_fine_del { // The class
  private:
	nodeFine<T> *pointer;

  public:
	node_fine_del(nodeFine<T> *pointer, std::atomic<uint32_t> *snap, size_t T_max);
	~node_fine_del();
	uint32_t *snap;
};

#endif
