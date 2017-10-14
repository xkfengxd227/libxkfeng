/**
 *	\file	hkmsupport.h
 *		some support methods for hkm class
 */
#ifndef		HKM_SUPPORT_H
#define	HKM_SUPPORT_H
#include "common.h"
#include <yael/hkm.h>

/**
 *	\brief	check out the existence of datatype
 *		data includes: index.hkm, assign.hkm
 */
bool hkm_check_exists(const char *folder, int datatype);

/**
 *	\brief	load data from file
 */
bool hkm_from_file(hkm_t **phkm, int **passign, const char *folder, int n, int datatype);


#endif