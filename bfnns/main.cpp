/**
 *  main script of brute-force search for kNN
 */
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "cost.h"
#include "common.h"
#include "mainext.h"
#include "/media/xikafe/dataset/dsbconfig.h"
extern "C"{
    #include <yael/vector.h>
    #include <yael/sorting.h>
}

int main(int argc, char *argv[]){
    /* config */
    DSBConfig   dscfg;
    int 		k = -1;
    int         nq = -1;
    int         logOn = 0;          /* default not to report */
    int         gt = 0;             /* whether to save the groundtruth results: dis, idx */
    char        logBuff[255];

    /* accept arguments */
    LOGINFO(logOn, "accepting parameters...");
    if(!_input_parameters(argc, argv, dscfg.dsname, &k, &nq, &logOn, &gt)){
        exit(-1);
    }
    config_dataset(dscfg.dsname, &dscfg);

    /* load data: baseset, queryset, NNG */
    int         n = dscfg.n;
    int         d = dscfg.d;
    float       *bv = fvec_new(n * d);
    float       *qv = fvec_new(nq * d);

    LOGINFO(logOn, "loading datasets...");
    fvecs_read(dscfg.basefile, d, n, bv);
    fvecs_read(dscfg.queryfile, d, nq, qv);

    // load groundtruth
    int dGT = dscfg.dGT;
    int         *gt = ivec_new(dGT * nq);
    float       *gtdis = fvec_new(dGT * nq);
    ivecs_read(dscfg.gtfile, dGT, nq, gt);
    fvecs_read(dscfg.gtdisfile, dGT, nq, gtdis);
    LOGINFO(logOn, "\tgroundtruth loaded...");

    /* knn search */
    int         *knn = ivec_new(k * nq);                    // knn results of whole knn search
    float       *knndis = fvec_new(k * nq);
    DoubleIndex	*cand = (DoubleIndex*)malloc(sizeof(DoubleIndex) * n);
    Cost cost, icost;
    struct timeval tvb, tve;

    CostInit(&cost);
    for(int qi = 0; qi < nq; qi++){
        /* the [qi]-th query */
        sprintf(logBuff, "[%d]-querying...", qi);
        LOGINFO(logOn, logBuff);

        CostInit(&icost);
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
            knn[k*qi+ki] = cand[ki].id;
            knndis[k*qi+ki] = cand[ki].val;
        }
        gettimeofday(&tve, NULL);
        icost.timer[1] = timediff(tvb, tve);
        icost.search = icost.timer[0] + icost.timer[1];

        CostCombine(&cost, &icost);
        LOGINFO(logOn, "\tdone search...");
    }
    CostMultiply(&cost, 1 / (float)nq);

    /* write the groundtruth file */
    if(gt == 1){
        char fname[255];
        // the gt-idx
        sprintf(fname, "%s_gt.ivecs", dscfg.dsname);
        ivecs_write(fname, d, nq, knn);
        // the gt-dis
        sprintf(fname, "%s_gtdis.fvecs", dscfg.dsname);
        fvecs_write(fname, d, nq, knndis);
        LOGINFO(logOn, "\tdone groundtruth saving...");
    }


    /* performance report */
    float ratio, recall, precision;
    // performance
    knn_accuracy(gt, gtdis, knn, knndis, dscfg.dGT, k, nq, &ratio, &recall, &precision);
    printf("ratio\trecall\tprecision\ttotal(s)\ttraverse(s)\tsort(s)\tno. of points\n%.6f\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\n",
                ratio, recall, precision, 
                cost.search / (double)1e6, cost.timer[0] / (double)1e6, cost.timer[1] / (double)1e6,
                (double)n);

    /* release */
    LOGINFO(logOn, "cleaning pointers...");
    FREE(bv);
    FREE(qv);
    FREE(knn);
    FREE(knndis);
    FREE(cand);
    FREE(gt);
    FREE(gtdis);
    return 0;
}
