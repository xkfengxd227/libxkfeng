/** \file cost.h
 *  \brief Record program's costs
 *      including I/O and cpu runtime
 */
#ifndef COST_H
#define COST_H

#define	CNT_NUM	8				/* number of self-delf counters */

/** \brief a struct for cost 
 *	
 *	Notice: since early version is not sure in the description of each variables, thus, note to 
 *			check out the real meaning in those codes.
 *			
 *			The meaning below are determined after the git of libxkfeng (2016-3-15)
 */

typedef struct
{
    long    point;				/*!< points that be calculated */
    long    page;				/*!< data pages accessed */
    long    cpu;				/*!< calculation cost */
    
    long    io;					/*!< time for io operations */
    long    lowerbound;			/*!< important component time cost: (lowerbound is specied to hb series methods */
    long    search;				/*!< total time cost during search */
    
    /* self-define counters */
    long	counter[CNT_NUM];
    long	timer[CNT_NUM];
    
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
