#ifndef WINDOW_H__
#define WINDOW_H__

template <typename T> struct Window_t {
	nodeFine<T> *pred;
	nodeFine<T> *curr;
};

template <typename node_t> struct Window_at_t {
	node_t *pred;
	node_t *curr;
};


#endif