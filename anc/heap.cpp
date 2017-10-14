/**
 *  the implementation for heap class
 */
#include "heap.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/// basic pointer operations
bool Heap::isempty(){  return (length == 0); }
bool Heap::isfull(){  return (length >= MaxNum); }
int Heap::left(int i)    {   return (2*i+1);     }
int Heap::right(int i)   {   return ((i+1)*2);   }
int Heap::parent(int i)  {   return ((i-1)/2);   }
/// exchange two elements
void Heap::exchange(int i, int j){
    DoubleIndex temp;
    memcpy(&temp, &elem[i], sizeof(DoubleIndex));
    memcpy(&elem[i], &elem[j], sizeof(DoubleIndex));
    memcpy(&elem[j], &temp, sizeof(DoubleIndex));
}
/// display a heap
void Heap::display()
{
    int i;
    putchar('[');
    for(i = 0; i < length; i++)
    {
        printf("%d:%d-%f, ", i, elem[i].id, elem[i].val);
    }
    putchar(']');
    putchar('\n');
}

/**************************** operations for max-heap ******************************/
/// maintain a max-heap
void Heap::max_heapify(int i)
{
    int l = left(i);
    int r = right(i);
    int large = i;
    // select the maximum element among the i-th, l, r element
    if(l < length && f_bigger(elem[l].val, elem[i].val))
    {
        large = l;
    }
    else
    {
        large = i;
    }
    if(r < length && f_bigger(elem[r].val, elem[large].val))
    {
        large = r;
    }

    if(large != i)
    {
        exchange(i, large);
        max_heapify(large);
    }
}
/// build a max-heap, or adjust into a max-heap
void Heap::max_build()
{
    if(length <= 0)
    {
        printf("error: no elements yet");
        exit(-1);
    }
    int i;
    for(i = length / 2; i >= 0; i--)
    {
        max_heapify(i);
    }
}
/// update value of a key(new key is larger)
void Heap::max_increase_key(int i, DoubleIndex *di)
{
    if(i >= length){
        printf("error: index exceeds element number\n");
        exit(-1);
    }
    if(f_bigger(elem[i].val, di->val)){
        printf("error: increase value is small\n");
        exit(-1);
    }
    memcpy(&elem[i], di, sizeof(DoubleIndex));
    while(i > 0 && f_bigger(elem[i].val, elem[parent(i)].val))
    {
        exchange(i, parent(i));
        i = parent(i);
    }
}
void Heap::max_insert(DoubleIndex *di)
{
    if(length < MaxNum)
    {// heap is not full
        elem[length].val = -FLOAT_MAX;
        length = length + 1;
        max_increase_key(length-1,  di);
    }
    else
    {// heap is full, just replace the maximum element
        memcpy(&elem[0], di, sizeof(DoubleIndex));
        max_heapify(0);
    }
}
void Heap::max_insert_wisely(DoubleIndex *di){
    if(length < MaxNum)
    {// heap is not full
        elem[length].val = -FLOAT_MAX;
        length = length + 1;
        max_increase_key(length-1,  di);
    }
    else if(f_bigger(topValue(), di->val))
    {
        memcpy(&elem[0], di, sizeof(DoubleIndex));
        max_heapify(0);
    }
}

void Heap::max_expand(DoubleIndex *di){
    /** check out the memory space */
    /* error status: pointer exceeds limit */
    ASSERTINFO(length > MaxNum, "Heap::max_Expand: error, the pointer exceeds the space limit");
    if(length == MaxNum){
        /* heap is full */
        elem = (DoubleIndex*)realloc(elem, sizeof(DoubleIndex)*(MaxNum + HeapIncreaseStep));
        ASSERTINFO(elem == NULL, "Heap::max_Expand: expand error, failed to allocate new space to expand");
        MaxNum += HeapIncreaseStep;
    }else{
        /* free space left, insert */
        elem[length].val = -FLOAT_MAX;
        length = length + 1;
        max_increase_key(length-1, di);
    }
}

/// extract the maximum element
void Heap::max_extract(DoubleIndex *di)
{
    ASSERTINFO(length <= 0, "Heap::max_Extract: no elements in heap");

    memcpy(di, &elem[0], sizeof(DoubleIndex));
    memcpy(&elem[0], &elem[length-1], sizeof(DoubleIndex));
    length = length - 1;
    max_heapify(0);
}

/**************************** operations for min-heap ******************************/
/// maintain a min-heap
void Heap::min_heapify(int i)
{
    int l = left(i);
    int r = right(i);
    int small = i;
    if(l < length && f_bigger(elem[i].val, elem[l].val))
    {
        small = l;
    }
    else
    {
        small = i;
    }
    if(r < length && f_bigger(elem[small].val, elem[r].val))
    {
        small = r;
    }

    if(small != i)
    {
        exchange(i, small);
        min_heapify(small);
    }
}
/// build a min-heap, or adjust into a min-heap
void Heap::min_build()
{
    if(length <= 0)
    {
        printf("error: no elements yet");
        exit(-1);
    }
    int i;
    for(i = length / 2; i >= 0; i--)
    {
        min_heapify(i);
    }
}
/// update value of a key(new key is larger)
void Heap::min_decrease_key(int i, DoubleIndex *di)
{
    // make sure a smaller key value
    if(i >= length)
    {
        printf("error: index exceeds length\n");
        exit(-1);
    }
    if(f_bigger(di->val, elem[i].val))
    {
        printf("error: make sure a smaller value\n");
        exit(-1);
    }
    memcpy(&elem[i], di, sizeof(DoubleIndex));
    while(i > 0 && f_bigger(elem[parent(i)].val, elem[i].val))
    {
        exchange(i, parent(i));
        i = parent(i);
    }
}
/// insert a new element into the max-heap, if full, replace the max element
void Heap::min_insert(DoubleIndex *di)
{
    if(length < MaxNum)
    {// enough space
        elem[length].val = FLOAT_MAX;
        length = length + 1;
        min_decrease_key(length-1, di);
    }
    else
    {// heap is full
        memcpy(&elem[0], di, sizeof(DoubleIndex));
        min_heapify(0);
    }
}
void Heap::min_insert_wisely(DoubleIndex *di)
{
    if(length < MaxNum)
    {// enough space
        elem[length].val = FLOAT_MAX;
        length = length + 1;
        min_decrease_key(length-1, di);
    }
    else if(f_bigger(di->val, topValue()))
    {// heap is full
        memcpy(&elem[0], di, sizeof(DoubleIndex));
        min_heapify(0);
    }
}
void Heap::min_expand(DoubleIndex *di){
    /** check out the memory space */
    /* error status: pointer exceeds limit */
    ASSERTINFO(length > MaxNum, "Heap::min_Expand: error, the pointer exceeds the space limit");
    if(length == MaxNum){
        /* heap is full */
        elem = (DoubleIndex*)realloc(elem, sizeof(DoubleIndex)*(MaxNum + HeapIncreaseStep));
        ASSERTINFO(elem == NULL, "Heap::min_Expand: expand error, failed to allocate new space to expand");
        MaxNum += HeapIncreaseStep;
    }else{
        /* free space left, insert */
        elem[length].val = FLOAT_MAX;
        length = length + 1;
        min_decrease_key(length-1, di);
    }
}

/// extract the maximum element
void Heap::min_extract(DoubleIndex *di)
{
    ASSERTINFO(length <= 0, "Heap::min_Extract: no elements yet");

    memcpy(di, &elem[0], sizeof(DoubleIndex));
    memcpy(&elem[0], &elem[length-1], sizeof(DoubleIndex));
    length = length - 1;
    min_heapify(0);
}
