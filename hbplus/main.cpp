#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "mainext.h"
#include "hb.h"
#include "common.h"
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
	int 	n, d, nl;			// n-base, dimension, n-learning
	int		nq, k;				// n-query, [k]nn
	int		K, h;				// sub-cluster factor, depth of hierarchical clusters
	int 	m;					// reducted dimension
	float 	alpha;				// alpha


	int		i;
	FILE	*fp;
	long	iocount;			// iocount
	float	fe;				// filter ratio
	DSBConfig	dscfg;
	char	hbif[255];			// index folder
	int 	alg;			// algorithm flag

	/* >>> accept needed parameters: parameters for all algorithm branches */
	if(!_input_parameters(argc, argv, dscfg.dsname, &alg, &K, &h, &m, &alpha, &nq, &k)){
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

	// assemble the index folder under [K]
	sprintf(hbif, "%s/%s", IndexFolder, dscfg.dsname);		// index folder of [hb]
	makesure_folder(hbif);
	sprintf(hbif, "%s/K=%d", hbif, K);		// index folder of [hb]
	makesure_folder(hbif);

	/* switch to different algorithm branch according to "alg" */
	if(alg == Algorithm_Indexing){
		/* building index and save */
		printf("checking parameters ... \nhb: -ds %s -n %d -d %d -nl %d -K %d -h %d\n",
		dscfg.dsname, n, d, nl, K, h);

		/* >>> load data: base/learn dataset */
		fDataSet_init(&ds, n, d);
		fDataSet_init(&lds, nl, d);
		fvecs_read(dscfg.basefile, d, n, ds.data);
		fvecs_read(dscfg.learnfile, d, nl, lds.data);

		/* >>> constructing index */
		HB hb(K, d, n);				// init an HB object
		hb.generate_cluster(&ds, &lds, CLUSTERING_NITER, CLUSTERING_NTHREAD, CLUSTERING_SEED, CLUSTERING_NREDO);
		hb.inner_lb_distance_OnePerPoint(&ds);
		hb.index_into_file(hbif);		// index into files

		/* >>> free allocation */
		fDataSet_unset(&ds);
		fDataSet_unset(&lds);

		puts("finished building");

	}else if(alg == Algorithm_Search || alg == Algorithm_Search_TrueLB || alg == Algorithm_Search_CrossLB){
		/* load index and search */
		printf("checking parameters ... \nhb: -ds %s -n %d -d %d -K %d -h %d -nq %d -k %d\n",
		dscfg.dsname, n, d, K, h, nq, k);

		/* >>> load data: base/learn/query dataset, groundtruth */
		fDataSet_init(&ds, n, d);
		fDataSet_init(&qds, nq, d);
		fvecs_read(dscfg.basefile, d, n, ds.data);
		fvecs_read(dscfg.queryfile, d, nq, qds.data);
		DoubleIndex 	*groundtruth = (DoubleIndex*)malloc(sizeof(DoubleIndex) * nq * dscfg.dGT);
		load_groundtruth(dscfg.gtfile, dscfg.gtdisfile, dscfg.dGT, nq, groundtruth);

		/* ------------------------------- performance collectors ----------------------------------*/
		float		hb_accuracy;			// hb-accuracy
		Cost		hb_cost;			// hb-cost
		DoubleIndex 	**hb_knnset;			// hb-query results

		/* >>> loading index */
		HB hb(K, d, n);				// init an HB object
		hb.load_index(hbif);

		/* >>> online query */
		printf("\n######################## %d clusters, %d-nn: ##########################\n", K, k);
		printf("data set: %s\n", dscfg.basefile);
		printf("n: %d, d: %d\n", n, d);

		hb_knnset = (DoubleIndex **)malloc(sizeof(DoubleIndex*)*nq);
		CostInit(&hb_cost);
		hb.search(&ds, &qds, hbif, k, hb_knnset, &hb_cost, alg);
		hb_accuracy = knn_precision(hb_knnset, groundtruth, nq, k, dscfg.dGT);
		iocount = hb_cost.page + hb_cost.point / ( PAGESIZE/(d*sizeof(float)) ) / PAGE_RAN_SE;
		fe = (1-(float)hb_cost.point / n) * 100;

		/* ------------------------------- report performance --------------------------- */
		printf("------------------ HB ---------------\n");
		printf("accuracy:\t%f\n", hb_accuracy);
		printf("FE(\%): %g\n", fe);
		printf("IO:\t%ld\n", iocount);
		CostDisplay(hb_cost);
		printf("%ld\t%ld\t%ld\t%g\t%ld\t%ld\t%ld\t%ld\t%f\n",
			iocount, hb_cost.page, hb_cost.point, fe, hb_cost.cpu, hb_cost.lowerbound, hb_cost.io, hb_cost.search, hb_accuracy);

		// record search result into file
		char resultLog[255];
		sprintf(resultLog, "%s.%s", ResultLogFile, dscfg.dsname);
		fp = open_file(resultLog, "a");
		fprintf(fp, "------ %s K-%d k-%d\n", dscfg.dsname, K, k);
		fputs("accu\tfe(\%)\tio\tpage\tpts\tttl(us)\tlb(us)\tio(us)\tsch(us)\n", fp);
		fprintf(fp, "%g\t%g\t%ld\t%ld\t%ld\t%ld\t%ld\t%ld\t%ld\t\n\n",
			hb_accuracy, fe,
			iocount, hb_cost.page, hb_cost.point,
			hb_cost.cpu, hb_cost.lowerbound, hb_cost.io, hb_cost.search);
		fclose(fp);

		/* ------------------------------- clean memory space -----------------------*/
		/* >>> free knn results */
		for(i = 0; i < nq; i++)
		{
			free(hb_knnset[i]); hb_knnset[i] = NULL;
		}
		free(hb_knnset); hb_knnset = NULL;

	    	/* >>> free allocation */
		fDataSet_unset(&ds);
		fDataSet_unset(&qds);
		free(groundtruth); groundtruth = NULL;
	}
	
	return 0;
}
