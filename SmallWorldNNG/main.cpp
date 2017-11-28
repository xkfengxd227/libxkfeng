/**
 *  main script for NN search on SmallWorld NNG
 */
#include "common.h"
#include "mainext.h"
#include "NeighborGraph.h"
#include "/media/xikafe/dataset/dsbconfig.h"
extern "C"{
    #include <yael/vector.h>
}

int main(int argc, char *argv[]){
    /* variables */
    DSBConfig   dscfg;
    int         K = -1;             /*< K-NNG */
    int         nq = -1;            /*< number of query */
    int         k = -1;             /*< k-NN search */
    int         logOn = 0;          /*< default not to report */
    char        alg[255];           /*< which algorithm to execute: index, search */
    char        logBuff[255], path[255];

    /* accept arguments */
    LOGINFO(logOn, "accepting parameters...");
    if(!_input_parameters(argc, argv, dscfg.dsname, alg, &K, &nq, &k, &logOn)){
        exit(-1);
    }
    config_dataset(dscfg.dsname, &dscfg);

    makesure_folder(IndexFolder);
    sprintf(path, "%s/%s", IndexFolder, dscfg.dsname);
    makesure_folder(path);

    /* switch to different algorithm branch according to specified algorithm indicators */
    if(strcmp(alg, "Indexing") == 0){
        /// building index
        /* load data: baseset */
        int         n = dscfg.n;
        int         d = dscfg.d;
        float       *bv = fvec_new(n * d);
        LOGINFO(logOn, "loading base dataset...");
        fvecs_read(dscfg.basefile, d, n, bv);

        /* graph construction */
        NeighborGraph graph(n, d, true);                    // init a nearest(true) neighbor graph
        graph.construct_neat_graph_incrementally(bv, K);                 // build the graph in incremental manner

        /* save graph */
        graph.save_graph(IndexFolder, dscfg.dsname, K);
        LOGINFO(logOn, "graph save done.");

        /* release */
        FREE(bv);
    }else if(strcmp(alg, "Search") == 0){
        /// kNN search
    }else{
        /// do nothing
        printf("Unsupported algorithm indicator: '%s'\n", alg);
    }

    return 0;
}
