#ifndef NODE_H__
#define NODE_H__
#include <mutex> // std::mutex, std::lock_guard
#include <stdint.h>
#include "key.h"
#include <atomic>

template <typename T> 
class node {

  public:
	int32_t key;
	node *next;
	T item;
	explicit node(T item);
	explicit node(T item, int32_t key);
	int32_t hash();
};


template <typename T> 
class nodeFine : public node<T> {
  private:
	std::mutex mtx;
	

  public:
  	using node<T>::node;
	nodeFine *next;
	void lock();
	void unlock();
}; 

template <typename T> 
class nodeAtom : public node<T> {
  private:

  public:
  	using node<T>::node;
	std::atomic<nodeAtom<T>*> next;
}; 

#endif