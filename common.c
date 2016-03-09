/**
 *	\brief implementation of some common operations
 */
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <yael/vector.h>
#include <sys/time.h>

void fDataSet_Init(fDataSet *ds, int n, int d){
    ASSERTINFO(ds == NULL, "fDataSet Init: a null dataset that cannot be initialized");
    ds->d = d;
    ds->n = n;
    ds->data = (float*)malloc(sizeof(float)*n*d);
    ASSERTINFO(ds->data == NULL, "fDataSet Init: failed to allocate space for data points");
}

void fDataSet_Reset(fDataSet *ds){
    ASSERTINFO(ds == NULL, "fDataSet Reset: a null dataset that cannot be reset");
    if(ds->data != NULL){
        free(ds->data); ds->data = NULL;
    }
    ds->d = -1;
    ds->n = -1;
}

void fDataSet_Subset(const fDataSet *ds, int from, int to, fDataSet *subds){
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


long timediff(struct timeval begin, struct timeval end)
{
    return ((end.tv_sec - begin.tv_sec) * 1000000 + (end.tv_usec - begin.tv_usec));
}

float odistance_square(const float *a, const float *b, int d)
{
    float dis = 0;
    int i;
    for(i = 0; i < d; i++)
    {
        dis = dis + (a[i]-b[i]) * (a[i]-b[i]);
    }
    return dis;
}

float odistance(const float *a, const float *b, int d)
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

	FILE *fp = fopen("member.txt", "w");
	int j;
	for(i = 0; i < ncenter; i++)
	{
		fprintf(fp, "%d:%d\t", i, pos[i]);
		for(j = 0; j < pos[i]; j++)
		{
			fprintf(fp, " %d", member[i][j]);
		}
		fputc('\n', fp);
	}
	fclose(fp);

	free(pos); pos = NULL;
}
// compare two float number, return true when a > b
int f_bigger(float a, float b)
{
    return ((a-b > FLOAT_ZERO) ? 1:0);
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

void load_groundtruth(char *filename, int dGT, int nq, DoubleIndex *groundtruth)
{
    FILE *fp;
    fp = open_file(filename, "rb");
    int i, ki, nret;
    int *stream = NULL;
    int *ps = NULL;

    int d;
    fread(&d, sizeof(int), 1, fp);
    if(dGT > d)
    {
        printf("not enough groundtruth\n");
        fclose(fp);
        exit(-1);
    }
    stream = ivec_new(nq*(d+1));
    fseek(fp, 0, SEEK_SET);
    nret = fread(stream, sizeof(int), (nq*(d+1)), fp);
    printf("d: %d, read: %d\n", d, nret);

    fclose(fp);
    ps = stream + 1;
    for(i = 0; i < nq; i++)
    {
        for(ki = 0; ki < dGT; ki++)
        {
            memcpy(&groundtruth[i*dGT+ki].id, ps+ki, sizeof(int)*1);
            groundtruth[i*dGT+ki].val = -1;
        }
        ps = ps + d + 1;
    }
    free(stream);
    stream = NULL;
    ps = NULL;
}

float verify_knn(DoubleIndex **knnset, DoubleIndex *groundtruth, int nq, int nk, int dGT)
{
    int i, j, count = 0, k;
    float accuracy = 0;
    bool match = false;

    int *status = ivec_new(nq*nk);
    bool direct = false;

    for(i = 0; i < nq; i++)
    {
        // re-order
        DI_MergeSort(knnset[i], 0, nk-1);

        for(j = 0; j < nk; j++)
        {
            match = false;
            direct = true;

            if(knnset[i][j].id == groundtruth[i*dGT+j].id)
            {
                match = true;
            }
            else
            {// miss match, shake range
                for(k = j - SHAKERANGE; k <= j+SHAKERANGE; k++)
                {
                    if(k >= 0 && k < dGT && k != j)
                    {
                        if(knnset[i][j].id == groundtruth[i*dGT+k].id)
                        {
                            match = true;
                            direct = false;
                        }
                    }
                }
            }
            count = count + (match ? 1 : 0);
            if(!match)
            {
                status[i*nk+j] = 0;
            }
            else if(direct)
            {
                status[i*nk+j] = 1;
            }
            else
            {
                status[i*nk+j] = 9;
            }
        }
    }

    printf("match:%d\n", count);
    accuracy = (float)count / (nq * nk);

    free(status); status = NULL;
    return accuracy;
}


float knn_match(DoubleIndex *knn, DoubleIndex *gt, int nq, int nk){
    ASSERTINFO(knn == NULL || gt == NULL, "knn_match: invalid parameters");

    /** basic parameters */
    int i, j, count = 0, k;                         /* pointers */
    float ratio = 0;                             /* accuracy to be returned */
    bool match = false;                             /* match flag */
    DoubleIndex *iknn = (DoubleIndex*)malloc(sizeof(DoubleIndex) * nk);
                                                    /* to buffer the i-th knn */
    /** evaluate ratio */
    for(i = 0; i < nq; i++)
    {
        /* re-order the i-th knn */
        memcpy(iknn, knn+i*nk, sizeof(DoubleIndex)*nk);
        DI_MergeSort(iknn, 0, nk-1);

        for(j = 0; j < nk; j++)
        {
            /** for the j-th nn, we try to locate it in the groundtruth, even with SHAKERANGE of position */
            match = false;
            if(knn[i*nk+j].id == gt[i*nk+j].id)
            {/* first check out whether the current position id matched */
                match = true;
            }
            else
            {/* id mis-matched, search around with SHAKERANGE */
                for(k = j - SHAKERANGE; k <= j+SHAKERANGE; k++)
                {
                    if(k >= 0 && k < nk && k != j)
                    {
                        if(knn[i*nk+j].id == gt[i*nk+k].id)
                        {
                            match = true;
                        }
                    }
                }
            }

            /** update match */
            count = count + (match ? 1 : 0);
        }
    }

    ratio = (float)count / (nq * nk);
    return ratio;
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
