#ifndef NODE_H__
#define NODE_H__
#include <mutex> // std::mutex, std::lock_guard
#include <stdint.h>

template <typename T> 
class node {

  public:
	uint32_t key;
	node *next;
	T item;
	node(T item);
	node(T item, uint32_t key);
	uint32_t hash();
};


template <typename T> 
class nodeFine : public node<T> {
  private:
	std::mutex mtx;

  public:
	void lock();
	void unlock();
}; 

#endif