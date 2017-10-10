/**
 *	\brief	implementation of the LSHIndex classes
 *
 *	author:	xikafe
 *	date:	2017-1-19
 */
#include "common.h"
#include "lsh.h"
#include "rand.h"
#include "dyarray.h"
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

/* ------------------------------------------------- */

void LSHIndex_init(LSHIndex *index, int l, int m, int n, int d, int nbit, float T, float w){
	ASSERTINFO(index == NULL || l <= 0 || m <= 0 || n <= 0 || d <= 0 || nbit <= 0 || T <= 0 || w <= 0,
		"invalid passing parameters");
	int	i;

	index->lsh = (LSH*)malloc(sizeof(LSH)*l);
	index->code = (int**)malloc(sizeof(int*)*l);
	for(i = 0; i < l; i++){
		index->code[i] = (int*)malloc(sizeof(int)*n*m);
		LSH_init(&index->lsh[i], m, d, T, w);
	}

	index->l = l;
	index->m = m;
	index->n = n;
	index->d = d;
	index->nbit = nbit;
	index->w = w;
	index->T = T;
}

void LSHIndex_generate_lsh(LSHIndex *in){
	ASSERTINFO(in == NULL || in->l <= 0, "IPP");
	int i;

	for(i = 0; i < in->l; i++){
		LSH_generate_functions(&in->lsh[i]);
	}
}

void LSHIndex_unset(LSHIndex *index){
	ASSERTINFO(index == NULL, "IPP");

	int i;
	/* lsh's turn */
	if(index->lsh != NULL){
		for(i = 0; i < index->l; i++){
			LSH_unset(&index->lsh[i]);
		}
		free(index->lsh);	index->lsh = NULL;
	}
	LSHIndex_release_codes(index);

	index->l = -1;
	index->m = -1;
	index->n = -1;
	index->d = -1;
	index->nbit = -1;
	index->w = -1.;
	index->T = -1.;
}

void LSHIndex_release_codes(LSHIndex *index){
	int 	i;

	if(index->code != NULL){
		for(i = 0; i < index->l; i++){
			if(index->code[i] != NULL){
				free(index->code[i]); index->code[i] = NULL;
			}
		}
		free(index->code);	index->code = NULL;
	}
}

bool LSHIndex_fullycheck(const LSHIndex *in){
	int i;
	if(in == NULL || in->l <= 0 || in->m <= 0
		|| in->n <= 0 || in->d <= 0 || in->lsh == NULL || in->code == NULL){
		return false;
	}else{
		for(i = 0; i < in->l; i++){
			if(!LSH_fullycheck(&in->lsh[i]) || in->code[i] == NULL){
				return false;
			}
		}
	}

	return true;
}

void LSHIndex_encode(LSHIndex *index, const fDataSet *ds){
	/* checkout the index and the dataset */
	ASSERTINFO(!LSHIndex_fullycheck(index) || ds == NULL, "IPP");
	int	i;
	for(i = 0; i < index->l; i++){
		LSH_encode(&index->lsh[i], ds, index->code[i]);
	}
}

DyArray *LSHIndex_generate_binreg(LSHIndex *in){
	ASSERTINFO(!LSHIndex_fullycheck(in), "IPP");
	int 	m = in->m;
	int 	nbit = in->nbit;
	int 	n = in->n;
	int 	nBin = (int)pow(2.0, nbit);
	int 	i, ni, mi;
	int 	proj_value;

	/* prepare space */
	DyArray	*binreg = (DyArray*)malloc(sizeof(DyArray) * m * nbit);
	ASSERTINFO(binreg == NULL, "failed to allocate space for binreg list");

	/* init registers */
	for(i = 0; i < m * nbit; i++){
		DyArray_Init(&binreg[i], sizeof(int), n / nBin);
	}

	/* do regist */
	for(ni = 0; ni < n; ni++){
		for(mi = 0; mi < m; mi++){
			// the mi-th code of the ni-th object
			proj_value = in->code[0][ni*m+mi];
			// which is also the bin-id that the ni-th object locates
			DyArray_Add(&binreg[mi * nBin + proj_value], &ni, 1);
		}
	}

	return binreg;
}

int **LSHIndex_export_encode(const LSHIndex *in, const fDataSet *ds){
	/* checkout the index and the dataset */
	ASSERTINFO(in == NULL || in->lsh == NULL || ds == NULL, "IPP");

	int i;
	int n = ds->n;

	int **code = (int**)malloc(sizeof(int*) * in->l);
	ASSERTINFO(code == NULL, "failed to allocate space for codes");

	for(i = 0; i < in->l; i++){
		code[i] = (int*)malloc(sizeof(int) * n * in->m);
		ASSERTINFO(code[i] == NULL, "failed to allocate space for codes");

		LSH_encode(&in->lsh[i], ds, code[i]);
	}

	return code;
}

void LSHIndex_into_file(LSHIndex *in, const char *indexfolder, const char *dsname, int _idt){
	ASSERTINFO(!LSHIndex_fullycheck(in) || indexfolder == NULL, "IPP");
	char	foldername[255], filename[255];
	FILE	*fp;
	int	li, ni, mi, bini;
	int 	nBin;
	int 	_count;
	DyArray 	*binreg = NULL;

	/* check and create folder for index */
	if(-1 == access(indexfolder, F_OK)){
		mkdir(indexfolder, 0755);
	}else{
		sprintf(foldername, "%s/%s_l%d_m%d_T%.6f_w%.6f", indexfolder, dsname, in->l, in->m, in->T, in->w);
		if(-1 == access(foldername, F_OK)){
			mkdir(foldername, 0755);
		}
	}

	if(LSHIndexData_codes && _idt){		/* check to write code data */
		/* write into files: parameters and code */
		sprintf(filename, "%s/para.lsh", foldername);
		fp = open_file(filename, "wb");
		for(li = 0; li < in->l; li++){
			fwrite(in->lsh[li].a, sizeof(float), in->d*in->m, fp);
			fwrite(in->lsh[li].b, sizeof(float), in->m, fp);
		}
		fclose(fp);
		sprintf(filename, "%s/code.lsh", foldername);
		fp = open_file(filename, "wb");
		for(li = 0; li < in->l; li++){
			fwrite(in->code[li], sizeof(int), in->n*in->m, fp);
		}
		fclose(fp);

		/* --- write into text files, for verify --- */
		sprintf(filename, "%s/para.txt", foldername);
		fp = open_file(filename, "w");
		for(li = 0; li < in->l; li++){
			for(mi = 0; mi < in->m; mi++){
				fvec_fprintf(fp, in->lsh[li].a+mi*in->d, in->d, " %f");
				fputc('\n', fp);
			}
			fvec_fprintf(fp, in->lsh[li].b, in->m, " %f");
		}
		fclose(fp);
		sprintf(filename, "%s/code.txt", foldername);
		fp = open_file(filename, "w");
		for(li = 0; li < in->l; li++){
			for(ni = 0; ni < in->n; ni++){
				ivec_fprintf(fp, in->code[li]+ni*in->m, in->m, " %d");
				fputc('\n', fp);
			}
		}
		fclose(fp);
	}else if(LSHIndexData_binreg && _idt){	/* check to write binreg data */
		// first to generate binreg data
		binreg = LSHIndex_generate_binreg(in);
		// check and create folder for binreg data
		sprintf(foldername, "%s/%s_l%d_m%d_T%.6f_w%.6f/%s", indexfolder, dsname, in->l, in->m, in->T, in->w, LSHIndexData_BinReg_Path);
		if(-1 == access(foldername, F_OK)){
			mkdir(foldername, 0755);
		}


		// write into files
		nBin = (int)pow(2.0, in->nbit);
		for(mi = 0; mi < in->m; mi++){
			for(bini = 0; bini < nBin; bini++){
				sprintf(filename, "%s/m%d_bin%d.reg", foldername, mi, bini);
				fp = open_file(filename, "wb");
				_count = binreg[mi*in->m+bini].count;
				fwrite(&_count, sizeof(int), 1, fp);
				fwrite((int*)binreg[mi*nBin+bini].elem, sizeof(int), _count, fp);
				fclose(fp);
			}
		}

		free(binreg);	binreg = NULL;
	}
}

bool LSHIndex_check_exists(const char *indexfolder, const char *dsname, int l, int m, float T, float w, int _idt){
	/* regular check */
	ASSERTINFO(indexfolder == NULL || dsname == NULL || l <= 0 || m <= 0 || w <= 0, "IPP");
	char filename[255];

	/* default check: code data: both the parameter file and the code file */
	sprintf(filename, "%s/%s_l%d_m%d_T%.6f_w%.6f/para.lsh", indexfolder, dsname, l, m, T, w);
	if(-1 == access(filename, F_OK)){
		return false;
	}
	sprintf(filename, "%s/%s_l%d_m%d_T%.6f_w%.6f/code.lsh", indexfolder, dsname, l, m, T, w);
	if(-1 == access(filename, F_OK)){
		return false;
	}

	/** 	\brief:		extra check for binreg data file
	 *	\notice:		only check the first file
	 */
	if(LSHIndexData_binreg && _idt){
		sprintf(filename, "%s/%s_l%d_m%d_T%.6f_w%.6f/%s/m0_bin0.reg", indexfolder, dsname, l, m, T, w, LSHIndexData_BinReg_Path);
		if(-1 == access(filename, F_OK)){
			return false;
		}
	}
	return true;
}

bool LSHIndex_from_file(LSHIndex *in, const char *indexfolder, const char *dsname, int _idt){
	ASSERTINFO(in == NULL || indexfolder == NULL || dsname == NULL, "IPP");
	/* check if exists LSH index */
	if(!LSHIndex_check_exists(indexfolder, dsname, in->l, in->m, in->T, in->w, _idt)){
		return false;
	}
	char	file[255];
	FILE	*fp;
	int	li;

	/* default to load parameters */
	sprintf(file, "%s/%s_l%d_m%d_T%.6f_w%.6f/para.lsh", indexfolder, dsname, in->l, in->m, in->T, in->w);
	fp = open_file(file, "rb");
	for(li = 0; li < in->l; li++){
		fread(in->lsh[li].a, sizeof(float), in->m*in->d, fp);
		fread(in->lsh[li].b, sizeof(float), in->m, fp);
	}
	fclose(fp);

	/* choose to load codes */
	if(LSHIndexData_codes && _idt){
		sprintf(file, "%s/%s_l%d_m%d_T%.6f_w%.6f/code.lsh", indexfolder, dsname, in->l, in->m, in->T, in->w);
		fp = open_file(file, "rb");
		for(li = 0; li < in->l; li++){
			fread(in->code[li], sizeof(int), in->n*in->m, fp);
		}
		fclose(fp);
	}else{
		/* if not load codes, release the space */
		LSHIndex_release_codes(in);
	}

	return true;
}
