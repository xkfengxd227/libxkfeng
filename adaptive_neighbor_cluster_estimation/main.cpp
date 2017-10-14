/**
 *	the main script for [adaptive neighbor cluster estimation]
 *
 *	>>> current task: to see neighbor clusters
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mainext.h"
// #include "HB.h"
#include "common.h"
#include "cost.h"
#include "clustering.h"
#include "heap.h"
#include "/media/xikafe/dataset/dsbconfig.h"
extern "C"{
#include <yael/vector.h>
#include <yael/machinedeps.h>
}
#include <iostream>
using namespace std;

int main(int argc, char *argv[])
{
	/* ------------------------------- running parameters ----------------------------------*/
	int		n, d, nl;			// n-base, dimension, n-learning
	int		nq, k;				// n-query, [k]nn
	int		K, h;				// sub-cluster factor, depth of hierarchical clusters
	int		i;
	FILE		*fp;
	int		iocount;			// iocount
	float		fe;				// filter ratio
	DSBConfig	dscfg;
	char		hbif[255];			// index folder
	sprintf(hbif, "%s/hb", IndexFolder);		// index folder of [hb]

	/* >>> accept needed parameters */
	if(!_input_parameters(argc, argv, dscfg.dsname, &K, &h, &nq, &k)){
		exit(-1);
	}
	config_dataset(dscfg.dsname, &dscfg);		// assemble dataset parameters

	/* ------------------------------- data ----------------------------------*/
	fDataSet	ds;				// base dataset
	fDataSet	lds;				// train(learn) dataset
	fDataSet	qds;				// query dataset
	n = dscfg.n;
	d = dscfg.d;
	nl = dscfg.nl;

	printf("checking parameters ... \nhb: -ds %s -n %d -d %d -nl %d -K %d -h %d -nq %d -k %d\n",
		dscfg.dsname, n, d, nl, K, h, nq, k);

	/* >>> load data: base/learn/query dataset, groundtruth */
	fDataSet_init(&ds, n, d);
	fDataSet_init(&lds, nl, d);
	fDataSet_init(&qds, nq, d);
	fvecs_read(dscfg.basefile, d, n, ds.data);
	fvecs_read(dscfg.learnfile, d, nl, lds.data);
	fvecs_read(dscfg.queryfile, d, nq, qds.data);
	DoubleIndex 	*groundtruth = (DoubleIndex*)malloc(sizeof(DoubleIndex) * nq * dscfg.dGT);
	load_groundtruth(dscfg.gtfile, dscfg.gtdisfile, dscfg.dGT, nq, groundtruth);

 	/* ------------------------------- performance collectors ----------------------------------*/
	float		hb_accuracy;			// hb-accuracy
	Cost		hb_cost;			// hb-cost
	DoubleIndex 	**hb_knnset;			// hb-query results

	/* ------------------------------- preprocessing ----------------------------------*/
	/* >>> constructing index */
	Clustering	clustering(K);				// init clustering object
	clustering.generate_cluster(&ds, &lds, CLUSTERING_NITER, CLUSTERING_NTHREAD, CLUSTERING_SEED, CLUSTERING_NREDO);			// clustering
	clustering.neighbor_cluster_estimation(&ds, CLUSTERING_NTHREAD);		// neighbor cluster estimation
	

    /* >>> free allocation */
	fDataSet_unset(&ds);
	fDataSet_unset(&lds);
	fDataSet_unset(&qds);
	free(groundtruth); groundtruth = NULL;
	return 0;
}
