/** @file setlist.hpp
 * @author Noah Bruns
 * @date 2.6.2020
 *
 * @brief Interface for the datastructures
 */
#ifndef SETLIST_H__
#define SETLIST_H__
#include "benchmark.hpp"



template <typename T> class SetList {
  public:
	virtual bool add(T item, sub_benchMark_t *benchMark) = 0;
	virtual bool remove(T item, sub_benchMark_t *benchMark) = 0;
	virtual bool contains(T item, sub_benchMark_t *benchMark) = 0;
	virtual void emptyQueue(bool final)=0;
	SetList() { ; }
	virtual ~SetList() { ; }
};

#endif