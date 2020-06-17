/** @file Lock_free_impr_mem.hpp
 * @author Daniel Zainzinger
 * @date 2.6.2020
 *
 * @brief Lockfree list based set, with improvement for linkout nodes with memory management.
 */
#ifndef LOCKFREE_IMPR_MEM_H__
#define LOCKFREE_IMPR_MEM_H__
#include <iostream>
using namespace std;
#include "node.hpp"
#include <omp.h>
#include <stdint.h>

#include "Window.hpp"
#include "setlist.hpp"

#include <memory>

template <typename T> class LockFree_impr_mem : public SetList<T> { // The class
  private:
	T item;
	Window_t<nodeAtom<T>> find(T item, sub_benchMark_t *benchMark);
	nodeAtom<T> *getPointer(nodeAtom<T> *pointer);
	std::atomic<uint32_t> *snap;
	size_t Tmax;

	void setFlag(nodeAtom<T> **pointer);
	void resetFlag(nodeAtom<T> **pointer);
	void resetFlag(atomic<nodeAtom<T> *> *pointer);
	bool getFlag(nodeAtom<T> *pointer);

  public:
	nodeAtom<T> *head;
	LockFree_impr_mem(size_t Tmax);
	~LockFree_impr_mem();
	bool add(T item, sub_benchMark_t *benchMark);
	bool remove(T item, sub_benchMark_t *benchMark);
	bool contains(T item, sub_benchMark_t *benchMark);
	void emptyQueue(bool final);
};



#endif
