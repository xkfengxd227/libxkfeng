/**
 *	the Bitmap class
 */
#ifndef	BITMAP_H
#define BITMAP_H
#include <math.h>
#include "common.h"

typedef	int		BLOCK_TYPE;			/* the basic block to store bits */
#define	BLOCK_BITS	32				/* block size in bits */
#define BLOCK_SHIFT	5				/* [i >> 5] means [i / 32]*/
#define BLOCK_MASK	0x1F			/* 31 */

/**
 * the BitVector class
 */
class BitVector{
private:
	int				n;				/* vector dimension */
	int 			blockNum;		/* number of blocks */
	BLOCK_TYPE		*blocks;		/* bits in vector */
public:
	BitVector(int _n){
		n = _n;
		blockNum = (int)ceil(n / BLOCK_BITS);
		blocks = (BLOCK_TYPE*)malloc(blockNum * sizeof(BLOCK_TYPE));
	}
	~BitVector(){
		n = -1;
		blockNum = -1;
		FREE(blocks);
	}


	/*
	 * 置位函数——用"|"操作符,i&MASK相当于mod操作
	 * m mod n 运算，当n = 2的X次幂的时候,m mod n = m&(n-1)
	 */
	void set(int i) {
		blocks[i >> BLOCK_SHIFT] |= (1 << (i & BLOCK_MASK));
	}
	/* 清除位操作，用&~操作符 */
	void clear(int i) {
		blocks[i >> BLOCK_SHIFT] &= ~(1 << (i & BLOCK_MASK));
	}
	/* 测试位操作用&操作符 */
	int get(int i) {
		return blocks[i >> BLOCK_SHIFT] & (1 << (i & BLOCK_MASK));
	}
};

#endif