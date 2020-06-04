/** @file Fine_Grained.hpp
 * @author Daniel Zainzinger
 * @date 2.6.2020
 *
 * @brief list based set, which locks node, when it is reading or writing on it
 */
#ifndef FINE_GRAINED_H__
#define FINE_GRAINED_H__
#include <iostream>
using namespace std;
#include "node.hpp"
#include <mutex> // std::mutex, std::lock_guard
#include <omp.h>
#include <stdint.h>

#include "setlist.hpp"

template <typename T> class FineList : public SetList<T> { // The class
  private:
	T item;
	std::mutex mtx;

  public:
	nodeFine<T> *head;
	FineList();
	~FineList();
	bool add(T item,sub_benchMark_t *benchMark);
	bool remove(T item,sub_benchMark_t *benchMark);
	bool contains(T item,sub_benchMark_t *benchMark);
	void emptyQueue(bool final){}; //not used
};
#endif