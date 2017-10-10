/**
 * =====================================================================================
 *
 *          \file  dyarray.h
 *
 *    	   \brief  declaration file of the [dynamic array] class
 *
 *        Version:  2.0
 *        Created:  2017-1-18
 *       Revision:  yes
 *       Compiler:  g++
 *
 *         Author:  xikafe
 *   Organization:  cui1001
 *
 * =====================================================================================
 */
#ifndef TOOLS_TYPE_H
#define TOOLS_TYPE_H

#define     DyArrayDfltExpandStep     100          /* default step length (element count) of dynamic array to expand */

/** \brief A dynamic array class: support insertion of a set of elements */
class DyArray{
public:
  int _usize;             /*< unit size of an element */
  int _max_count;         /*< the maximum count of element */
  int count;              /*< current count of elements */
  void *elem;             /*< the element list */

  DyArray(int usize, int maxcount);
  ~DyArray();             /*< reset the dynamic array: release space, and reset scalar values */

  void prepare_enugh_space(int count);
                          /*< expand to make sure the space be the specified one */

  /** check whether the array has enough space, if not, expand until enough
   *  @param  arr         the array
   *  @param  up_range    required space range
   *  @param  expdcnt     expand count (if expdcnt == INT_DEFAULT(-1), expand=DyArrayDfltExpandStep)
   */
  void check_expand(int up_range, int expdcnt);

  /*< get the i-th element (or cnt elements which will check whether there is enough elements from i)
   *  @param  arr     the array
   *  @param  i       the index of element
   *  @return pointer to the i-th element
   */
  void *get(int i, int cnt);

  /*< update the value from position i, maybe a set of values with cnt-length, self-define expand range: expdcnt, can be set as INT_DEFAULT, if so, use DyArrayDfltExpandStep
   *  @param  arr     the array
   *  @param  i       the begining position
   *  @param  val     value (set)
   *  @param  cnt     value count
   *  @param  expdcnt self-defile expand count
   */
  void set(int i, void *val, int cnt, int expdcnt);

  /**
   *		add a (set of) new element(s) to the end of the array
   *	@param	arr		the dynamic array
   *	@param	val		the value of new element(s)
   *	@param	cnt		count of new element(s)
   */
  void add(void *val, int cnt);
};

#endif
