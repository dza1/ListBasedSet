#ifndef SETLIST_H__
#define SETLIST_H__

template <typename T> class SetList {
  public:
	virtual bool add(T item) = 0;
	virtual bool remove(T item) = 0;
	virtual bool contains(T item) = 0;
};

#endif