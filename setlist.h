#ifndef SETLIST_H__
#define SETLIST_H__

template <typename T> class SetList {
  public:
	virtual bool add(T item, int *benchMark) = 0;
	virtual bool remove(T item, int *benchMark) = 0;
	virtual bool contains(T item, int *benchMark) = 0;
	SetList(){;}
	virtual ~SetList(){; } 
};

#endif