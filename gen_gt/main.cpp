/**
 *  generate groundtruth (nn, idx) for a baseset and a query set
 *  notice: data in binary
 */
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "common.h"
#include "mainext.h"
extern "C"{
    #include <yael/vector.h>
    #include <yael/sorting.h>
}

int main(int argc, char *argv[]){
    /* config */
    char 	dsname[255], basefile[255], queryfile[255];
    int   n = -1,
          d = -1,
          nq = -1,
          dGT = -1;

    int logOn = 1;

    /* accept arguments */
    LOGINFO(logOn, "accepting parameters...");
    if(!_input_parameters(argc, argv, dsname, basefile, queryfile, &n, &d, &nq, &dGT)){
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
    int         *gt = ivec_new(dGT * nq);
    float       *gtdis = fvec_new(dGT * nq);
    DoubleIndex	*cand = (DoubleIndex*)malloc(sizeof(DoubleIndex) * n);
    Cost cost;
    struct timeval tvb, tve;

    for(int qi = 0; qi < nq; qi++){
        /* the [qi]-th query */
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
        for(int ki = 0; ki < dGT; ki++){
            gt[dGT*qi+ki] = cand[ki].id;
            gtdis[dGT*qi+ki] = cand[ki].val;
        }
        gettimeofday(&tve, NULL);
        icost.timer[1] = timediff(tvb, tve);
        icost.search = icost.timer[0] + icost.timer[1];

        cost.combine(&icost);
        // LOGINFO(logOn, "\tdone search...");
    }
    cost.multiply(1 / (float)nq);

    /* save the groundtruth data */
    char fname[255];
    // the gt-idx
    sprintf(fname, "%s_gt.ivecs", dsname);
    ivecs_write(fname, dGT, nq, gt);
    sprintf(fname, "%sGT.txt", dsname);
    ivecs_write_txt(fname, dGT, nq, gt);

    // the gt-dis
    sprintf(fname, "%s_gtdis.fvecs", dsname);
    fvecs_write(fname, dGT, nq, gtdis);
    sprintf(fname, "%sGTDIS.txt", dsname);
    fvecs_write_txt(fname, dGT, nq, gtdis);
    LOGINFO(logOn, "\tdone groundtruth saving...");

    /* release */
    LOGINFO(logOn, "cleaning pointers...");
    FREE(bv);
    FREE(qv);
    FREE(gt);
    FREE(gtdis);
    FREE(cand);
    return 0;
}
