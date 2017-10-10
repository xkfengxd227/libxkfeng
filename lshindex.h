/**
 *	\file	definition of the LSHIndex class
 *
 *	author:	xikafe
 *	date:	2017-1-19
 */
#ifndef	LSHIndex_H
#define	LSHIndex_H
#include "common.h"
#include "dyarray.h"

/**	\brief	LSH index data type
 *	including: none, codes, binreg
 *	notice: we default that binreg data only appears in one-index structure, i.e., L=1
 */
#define		LSHIndexData_none 		0x00000000
#define		LSHIndexData_codes 		0x00000001
#define		LSHIndexData_binreg 	0x00000010
/**	\brief	folder to store binreg data
 */
#define		LSHIndexData_BinReg_Path	"binreg"

/**
 *	\brief	LSH index class, may contain several set of compound functions
 */
typedef	struct LSHIndex{
	int			l;			/*!< number of indexes */
	int			m;			/*!< capacity of compound LSH functions */
	int			n;			/*!< number of data objects */
	int			d;			/*!< data dimension */
	int 			nbit;			/*!< bits that allocate to each LSH direction */
	float			w;			/*!< same widths of internal along each LSH function */
	float			T;			/*!< maximum coordidate value */
	LSH			*lsh;			/*!< the compound LSH functions */
	int			**code;			/*!< the LSH codes */
}LSHIndex;

/**
 *	\brief	initialize the LSH index
 */
void LSHIndex_init(LSHIndex *index, int l, int m, int n, int d, int nbit, float T, float w);

/**
 *	\brief	unset the LSH index
 */
void LSHIndex_unset(LSHIndex *index);

/**
 *	\brief	no need of codes, release them
 */
void LSHIndex_release_codes(LSHIndex *);

/**
 *	\brief	fully check out the LSHIndex object
 */
bool LSHIndex_fullycheck(const LSHIndex *in);

/**
 *	\brief	generate LSH function(s)
 */
void LSHIndex_generate_lsh(LSHIndex *index);

/**
 *	\brief	encode a dataset with LSH functions, store in the index
 */
void LSHIndex_encode(LSHIndex *index, const fDataSet *ds);

/**
 *	\brief	statistic the LSH codes into bin register datas
 */
DyArray *LSHIndex_generate_binreg(LSHIndex *in);

/**
 *	\brief	encode a dataset and export the codes, not store in the index
 */
int **LSHIndex_export_encode(const LSHIndex *in, const fDataSet *ds);

/**
 *	\brief	save index into file: including codes, binreg
 */
void LSHIndex_into_file(LSHIndex *in, const char *indexfolder, const char *dsname, int _idt);

/**
 *	\brief	check if exists index
 */
bool LSHIndex_check_exists(const char *indexfolder, const char *dsname, int l, int m, float T, float w, int _idt);

/**
 *	\brief	load index from file
 *	\notice	we may or may not load the codes, and we will not load binreg data
 *		if we do not load codes, release the codes' space
 */
bool LSHIndex_from_file(LSHIndex *in, const char *indexfolder, const char *dsname, int _idt);

#endif
