/**
 *	main script for clustering
 *	
 *	task: generate clusters for a specific dataset
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mainext.h"
#include "common.h"
#include "/media/xikafe/dataset/dsbconfig.h"
extern "C"{
	#include <yael/nn.h>
	#include <yael/vector.h>
	#include <yael/kmeans.h>
	#include <yael/machinedeps.h>
}

int main(int argc, char *argv[])
{
	/* ------------------------------- running parameters ----------------------------------*/
	int		n, d, nl;			// n-base, dimension, n-learning
	int		K, h;				// sub-cluster factor, depth of hierarchical clusters
	int 	mk;					// member's k-NN
	DSBConfig	dscfg;
	int 	logOn = 0;

	/* >>> accept needed parameters */
	if(!_input_parameters(argc, argv, dscfg.dsname, &K, &h, &mk, &logOn)){
		exit(-1);
	}
	config_dataset(dscfg.dsname, &dscfg);		// assemble dataset parameters

	/* ------------------------------- data ----------------------------------*/
	fDataSet	ds;				// base dataset
	fDataSet	lds;				// train(learn) dataset
	n = dscfg.n;
	d = dscfg.d;
	nl = dscfg.nl;

	printf("checking parameters ... \nhb: -ds %s -n %d -d %d -nl %d -K %d -h %d -mk %d\n",
		dscfg.dsname, n, d, nl, K, h, mk);

	/* >>> load data: base/learn/query dataset */
	fDataSet_init(&ds, n, d);
	fDataSet_init(&lds, nl, d);
	fvecs_read(dscfg.basefile, d, n, ds.data);
	fvecs_read(dscfg.learnfile, d, nl, lds.data);

	/* ------------------------------- clustering ----------------------------------*/
	/// learning for centroids
	float *centroid = fvec_new(K * d);
	printf("-------------- kmeans on learning set -------------\nnlearing-%d, nt-%d\n", nl, CLUSTERING_NTHREAD);
	float quantierror = kmeans(d, nl, K, CLUSTERING_NITER, lds.data, CLUSTERING_NTHREAD, CLUSTERING_SEED, CLUSTERING_NREDO, centroid, NULL, NULL, NULL);
	printf(">>> finished clustering learning, quantization error: %f\n", quantierror);

	/// assign centroids for each base vectors: by finding 1-nn among all centroids for each base vector: query=basedata, dataset=centroids
	int 	*assign = ivec_new_set(n*mk, -1);
	float	*tmp_dis = fvec_new_set(n*mk, -1);
	knn_full_thread (	2,		// euclidean distance
				n, K, d, 
				mk, 			// k-NNs
				centroid, ds.data, NULL, assign, tmp_dis, CLUSTERING_NTHREAD);

	/// store clustering results: centroid assign
	char 	filepath[255];
	sprintf(filepath, "%s/%s/bin/%s_cluster%d.centroid.fvecs", DATASET_FOLDER, dscfg.dsname, dscfg.dsname, K);
	fvecs_write(filepath, d, K, centroid);
	sprintf(filepath, "%s/%s/bin/%s_cluster%d.assign.ivecs", DATASET_FOLDER, dscfg.dsname, dscfg.dsname, K);
	ivecs_write(filepath, mk, n, assign);

	/* ------------------------------- clean memory space -----------------------*/
	fDataSet_unset(&ds);
	fDataSet_unset(&lds);
	FREE(centroid);
	FREE(assign);
	FREE(tmp_dis);
	return 0;
}
