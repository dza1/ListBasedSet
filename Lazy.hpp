/** @file Lazy.hpp
 * @author Daniel Zainzinger and Noah Bruns
 * @date 2.6.2020
 *
 * @brief TODO 
 */
#ifndef LAZY_H__
#define LAZY_H__
#include <iostream>
using namespace std;
#include "node.hpp"
#include <mutex> // std::mutex, std::lock_guard
#include <omp.h>
#include <stdint.h>
#include "Window.hpp"
#include "setlist.hpp"

template <typename T> class Lazy : public SetList<T> { // The class
  private:
	T item;
	std::mutex mtx;
	Window_t<nodeFine<T>> find(T item, sub_benchMark_t *benchMark);
	void lock(Window_t<nodeFine<T>> w);
	void unlock(Window_t<nodeFine<T>> w);
	bool validate(Window_t<nodeFine<T>> w);

  public:
	nodeFine<T> *head;
	Lazy();
	~Lazy();
	bool add(T item, sub_benchMark_t *benchMark);
	bool remove(T item, sub_benchMark_t *benchMark);
	bool contains(T item, sub_benchMark_t *benchMark);
	void emptyQueue(bool final){}; //not used

};
#endif
