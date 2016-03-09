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
 *
 * =====================================================================================
 */
#include "common.h"
#include "toolstype.h"
#include <stdlib.h>
#include <string.h>

void DyArray_Init(DyArray *arr, int usize, int maxcount){
    ASSERTINFO(arr == NULL || usize <= 0 || maxcount <= 0, "DyArray_Init: invalid parameters");
    /** init */
    arr->elem = (void*)malloc(usize * maxcount);
    ASSERTINFO(arr->elem == NULL, "DyArray_Init: failed to allocate space for initial elements");
    arr->_usize = usize;
    arr->_max_count = maxcount;
    arr->count = 0;
}

void DyArray_Reset(DyArray *arr){
    if(arr != NULL){
        if(arr->elem != NULL){
            /** free(arr->elem); */         /** here is always _int_free() error, close it temprally */
            arr->elem = NULL;
        }
        arr->count = 0;
        arr->_max_count = -1;
        arr->_usize = -1;
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

void *DyArray_Get(const DyArray *arr, int i, int cnt){
    ASSERTINFO(arr == NULL || i < 0 || i >= arr->count || cnt <= 0, "DyArray_Get: null array or invalid position or invalid count");
    ASSERTINFO(i+cnt > arr->count, "DyArray_Get: not enough elements");
    unsigned char *p = arr->elem;
    p = p + i * arr->_usize;
    return (void*)p;
}

void DyArray_Set(DyArray *arr, int i, void *val, int cnt, int expdcnt){
    ASSERTINFO(arr == NULL || arr->elem == NULL || i < 0 || i > arr->count || val == NULL, "DyArray_Set: invalid parameters");
    /** check to expand space */
    DyArray_CheckExpand(arr, i+cnt-1, expdcnt);
    /** update value and counter */
    memcpy((unsigned char*)arr->elem + i * arr->_usize, val, arr->_usize * cnt);
    arr->count += cnt;
}
