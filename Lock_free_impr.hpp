/** @file Lock_free_impr.hpp
 * @author Daniel Zainzinger
 * @date 2.6.2020
 *
 * @brief Lockfree list based set, with improvement for linkout nodes without memory management.
 */
#ifndef LOCKFREE_impr_H__
#define LOCKFREE_impr_H__
#include <iostream>
using namespace std;
#include "node.hpp"
#include <mutex> // std::mutex, std::lock_guard
#include <omp.h>
#include <stdint.h>
#include "setlist.hpp"
#include "Window.hpp"
#include <memory>

#define FLAG_POS 63
#define MASK 1ULL<<FLAG_POS


template <typename T> class LockFree_impr : public SetList<T> { // The class
  private:
	T item;
	Window_t<nodeAtom<T>> find(T item, sub_benchMark_t *benchMark);
	nodeAtom<T>* getPointer(nodeAtom<T>* pointer);
	void setFlag(nodeAtom<T>** pointer);
	void resetFlag(nodeAtom<T>** pointer);
	void resetFlag(atomic<nodeAtom<T>*>* pointer);
	bool getFlag(nodeAtom<T>* pointer);

  public:
	nodeAtom<T>* head;
	LockFree_impr();
	~LockFree_impr();
	bool add(T item,sub_benchMark_t *benchMark);
	bool remove(T item,sub_benchMark_t *benchMark);
	bool contains(T item,sub_benchMark_t *benchMark);
	void emptyQueue(bool final){}; //not used
};
#endif
