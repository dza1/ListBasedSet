/** @file Lock_free_impr_mem.hpp
 * @author Daniel Zainzinger
 * @date 2.6.2020
 *
 * @brief Header for list based set, which is lockfree with included memory management. 
 */
#ifndef LOCKFREE_IMPR_MEM_H__
#define LOCKFREE_impr_mem_H__
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
	LockFree_impr_mem();
	~LockFree_impr_mem();
	bool add(T item, sub_benchMark_t *benchMark);
	bool remove(T item, sub_benchMark_t *benchMark);
	bool contains(T item, sub_benchMark_t *benchMark);
	void emptyQueue(bool final);
};

template <typename T> class node_atomic_del { // The class
  private:
	nodeAtom<T> *pointer;

  public:
	node_atomic_del(nodeAtom<T> *pointer, std::atomic<uint32_t> *snap, size_t T_max);
	~node_atomic_del();
	uint32_t *snap;
};

#endif
