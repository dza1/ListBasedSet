/** @file Window.hpp
 * @author Daniel Zainzinger
 * @date 2.6.2020
 *
 * @brief struct wich includes two node pointers to represent a window
 */
#ifndef WINDOW_H__
#define WINDOW_H__


template <typename node_t> struct Window_t {
	node_t *pred;
	node_t *curr;
};


#endif