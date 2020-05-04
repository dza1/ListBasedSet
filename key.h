
#ifndef KEY_H__
#define KEY_H__

#include <stdint.h>

template <typename T> static int32_t key_calc(T item) {
	int32_t key = (int32_t)std::hash<T>()(item);
	if (key == INT32_MIN || key == INT32_MAX) {
		key = key >> 1;
	}
	return key;
}

#endif