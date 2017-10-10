/** 
 *	\file	regist bins for codes 
 *	\brief 	- a code set contains n m-th dimension codes (int), we statistic objects locates in each
 *		each bins, like an inverted-file
 *		- specify a list of nBin to indicates number of bins in each dimension
 *		- default into bins, not bin boundaries
 *		- we specify m to indicate the dimension of codes
 *		- we support to define ngRange bins as a bin, when ngRange is set to 0, means each independent bin
 *		- we default that each dimension share the same number of bins
 */
#ifndef	BIN_REG_H
#define BIN_REG_H

#include "dyarray.h"
#include "common.h"

/**
 *	\brief	initialize a DyArray to store register-bins: space and counter
 *	@param	m 		dimension of codes
 *	@param	nBin 		number of bins in each dimension
 *	@param	_init_count 	init count of members in each bin
 *	@return 	the inited dyarray
 */
DyArray *BinReg_init(int m, int nBin, int _init_count);

/**
 *	\brief	regist all objects into relative bins
 *	@param	stat 
 *	@param	codes 		codes to be registed
 *	@param	n 		number of codes
 *	@param	m
 *	@param	nBin
 *	@param	ngRange	indicates the range of (how far) neighbor bins to be a to-be-registed bin
 */
void BinReg_regist(DyArray *stat, int *codes, int n, int m, int nBin, int ngRange);

/**
 *	\brief	save the Register-Bin index into file
 *	@param	stat
 *	@param	m
 *	@param	nBin
 *	@param	root		binreg root folder
 *	@param	dsname	the dsname of codes
 *	@param	b 		nBin=2^b
 *	@param	ngRange
 */
void BinReg_into_files(DyArray *stat, int m, int nBin, const char *root, const char *dsname, int b, int ngRange);

/**
 *	\brief	check out the integrity of Register-Bin index files
 *	@param	root
 *	@param	dsname
 *	@param	b
 *	@param	ngRange
 *	@param	m
 */
bool BinReg_check_exists(const char *root, const char *dsname, int b, int ngRange, int m);

/**
 *	\brief	load all Register-Bin index from files
 *	@param	stat
 *	@param	root
 *	@param	dsname
 *	@param	b
 *	@param	ngRange
 *	@param	m
 *	@param	nBin
 */
bool BinReg_from_files(DyArray *stat, const char *root, const char *dsname, int b, int ngRange, int m, int nBin);

/**
 *	\brief	load a specific register-bin from a file (determined by _im, and _ibin)
 *	@param	stat
 *	@param	root
 *	@param	dsname
 *	@param	b
 *	@param	ngRange
 *	@param	_im		which dimension
 *	@param	_ibin		which bin
 */
void BinReg_load_a_bin(DyArray *stat, const char *root, const char *dsname, int b, int ngRange, int _im, int _ibin);

#endif
