/* -----------------------------------------------------------------
 * mainext:	define some extension contents for the main.c file
 *      - analysis argv: _input_parameters()
 *      - 
 * author: xkfeng
 * time: 2016-3-1
 * -----------------------------------------------------------------*/
#ifndef	MAINEXT_H
#define	MAINEXT_H
#include "common.h"

/*  Analysis the input and extract the parameters from argv[]:
 *
 *  This can be self-define.
 */
bool _input_parameters(int argc, char *argv[], char *dsname, int *bf, int *h, int *g, int *k, int *pk);

#endif
