/**
 *  generate groundtruth (nn, idx) for a baseset and a query set
 *  notice: data in binary
 */
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <math.h>
#include "common.h"
#include "mainext.h"
extern "C"{
    #include <yael/vector.h>
    #include <yael/sorting.h>
    #include <yael/nn.h>
}

int main(int argc, char *argv[]){
    /* config */
    char 	dsname[255], basefile[255], queryfile[255];
    char	type[255];
    int   n = -1,
          d = -1,
          nq = -1,
          k = -1;

    int logOn = 1;

    /* accept arguments */
    LOGINFO(logOn, "accepting parameters...");
    if(!_input_parameters(argc, argv, dsname, type, basefile, queryfile, &n, &d, &nq, &k)){
        exit(-1);
    }

    /* load data: baseset, queryset, NNG */
    float       *bv = fvec_new(n * d);
    float       *qv = fvec_new(nq * d);
    fvecs_read(basefile, d, n, bv);
    fvecs_read(queryfile, d, nq, qv);
    LOGINFO(logOn, "data and query loaded");

	fvec_print(bv, 2*d);
    fvec_print(qv, 2*d);

    /* groundtruth */
    int         *assign = ivec_new(k * nq);
    float       *nndis = fvec_new(k * nq);
    Cost cost;
    struct timeval tvb, tve;

	gettimeofday(&tvb, NULL);

    int nth = 8;
	knn_full_thread(2, nq, n, d, k, bv, qv, NULL, assign, nndis, nth);
	for(int i = 0; i < nq * k; i++){
		nndis[i] = sqrt(nndis[i]);
	}

	gettimeofday(&tve, NULL);
	cost.timer[0] = timediff(tvb, tve);
	cost.search = cost.timer[0] + cost.timer[1];

	/* executed by traditional methods: slow
    for(int qi = 0; qi < nq; qi++){
        // the [qi]-th query 
        char logBuff[255];

        if(qi % 100 == 0){
          sprintf(logBuff, "[%d]-querying...", qi);
          LOGINFO(logOn, logBuff);
        }


        Cost icost;
        gettimeofday(&tvb, NULL);



        
        for(int ni = 0; ni < n; ni++){
        	cand[ni].id = ni;
        	cand[ni].val = odistance(qv+qi*d, bv+ni*d, d);
        }
        gettimeofday(&tve, NULL);
        icost.timer[0] = timediff(tvb, tve);

        gettimeofday(&tvb, NULL);
        DI_MergeSort(cand, 0, n-1);
        for(int ki = 0; ki < k; ki++){
            gt[k*qi+ki] = cand[ki].id;
            gtdis[k*qi+ki] = cand[ki].val;
        }
        


        gettimeofday(&tve, NULL);
        icost.timer[1] = timediff(tvb, tve);
        icost.search = icost.timer[0] + icost.timer[1];

        cost.combine(&icost);
        // LOGINFO(logOn, "\tdone search...");
    }
    cost.multiply(1 / (float)nq);
    */

	if(strcmp(type, "gt") == 0){
	    /* save the groundtruth data */
		LOGINFO(logOn, ">> saving groundtruth data ...");

	    char fname[255];
	    // the gt-idx
	    sprintf(fname, "%s_gt.ivecs", dsname);
	    ivecs_write(fname, k, nq, assign);
	    sprintf(fname, "%sGT.txt", dsname);
	    ivecs_write_txt(fname, k, nq, assign);

	    // the gt-dis
	    sprintf(fname, "%s_gtdis.fvecs", dsname);
	    fvecs_write(fname, k, nq, nndis);
	    sprintf(fname, "%sGTDIS.txt", dsname);
	    fvecs_write_txt(fname, k, nq, nndis);
	    LOGINFO(logOn, "\tdone groundtruth saving...");
	}else if(strcmp(type, "nng") == 0){
		/* save the groundtruth data */
		LOGINFO(logOn, ">> saving nng data ...");

	    char fname[255];
	    // the nng-idx
	    sprintf(fname, "%s_nng.ivecs", dsname);
	    ivecs_write(fname, k, nq, assign);
	    sprintf(fname, "%sNNG.txt", dsname);
	    ivecs_write_txt(fname, k, nq, assign);

	    // the gt-dis
	    sprintf(fname, "%s_nngdis.fvecs", dsname);
	    fvecs_write(fname, k, nq, nndis);
	    sprintf(fname, "%sNNGDIS.txt", dsname);
	    fvecs_write_txt(fname, k, nq, nndis);
	    LOGINFO(logOn, "\tdone groundtruth saving...");
	}

    /* release */
    LOGINFO(logOn, "cleaning pointers...");
    FREE(bv);
    FREE(qv);
    FREE(assign);
    FREE(nndis);
    return 0;
}
