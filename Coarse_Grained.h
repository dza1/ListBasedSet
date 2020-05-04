#ifndef COARSE_GRAINED_H__
#define COARSE_GRAINED_H__
#include <iostream>
using namespace std;
#include "node.h"
#include <mutex> // std::mutex, std::lock_guard
#include <omp.h>
#include <stdint.h>

template <typename T> class CoarseList { // The class
  private:
	T item;
	std::mutex mtx;

  public:
	node<T> *head;
	CoarseList();
	bool add(T item);
	bool remove(T item);
	bool contains(T item);
};
#endif