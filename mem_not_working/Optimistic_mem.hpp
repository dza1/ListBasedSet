#ifndef OPTIMISTIC_mem_H__
#define OPTIMISTIC_mem_H__
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
	Window_t<nodeFine_mem<T>> find(T item);
	void lock(Window_t<nodeFine_mem<T>> w);
	void unlock(Window_t<nodeFine_mem<T>> w);
	bool validate(Window_t<nodeFine_mem<T>> w);
	void deleteNodes();

  public:
	nodeFine_mem<T> *head;
	Optimistic_mem();
	~Optimistic_mem();
	bool add(T item,sub_benchMark_t *benchMark);
	bool remove(T item,sub_benchMark_t *benchMark);
	bool contains(T item,sub_benchMark_t *benchMark);
};
#endif
