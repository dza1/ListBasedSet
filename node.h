#ifndef NODE_H__
#define NODE_H__
#include <stdint.h>
#include <mutex> // std::mutex, std::lock_guard

template <typename T> class node {
  private:
	std::mutex mtx;

  public:
	uint32_t key;
	node *next;
	T item;
	node(T item);
	node(T item, uint32_t key);
	uint32_t hash();
	void lock();
	void unlock();
};
#endif