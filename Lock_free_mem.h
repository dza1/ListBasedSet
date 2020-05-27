#ifndef LOCKFREE_MEM_H__
#define LOCKFREE_MEM_H__
#include <iostream>
using namespace std;
#include "node.h"
#include <mutex> // std::mutex, std::lock_guard
#include <omp.h>
#include <stdint.h>

#include "setlist.h"
#include "Window.h"

#include <memory>

#define FLAG_POS 63
#define MASK 1ULL<<FLAG_POS




template <typename T> class LockFree_mem : public SetList<T> { // The class
  private:
	T item;
	std::mutex mtx;
	Window_at_t<nodeAtom<T>> find(T item);
	nodeAtom<T>* getPointer(nodeAtom<T>* pointer);
	void setFlag(nodeAtom<T>** pointer);
	void resetFlag(nodeAtom<T>** pointer);
	void resetFlag(atomic<nodeAtom<T>*>* pointer);
	bool getFlag(nodeAtom<T>* pointer);
	void deleteNodes();

  public:
	nodeAtom<T>* head;
	LockFree_mem();
	~LockFree_mem();
	bool add(T item,int *benchMark);
	bool remove(T item,int *benchMark);
	bool contains(T item,int *benchMark);
	void empty_garb();
};
#endif
