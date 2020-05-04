#ifndef FINE_GRAINED_H__
#define FINE_GRAINED_H__
#include <iostream>
using namespace std;
#include "node.h"
#include <mutex> // std::mutex, std::lock_guard
#include <omp.h>
#include <stdint.h>

#include "setlist.h"

template <typename T> class FineList : public SetList<T> { // The class
  private:
	T item;
	std::mutex mtx;

  public:
	nodeFine<T> *head;
	FineList();
	bool add(T item);
	bool remove(T item);
	bool contains(T item);
};
#endif