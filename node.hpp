/** @file node.hpp
 * @author Daniel Zainzinger
 * @date 2.6.2020
 *
 * @brief nodes for the datastructure and the delete queue
 */
#ifndef NODE_H__
#define NODE_H__
#include <atomic>
#include <mutex> 
#include <stdint.h>

/////////////////////////node for inheritance/////////////////////////////
template <typename T> class node_virt {
  public:
	int32_t key;
	T item;
	node_virt(T item);
	node_virt(T item, int32_t key);
	// std::atomic<int> cnt;
};

/////////////////////////node/////////////////////////////
template <typename T> class node : public node_virt<T> {

  public:
	using node_virt<T>::node_virt;
	node *next = nullptr;
};

/////////////////////////nodeFine/////////////////////////////
template <typename T> class nodeFine : public node_virt<T> {
  private:
	std::mutex mtx;

  public:
	using node_virt<T>::node_virt;
	nodeFine *next = nullptr;
	void lock();
	void unlock();
};

/////////////////////////nodeLazy/////////////////////////////
template <typename T> class nodeLazy : public nodeFine<T> {
  public:
	using nodeFine<T>::nodeFine;
	nodeLazy *next = nullptr;
	bool marked = false;
};

/////////////////////////node_del/////////////////////////////
template <typename T> class node_del { // The class
  private:
	T *pointer;

  public:
	node_del(T *pointer, std::atomic<uint32_t> *snap, size_t T_max);
	~node_del();
	uint32_t *snap;
};

/////////////////////////nodeAtom/////////////////////////////
template <typename T> class nodeAtom : public node_virt<T> {
  private:
  public:
	using node_virt<T>::node_virt;
	std::atomic<nodeAtom<T> *> next{NULL};
};



#endif