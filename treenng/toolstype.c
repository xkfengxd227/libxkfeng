/**
 * =====================================================================================
 *
 *       Filename:  toolstype.h
 *
 *    Description:  defines some tools types and operations, including #DyArray, #
 *
 *        Version:  1.0
 *        Created:  2015年10月06日
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  xkfeng
 *   Organization:  cui1038
 *	   Change Log:
 *			- [Notice][16-4-17] the address of [elem] may change when expanding the space
 *
 *
 *
 * =====================================================================================
 */
#include "common.h"
#include "toolstype.h"
#include <stdlib.h>
#include <string.h>

void DyArray_init(DyArray *arr, int usize, int maxcount){
    ASSERTINFO(arr == NULL || usize <= 0, "DyArray_init: invalid parameters");
    /** init */
    if(maxcount <= 0){
    	arr->elem = NULL;
    	arr->_usize = usize;
    	arr->_max_count = 0;
    	arr->count = 0;
    }else{
	    arr->elem = (void*)malloc(usize * maxcount);
	    ASSERTINFO(arr->elem == NULL, "DyArray_init: failed to allocate space   for initial elements");
    	arr->_usize = usize;
    	arr->_max_count = maxcount;
    	arr->count = 0;
    }
}

void DyArray_unset(DyArray *arr){
    if(arr != NULL){
        if(arr->elem != NULL){
            free(arr->elem);    arr->elem = NULL;
        }
        arr->count = 0;
        arr->_max_count = -1;
        arr->_usize = -1;
    }
}

void DyArray_PrepareEnoughSpace(DyArray *arr, int count){
	ASSERTINFO(arr == NULL || count < 0 || arr->elem == NULL, "IPP");

	if(count > arr->_max_count){
		arr->elem = (void*)realloc(arr->elem, arr->_usize * count);
		ASSERTINFO(arr->elem == NULL, "failed to allocate new space");
		// update counter <important>
		arr->_max_count = count;
	}
}

void DyArray_CheckExpand(DyArray *arr, int up_range, int expdcnt){
    ASSERTINFO(arr == NULL || arr->elem == NULL || up_range < 0,
        "DyArray_FullExpand: null array or null element list or error counter");
    
    expdcnt = (expdcnt == INT_DEFAULT) ? DyArrayDfltExpandStep : expdcnt;
    while(up_range >= arr->_max_count){
        /** up_range exceeds max_count, expand */
        arr->elem = (void*)realloc(arr->elem, arr->_usize * (arr->_max_count + expdcnt));
        ASSERTINFO(arr->elem == NULL, "DyArray_FullExpand: failed to allocate new space for expanding elements");
        arr->_max_count += expdcnt;
    }
}

void *DyArray_get(const DyArray *arr, int i, int cnt){
    ASSERTINFO(arr == NULL || i < 0 || i >= arr->count || cnt <= 0, "DyArray_get: null array or invalid position or invalid count");
    ASSERTINFO(i+cnt > arr->count, "DyArray_get: not enough elements");
    unsigned char *p = arr->elem;
    p = p + i * arr->_usize;
    return (void*)p;
}

void DyArray_set(DyArray *arr, int i, void *val, int cnt, int expdcnt){
    ASSERTINFO(arr == NULL || arr->elem == NULL || i < 0 || i > arr->count || val == NULL, "DyArray_set: invalid parameters");
    /** check to expand space */
    DyArray_CheckExpand(arr, i+cnt-1, expdcnt);
    /** update value and counter */
    memcpy((unsigned char*)arr->elem + i * arr->_usize, val, arr->_usize * cnt);
    arr->count += cnt;
}

void DyArray_add(DyArray *arr, void *val, int cnt){
	__assertinfo(arr == NULL || val == NULL || cnt <= 0, "IPP");
	DyArray_set(arr, arr->count, val, cnt, INT_DEFAULT);
}

