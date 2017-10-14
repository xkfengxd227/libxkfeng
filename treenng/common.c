/**
 *	\brief implementation of some common operations
 */
#include "common.h"
#include "cost.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <yael/vector.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

void fDataSet_init(fDataSet *ds, int n, int d){
    ASSERTINFO(ds == NULL || n <= 0 || d <= 0, "IPP");
    ds->d = d;
    ds->n = n;
    ds->data = (float*)malloc(sizeof(float)*n*d);
    ASSERTINFO(ds->data == NULL, "failed to allocate space for data points");
}
void iDataSet_init(iDataSet *ds, int n, int d){
    ASSERTINFO(ds == NULL || n <= 0 || d <= 0, "IPP");
    ds->d = d;
    ds->n = n;
    ds->data = (int*)malloc(sizeof(int)*n*d);
    ASSERTINFO(ds->data == NULL, "failed to allocate space for data points");
}

void fDataSet_unset(fDataSet *ds){
	if(ds != NULL && ds->data != NULL){
		free(ds->data); ds->data = NULL;
		ds->d = -1;
		ds->n = -1;
	}
}
void iDataSet_unset(iDataSet *ds){
	if(ds != NULL && ds->data != NULL){
        		free(ds->data); ds->data = NULL;
        		ds->d = -1;
		ds->n = -1;
        	}
}

void fDataSet_subdimset(const fDataSet *ds, int from, int to, fDataSet *subds){
    /// check the validity of parameters
    ASSERTINFO(ds == NULL || ds->data == NULL, "extract subset: invalid data set");
    ASSERTINFO(from < 0 || to > ds->d || from > to, "extract subset: invalid range");

    int n = ds->n;
    int d = ds->d;
    int subd = to - from;       /** the dimension of subset */

    /// prepare for sub-space
    if(subds == NULL){
        subds = (fDataSet*)malloc(sizeof(fDataSet));
        ASSERTINFO(subds == NULL, "extract subset: failed to allocate new space for subset");
        subds->n = n;
        subds->d = subd;
        subds->data = (float*)malloc(sizeof(float)*subd*n);
        ASSERTINFO(subds->data == NULL, "extract subset: failed to allocate new space for points in subset");
    }

    /// copy to subset
    int ni;
    for(ni = 0; ni < n; ni++){
        memcpy(subds->data+ni*subd, ds->data+ni*d+from, sizeof(float)*subd);
    }
}
void iDataSet_subdimset(const iDataSet *ds, int from, int to, iDataSet *subds){
    /// check the validity of parameters
    ASSERTINFO(ds == NULL || ds->data == NULL, "extract subset: invalid data set");
    ASSERTINFO(from < 0 || to > ds->d || from > to, "extract subset: invalid range");

    int n = ds->n;
    int d = ds->d;
    int subd = to - from;       /** the dimension of subset */

    /// prepare for sub-space
    if(subds == NULL){
        subds = (iDataSet*)malloc(sizeof(iDataSet));
        ASSERTINFO(subds == NULL, "extract subset: failed to allocate new space for subset");
        subds->n = n;
        subds->d = subd;
        subds->data = (int*)malloc(sizeof(int)*subd*n);
        ASSERTINFO(subds->data == NULL, "extract subset: failed to allocate new space for points in subset");
    }

    /// copy to subset
    int ni;
    for(ni = 0; ni < n; ni++){
        memcpy(subds->data+ni*subd, ds->data+ni*d+from, sizeof(int)*subd);
    }
}

void fDataSet_load_part(const char *filename, int d, int from, int num, float *vec){
	ASSERTINFO(filename == NULL || d <= 0 || from < 0 || num <= 0 || vec == NULL, "IPP");

	int 	count = (d+1) * num;
	FILE	*fp = open_file (filename, "rb");
	
	fseek(fp, from*(d+1)*sizeof(float), SEEK_SET);
	ASSERTINFO(count != fread(vec, sizeof(float), count, fp), "read error");
	fclose(fp);
}
void iDataSet_load_part(const char *filename, int d, int from, int num, int *vec){
	ASSERTINFO(filename == NULL || d <= 0 || from < 0 || num <= 0 || vec == NULL, "IPP");

	int 	count = (d+1) * num;
	FILE	*fp = open_file (filename, "rb");
	
	fseek(fp, from*(d+1)*sizeof(int), SEEK_SET);
	ASSERTINFO(count != fread(vec, sizeof(int), count, fp), "read error");
	fclose(fp);
}

void fDataSet_load_partd(const char *filename, int d, int pd, int num, float *vec){
	ASSERTINFO(filename == NULL || d <= 0 || pd <= 0 || pd > d || num <= 0 || vec == NULL, "IPP");
	FILE *fp;
	int i;
	float *fvec = fvec_new(d+1);

	fp = open_file(filename, "rb");
	for(i = 0; i < num; i++){
		ASSERTINFO((d+1) != fread(fvec, sizeof(float), d+1, fp), "read a point error");
		memcpy(vec+pd*i, fvec+1, sizeof(float) * pd);
	}
	fclose(fp);
	free(fvec); fvec = NULL;
}
void iDataSet_load_partd(const char *filename, int d, int pd, int num, int *vec){
	ASSERTINFO(filename == NULL || d <= 0 || pd <= 0 || pd > d || num <= 0 || vec == NULL, "IPP");
	FILE *fp;
	int i;
	int *ivec = ivec_new(d+1);

	fp = open_file(filename, "rb");
	for(i = 0; i < num; i++){
		ASSERTINFO((d+1) != fread(ivec, sizeof(int), d+1, fp), "read a point error");
		memcpy(vec+pd*i, ivec+1, sizeof(int) * pd);
	}
	fclose(fp);
	free(ivec); ivec = NULL;
}

DoubleIndex *dilist_new(long n){
	ASSERTINFO(n <= 0, "IPP");
	DoubleIndex *dilist = (DoubleIndex *)malloc(sizeof(DoubleIndex) * n);
	ASSERTINFO(dilist == NULL, "failed to allocate space for a DoubleIndex list");
	
	return dilist;
}
void dilist_unset(DoubleIndex *dilist){
	if(dilist != NULL){
		free(dilist); dilist = NULL;
	}
}
IntIndex *iilist_new(long n){
	ASSERTINFO(n <= 0, "IPP");
	IntIndex *iilist = (IntIndex *)malloc(sizeof(IntIndex) * n);
	ASSERTINFO(iilist == NULL, "failed to allocate space for a IntIndex list");
	
	return iilist;
}
void iilist_unset(IntIndex *iilist){
	if(iilist != NULL){
		free(iilist); iilist = NULL;
	}
}
 
int iicompare_asc(const void *va, const void *vb){
	IntIndex *iia = (IntIndex*)va;
	IntIndex *iib = (IntIndex*)vb;

	if(iia->val < iib->val){
		return -1;
	}else if (iia->val == iib->val){
		return 0;
	}else{
		return 1;
	}
}
int iicompare_des(const void *va, const void *vb){
	return 0-iicompare_asc(va, vb);
}

int dicompare_asc(const void *va, const void *vb){
	DoubleIndex *dia = (DoubleIndex*)va;
	DoubleIndex *dib = (DoubleIndex*)vb;

	if(f_bigger(dia->val, dib->val)){
		return 1;
	}else if(f_bigger(dib->val, dia->val)){
		return -1;
	}else{
		return 0;
	}
}
int dicompare_des(const void *va, const void *vb){
	return 0 - dicompare_asc(va, vb);
}

long timediff(struct timeval begin, struct timeval end)
{
    return ((end.tv_sec - begin.tv_sec) * 1000000 + (end.tv_usec - begin.tv_usec));
}

double odistance_square(const float *a, const float *b, int d)
{
    double dis = 0;
    int i;
    for(i = 0; i < d; i++)
    {
        dis = dis + ((double)a[i]-(double)b[i]) * ((double)a[i]-(double)b[i]);
    }
    return dis;
}

double odistance(const float *a, const float *b, int d)
{
    return sqrt(odistance_square(a, b, d));
}

float inner_product(float *a, float *b, int d)
{
    float rslt = 0;
    int i;
    for(i = 0; i < d; i++)
    {
        rslt = rslt + a[i] * b[i];
    }
    return rslt;
}

void linear_knn(float *vb, int nb, float *q, int d, int k, int *nn, float *dis){
	__assertinfo(vb == NULL || nb <= 0 || q == NULL || d <= 0 || k <= 0 || k > nb 
		|| nn == NULL || dis == NULL, "IPP");
	DoubleIndex		*dilist = dilist_new(nb);
	int 			i;
	
	/* init */
	for(i = 0; i < nb; i++){
		dilist[i].id = i;
		dilist[i].val = odistance_square(vb+i*d, q, d);
	}
	/* sort */
	DI_MergeSort(dilist, 0, nb-1);
	
	/* extract results */
	for(i = 0; i < k; i++){
		nn[i] = dilist[i].id;
		dis[i] = sqrt(dilist[i].val);
	}
}

void extract_members(int *assign, int *nassign, int **member, int n, int ncenter)
{
	int i;
	int *pos = ivec_new_0(ncenter);

	// allocate space
	int total = 0;
	for(i = 0; i < ncenter; i++)
	{
		member[i] = ivec_new_set(nassign[i], -1);

		total = total + nassign[i];
	}
	printf("total: %d\n", total);

	// extract the id of the data points belonging to each cluster
	int icenter = -1;
	for(i = 0; i < n; i++)
	{
		// the i-th data point belongs to the icenter-th cluster
		icenter = assign[i];
		// add the i-th data point to the end of the list of the icenter-th cluster's members
		member[icenter][pos[icenter]] = i;
		pos[icenter] ++;
	}

	free(pos); pos = NULL;
}
// compare two float number, return true when a > b
int f_bigger(float a, float b)
{
    return ((a-b > FLOAT_ZERO) ? 1:0);
}

bool f_iszero(float val){
	return (fabs(val) <= FLOAT_ZERO);
}

int i_min(int va, int vb){
	return (va < vb) ? va : vb;
}
int i_max(int va, int vb){
	return (va+vb)-i_min(va, vb);
}
float f_min(float va, float vb){
	return (va+vb)-f_max(va, vb);
}
float f_max(float va, float vb){
	return (f_bigger(va, vb)) ? va : vb;
}

void ivec_abs(int *vec, long d){
	long i;
	for(i = 0; i < d; i++){
		vec[i] = abs(vec[i]);
	}
}
void fvec_abs(float *vec, long d){
	long i;
	for(i = 0; i < d; i++){
		vec[i] = fabs(vec[i]);
	}
}
double ivec_fmean(const int *vec, long d){
	long i, sum;
	sum = 0;
	for(i = 0; i < d; i++){
		sum += vec[i];
	}
	
	return (double)sum / (double)d;
}
long ivec_norm2sqr(const int *vec, long d){
	long norm = 0;
	long i;
	for(i = 0; i < d; i++){
		norm += vec[i] * vec[i];
	}
	return norm;
}
int ivecs_read(const char *filename, int d, int n, int *data){
	ASSERTINFO(filename == NULL || d <= 0 || n <= 0 || data == NULL, "IPP");
	FILE 	*fp = open_file (filename, "r");
	int 	i;
	int 	new_d;

	for(i = 0; i < n; i++){
		fread(&new_d, sizeof(int), 1, fp);
		fread(data+i*d, sizeof(int), d, fp);
	}
	
	fclose(fp);
	return i;
}


FILE *open_file(const char *filename, const char *format)
{
    FILE *fp;
    fp = fopen(filename, format);
    if(fp == NULL)
    {
        printf("failed to open file %s\n", filename);
        exit(-1);
    }
    return fp;
}
void makesure_folder(const char *folder){
	ASSERTINFO(folder == NULL || strlen(folder) == 0, "IPP");
	if(-1 == access(folder, F_OK)){
		ASSERTINFO(-1 ==  mkdir(folder, 0755), "failed to create a new folder");
	}
}
bool file_exists(const char *filename){
	ASSERTINFO(filename == NULL, "IPP");
	return (-1 != access(filename, F_OK));
}

void DI_MergeSort(DoubleIndex *di, int l, int r)
{
	int m;
    if (l < r)                                                                                                          // split
    {
        m = (l + r) / 2;                                                                                            // the middle position
        DI_MergeSort(di, l, m);                                                                                     // merge the left side
        DI_MergeSort(di, m + 1, r);                                                                                 // merge the right side
        DI_Merge(di, l, m, r);                                                                                      // merge two sides
    }
}

void DI_Merge(DoubleIndex *di, int l, int m, int r)
{
	int i;
    ///
    /// the length of two sides
    ///
    int llen = m - l + 1;
    int rlen = r - m;
    DoubleIndex *llb = (DoubleIndex*)malloc(sizeof(DoubleIndex) * (llen+1));
    DoubleIndex *rlb = (DoubleIndex*)malloc(sizeof(DoubleIndex) * (rlen+1));

    for (i = 0; i < llen; i++)
    {
    	memcpy(&llb[i], &di[l+i], sizeof(DoubleIndex));
    }
    for (i = 0; i < rlen; i++)
    {
    	memcpy(&rlb[i], &di[m + 1 + i], sizeof(DoubleIndex));
    }
    ///
    /// add a maximum value at the end of each group
    ///
    llb[llen].id = llen + 1;
    llb[llen].val = FLOAT_MAX;
    rlb[rlen].id = rlen + 1;
    rlb[rlen].val = FLOAT_MAX;

    int li = 0,
        ri = 0,
        k = 0;
    for (i = l; i <= r; i++)
    {
        if (f_bigger(rlb[ri].val, llb[li].val))                                            // left is smaller
        {
        	memcpy(&di[l+k], &llb[li], sizeof(DoubleIndex));
            li++;
            k++;
        }
        else
        {
        	memcpy(&di[l+k], &rlb[ri], sizeof(DoubleIndex));
            ri++;
            k++;
        }
    }

    free(rlb); rlb = NULL;
    free(llb); llb = NULL;
}




/* ---------------------------------------------------------------------------------
 *	the groundtruth block
 * --------------------------------------------------------------------------------- */

void load_groundtruth(const char *gtfile, const char *gtdisfile, int dGT, int nq, DoubleIndex *groundtruth)
{
    FILE *fp;
    int i, ki, nret;
    int *istream = NULL;
    float *fstream = NULL;
    int *ps = NULL;
    float *fps = NULL;

    int d;
    
    /* the ids */
    fp = open_file(gtfile, "rb");
    fread(&d, sizeof(int), 1, fp);
    if(dGT > d)
    {
        printf("not enough groundtruth\n");
        fclose(fp);
        exit(-1);
    }
    istream = ivec_new(nq*(d+1));
    fseek(fp, 0, SEEK_SET);
    nret = fread(istream, sizeof(int), (nq*(d+1)), fp);
    fclose(fp);
    ps = istream + 1;
    for(i = 0; i < nq; i++)
    {
        for(ki = 0; ki < dGT; ki++)
        {
            memcpy(&groundtruth[i*dGT+ki].id, ps+ki, sizeof(int));
        }
        ps = ps + d + 1;
    }
    free(istream);
    istream = NULL;
    ps = NULL;
    
    /* the gtdistance */
    fp = open_file(gtdisfile, "rb");
    fread(&d, sizeof(int), 1, fp);
    if(dGT > d)
    {
        printf("not enough groundtruth\n");
        fclose(fp);
        exit(-1);
    }
    fstream = fvec_new(nq*(d+1));
    fseek(fp, 0, SEEK_SET);
    nret = fread(fstream, sizeof(float), (nq*(d+1)), fp);
    fclose(fp);
    
    fps = fstream + 1;
    for(i = 0; i < nq; i++)
    {
        for(ki = 0; ki < dGT; ki++)
        {
        	groundtruth[i*dGT+ki].val = (double)(*(fps+ki));
        }
        fps = fps + d + 1;
    }
    free(fstream);
    fstream = NULL;
    fps = NULL; 
}

void calculate_groundtruth(float *data, float *query, int n, int d, int nq, int nk, DoubleIndex *groundtruth)
{
    DoubleIndex *knn = (DoubleIndex*)malloc(sizeof(DoubleIndex)*n);
    float *q = fvec_new(d);
    float *x = fvec_new(d);
    int i, j;
    for(i = 0; i < nq; i++)
    {
        memcpy(q, query+i*d, sizeof(float)*d);
        for(j = 0; j < n; j++)
        {
            memcpy(x, data+j*d, sizeof(float)*d);
            knn[j].id = j;
            knn[j].val = odistance(q, x, d);
        }
        DI_MergeSort(knn, 0, n-1);
        memcpy(groundtruth+i*nk, knn, sizeof(DoubleIndex)*nk);
    }
    free(knn); knn = NULL;
    free(q); q = NULL;
    free(x); x = NULL;

    FILE *fp = open_file("1000.groundtruth", "w");
    for(i = 0; i < nq; i++)
    {
        // fprintf(fp, "%d", d);
        for(j = 0; j < nk; j++)
        {
            fprintf(fp, "%d ", groundtruth[i*nk+j].id);
        }
        fputc('\n', fp);
    }
    fclose(fp);
}

void fullfill_gtdistance(DoubleIndex *gt, int nq, int dGT, const float *data, const float *query, int d){
	ASSERTINFO(gt == NULL || nq <= 0 || dGT <= 0 || data == NULL || query == NULL, "IPP");

	int i, qi;
	for(i = 0; i < nq * dGT; i++){
		qi = i / dGT;
		gt[i].val = odistance(data+gt[i].id*d, query + qi * d, d);
	}
}




/* ---------------------------------------------------------------------------------
 *	the accuracy block: make sure ids and distances in knn and gt are prepared
 *	therefore, we set knn and gt be constant
 * --------------------------------------------------------------------------------- */
float knn_precision(DoubleIndex **knn, const DoubleIndex *gt, int nq, int nk, int dGT){
	ASSERTINFO(knn == NULL || gt == NULL || nq <= 0 || nk <= 0 || dGT <= 0, "IPP");
	ASSERTINFO(nk > dGT, "impossible: nk exceeds dGT");
	
	int		count;
	int		qi, ki, gi;
	int 	nnid;
	
	count = 0;
	for(qi = 0; qi < nq; qi++){
		for(gi = 0; gi < nk; gi++){
			// for a real nk-nn's id
			nnid = gt[qi*dGT+gi].id;
			// whether has been retrieved
			for(ki = 0; ki < nk; ki++){
				if(knn[qi][ki].id == nnid){
					count++;
					break;
				}
			}
		}
	}
	
	return (float)count / nq / nk;
}

float knn_ratio(DoubleIndex **knn, const DoubleIndex *gt, int nq, int nk, int dGT, int verbose){
	ASSERTINFO(knn == NULL || gt == NULL || nq <= 0 || nk <= 0 || dGT <= 0, "IPP");	
	ASSERTINFO(nk > dGT, "nk exceeds dGT");

	float		ratio, iratio;
	int			qi, ik;
	int			_valid_count;
	
	ratio = 0;
	for(qi = 0; qi < nq; qi++){
		// init counters
		iratio = 0;
		_valid_count = nk;

		// statistic
		for(ik = 0; ik < nk; ik++){
			//ASSERTINFO(gt[qi*dGT+ik].id != knn[qi][ik].id 
			//	&& f_bigger(gt[qi*dGT+ik].val, knn[qi][ik].val), 
			//	 "impossible, gt greater than real results");
			if(gt[qi*dGT+ik].id != knn[qi][ik].id && f_bigger(gt[qi*dGT+ik].val, knn[qi][ik].val)){
				printf("%d-%d: gt-(%d, %f), knn-(%d, %f)\n", qi, ik, gt[qi*dGT+ik].id, gt[qi*dGT+ik].val,
					knn[qi][ik].id, knn[qi][ik].val);
			}
				

			// check if id matches
			if(gt[qi*dGT+ik].id == knn[qi][ik].id){
				iratio += 1;
			}else{
				// check if groundtruth distance is zero
				if(f_iszero(gt[qi*dGT+ik].val)){
					if(f_iszero(knn[qi][ik].val)){
						iratio += 1;
					}else{
						// make this in-valid
						_valid_count--;
					}
				}else{
					iratio += knn[qi][ik].val / gt[qi*dGT+ik].val;
				}
			}
		}

		if(verbose){
			printf("qi-%d: %g\t", qi, iratio / _valid_count);
		}
		ratio += iratio / _valid_count;
	}

	return ratio / nq;	
}

float knn_recall(DoubleIndex **knn, const DoubleIndex *gt, int nq, int nk, int dGT){
	ASSERTINFO(knn == NULL || gt == NULL || nq <= 0 || nk <= 0 || dGT <= 0, "IPP");
	ASSERTINFO(nk > dGT, "impossible: nk exceeds dGT");

	int		count;
	int 	qi, pos;
	/* self-define shakerange to seek nn */
	//int		shakerange = i_min(nk-1, SHAKERANGE);
	/* seek all nk-nn for nn */
	int 		shakerange = nk-1;
	int		nnid;
	
	count = 0;
	for(qi = 0; qi < nq; qi++){
		nnid = gt[qi*dGT].id;
		if(knn[qi][0].id == nnid){
			count++;
		}else{
			for(pos = 1; pos <= shakerange; pos++){
				if(knn[qi][pos].id == nnid){
					count ++;
					break;
				}
			}
		}
	}
	return (float)count / nq;
}

float knn_recall_at(DoubleIndex **knn, const DoubleIndex *gt, int nq, int nk, int dGT, int at){
	ASSERTINFO(knn == NULL || gt == NULL || nq <= 0 || nk <= 0 || dGT <= 0 || at <= 0, "IPP");
	ASSERTINFO(nk > dGT || at > nk, "impossible: nk exceeds dGT or at exceeds nk");
	
	int		count;
	int		qi, ki, ai;
	int 	nnid;
	bool	found;
	count = 0;
	
	for(qi = 0; qi < nq; qi++){
		found = false;
		for(ai = 0; ai < at; ai++){
			nnid = gt[qi*dGT+ai].id;
			for(ki = 0; ki < nk; ki++){
				if(knn[qi][ki].id == nnid){
					found = true;
					break;
				}
			}
			if(found){
				count++;
				break;
			}
		}
	}
	
	return (float)count / nq ;
}



float *fvec_sign(const float *vec, int d){
	int i;
	float *sgnv = (float*)malloc(sizeof(float)*d);
	for(i = 0; i < d; i++){
		sgnv[i] = (f_bigger(vec[i], FLOAT_ZERO)) ? 1. : -1.;
	}
	return sgnv;
}

PARA_TYPE get_para_type(const char *para){
        PARA_TYPE ptype = PNULL;
        if(para != NULL && strlen(para) > 0){
                if(para[0] == '-'){
                        ptype = PNAME;
                }else{
                        ptype = PVALUE;
                }
        }
        return ptype;
}

void extract_para_name(const char *para, char *pname){
        memset(pname, 0, sizeof(char)*255);
        memcpy(pname, para+1, sizeof(char)*(strlen(para)-1));
}

void performance_report(const DoubleIndex **knn, int nq, int k, const DoubleIndex *gt, int dGT, Cost cost, const char *envstr){
	FILE *fp;
	/* --------------------- report the performance ------------------------ */
	float ratio = knn_ratio(knn, gt, nq, k, dGT, 0);
	float recall = knn_recall(knn, gt, nq, k, dGT);
	float precision = knn_precision(knn, gt, nq, k, dGT);
	puts(" ratio \t recall precision search(s) \t cpu(s) points pages");
	printf("%g\t%g\t%g\t%g\t%g\t%ld\t%ld\n", ratio, recall, precision, (double)cost.search / 1000000.0, (double)cost.cpu / 1000000.0, cost.point, cost.page);
	printf("filter %g(s), sort %g(s), refine %g(s)\n",
		(double)cost.timer[0] / 1000000.0,
		(double)cost.timer[1] / 1000000.0,
		(double)cost.timer[2] / 1000000.0);
	printf("reg io %g(s), point io %g(s)\n",
		(double)cost.timer[6] / 1000000.0,
		(double)cost.timer[7] / 1000000.0);
	printf("loaded points: %d\n", cost.counter[0]);
	
	fp = fopen(ResultLogFile, "a");
	ASSERTINFO(fp == NULL, "result log file open failed");
	fputs("--------------------------------------------------\n", fp);
	fputs(envstr, fp);
	fputs(" ratio \t recall precision search(s) \t cpu(s) points pages\n", fp);
	fprintf(fp, "%g\t%g\t%g\t%g\t%g\t%ld\t%ld\n", ratio, recall, precision, (double)cost.search / 1000000.0, (double)cost.cpu / 1000000.0, cost.point, cost.page);
	fprintf(fp, "filter %g(s), sort %g(s), refine %g(s)\n", 
		(double)cost.timer[0] / 1000000.0,
		(double)cost.timer[1] / 1000000.0,
		(double)cost.timer[2] / 1000000.0);
	fprintf(fp, "reg io %g(s), point io %g(s)\n\n",
		(double)cost.timer[6] / 1000000.0,
		(double)cost.timer[7] / 1000000.0);
	fprintf(fp, "loaded points: %d\n", cost.counter[0]);
	fclose(fp);
}
