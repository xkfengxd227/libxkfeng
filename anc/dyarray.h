/**
 * =====================================================================================
 *
 *          \file  toolstype.h
 *
 *    	   \brief  defines some tools types and operations, including #DyArray,
 *
 *        Version:  1.0
 *        Created:  2015年10月06日
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  xkfeng
 *   Organization:  cui1038
 *
 * =====================================================================================
 */
#ifndef TOOLS_TYPE_H
#define TOOLS_TYPE_H

#define     DyArrayDfltExpandStep     100          /* default step length (element count) of dynamic array to expand */

/** \brief A dynamic array class: support insertion of a set of elements */
typedef struct{
    int _usize;             /*< unit size of an element */
    int _max_count;         /*< the maximum count of element */
    int count;              /*< current count of elements */
    void *elem;             /*< the element list */
} DyArray;

/** \brief Initialize a dynamic array
 *  @param  arr         the array
 *  @param  usize       unit size
 *  @param  maxcount    max count of clement
 */
void DyArray_init(DyArray *arr, int usize, int maxcount);

/** \brief reset the dynamic array: release space, and reset scalar values */
void DyArray_unset(DyArray *arr);

/** \brief expand to make sure the space be the specified one */
void DyArray_PrepareEnoughSpace(DyArray *arr, int count);

/** \brief  check whether the array has enough space, if not, expand until enough
 *  @param  arr         the array
 *  @param  up_range    required space range
 *  @param  expdcnt     expand count (if expdcnt == INT_DEFAULT(-1), expand=DyArrayDfltExpandStep)
 */
void DyArray_CheckExpand(DyArray *arr, int up_range, int expdcnt);

/** \brief  get the i-th element (or cnt elements which will check whether there is enough elements from i)
 *  @param  arr     the array
 *  @param  i       the index of element
 *  @return pointer to the i-th element
 */
void *DyArray_get(const DyArray *arr, int i, int cnt);

/** \brief  update the value from position i, maybe a set of values with cnt-length, self-define expand range: expdcnt, can be set as INT_DEFAULT, if so, use DyArrayDfltExpandStep
 *  @param  arr     the array
 *  @param  i       the begining position
 *  @param  val     value (set)
 *  @param  cnt     value count
 *  @param  expdcnt self-defile expand count
 */
void DyArray_set(DyArray *arr, int i, void *val, int cnt, int expdcnt);

/**
 *	\brief	add a (set of) new element(s) to the end of the array
 *	@param	arr		the dynamic array
 *	@param	val		the value of new element(s)
 *	@param	cnt		count of new element(s)
 */
void DyArray_add(DyArray *arr, void *val, int cnt);

#endif
