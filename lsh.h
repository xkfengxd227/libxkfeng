/**
 *	\file	definitions of the LSH class
 *
 *	author:		xikafe
 *	date:			2017-1-19
 */
#ifndef	LSH_H
#define	LSH_H
#include "common.h"
#include "dyarray.h"

/**
 *	\brief	Compound LSH function class, function refers to Equation 2 in [doc:lshfunction]
 */
class CompLSH{
public:
	int			m;			/*!< number of hashing functions */
	int			d;			/*!< dimensions for random vectors */
	float		*a;			/*!< random vectors: [m x d] */
	float		T;			/*!< offset to make sure the projection value is positive */
	float		b;			/*!< random offset: scalar */
	float		w;			/*!< interval widths: scalar */

	CompLSH(int _m, int _d, float _t, float _w);
	~CompLSH();
	bool fully_check();													/*!< check for necessary members */
	void generate_functions();									/*!< generate variables related to lsh functions */
	void encode(const fDataSet *ds, int *code);	/*!< encode a dataset */
	void encode(float *v, int n, int data_d, int *code);
};

#endif
