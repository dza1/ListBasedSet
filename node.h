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
	// std::atomic<int> cnt;
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

template <typename T> class nodeAtom {
  private:
  public:
  	int32_t key;
	T item;
	std::atomic<int64_t> hash_mem{0};
	std::atomic<nodeAtom<T> *> next{NULL};
	nodeAtom(T item);
	nodeAtom(T item, int32_t key);
};

template <typename T> class nodeFine_mem : public nodeFine<T> {
  private:
  public:
	std::atomic<int64_t> hash_mem{0};
	nodeFine_mem *next = nullptr;
	nodeFine_mem(T item);
	nodeFine_mem(T item, int32_t key);
};

#endif