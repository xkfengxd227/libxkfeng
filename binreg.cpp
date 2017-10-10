/** 
 *	\file 	implementation of the binreg class
 */
#include "binreg.h"
#include "dyarray.h"
#include "common.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

DyArray *BinReg_init(int m, int nBin, int _init_count){
	ASSERTINFO(m <= 0 || nBin <= 0 || _init_count <= 0, "IPP");
	int i;

	// allocate space for statistor, and initlize it
	DyArray *neighbor_stat = (DyArray*)malloc(sizeof(DyArray) * m * nBin);
	ASSERTINFO(neighbor_stat == NULL, "failed to allocate space for neighbor_stat");
	for(i = 0; i < m*nBin; i++){
		DyArray_Init(&neighbor_stat[i], sizeof(int), _init_count);
	}
	
	return neighbor_stat;
}

void BinReg_regist(DyArray *stat, int *codes, int n, int m, int nBin, int ngRange){
	ASSERTINFO(stat == NULL || codes == NULL || n <= 0 || m <= 0 || nBin <= 0, "IPP");

	int ib, icode, in, im, code, ing;
	
	// prepare lookup table
	int	*nglookup = (int*)malloc(sizeof(int)*nBin*2);
	ASSERTINFO(nglookup == NULL, "failed to allocate space for neighbor lookup table");
	for(ib = 0; ib < nBin; ib++){
		nglookup[ib*2] = i_max(0, ib - ngRange);
		nglookup[ib*2+1] = i_min(nBin, ib + ngRange);
		
		printf("bin-%d: [%d, %d]\n", ib, nglookup[ib*2], nglookup[ib*2+1]);
	}

	// regist
	for(icode = 0; icode < n * m; icode++){
		im = icode % m;
		in = icode / m;
		code = codes[icode];
		for(ing = nglookup[code*2]; ing <= nglookup[code*2+1]; ing++){
			DyArray_Add(&stat[im * nBin + ing], (void*)(&in), 1);
		}
	}
	
	free(nglookup); nglookup = NULL;
}

void BinReg_into_files(DyArray *stat, int m, int nBin, const char *root, const char *dsname, int b, int ngRange){
	FILE 	*fp;
	char	folder[255], statfile[255];
	int		im, ibin;

	// check for folder
	sprintf(folder, "%s/%s_b%d_range%d", root, dsname, b, ngRange);
	if(-1 == access(folder, F_OK)){
		mkdir(folder, 0755);
	}

	// write: split into [d*nBin] files
	for(im = 0; im < m; im++){
		for(ibin = 0; ibin < nBin; ibin++){
			// assemble the file name
			sprintf(statfile, "%s/m%d_bin%d.ng", folder, im, ibin);
			fp = open_file(statfile, "wb");
			// the count then the ids
			fwrite(&stat[im*nBin+ibin].count, sizeof(int), 1, fp);
			fwrite(stat[im*nBin+ibin].elem, sizeof(int), stat[im*nBin+ibin].count, fp);
			fclose(fp);
		}
	}
}

bool BinReg_check_exists(const char *root, const char *dsname, int b, int ngRange, int m){
	ASSERTINFO(root == NULL || dsname == NULL || b <= 0 || ngRange < 0, "IPP");
	char	folder[255], file[255];

	sprintf(folder, "%s/%s_b%d_range%d", root, dsname, b, ngRange);
	if(-1 == access(folder, F_OK)){
		mkdir(folder, 0755);
		return false;
	}else{
		// check the ng files: d0.ng, d[d-1].ng
		sprintf(file, "%s/m0_bin0.ng", folder);
		if(-1 == access(file, F_OK)){	return false; }
		sprintf(file, "%s/m%d_bin0.ng", folder, m-1);
		if(-1 == access(file, F_OK)){	return false; }
	}
	return true;
}

bool BinReg_from_files(DyArray *stat, const char *root, const char *dsname, int b, int ngRange, int m, int nBin){
	ASSERTINFO(stat == NULL || root == NULL || dsname == NULL || b <= 0 || ngRange < 0 || m <= 0 || nBin <= 0, "IPP");
	if(!BinReg_check_exists(root, dsname, b, ngRange, m)){
		return false;
	}

	char	folder[255], file[255];
	int		im, ibin, _count;
	DyArray	*pstat;
	FILE	*fp;

	// read
	sprintf(folder, "%s/%s_b%d_range%d", root, dsname, b, ngRange);
	pstat = stat;
	for(im = 0; im < m; im++){
		// read all bins
		for(ibin = 0; ibin < nBin; ibin++){
			// equip the ng file
			sprintf(file, "%s/m%d_bin%d.ng", folder, im, ibin);
			fp = open_file(file, "rb");
			// read the count in this bin
			fread(&_count, sizeof(int), 1, fp);
			// prepare enough space first, then read out
			DyArray_PrepareEnoughSpace(pstat, _count + pstat->count);
			fread(((int*)pstat->elem) + pstat->count, sizeof(int), _count, fp);
			fclose(fp);
			// update counter <important>			
			pstat->count += _count;

			pstat++;
		}
	}
	return true;
}

void BinReg_load_a_bin(DyArray *stat, const char *root, const char *dsname, int b, int ngRange, int _im, int _ibin){
	ASSERTINFO(stat == NULL || root == NULL || dsname == NULL || b <= 0 || ngRange < 0 || _im < 0 || _ibin < 0, "IPP");
	
	char	file[255];
	sprintf(file, "%s/%s_b%d_range%d/m%d_bin%d.ng", root, dsname, b, ngRange, _im, _ibin);
	FILE	*fp;
	int	_count;
	
	fp = open_file(file, "rb");
	// read the count in this bin
	fread(&_count, sizeof(int), 1, fp);
	// prepare enough space first, then read out
	DyArray_PrepareEnoughSpace(stat, _count + stat->count);
	fread(((int*)stat->elem) + stat->count, sizeof(int), _count, fp);
	fclose(fp);
	// update counter <important>			
	stat->count += _count;
}
