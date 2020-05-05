#ifndef OPTIMISTIC_H__
#define OPTIMISTIC_H__
#include <iostream>
using namespace std;
#include "node.h"
#include <mutex> // std::mutex, std::lock_guard
#include <omp.h>
#include <stdint.h>

#include "setlist.h"

template <typename T>
struct Window_t{
	nodeFine<T> *pred;
	nodeFine<T> *curr;
};

template <typename T> class Optimistic : public SetList<T> { // The class
  private:
	T item;
	std::mutex mtx;
	Window_t<T> find(T item);
	void lock(Window_t<T> w);
	void unlock(Window_t<T> w);
	bool validate(Window_t<T> w);

  public:
	nodeFine<T> *head;
	Optimistic();
	bool add(T item);
	bool remove(T item);
	bool contains(T item);
};
#endif
