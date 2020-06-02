#ifndef LOCKFREE_IMPR_MEM_H__
#define LOCKFREE_impr_mem_H__
#include <iostream>
using namespace std;
#include "node.hpp"
#include <mutex> // std::mutex, std::lock_guard
#include <omp.h>
#include <stdint.h>

#include "Window.hpp"
#include "setlist.hpp"

#include <memory>

template <typename T> class LockFree_impr_mem : public SetList<T> { // The class
  private:
	T item;
	Window_at_t<nodeAtom<T>> find(T item, sub_benchMark_t *benchMark);
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
	void emteyQueue(bool final);
};

template <typename T> class node_atomic_del { // The class




  public:
	node_atomic_del(nodeAtom<T> *pointer, std::atomic<uint32_t> *snap, size_t T_max);
	~node_atomic_del();

	nodeAtom<T> *pointer;
	std::atomic<uint32_t> *snap;
};

#endif
