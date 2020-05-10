#ifndef NODE_H__
#define NODE_H__
#include <atomic>
#include <mutex> // std::mutex, std::lock_guard
#include <stdint.h>

template <typename T> class node_virt {

  public:
	int32_t key;
	T item;
	node_virt(T item);
	node_virt(T item, int32_t key);
	int32_t hash();
	std::atomic<int> cnt;
};

template <typename T> class node : public node_virt<T> {

  public:
	using node_virt<T>::node_virt;
	node *next = nullptr;
};

template <typename T> class nodeFine : public node_virt<T> {
  private:
	std::mutex mtx;

  public:
	using node_virt<T>::node_virt;
	nodeFine *next = nullptr;
	void lock();
	void unlock();
};

template <typename T> class nodeAtom : public node_virt<T> {
  private:
  public:
	using node_virt<T>::node_virt;
	std::atomic<nodeAtom<T> *> next {NULL};
};

#endif