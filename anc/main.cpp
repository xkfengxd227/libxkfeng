#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mainext.h"
#include "anc.h"
#include "common.h"
#include "cost.h"
#include "clustering.h"
#include "heap.h"
#include "/media/xikafe/dataset/dsbconfig.h"
extern "C"{
#include <yael/vector.h>
#include <yael/machinedeps.h>
}

int main(int argc, char *argv[])
{
	/* ------------------------------- running parameters ----------------------------------*/
	int		n, d, nl;			// n-base, dimension, n-learning
	int		nq, k;				// n-query, [k]nn
	int		K, h;				// sub-cluster factor, depth of hierarchical clusters
	int 	g;					// rank range of neighbor clusters
	int		i;
	FILE		*fp;
	long		iocount;			// iocount
	float		fe;				// filter ratio
	DSBConfig	dscfg;
	char		ancif[255];			// index folder
	int 	alg;			// algorithm flag

	/* >>> accept needed parameters */
	if(!_input_parameters(argc, argv, &alg, dscfg.dsname, &K, &h, &g, &nq, &k)){
		exit(-1);	
	}
	config_dataset(dscfg.dsname, &dscfg);		// assemble dataset parameters

	/* ------------------------------- data ----------------------------------*/
	fDataSet	ds;					// base dataset
	fDataSet	lds;				// train(learn) dataset
	fDataSet	qds;				// query dataset
	n = dscfg.n;
	d = dscfg.d;
	nl = dscfg.nl;

	// assemble the index folder under [K]
	sprintf(ancif, "%s/%s", IndexFolder, dscfg.dsname);		// index folder of [anc]
	makesure_folder(ancif);
	sprintf(ancif, "%s/K=%d,g=%d", ancif, K, g);		// index folder of [anc]
	makesure_folder(ancif);

	// switch to different branches according to [alg]
	if((int)Algorithm_Indexing == alg){			
		/*** build index ***/
		printf("checking parameters ... \nanc: -alg %d -ds %s -n %d -d %d -nl %d -K %d -h %d -g %d\n",
			alg, dscfg.dsname, n, d, nl, K, h, g);

		/* >>> load data: base/learn set */
		fDataSet_init(&ds, n, d);
		fDataSet_init(&lds, nl, d);
		fvecs_read(dscfg.basefile, d, n, ds.data);
		fvecs_read(dscfg.learnfile, d, nl, lds.data);

		/* >>> constructing index */
		ANC anc(K, g, d, n);				// init an HB object
		anc.generate_cluster(&ds, &lds, CLUSTERING_NITER, CLUSTERING_NTHREAD, CLUSTERING_SEED, CLUSTERING_NREDO);
		float avg_neighbor = anc.neighbor_cluster_estimation(&ds, CLUSTERING_NTHREAD);
		anc.inner_lb_distance_OnePerPoint(&ds);
		anc.index_into_file(ancif);		// index into files

		/* >>> disallocation */
		fDataSet_unset(&ds);
		fDataSet_unset(&lds);

		printf("index built successfully: avg_neighbor:\t%f\n", avg_neighbor);

	}else if((int)Algorithm_Search == alg || (int)Algorithm_Search_CrossLB == alg){
		/*** knn search ***/
		printf("checking parameters ... \nanc: -alg %d -ds %s -n %d -d %d -K %d -h %d -g %d -nq %d -k %d\n",
			alg, dscfg.dsname, n, d, K, h, g, nq, k);

		/* >>> load data: base, query set, groundtruth */
		fDataSet_init(&ds, n, d);
		fDataSet_init(&qds, nq, d);
		fvecs_read(dscfg.basefile, d, n, ds.data);
		fvecs_read(dscfg.queryfile, d, nq, qds.data);
		DoubleIndex 	*groundtruth = (DoubleIndex*)malloc(sizeof(DoubleIndex) * nq * dscfg.dGT);
		load_groundtruth(dscfg.gtfile, dscfg.gtdisfile, dscfg.dGT, nq, groundtruth);

	 	/* >>> performance collectors */
		float		anc_accuracy;			// anc-accuracy
		Cost		anc_cost;			// anc-cost
		DoubleIndex 	**anc_knnset;			// anc-query results

		/* >>> loading index */
		ANC anc(K, g, d, n);
		anc.load_index(ancif);

		/* >>> online query */
		printf("\n######################## %d clusters, %d-nn: ##########################\n", K, k);
		printf("data set: %s\n", dscfg.basefile);
		printf("n: %d, d: %d\n", n, d);

		anc_knnset = (DoubleIndex **)malloc(sizeof(DoubleIndex*)*nq);
		CostInit(&anc_cost);
		anc.search(&ds, &qds, ancif, k, anc_knnset, &anc_cost, alg);
		anc_accuracy = knn_precision(anc_knnset, groundtruth, nq, k, dscfg.dGT);
		iocount = anc_cost.page + anc_cost.point / ( PAGESIZE/(d*sizeof(float)) ) / PAGE_RAN_SE;
		fe = (1-(float)anc_cost.point / n) * 100;

		/* ------------------------------- report performance --------------------------- */
		printf("------------------ ANC ---------------\n");
		printf("accuracy:\t%f\n", anc_accuracy);
		printf("FE(\%): %g\n", fe);
		printf("IO:\t%ld\n", iocount);
		CostDisplay(anc_cost);
		printf("%ld\t%ld\t%ld\t%g\t%ld\t%ld\t%ld\t%ld\t%f\t%f\n",
			iocount, anc_cost.page, anc_cost.point, fe, anc_cost.cpu, anc_cost.lowerbound, anc_cost.io, anc_cost.search, anc_accuracy, anc.average_neighbor());

		// record search result into file
		char resultLog[255];
		sprintf(resultLog, "%s.%s", ResultLogFile, dscfg.dsname);
		fp = open_file(resultLog, "a");
		fprintf(fp, "------ %s K-%d g-%d k-%d nq-%d\n", dscfg.dsname, K, g, k, nq);
		fputs("accu\tfe(\%)\tio\tpage\tpts\tttl(us)\tlb(us)\tio(us)\tsch(us)\tavg_neighbor\n", fp);
		fprintf(fp, "%g\t%g\t%ld\t%ld\t%ld\t%ld\t%ld\t%ld\t%ld\t%f\n",
			anc_accuracy, fe,
			iocount, anc_cost.page, anc_cost.point,
			anc_cost.cpu, anc_cost.lowerbound, anc_cost.io, anc_cost.search, anc.average_neighbor());
		fclose(fp);

		/* ------------------------------- clean memory space -----------------------*/
		/* >>> free knn results */
		for(i = 0; i < nq; i++)
		{
			FREE(anc_knnset[i]);
		}
		FREE(anc_knnset);

	    	/* >>> free allocation */
		fDataSet_unset(&ds);
		fDataSet_unset(&qds);
		FREE(groundtruth);
	}

	return 0;
}
