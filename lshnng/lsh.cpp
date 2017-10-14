/**
 *	\brief	implementation of the LSH class
 *
 *	author:	xikafe
 *	date:	2016-1-19
 */
#include "common.h"
#include "lsh.h"
#include "rand.h"
#include <time.h>
#include <math.h>
#include <stdlib.h>
extern "C"{
	#include <yael/vector.h>
}
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

CompLSH::CompLSH(int _m, int _d, float _t, float _w){
	ASSERTINFO(_m <= 0 || _d <= 0 || _t <= 0 || _w <= 0, "IPP");
	m = _m;
	d = _d;
	T = _t;
	w = _w;
	a = fvec_new(m * d);
	b = -1.;
	ASSERTINFO(a == NULL, "failed to allocate space");
}

CompLSH::~CompLSH(){
	FREE(a);
	m = -1;
	d = -1;
	T = -1.;
	w = -1.;
	b = -1.;
}

bool CompLSH::fully_check(){
	return !(m <= 0 || d <= 0 || T <= 0 || a == NULL || w <= 0);
}

void CompLSH::generate_functions(){
	ASSERTINFO(!fully_check(), "insufficient valid parameters to generate lsh functions");

	int			i;

	/* generate parameters (a) */
	srand((unsigned int)time(NULL));
	for(i = 0; i < m * d; i++){
		a[i] = gaussian(0, 1.0);
	}
	/* normalize (a) */
	for(i = 0; i < m; i++){
		fvec_normalize(a + i*d, d, 2);
	}
	/* b~U[0, w] */
	b = uniform(0, w);
}

void CompLSH::encode(const fDataSet *ds, int *code){
	encode(ds->data, ds->n, ds->d, code);
}
void CompLSH::encode(float *v, int n, int data_d, int *code){
	ASSERTINFO(!fully_check() || v == NULL || code == NULL, "IPP");
	ASSERTINFO(data_d != d, "dimensions between data and lsh do not match");

	/* necessary parameters */
	int			ni, mi;
	float		tempip;
	int			*pcode = code;
	float		*pv = v;
	float		*pa;

	/* encode all data points */
	for(ni = 0; ni < n; ni++){
		/* travese all m LSH functions */
		pa = a;
		for(mi = 0; mi < m; mi++){
			/* the mi-th LSH function */
			tempip = fvec_inner_product(pa, pv, d);
			*pcode = (int)floor((tempip + T + b) / w);		/*< [T] to make sure code positive */
			/* move to next a and next code */
			pcode++;
			pa += d;
		}
		/* move to next data point */
		pv += d;
	}
}
