/**
 *  implement for cost
 */
#include "cost.h"
#include <stdio.h>
#include <math.h>

/* initialize a cost structure */
void CostInit(Cost *c)
{
    c->point = 0L;
    c->page = 0L;
    c->cpu = 0L;
    c->io = 0L;
    c->lowerbound = 0L;
    c->search = 0L;

    int	i;
	for(i = 0; i < CNT_NUM; i++){
		c->counter[i] = 0L;
		c->timer[i] = 0L;
	}
}
/* cost plus */
void CostCombine(Cost *tgt, const Cost *src)
{
    tgt->point = tgt->point + src->point;
    tgt->page = tgt->page + src->page;

    tgt->cpu = tgt->cpu + src->cpu;
    tgt->io = tgt->io + src->io;
    tgt->lowerbound = tgt->lowerbound + src->lowerbound;
    tgt->search = tgt->search + src->search;

    int	i;
	for(i = 0; i < CNT_NUM; i++){
		tgt->counter[i] = tgt->counter[i] + src->counter[i];
		tgt->timer[i] = tgt->timer[i] + src->timer[i];
	}
}
/* cost subtraction */
void CostSubtract(Cost *tgt, const Cost *src)
{
    tgt->point = tgt->point - src->point;
    tgt->page = tgt->page - src->page;

    tgt->cpu = tgt->cpu - src->cpu;
    tgt->io = tgt->io - src->io;
    tgt->lowerbound = tgt->lowerbound - src->lowerbound;
    tgt->search = tgt->search - src->search;

    int	i;
	for(i = 0; i < CNT_NUM; i++){
		tgt->counter[i] = tgt->counter[i] - src->counter[i];
		tgt->timer[i] = tgt->timer[i] - src->timer[i];
	}
}
/* Cost multiply(division also) */
void CostMultiply(Cost *tgt, float num)
{
    tgt->point = (long)ceil(tgt->point * num);
    tgt->page = (long)ceil(tgt->page * num);

    tgt->cpu = (long)ceil(tgt->cpu * num);
    tgt->io = (long)ceil(tgt->io * num);
    tgt->lowerbound = (long)ceil(tgt->lowerbound * num);
    tgt->search = (long)ceil(tgt->search * num);

    int	i;
	for(i = 0; i < CNT_NUM; i++){
		tgt->counter[i] = (long)(tgt->counter[i] * num);
		tgt->timer[i] = (long)(tgt->timer[i] * num);
	}
}
void CostDisplay(Cost c)
{
    printf("page:\t%ld\npoint:\t%ld\n", c.page, c.point);
    printf("total:\t%ldus\n", c.cpu);
    printf("\t|__lb:\t%ldus\n", c.lowerbound);
    printf("\t|__io:\t%ldus\n", c.io);
    printf("\t|__search:\t%ldus\n", c.search);
    printf("\t|__cpu:\t%ldus\n", c.cpu-c.io);
}
