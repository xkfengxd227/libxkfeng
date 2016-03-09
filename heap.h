/**
 * \file heap.h
 * \brief Some declaration for heap and some operations for max-heap
 */

#ifndef HEAP_H
#define HEAP_H
#include "common.h"

#define     HeapIncreaseStep    10      /// the increase step of a heap

/** \brief the Heap structure definition */
typedef struct
{
    int             MaxNum;           // maximum member of heap
    int             length;             // number of current members
    DoubleIndex     *elem;              // elements
} Heap;

/** basic pointer operations */
/**
 * \brief Initialize a heap
 * @param	h		the heap
 * @param	max_num the max number of element
 */
void Heap_Init(Heap *h, int max_num);

/**
 * \brief Delete elements in a heap
 * @param	h	the heap
 */
void Heap_Delete(Heap *h);

/**
 * \brief Find the left element in a heap
 * @param	i	current position
 * @return	the id of the left element
 */
int Heap_Left(int i);

/**
 * \brief The right element in a heap
 * @param	i	current position
 * @return	the id of the right element
 */
int Heap_Right(int i);

/**
 * \brief Find the parent in a heap
 * @param	i	the current position
 * @return	the id of the parent
 */
int Heap_Parent(int i);

/**
 * \brief Exchange two elements in a heap
 * @param	h	the heap
 * @param	i
 * @param	j
 */
void Heap_Exchange(Heap *h, int i, int j);

/**
 * \brief Display a heap
 * @param	h	the heap
 */
void Heap_Display(Heap *h);

/**************************** operations for max-heap ******************************/
/**
 * \brief Maintain a max-heap
 * @param	h	the heap
 * @param	i	the begining position to maintain
 */
void MaxHeap_Heapify(Heap *h, int i);

/**
 * \brief Build a max-heap, or adjust into a max-heap
 * @param	h	the heap
 */
void MaxHeap_Build(Heap *h);

/**
 * \brief Update value of a key(new key is larger)
 * @param	h	the heap
 * @param	i	the position to update
 * @param	di	...
 */
void MaxHeap_IncreaseKey(Heap *h, int i, DoubleIndex *di);

/**
 * \brief Insert a new element into the max-heap, if full, replace the max element
 * @param	h	the heap
 * @param	di	the new element to insert
 */
void MaxHeap_Insert(Heap *h, DoubleIndex *di);

/**
 * \brief insert a new element into the max-heap, if full, expand the capacity
 * @param   h   the heap
 * @param   di  the new element to insert
 */
void MaxHeap_Expand(Heap *h, DoubleIndex *di);

/**
 * \brief Extract the maximum element
 * @param	h	the heap
 * @param	di	the extracted maximum element
 */
void MaxHeap_Extract(Heap *h, DoubleIndex *di);

/**************************** operations for min-heap ******************************/
/**
 * \brief Maintain a min-heap
 * @param	h	a heap
 * @param	i	the position begin to maintain
 */
void MinHeap_Heapify(Heap *h, int i);

/**
 * \brief Build a min-heap, or adjust into a min-heap
 * @param	h	a heap
 */
void MinHeap_Build(Heap *h);

/**
 * \brief Update value of a key(new key is larger)
 * @param	h	...
 * @param	i	...
 * @param	di	...
 */
void MinHeap_DecreaseKey(Heap *h, int i, DoubleIndex *di);

/**
 * \brief Insert a new element into the max-heap, if full, replace the max element
 * @param	h	a heap
 * @param	di	the element to insert
 */
void MinHeap_Insert(Heap *h, DoubleIndex *di);

/**
 * \brief insert a new element into the min-heap, if full, expand the capacity
 * @param   h   the heap
 * @param   di  the new element to insert
 */
void MinHeap_Expand(Heap *h, DoubleIndex *di);

/**
 * \brief Extract the minimum element
 * @param	h	a heap
 * @param	di	the extracted element
 */
void MinHeap_Extract(Heap *h, DoubleIndex *di);

#endif
