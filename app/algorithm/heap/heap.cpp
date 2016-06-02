/*************************************************************************
    > File Name: heap.cpp
    > Author: linhu
    > Mail: ylh@hotmail.com 
    > Created Time: Sat 19 Dec 2015 06:49:00 PM CET
 ************************************************************************/

#include<iostream>
using namespace std;

template <class T>
class MaxHeap{
  public:
    MaxHeap();
	MaxHeap(int capacity);
	~MaxHeap();

	// Get data indext in the heap.
	int getIndex(T data);

	// Delete data from the heap.
	int remove(T data);

	// insert data into the heap.
	int insert(T data);

	void swap(int i, int j);

	void print();

  private:
	void filterdown(int start, int end);
    void filterup(int start);
	T * mHeap;
	int mCapacity;
	int mSize;
};

template <class T>
MaxHeap<T> :: MaxHeap() : mSize(0), mCapacity(30)
{
  new (this) MaxHeap(30);
}

template <class T>
MaxHeap<T> :: MaxHeap(int capacity) : mSize(0), mCapacity(capacity)
{
  mHeap = new T[capacity];
}

template <class T>
MaxHeap<T> :: ~MaxHeap()
{
  mSize = 0;
  mCapacity = 0;
  delete[] mHeap;
}

template <class T>
int MaxHeap<T> :: getIndex(T data)
{
  for(int i = 0; i < mSize; i++)
  {
    if(data == mHeap[i])
	{
	  return i;
	}
  }
  return -1;
}

template <class T>
void MaxHeap<T>::filterup(int start)
{
  int c = start;       
  int p = (c-1)/2;        
  T tmp = mHeap[c];

  while(c > 0)
  {
    if(mHeap[p] >= tmp)
	{
     break;
	}
	else
    {
	  mHeap[c] = mHeap[p];
      c = p;
	  p = (p-1)/2;   
	}       
  }
  mHeap[c] = tmp;
}

template <class T>
int MaxHeap<T>::insert(T data)
{
  if(mSize == mCapacity)
  {
    return -1;
  }

  cout << "-------------> insert: " << data << endl; 
  mHeap[mSize] = data;
  filterup(mSize);
  mSize ++;
  return 0;
}

template <class T>
void MaxHeap<T>::filterdown(int start, int end)
{
  int c = start;
  int l = 2*c + 1;
  T tmp = mHeap[c];

  while(l < end)
  {
    if(l < end && mHeap[l] < mHeap[l+1])
	{
	  l++;
	}

	if(tmp >= mHeap[l])
	{
	  break;
	}
	else
	{
	  swap(c, l);
	  c = l;
	  l = 2*l + 1;
	}
  }
}


template <class T>
int MaxHeap<T>::remove(T data)
{
  int index;
  if(mSize == 0)
  {
    return -1;
  }

  index = getIndex(data);
  if(index == -1)
  {
    return -1;
  }
  mHeap[index] = mHeap[--mSize];
  filterdown(index, mSize-1);
  return 0;
}


template<class T>
void MaxHeap<T>::swap(int i, int j)
{
  T tmp = 0;
  tmp = mHeap[i];
  mHeap[i] = mHeap[j];
  mHeap[j] = tmp;
}

template<class T>
void MaxHeap<T>::print()
{
  int i = 0;
  cout << "Heap size:" << endl;
  for(i = 0; i < mSize; i++)
  {
    cout << mHeap[i] << ", " << endl;
  }
}

int main()
{
  int data[10] = {4,5,6,3,2,7,9,0,1,8};
  int length = sizeof(data) / sizeof(int);
  int i = 0;

  MaxHeap<int> *maxHeap = new MaxHeap<int>(20);

  for(i = 0; i < length; i++)
  {
    maxHeap->insert(data[i]);
  }

  maxHeap->remove(8);

  maxHeap->print(); 

}



