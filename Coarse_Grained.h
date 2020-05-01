#include <iostream>
using namespace std;
#include "node.h"
#include <mutex> // std::mutex, std::lock_guard
#include <omp.h>
#include <stdint.h>


template<typename T>
class CoarseList { // The class
  private:
   T item;
	std::mutex mtx;

  public:
	node *head;
	CoarseList();
	bool add (T item);
};
