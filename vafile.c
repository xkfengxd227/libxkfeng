/**
 *  implementation for VA-File basic operations
 */
#include <yael/vector.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>	/* for mkdir() */
#include <unistd.h>		/* for access() */
#include "vafile.h"
#include "common.h"
#include "heap.h"
#include "cost.h"

/// initialize a VAConfig
void VA_Init(VAConfig *cfg, int bi)
{
    cfg->nbit = bi;
    cfg->n = -1;
    cfg->d = -1;	
	cfg->bound = NULL;
	cfg->inter_len = NULL;
	cfg->code = NULL;
	cfg->inter_alldiff_len_s = NULL;
}
/// destroy a VAConfig structure
void VA_Destroy(VAConfig *cfg)
{
	int		di;

    if(cfg->bound != NULL){
        free(cfg->bound); cfg->bound = NULL;
    }
    if(cfg->inter_len != NULL){
        free(cfg->inter_len); cfg->inter_len = NULL;
    }
    if(cfg->code != NULL){
        free(cfg->code); cfg->code = NULL;
    }
	if(cfg->inter_alldiff_len_s != NULL){	
		for(di = 0; di < cfg->d; di++){
			free(cfg->inter_alldiff_len_s[di]); cfg->inter_alldiff_len_s[di] = NULL;
		}
		free(cfg->inter_alldiff_len_s); cfg->inter_alldiff_len_s = NULL;
	}
}
/// dealing with a dataset, figure out the configrations
void VA_Config(VAConfig *cfg, const fDataSet *ds)
{
    /// statistic lower boundary and upper boundary of each dimension
    cfg->d = ds->d;
    cfg->n = ds->n;
    cfg->bound = fvec_new(2*ds->d);
    cfg->inter_len = fvec_new(ds->d);

    FILE *fp;

    /// 1. statistic lower and upper boundary of all dimensions
    int ni, di;
    float max, min;
    int cur;
    int ninter = (int)pow(2, cfg->nbit);
    for(di = 0; di < ds->d; di++)
    {
        max = -FLOAT_MAX;
        min = FLOAT_MAX;
        for(ni = 0; ni < ds->n; ni++)
        {
            cur = ni*ds->d+di;
            if(f_bigger(min, ds->data[cur])){
                min = ds->data[cur];
            }
            else if(f_bigger(ds->data[cur], max)){
                max = ds->data[cur];
            }
        }
        cfg->bound[2*di] = min;
        cfg->bound[2*di+1] = max;
        cfg->inter_len[di] = (max-min) / ninter;
    }
}
/// generate a va-file code for all data points in a dataset
void VA_Encode(VAConfig *cfg, const fDataSet *ds)
{
    /// generate the config
    VA_Config(cfg, ds);

    /// encode
    cfg->code = ivec_new(cfg->n*cfg->d);
    int ni, di;
    int pos;
    int ninter = (int)pow(2,cfg->nbit);
    for(ni = 0; ni < cfg->n; ni++)
    {
        /// for a data point
        for(di = 0; di < cfg->d; di++)
        {
            /// for a certain dimension
            pos = floor((ds->data[ni*ds->d+di] - cfg->bound[di*2]) / cfg->inter_len[di]);
            pos = (pos >= ninter) ? (ninter - 1) : pos;     // in case the upper boundary
            cfg->code[ni*cfg->d+di] = pos;
        }
    }
}
/// write a va-file index structure into files
void VA_IndexIntoFile(const VAConfig *cfg, const char *indexfolder, const char *dsname)
{
    int di;
    FILE *fp;
    char path[255], filename[255];

	/// check for folder
	sprintf(path, "%s/%s", indexfolder, dsname);
	if(-1 == access(path, F_OK)){
		mkdir(path, 0777);
	}

    /// write the config file: n,d,boundaries, text-format
    sprintf(filename, "%s/%s/%d.config", indexfolder, dsname, cfg->nbit);
    fp = open_file(filename, "w");
    fprintf(fp, "%d %d\n", cfg->n, cfg->d);
    for(di = 0; di < cfg->d; di++)
    {
        fprintf(fp, "%d %f %f %f\n", di, cfg->inter_len[di], cfg->bound[di*2], cfg->bound[di*2+1]);
    }
    fclose(fp);

    /// write the code file: nxd code, binary-format
    sprintf(filename, "%s/%s/%d.vafile", indexfolder, dsname, cfg->nbit);
    fp = open_file(filename, "wb");
    fwrite(cfg->code, sizeof(int), cfg->d*cfg->n, fp);
    fclose(fp);
}
bool VA_CheckExistsIndex(const char *indexfolder, const char *dsname, int b){
	char	filename[255];
	/// check the config file
	sprintf(filename, "%s/%s/%d.config", indexfolder, dsname, b);
	if(-1 == access(filename, F_OK)){
		return false;
	}else{
		sprintf(filename, "%s/%s/%d.vafile", indexfolder, dsname, b);
		if(-1 == access(filename, F_OK)){
			return false;
		}
	}
	return true;
}
bool VA_IndexFromFile(VAConfig *cfg, const char *indexfolder, const char *dsname, int b){
	/// first to make sure the index exist
	if(!VA_CheckExistsIndex(indexfolder, dsname, b)){
		return false;
	}
	
	char	filename[255];
	int		di, temp_int;
	FILE	*fp;

	/// ok to load the index
	// first, the config file (text format): n, d
	cfg->nbit = b;
	sprintf(filename, "%s/%s/%d.config", indexfolder, dsname, cfg->nbit);
	fp = open_file(filename, "r");
	fscanf(fp, "%d %d", &cfg->n, &cfg->d);
	
	// the allocate space for boundaries and interval lengths
	cfg->inter_len = (float *)malloc(sizeof(float) * cfg->d);
	ASSERTINFO(cfg->inter_len == NULL, "VA_IndexFromFile: failed to allocate space for interval lengths");
	cfg->bound = (float *)malloc(sizeof(float) * 2 * cfg->d);
	ASSERTINFO(cfg->bound == NULL, "VA_IndexFromFile: failed to allocate space for boundaries");
	// load interval lengths and boundaries
	for(di = 0; di < cfg->d; di++)
	{	
		fscanf(fp, "%d %f %f %f", &temp_int, &cfg->inter_len[di], &cfg->bound[di*2], &cfg->bound[di*2+1]);
	}
	fclose(fp);

	/// read the codes: nxd code, binary-format
	cfg->code = (int*)malloc(sizeof(int) * cfg->d *cfg->n);
	ASSERTINFO(cfg->code == NULL, "VA_IndexFromFile: failed to allocate space for codes");
	sprintf(filename, "%s/%s/%d.vafile", indexfolder, dsname, cfg->nbit);
	fp = open_file(filename, "rb");
	fread(cfg->code, sizeof(int), cfg->d*cfg->n, fp);
	fclose(fp);

	return true;
}

/// search knn via vafile
void VA_Search(const VAConfig *cfg, const fDataSet *qds, int nk, const char *datafile, DoubleIndex **knn, Cost *cost)
{
    int qi, di, i;
    int nq = qds->n;
    int d = qds->d;
    int ninter = (int)pow(2,cfg->nbit);
    float *query = fvec_new(d);

    VAElem *elem, *rough;
    int rough_len;
    DoubleIndex *candidate;
    Heap h;
    bool stop;
    FILE *fp;
    float *x = fvec_new(d);
    DoubleIndex can_di;
    double knn_R;
    Cost costi;
    struct timeval tvb, tve, tvb_io, tve_io;
    long tablediff;
    int point_per_page = PAGESIZE/(d*sizeof(float));

    /// distance table, boundary
    float **sdisqb = (float **)malloc(sizeof(float*)*d);
    float **bound = (float **)malloc(sizeof(float*)*d);

    gettimeofday(&tvb, NULL);
    for(di = 0; di < d; di++)
    {
        sdisqb[di] = fvec_new(ninter+1);
        bound[di] = fvec_new(ninter+1);
        for(i = 0; i < ninter+1; i++)
        {
            sdisqb[di][i] = -1;
            bound[di][i] = cfg->inter_len[di]*i+cfg->bound[di*2];
        }
    }
    gettimeofday(&tve, NULL);
    tablediff = timediff(tvb, tve);

    /// execute search
    fp = open_file(datafile, "rb");
    for(qi = 0; qi < nq; qi++)
    {
        CostInit(&costi);
        gettimeofday(&tvb, NULL);

        knn[qi] = (DoubleIndex*)malloc(sizeof(DoubleIndex)*nk);
        memcpy(query, qds->data+qi*d, sizeof(float)*d);

        /// prepare the lookup table of distances between query and all boundaries of each dimension
        for(di = 0; di < d; di++){
            for(i = 0; i < ninter+1; i++){
                sdisqb[di][i] = (query[di]-bound[di][i]) * (query[di]-bound[di][i]);
            }
        }

        /// scan codes, calculate lower and upper bound distance, roughly filter
        rough = (VAElem*)malloc(sizeof(VAElem)*cfg->n);
        elem = (VAElem*)malloc(sizeof(VAElem)*cfg->n);
        // VA_LowerBoundDistanceAll(cfg, query, sdisqb, bound, elem);
        VA_BoundDistanceAll(cfg, query, sdisqb, bound, elem);

        if(nq == 1)
        {
            FILE *_fp = open_file("lb.txt", "w");
            for(i = 0; i < cfg->n; i++)
            {
                fprintf(_fp, "%d: %f %f\n", i, elem[i].lb, elem[i].ub);
            }
            fclose(_fp);
        }

        rough_len = 0;
        Heap_Init(&h, nk);
        for(i = 0; i < cfg->n; i++)
        {
            // VA_LowerBoundDistance(cfg, query, sdisqb, bound, i, &elem[i]);
            elem[i].pid = i;
            // VA_UpperBoundDistance(cfg, query, sdisqb, bound, i, &elem[i]);

            can_di.id = elem[i].pid;
            can_di.val = elem[i].ub;

            if(rough_len < nk || !f_bigger(elem[i].lb, h.elem[0].val) )
            {// candidate not full yet or new lower bound not exceeds the current kmin upper bound
                /// here, the max-heap keeps the k-th minimum upper bound
                /// rough keeps candidates
                MaxHeap_Insert(&h, &can_di);
                memcpy(&rough[rough_len], &elem[i], sizeof(VAElem));
                rough_len = rough_len + 1;
            }
        }
        // printf("%d --> ", rough_len);

        /// convert rough into a DoubleIndex list, and sort with the lower bound distance
        candidate = (DoubleIndex*)malloc(sizeof(DoubleIndex)*rough_len);
        for(i = 0; i < rough_len; i++)
        {
            candidate[i].id = rough[i].pid;
            candidate[i].val = rough[i].lb;
        }
        DI_MergeSort(candidate, 0, rough_len-1);
        Heap_Delete(&h);

        /// refine knn
        i = 0;
        Heap_Init(&h, nk);
        stop = false;
        knn_R = FLOAT_MAX;
        while(!stop)
        {
            /// load data
            gettimeofday(&tvb_io, NULL);
            fseek(fp, (sizeof(int)+sizeof(float)*d)*candidate[i].id+sizeof(int), SEEK_SET);
            fread(x, sizeof(float), d, fp);
            gettimeofday(&tve_io, NULL);
            costi.io = costi.io + timediff(tvb_io, tve_io);
            if(i > 0 && abs(candidate[i].id-candidate[i-1].id) < point_per_page)
            {
                costi.point = costi.point + 1;
            }
            else
            {
                costi.page = costi.page + 1;
            }

            can_di.val = odistance_square(query, x, d);
            can_di.id = candidate[i].id;

            // printf("%d: %d-%f\n", i, can_di.id, can_di.val);

            if(h.length < h.MaxNum || !f_bigger(can_di.val, h.elem[0].val))
            {// not full or a smaller distance
                MaxHeap_Insert(&h, &can_di);
            }
            knn_R = h.elem[0].val;
            i++;

            if(i >= rough_len || (h.length == h.MaxNum && f_bigger(candidate[i].val, knn_R)))
            {// candidate run out, or knn is full and next lb exceeds knn_R
                stop = true;
            }
        }
        // printf("nn: %d, i: %d, point: %d, page: %d\n", h.length, i, costi.point, costi.page);
        memcpy(knn[qi], h.elem, sizeof(DoubleIndex)*nk);

        gettimeofday(&tve, NULL);
        costi.cpu = timediff(tvb, tve);
        costi.search = costi.cpu - costi.io;
        CostCombine(cost, &costi);

        /// some destroyment
        free(rough); rough = NULL;
        free(elem); elem = NULL;
        free(candidate); candidate = NULL;
    }
    fclose(fp);

    CostMultiply(cost, 1/(float)nq);
    cost->cpu = cost->cpu + tablediff;

    free(query); query = NULL;
    free(x); x = NULL;
    for(di = 0; di < d; di++)
    {
        free(sdisqb[di]); sdisqb[di] = NULL;
        free(bound[di]); bound[di] = NULL;
    }
    free(sdisqb); sdisqb = NULL;
    free(bound); bound = NULL;
    Heap_Delete(&h);
}

void VA_SquareInternalDiffLength(VAConfig *cfg){
	ASSERTINFO(cfg == NULL || cfg->d < 0 || cfg->nbit < 0|| cfg->inter_len == NULL, "VA_SquareInternalLength: insufficient data to compute");
	int		di, iinter;
	float	inter_len_s;
	int		inter_num = (int)pow(2.0, cfg->nbit);
	int		*diff_s = (int*)malloc(sizeof(int) * inter_num);

	/* preparation */
	// allocate space
	cfg->inter_alldiff_len_s = (float**)malloc(sizeof(float*)*cfg->d);
	ASSERTINFO(cfg->inter_alldiff_len_s == NULL, "VA_SquareInternalLength: failed to allocate space for inter_alldiff_len_s");
	// square diff: 0;1,2,inter_num-1;
	diff_s[0] = 0;
	for(iinter = 0; iinter < inter_num-1; iinter++){
		diff_s[iinter+1] = iinter * iinter;
	}

	/* calculate all square diff lengths in each dimension */
	for(di = 0; di < cfg->d; di++){
		/* allocate space */
		cfg->inter_alldiff_len_s[di] = (float*)malloc(sizeof(float)*inter_num);
		ASSERTINFO(cfg->inter_alldiff_len_s[di] == NULL, "failed to allocate space for alldiff square inter lengths in one dimension");
		/* figure out */
		inter_len_s = cfg->inter_len[di] * cfg->inter_len[di];
		cfg->inter_alldiff_len_s[di][0] = 0;
		for(iinter = 1; iinter < inter_num; iinter++){
			cfg->inter_alldiff_len_s[di][iinter] = diff_s[iinter] * inter_len_s;
		}
	}

	/* free space */
	free(diff_s); diff_s = NULL;
}
/************************************* basic operations ************************************/
/// figure out the lower and upper distance between query and the i-th code
void VA_LowerBoundDistanceAll(const VAConfig *cfg, const float *query, float **sdisqb, const float **bound, VAElem *elem)
{
    int i, di;
    int d = cfg->d;
    int n = cfg->n;
    float lower, upper;
    int pos;

    for(i = 0; i < n; i++)
    {
        elem[i].lb = FLOAT_ZERO;
        // the ni-th data code
        for(di = 0; di < d; di++)
        {
            // the di-th dimension
            pos = cfg->code[i*d+di];
            lower = bound[di][pos];
            upper = bound[di][pos+1];
            if(f_bigger(lower, query[di]))
            {// query < lower
                elem[i].lb = elem[i].lb + sdisqb[di][pos];
            }
            else if(f_bigger(query[di], upper))
            {// query > upper
                elem[i].lb = elem[i].lb + sdisqb[di][pos+1];
            }
        }
    }
}
void VA_BoundDistanceAll(const VAConfig *cfg, const float *query, float **sdisqb, float **bound, VAElem *elem)
{
    int i, di;
    int d = cfg->d;
    int n = cfg->n;
    float lower, upper;
    int pos;

    for(i = 0; i < n; i++)
    {
        elem[i].lb = FLOAT_ZERO;
        elem[i].ub = FLOAT_ZERO;
        // the ni-th data code
        for(di = 0; di < d; di++)
        {
            // the di-th dimension
            pos = cfg->code[i*d+di];
            lower = bound[di][pos];
            upper = bound[di][pos+1];
            if(f_bigger(lower, query[di]))
            {// query < lower
                elem[i].lb = elem[i].lb + sdisqb[di][pos];
                elem[i].ub = elem[i].ub + sdisqb[di][pos+1];
            }
            else if(f_bigger(query[di], upper))
            {// query > upper
                elem[i].lb = elem[i].lb + sdisqb[di][pos+1];
                elem[i].ub = elem[i].ub + sdisqb[di][pos];
            }
            else
            {// between upper and lower, compare distance between query and upper and lower further
                if(f_bigger(query[di]-lower, upper-query[di]))
                {// near to upper, select upper
                    elem[i].ub = elem[i].ub + sdisqb[di][pos];
                }
                else
                {// near to lower, select upper
                    elem[i].ub = elem[i].ub + sdisqb[di][pos+1];
                }
            }
        }
    }
}
void VA_LowerBoundDistance(const VAConfig *cfg, const float *query, float **sdisqb, const float **bound, int i, VAElem *elem)
{
    int di;
    int d = cfg->d;
    float lower, upper;
    int pos;
    elem->lb = FLOAT_ZERO;
    for(di = 0; di < d; di++)
    {
        pos = cfg->code[i*d+di];
        lower = bound[di][pos];
        upper = bound[di][pos+1];
        if(f_bigger(lower, query[di]))
        {// query < lower
            elem->lb = elem->lb + sdisqb[di][pos];
        }
        else if(f_bigger(query[di], upper))
        {// query > upper
            elem->lb = elem->lb + sdisqb[di][pos+1];
        }
    }
}
void VA_UpperBoundDistance(const VAConfig *cfg, const float *query, float **sdisqb, const float **bound, int i, VAElem *elem)
{
    int di;
    int d = cfg->d;
    float lower, upper;
    int pos;
    elem->ub = FLOAT_ZERO;
    for(di = 0; di < d; di++)
    {
        pos = cfg->code[i*d+di];
        lower = bound[di][pos];
        upper = bound[di][pos+1];
        if(f_bigger(lower, query[di]))
        {// query < lower
            elem->ub = elem->ub + sdisqb[di][pos+1];
        }
        else if(f_bigger(query[di], upper))
        {// query > upper
            elem->ub = elem->ub + sdisqb[di][pos];
        }
        else
        {// between upper and lower, compare distance between query and upper and lower further
            if(f_bigger(query[di]-lower, upper-query[di]))
            {// near to upper, select upper
                elem->ub = elem->ub + sdisqb[di][pos];
            }
            else
            {// near to lower, select upper
                elem->ub = elem->ub + sdisqb[di][pos+1];
            }
        }
    }
}
float VA_LowerBoundDistance_Codes_S(const VAConfig *cfg, int ione, int iother){
	/* check out necessary contents: cfg, code, inter_len_s */
	ASSERTINFO(cfg == NULL || cfg->code == NULL || cfg->inter_alldiff_len_s == NULL, "insufficient data to figure out lower bound distance between codes");
	ASSERTINFO(ione < 0 || iother < 0 || ione >= cfg->n || iother >= cfg->n, "codes' indexes exceeds normal range, cannot figure out lower bound distance between codes");

	int		d = cfg->d;
	float	sum;
	int		di, pm, py, diff;				/* my code pointer, your code pointer */

	/* read out the codes and calculate the distance */
	pm = ione * d;
	py = iother * d;
	sum = 0;
	for(di = 0; di < d; di++){
		if(cfg->code[pm] != cfg->code[py]){
			diff = abs(cfg->code[pm] - cfg->code[py]);
			sum += cfg->inter_alldiff_len_s[di][diff];
		}
		pm++;
		py++;
	}
	return sum;
}

