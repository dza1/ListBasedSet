#ifndef OPTIMISTIC_H__
#define OPTIMISTIC_H__
#include <iostream>
using namespace std;
#include "node.hpp"
#include <mutex> // std::mutex, std::lock_guard
#include <omp.h>
#include <stdint.h>

#include "Window.hpp"
#include "setlist.hpp"

template <typename T> class Optimistic : public SetList<T> { // The class
  private:
	T item;
	std::mutex mtx;
	Window_t<nodeFine<T>> find(T item, sub_benchMark_t *benchMark);
	void lock(Window_t<nodeFine<T>> w);
	void unlock(Window_t<nodeFine<T>> w);
	bool validate(Window_t<nodeFine<T>> w);

  public:
	nodeFine<T> *head;
	Optimistic();
	~Optimistic();
	bool add(T item, sub_benchMark_t *benchMark);
	bool remove(T item, sub_benchMark_t *benchMark);
	bool contains(T item, sub_benchMark_t *benchMark);
	void emteyQueue(bool final){}; //not used

};
#endif
