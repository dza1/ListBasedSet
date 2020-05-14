#ifndef OPTIMISTIC_mem_H__
#define OPTIMISTIC_mem_H__
#include <iostream>
using namespace std;
#include "node.h"
#include <mutex> // std::mutex, std::lock_guard
#include <omp.h>
#include <stdint.h>

#include "setlist.h"
#include "Window.h"

template <typename T> class Optimistic_mem : public SetList<T> { // The class
  private:
	T item;
	std::mutex mtx;
	Window_at_t<nodeFine_mem<T>> find(T item);
	void lock(Window_at_t<nodeFine_mem<T>> w);
	void unlock(Window_at_t<nodeFine_mem<T>> w);
	bool validate(Window_at_t<nodeFine_mem<T>> w);

  public:
	nodeFine_mem<T> *head;
	Optimistic_mem();
	~Optimistic_mem();
	bool add(T item);
	bool remove(T item);
	bool contains(T item);
};
#endif
