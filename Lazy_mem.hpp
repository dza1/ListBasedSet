/** @file Lazy_mem.hpp
 * @author Daniel Zainzinger and Noah Bruns
 * @date 2.6.2020
 *
 * @brief TODO
 */
#ifndef LAYZ_MEM_H__
#define LAYZ_MEM_H__
#include <iostream>
using namespace std;
#include "node.hpp"
#include <mutex> // std::mutex, std::lock_guard
#include <omp.h>
#include <stdint.h>
#include <queue>
#include "Window.hpp"
#include "setlist.hpp"



template <typename T> class Lazy_mem : public SetList<T> { // The class
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
	Lazy_mem();
	~Lazy_mem();
	bool add(T item, sub_benchMark_t *benchMark);
	bool remove(T item, sub_benchMark_t *benchMark);
	bool contains(T item, sub_benchMark_t *benchMark);
	void emptyQueue(bool final); // not used
};



#endif