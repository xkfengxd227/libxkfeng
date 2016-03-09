/** \file cost.h
 *  \brief Record program's costs
 *      including I/O and cpu runtime
 */
#ifndef COST_H
#define COST_H

/** \brief a struct for cost */
typedef struct
{
    long    point;
    long    page;
    long    cpu;
    long    io;
    long    lowerbound;
    long    search;
} Cost;
/** \brief Initialize a cost structure 
 * @param	c	a cost sample
 */

void CostInit(Cost *c);
/** \brief Combine two cost records
 * @param	tgt	the first cost, also the sum target
 * @param	src	the second cost
 */
void CostCombine(Cost *tgt, const Cost *src);

/** \brief Cost subtraction 
 * @param	tgt	the first cost, also the subtraction target
 * @param	src	the second cost
 */
void CostSubtract(Cost *tgt, const Cost *src);

/** \brief Cost multiply(division also) 
 * @param	tgt	the original cost
 * @param	num	the multiple/division number
 */
void CostMultiply(Cost *tgt, float num);

/** \brief Display a cost 
 * @param	c	cost to display 
 */
void CostDisplay(Cost c);

#endif
