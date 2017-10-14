/**
 * \file heap.h
 * \brief Some declaration for heap and some operations for max-heap
 */

#ifndef HEAP_H
#define HEAP_H
#include "common.h"

#define     HeapIncreaseStep    10    /// the increase step of a heap

/** \brief the Heap class */
class Heap{
public:
  int             MaxNum;             /*< maximum member of heap */
  int             length;             /*< number of current members */
  DoubleIndex     *elem;              /*< elements */

  Heap(int max_num, double _init_val = 0.f){
      MaxNum = max_num;
      elem = (DoubleIndex*)malloc(sizeof(DoubleIndex)*max_num);
      length = 0;
  }
  ~Heap(){
    FREE(elem);
    MaxNum = -1;
    length = -1;
  }
  double topValue(){  return elem[0].val; }

  /* basic heap operations */
  bool isempty();
  bool isfull();
  int left(int i);                    /*< find the left child of node i */
  int right(int i);                   /*< find the right child of node i */
  int parent(int i);                  /*< find the parent of node i */
  void exchange(int i, int j);        /*< exchange node i and j */
  void display();                     /*< display all members */

  /* max-heap operations */
  void max_heapify(int i);            /*< maintain a max-heap, i is the start node */
  void max_build();                   /*< build a max-heap, or adjust into a max-heap */
  void max_increase_key(int i, DoubleIndex *di);
                                      /*< update value of a key at position i (new key is larger) */
  void max_insert(DoubleIndex *di);   /*< insert a new element into the max-heap, if full, replace the max element, do not care whether newer element is bigger or smaller */
  /** sometimes, we want to maintain a set of smallest values in a max-heap, 
   *  therefore we only welcome (smaller than top) values when the heap is full 
   */
  void max_insert_wisely(DoubleIndex *di);
  void max_expand(DoubleIndex *di);   /*< insert a new element into the max-heap, if full, expand the capacity */
  void max_extract(DoubleIndex *di);  /*< extract the maximum element */

  /* min-heap operations */
  void min_heapify(int i);            /*< maintain a min-heap, i is the start node */
  void min_build();                   /*< build a min-heap, or adjust into a min-heap */
  void min_decrease_key(int i, DoubleIndex *di);
                                      /*< update value of a key at position i (new key is larger) */
  /**
   *  sometimes we want to maintain a set of biggest values in a min-heap, 
   *  therefore we only welcome (bigger than top) values when the heap is full
   */
  void min_insert_wisely(DoubleIndex *di);   /*< insert a new element into the min-heap, if full, replace the min element, do not care whether newer element is bigger or smaller */
  void min_insert(DoubleIndex *di);   /*< insert a new element into the min-heap, if full, replace the min element, do not care whether newer element is bigger or smaller */
  void min_expand(DoubleIndex *di);   /*< insert a new element into the min-heap, if full, expand the capacity */
  void min_extract(DoubleIndex *di);  /*< extract the minimum element */
};

#endif
