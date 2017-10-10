/**
 * =====================================================================================
 *
 *       Filename:  dyarray.cpp
 *
 *    Description:  implementations of the DyArray class
 *
 *        Version:  2.0
 *        Created:  2017年1月18日
 *       Revision:  yes
 *       Compiler:  g++
 *
 *         Author:  xikafe
 *   Organization:  cui1001
 *	   Change Log:
 *			- [Notice][16-4-17] the address of [elem] may change when expanding the space
 *
 *
 *
 * =====================================================================================
 */
#include "common.h"
#include "dyarray.h"
#include <stdlib.h>
#include <string.h>

DyArray::DyArray(int usize, int maxcount){
    ASSERTINFO(usize <= 0, "IPP");
    /** init */
    if(maxcount <= 0){
    	elem = NULL;
    	_usize = usize;
    	_max_count = 0;
    	count = 0;
    }else{
	    elem = (void*)malloc(usize * maxcount);
	    ASSERTINFO(elem == NULL, "DyArray::init: failed to allocate space   for initial elements");
    	_usize = usize;
    	_max_count = maxcount;
    	count = 0;
    }
}
DyArray::~DyArray(){
  FREE(elem);
  count = 0;
  _max_count = -1;
  _usize = -1;
}

void DyArray::prepare_enugh_space(int count){
	ASSERTINFO(count < 0 || elem == NULL, "IPP");

	if(count > _max_count){
		elem = (void*)realloc(elem, _usize * count);
		ASSERTINFO(elem == NULL, "failed to allocate new space");
		// update counter <important>
		_max_count = count;
	}
}

void DyArray::check_expand(int up_range, int expdcnt){
    ASSERTINFO(elem == NULL || up_range < 0,
        "DyArray::FullExpand: null array or null element list or error counter");

    expdcnt = (expdcnt == INT_DEFAULT) ? DyArrayDfltExpandStep : expdcnt;
    while(up_range >= _max_count){
        /** up_range exceeds max_count, expand */
        elem = (void*)realloc(elem, _usize * (_max_count + expdcnt));
        ASSERTINFO(elem == NULL, "DyArray::FullExpand: failed to allocate new space for expanding elements");
        _max_count += expdcnt;
    }
}

void *DyArray::get(int i, int cnt){
    ASSERTINFO(i < 0 || i >= count || cnt <= 0, "DyArray::get: null array or invalid position or invalid count");
    ASSERTINFO(i+cnt > count, "DyArray::get: not enough elements");
    unsigned char *p = (unsigned char *)elem;
    p = p + i * _usize;
    return (void*)p;
}

void DyArray::set(int i, void *val, int cnt, int expdcnt){
    ASSERTINFO(elem == NULL || i < 0 || i > count || val == NULL, "DyArray::set: invalid parameters");
    /** check to expand space */
    check_expand(i+cnt-1, expdcnt);
    /** update value and counter */
    memcpy((unsigned char*)elem + i * _usize, val, _usize * cnt);
    count += cnt;
}

void DyArray::add(void *val, int cnt){
	__assertinfo(val == NULL || cnt <= 0, "IPP");
	set(count, val, cnt, INT_DEFAULT);
}
