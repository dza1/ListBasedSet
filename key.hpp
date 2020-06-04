/** @file key.hpp
 * @author Daniel Zainzinger
 * @date 2.6.2020
 *
 * @brief Calculate the key for an item which is necessary to stor it in a set
 */
#ifndef KEY_H__
#define KEY_H__
#include <iostream>
#include <stdint.h>
#include <assert.h> 

// Calculate the key for the node
template <typename T> static int32_t key_calc(T item) {
	int32_t key = (int32_t)std::hash<T>()(item);

	assert(key > INT32_MIN && key < INT32_MAX);

	return key;
}

#endif