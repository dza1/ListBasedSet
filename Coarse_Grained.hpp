#ifndef COARSE_GRAINED_H__
#define COARSE_GRAINED_H__
#include <iostream>
using namespace std;
#include "node.hpp"
#include <mutex> // std::mutex, std::lock_guard
#include <omp.h>
#include <stdint.h>

#include "setlist.hpp"

template <typename T> class CoarseList : public SetList<T> { // The class
  private:
	T item;
	std::mutex mtx;

  public:
	node<T> *head;
	CoarseList();
	~CoarseList();

	bool add(T item,sub_benchMark_t *benchMark);
	bool remove(T item,sub_benchMark_t *benchMark);
	bool contains(T item,sub_benchMark_t *benchMark);
};
#endif