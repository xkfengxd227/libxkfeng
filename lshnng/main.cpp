/**
 *  main script of lsh+nng for knn-search
 */
#include "common.h"
#include "lsh.h"
#include "mainext.h"
#include "nng.h"
#include "/media/xikafe/dataset/dsbconfig.h"
extern "C"{
    #include <yael/vector.h>
    #include <yael/sorting.h>
}

int main(int argc, char *argv[]){
    /* config */
    DSBConfig   dscfg;
    int         m = -1;
    float       w = -1.;
    int         g = -1;
    int         nc = -1;
    int         nq = -1;
    int         k = -1;
    int         logOn = 0;          /* default not to report */
    char        logBuff[255];

    /* accept arguments */
    LOGINFO(logOn, "accepting parameters...");
    if(!_input_parameters(argc, argv, dscfg.dsname, &m, &w, &g, &nc, &nq, &k, &logOn)){
        exit(-1);
    }
    config_dataset(dscfg.dsname, &dscfg);

    /* load data: baseset, queryset, NNG */
    int         n = dscfg.n;
    int         d = dscfg.d;
    float       *bv = fvec_new(n * d);
    float       *qv = fvec_new(nq * d);
    int         *G = ivec_new(g * n);

    LOGINFO(logOn, "loading datasets...");
    fvecs_read(dscfg.basefile, d, n, bv);
    fvecs_read(dscfg.queryfile, d, nq, qv);
    if(g > 0){  
        LOGINFO(logOn, "\tloading nng data...");
        ivecs_read(dscfg.nngfile, g, n, G); 
    }


    /* lsh encoding */
    CompLSH lsh = CompLSH(m, d, dscfg.T, w);
    lsh.generate_functions();
    int         *code = ivec_new(m * n);
    lsh.encode(bv, n, d, code);
    LOGINFO(logOn, "finished encoding...");

    /* knn search */
    int         *knn = ivec_new(k * nq);                    // knn results of whole knn search
    float       *knndis = fvec_new(k * nq);
    int         *candNeighbor_knn = ivec_new((nc+nc*g) * nq);     // knn results by loading first batch of neighbors
    float       *candNeighbor_knndis = fvec_new((nc+nc*g) * nq);
    int         *c2knn = ivec_new(nc * nq);                  // knn results of c2lsh
    float       *c2knndis = fvec_new(nc * nq);
    int         *status = ivec_new_set(n * nq, CleanNode);
    int         *qcode = ivec_new(d);
    int         *counter = NULL;
    int         *p_code = NULL;
    int         *cand = NULL;
    DoubleIndex *candDis = (DoubleIndex*)malloc(sizeof(DoubleIndex) * nc);
    DoubleIndex *candNeighborDis = (DoubleIndex*)malloc(sizeof(DoubleIndex) * (nc + nc * g));

    for(int qi = 0; qi < nq; qi++){
        /* the [qi]-th query */
        sprintf(logBuff, "[%d]-querying...", qi);
        LOGINFO(logOn, logBuff);

        // encode query
        lsh.encode(qv+qi*d, 1, d, qcode);
        LOGINFO(logOn, "\tencoded query...");

        /* collision count */
        counter = ivec_new_0(n);
        p_code = code;
        for(int ni = 0; ni < n; ni++){
            for(int mi = 0; mi < m; mi++){
                if(qcode[mi] == *p_code){
                    counter[ni] ++;
                }
                p_code++;
            }
        }
        LOGINFO(logOn, "\tencoded query...");

        // filter out [nc] biggest one
        cand = ivec_new(nc);
        fvec_k_max((float*)counter, n, cand, nc);

        /* one-round c2lsh */
        for(int ci = 0; ci < nc; ci++){
            candDis[ci].id = cand[ci];
            candDis[ci].val = odistance(qv+qi*d, bv+cand[ci]*d, d);
            // status[n*qi+cand[ci]] = CandidateNode; ### <after>
        }
        DI_MergeSort(candDis, 0, nc-1);
        for(int ki = 0; ki < k; ki++){
            c2knn[k*qi+ki] = candDis[ki].id;
            c2knndis[k*qi+ki] = candDis[ki].val;
        }
        LOGINFO(logOn, "\tone-round c2lsh...");

        /* if [g] > 0, execute nng convergence on NNG */
        if(g > 0){
            /* record the quality of first batch of candidates and their neighbors */
            int candNeighbor_count = 0;
            /*
            for(int ci = 0; ci < nc; ci++){
                candNeighborDis[ci].id = cand[ci];
                candNeighborDis[ci].val = odistance(qv+qi*d, bv+cand[ci]*d, d);
                status[n*qi+cand[ci]] = CandidateNode;
            }
            candNeighbor_count = nc;
            */

            for(int ci = 0; ci < nc; ci++){
                for(int gi = 0; gi < g; gi++){
                    int oid = cand[ci];
                    int gid = G[oid*g+gi];
                    if(status[n*qi+gid] != CandidateNode){
                        candNeighborDis[ci*g+gi].id = gid;
                        candNeighborDis[ci*g+gi].val = odistance(qv+qi*d, bv+gid*d, d);
                        status[n*qi+gid] = CandidateNode;

                        candNeighbor_count++;
                    }
                }
            }
            DI_MergeSort(candNeighborDis, 0, candNeighbor_count-1);
            for(int ki = 0; ki < k; ki++){
                candNeighbor_knn[k*qi+ki] = candNeighborDis[ki].id;
                candNeighbor_knndis[k*qi+ki] = candNeighborDis[ki].val;
            }
            printf("%d ", candNeighbor_count);
            LOGINFO(logOn, "\tcand + neighbor quality...");

            for(int ci = 0; ci < nc; ci++){
                status[n*qi+cand[ci]] = CandidateNode;
            }
            converge_to_knn_one_by_one(    bv, n, d,
                              qv+qi*d,
                              cand, nc,
                              G, g,
                              k,
                              knn+k*qi, knndis+k*qi, status+n*qi);
            LOGINFO(logOn, "\tnng convergence...");
        }

        /* update pointer */
        FREE(counter);
        FREE(cand);
    }

    /* performance report */
    // load groundtruth
    int         *gt = ivec_new(dscfg.dGT * nq);
    float       *gtdis = fvec_new(dscfg.dGT * nq);
    ivecs_read(dscfg.gtfile, dscfg.dGT, nq, gt);
    fvecs_read(dscfg.gtdisfile, dscfg.dGT, nq, gtdis);
    LOGINFO(logOn, "\tgroundtruth loaded...");

    float ratio, recall, precision;
    // performance of c2lsh
    knn_accuracy(gt, gtdis, c2knn, c2knndis, dscfg.dGT, k, nq, &ratio, &recall, &precision);
    printf("ratio\t\trecall\t\tprecision\tno. of points\n%.6f\t%.6f\t%.6f\t%.6f\n",
                ratio, recall, precision, (float)nc);

    // performance of c2lsh and first batch of neighbors
    knn_accuracy(gt, gtdis, candNeighbor_knn, candNeighbor_knndis, dscfg.dGT, k, nq, &ratio, &recall, &precision);
    printf("ratio\t\trecall\t\tprecision\tno. of points\n%.6f\t%.6f\t%.6f\t%.6f\n",
                ratio, recall, precision, (float)nc * g);

    // performance of c2+nng
    knn_accuracy(gt, gtdis, knn, knndis, dscfg.dGT, k, nq, &ratio, &recall, &precision);
    int cnt = 0;
    for(int i = 0; i < nq*n; i++){
      if(status[i] == CleanNode){
        cnt++;
      }
    }
    float avg_point = n - ((float)cnt / nq);
    printf("ratio\t\trecall\t\tprecision\tno. of points\n%.6f\t%.6f\t%.6f\t%.6f\n",
                ratio, recall, precision, avg_point);


    /* release */
    LOGINFO(logOn, "cleaning pointers...");
    FREE(bv);
    FREE(qv);
    FREE(G);
    FREE(code);
    FREE(knn);
    FREE(knndis);
    FREE(candNeighbor_knn);
    FREE(candNeighbor_knndis);
    FREE(c2knn);
    FREE(c2knndis);
    FREE(status);
    FREE(qcode);
    p_code = NULL;
    FREE(gt);
    FREE(gtdis);
    FREE(candDis);
    FREE(candNeighborDis);
    return 0;
}
