/**
 *	\file clustering.c
 *	\brief implementation of opeartions of the Clustering class
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <vector>
#include "clustering.h"
#include "common.h"
#include "dyarray.h"
extern "C"{
#include <yael/nn.h>
#include <yael/vector.h>
#include <yael/kmeans.h>
#include <yael/machinedeps.h>
}

void Cluster_init(Cluster *clu, int npts){
	ASSERTINFO(clu == NULL || npts <= 0, "IPP");
	clu->npts = npts;
	clu->data = NULL;
	clu->idx = ivec_new(npts);

	clu->type = ClusterType_Inner;
	DyArray_init(&clu->children, sizeof(int), -1);
	clu->cents = NULL;
}

void Cluster_unset(Cluster *clu){
	if(clu != NULL){
		FREE(clu->cents);
		FREE(clu->data);
		FREE(clu->idx);
		clu->npts = -1;
		DyArray_unset(&clu->children);
	}
}

int Cluster_getchild(Cluster *clu, int ichild){
	__assertinfo(clu == NULL || ichild < 0 || ichild >= clu->children.count, "IPP");
	return *(int*)DyArray_get(&clu->children, ichild, 1);
}

/* ------------------------------------------------- methods for the adaptive hierarchical clustering class -------------------------------------------- */

void ahc_clustering(DyArray *ahct, int bf, int rho, const fDataSet *ds){
	ASSERTINFO(ahct == NULL || bf <= 0 || rho <= 0 || ds == NULL, "IPP");

	int		n 	= ds->n;
	int		d 	= ds->d;
	Cluster	_clu, clu, *pclu = NULL, *p0clu = NULL;
	int		i;
	float	qerror;
	int		iclu, bfi, ni, ichild, ori_id;			// the pointer, branch factor and volume of the i-th cluster
	int 	*nassign = ivec_new_set(bf, 0);
	int 	*assign = NULL;
	float	*cent = fvec_new(d*bf);
	float	*mem_points = NULL;
	DyArray	*member = (DyArray*)malloc(sizeof(DyArray)*bf);

	/* initialize the first cluster (root) to add it to the ahc tree */
	Cluster_init(&clu, n);
	for(i = 0; i < n; i++){
		clu.idx[i] = i;
	}
	clu.type = ClusterType_Root;
	DyArray_add(ahct, (void*)&clu, 1);

	/* begin the loop of adaptive hierarchical clustering */
	iclu = 0;
	while(iclu < ahct->count){
		/* deal with the i-th cluster */
		// figure out the adaptive branch factor of the i-th cluster
		pclu = (Cluster*)DyArray_get(ahct, iclu, 1);
		ni = pclu->npts;
		bfi = i_min(bf, (int)round(ni / (float)rho));

		// deal with the cluster according to its size
		if(bfi < 2){
			/*
			 *	this is a leaf cluster
			 *	- mark it, release the children
			 *	* not necessary to store real data points
			 */
			pclu->type = ClusterType_Leaf;
		}else{
			printf("----------------- cluster %d, bfi-%d:\n", iclu, bfi);

			/*
			 * this is an inner cluster
			 * - divide it
			 */
			memcpy(&_clu, pclu, sizeof(Cluster));

			// extract data points from the original dataset according to the idx
			mem_points = fvec_new(ni * d);
			for(i = 0; i < ni; i++){
				memcpy(mem_points+i*d, ds->data+_clu.idx[i]*d, d);
			}

			// divide this cluster
			assign = ivec_new(ni);

			if(iclu == 30){
				int _a = 1;
				_a++;

				ivec_print(_clu.idx, _clu.npts);
			}

			qerror = kmeans(	d, ni, bfi, CLUSTERING_NITER, mem_points,
								CLUSTERING_NTHREAD | KMEANS_QUIET | KMEANS_INIT_BERKELEY, CLUSTERING_SEED, CLUSTERING_NREDO,
								cent, NULL, assign, nassign);

			// prepare space for members' ids
			for(i = 0; i < bfi; i++){
				DyArray_init(&member[i], sizeof(int), nassign[i]);
			}
			// extract member points' ids for each children cluster
			for(i = 0; i < ni; i++){
				ori_id = _clu.idx[i];
				DyArray_add(&member[assign[i]], (void*)&ori_id, 1);
			}

			// fulfill the type, centroids and the children of this cluster, add them to the ahct
			_clu.type = ClusterType_Inner;
			_clu.cents = fvec_new(d * bfi);
			memcpy(_clu.cents, cent, sizeof(float)*d*bfi);

			DyArray_init(&_clu.children, sizeof(int), bfi);
			for(i = 0; i < bfi; i++){
				Cluster_init(&clu, nassign[i]);
				memcpy(clu.idx, (int*)member[i].elem, sizeof(int)*nassign[i]);

				DyArray_add(&_clu.children, (void*)&ahct->count, 1);	/* the i-th child's position */
				DyArray_add(ahct, (void*)&clu, 1);						/* add the i-th child to the ahct */
			}

			/* as per the elems of ahct may change when expanding the space
			 * we decide to get the brand new address of the element
			 */
			pclu = (Cluster*)DyArray_get(ahct, iclu, 1);
			memcpy(pclu, &_clu, sizeof(Cluster));


			/* report */
			ivec_print(nassign, bfi);
			ivec_print((int*)_clu.children.elem, _clu.children.count);

			/* unset or release */
			FREE(mem_points);
			FREE(assign);
			for(i = 0; i < bfi; i++){
				DyArray_unset(&member[i]);
			}
		}

		// move to next cluster
		iclu++;
	}

	FREE(nassign);
	FREE(cent);
	FREE(member);
	pclu = NULL;
}

void ahc_index_into_file(DyArray *ahct, const char *folder, int d, int bf){
	ASSERTINFO(ahct == NULL || folder == NULL, "IPP");
	char	filename[255];
	FILE	*fp;
	int		i;
	Cluster	*pclu;
	int		_type, _bfi, _count;
	DyArray	leafs;

	/* traverse all elements in ahct, store them in split files */
	DyArray_init(&leafs, sizeof(int), ahct->count);
	for(i = 0; i < ahct->count; i++){
		pclu = (Cluster*)DyArray_get(ahct, i, 1);
		// assemble the cluster name
		sprintf(filename, "%s/%d%s", folder, i, HCluster_Postfix);
		fp = open_file(filename, "wb");

		// write into files:
		//		non-leaf: type, {bfi, centroids, children}, count, idx, [data]
		//			leaf: type, count, idx, [data]
		_type = (int)pclu->type;
		fwrite(&_type, sizeof(int), 1, fp);							/* type */

		if(pclu->type != ClusterType_Leaf){
			_bfi = pclu->children.count;
			fwrite(&_bfi, sizeof(int), 1, fp);							/* bfi */
			fwrite(pclu->cents, sizeof(float), d*_bfi, fp);				/* centroids */
			fwrite((int*)pclu->children.elem, sizeof(int), _bfi, fp);	/* children */
		}else{
			DyArray_add(&leafs, (void*)&i, 1);
		}
		_count = pclu->npts;
		fwrite(&_count, sizeof(int), 1, fp);						/* member count */
		fwrite(pclu->idx, sizeof(int), _count, fp);					/* member ids */

		fclose(fp);
	}

	/* generate the config file, storing:
	 *		- dimension
	 *		- total number of clusters
	 *		- total number of leaf clusters
	 *		- the leaf clusters' ids
	 */
	sprintf(filename, "%s/%s", folder, HCluster_ConfigFile);
	fp = open_file(filename, "wb");
	fwrite(&d, sizeof(int), 1, fp);								/* dimension */
	fwrite(&bf, sizeof(int), 1, fp);							/* branch factor */
	fwrite(&ahct->count, sizeof(int), 1, fp);					/* the total cluster count */
	fwrite(&leafs.count, sizeof(int), 1, fp);					/* count of leaf clusters */
	fwrite((int*)leafs.elem, sizeof(int), leafs.count, fp);		/* leaf clusters' ids */
	fclose(fp);

	/* release */
	pclu = NULL;
	DyArray_unset(&leafs);
}

bool ahc_check_index(const char *folder){
	__assertinfo(folder == NULL, "IPP");
	char	file[255];
	FILE	*fp;
	int		d, nclu;
	int		*buff = ivec_new(2);

	/* check out the config file */
	sprintf(file, "%s/%s", folder, HCluster_ConfigFile);
	if(!file_exists(file)){
		return false;
	}
	fp = open_file(file, "rb");
	fread(buff, sizeof(int), 2, fp);

	/* check out the existence of cluster files [first, end] */
	sprintf(file, "%s/0%s", folder, HCluster_Postfix);
	if(!file_exists(file)){
		return false;
	}else{
		sprintf(file, "%s/%d%s", folder, buff[1]-1, HCluster_Postfix);
		if(!file_exists(file)){
			return false;
		}
	}

	FREE(buff);
	return true;
}

bool ahc_index_from_file(DyArray *ahct, const char *folder){
	__assertinfo(ahct == NULL || folder == NULL, "IPP");
	char	file[255];
	int		i;
	FILE	*fp = NULL;
	int		d;
	int		nclu;
	Cluster	*pclu = NULL;
	int		bf;


	if(!ahc_check_index(folder)){
		return false;
	}

	// open the config file
	sprintf(file, "%s/%s", folder, HCluster_ConfigFile);
	fp = open_file(file, "rb");
	fread(&d, sizeof(int), 1, fp);
	fread(&bf, sizeof(int), 1, fp);
	fread(&nclu, sizeof(int), 1, fp);
	fclose(fp);

	// assemble the clustering list
	DyArray_init(ahct, sizeof(Cluster), nclu);
	for(i = 0; i < nclu; i++){
		pclu = ahc_load_a_cluster(folder, i, d, bf);
		DyArray_add(ahct, (void*)pclu, 1);
	}

	/* release */
	pclu = NULL;
	return true;
}

Cluster *ahc_load_a_cluster(const char *folder, int cid, int d, int bf){
	__assertinfo(folder == NULL || cid < 0, "IPP");
	Cluster	*clu = (Cluster*)malloc(sizeof(Cluster));
	char	file[255];
	FILE	*fp = NULL;
	int		_type, _count, _bfi;
	float	*cent = fvec_new(d*bf);
	int		*children = ivec_new(bf);

	sprintf(file, "%s/%d%s", folder, cid, HCluster_Postfix);
	fp = open_file(file, "rb");

	fread(&_type, sizeof(int), 1, fp);
	if(_type != (int)ClusterType_Leaf){
		fread(&_bfi, sizeof(int), 1, fp);
		fread(cent, sizeof(float), d*_bfi, fp);
		fread(children, sizeof(int), _bfi, fp);
	}

	fread(&_count, sizeof(int), 1, fp);
	Cluster_init(clu, _count);								/* count */
	fread(clu->idx, sizeof(int), _count, fp);				/* idx */
	fclose(fp);

	// fulfill type, centroids and children according to the type of the cluster
	clu->type = (ClusterType)_type;							/* type */
	if(ClusterType_Leaf == clu->type){
		DyArray_init(&clu->children, sizeof(int), 0);			/* bfi */
	}else{
		clu->cents = fvec_new(d * _bfi);
		memcpy(clu->cents, cent, sizeof(float) * d * _bfi);		/* cent */
		DyArray_init(&clu->children, sizeof(int), _bfi);			/* bfi */
		DyArray_add(&clu->children, (void*)children, _bfi);		/* children */
	}

	FREE(cent);
	FREE(children);
	return clu;
}

int	ahc_count_leaf(DyArray *ahct){
	__assertinfo(ahct == NULL, "IPP");
	int		count = 0;
	int		i;
	Cluster	*pclu;

	for(i = 0; i < ahct->count; i++){
		pclu = (Cluster*)DyArray_get(ahct, i, 1);
		if(pclu->type == ClusterType_Leaf){
			count++;
		}
	}
	return count;
}

int	ahc_quantize(DyArray *ahct, float *v, int d){
	__assertinfo(ahct == NULL || v == NULL || d <= 0, "IPP");
	int		iclu;
	Cluster	*pclu;
	int		nchild = 0;
	int		*_vassign = ivec_new(1);
	float	*_vdis = fvec_new(1);

	iclu = 0;
	/* traverse all clusters */
	while(true){
		// locate at the i-th cluster
		pclu = (Cluster*)DyArray_get(ahct, iclu, 1);
		if(pclu->type != ClusterType_Leaf){
			// if a cluster is not leaf, extract all centroid of its children
			nchild = pclu->children.count;

			// linear_knn(pclu->cents, nchild, v, d, 1, _vassign, _vdis);
			knn_full(2, 1, nchild, d, 1, pclu->cents, v, NULL, _vassign, _vdis);

			iclu = *(int*)DyArray_get(&pclu->children, _vassign[0], 1);
		}else{
			break;
		}
	}
	FREE(_vassign);
	FREE(_vdis);
	return iclu;
}

void ahc_unset(DyArray *ahct){
	int		i;
	Cluster	*pclu;

	if(ahct != NULL){
		for(i = 0; i < ahct->count; i++){
			pclu = (Cluster*)DyArray_get(ahct, i, 1);
			Cluster_unset(pclu);
		}
		DyArray_unset(ahct);
	}
	pclu = NULL;
}
/* ------------------------------------------------- methods for the Clustering class: usually for non-hierarchical clustering -------------------------------------------- */
Clustering::Clustering(int nc)
{
	ncenter = nc;
	centroid = NULL;
	assign = NULL;

	nassign = NULL;
	member = NULL;
	innerLB = NULL;
}

Clustering::~Clustering()
{
	int i;
	FREE(centroid);
	FREE(assign);
	FREE(nassign);
	if(member != NULL){
		for(i = 0; i < ncenter; i++){
			FREE(member[i]);
		}
		free(member);    member = NULL;
	}
	if(innerLB != NULL){
		for(i = 0; i < ncenter; i++){
			FREE(innerLB[i]);
		}
		free(innerLB);   innerLB = NULL;
	}
}

void Clustering::generate_cluster(fDataSet *ds, fDataSet *lds, int niter, int nth, int seed, int nredo)
{
	int	i, j;
	int	d = ds->d;
	int	n = ds->n;
	int	n_l = lds->n;

	/// allocate storage space for necessary data
	float	*tmp_dis = fvec_new_set(n, -1);
	centroid = fvec_new_set(ncenter*d, 0);
	assign = ivec_new_set(n, -1);
	nassign = ivec_new_0(ncenter);
	member = (int**)malloc(sizeof(int*)*ncenter);
	for(i = 0; i < ncenter; i++){
		member[i] = NULL;
	}

	/// learning for centroids
	printf("-------------- kmeans on learning set -------------\nn_l-%d, nt-%d\n", n_l, nth);
	float quantierror = kmeans(d, n_l, ncenter, niter, lds->data, nth, seed, nredo, centroid, NULL, NULL, NULL);
	printf(">>> finished clustering learning, quantization error: %f\n", quantierror);


	/// find 1-nn among all centroids for each base vector: query=basedata, dataset=centroids
	knn_full_thread (	
				2,				// euclidean distance
				n, ncenter, d, 
				1,				// 1-nn
				centroid, ds->data, NULL, assign, tmp_dis, nth);

	// assign the 1st-cluster to the assigned cluster
	// count members in each cluster
	for(i = 0; i < n; i++)
	{
		nassign[assign[i]] ++;
	}
	puts(">>> finished cluster assign for base dataset");

	/// extract clustering members
	extract_members(assign, nassign, member, n, ncenter);						// extract member points for each cluster
	puts(">>> after member points extraction");

	puts("member count for each cluster");
	for(i = 0;  i < ncenter; i++){
		printf("%d - %d\n", i, nassign[i]);
	}

	/// disallocation
	FREE(tmp_dis);
}

void Clustering::inner_lb_distance(fDataSet *ds)
{
    int i, j, nci, otheri;
    int d = ds->d;
    float dis = 0;
    float *xcenter = fvec_new(d);
    float *ocenter = fvec_new(d);
    float *x = fvec_new(d);
    float *centroid_dis_map = fvec_new_0(ncenter*ncenter);
    innerLB = (DoubleIndex **)malloc(sizeof(DoubleIndex*)*ncenter);
    for(i = 0; i < ncenter; i++){
		innerLB[i] = NULL;
	}
    /// prepare distances between each two centroids
    for(i = 0; i < ncenter; i++)
    {
        memcpy(xcenter, centroid+i*d, sizeof(float)*d);
        for(j = 0; j <= i; j++)
        {
            memcpy(ocenter, centroid+j*d, sizeof(float)*d);
            dis = odistance(xcenter, ocenter, d);
            centroid_dis_map[i*ncenter+j] = dis;
            if(i != j)
            {
                centroid_dis_map[j*ncenter+i] = dis;
            }
        }
    }

    // initialize the storing space for inner distance
    for(nci = 0; nci < ncenter; nci++)
    {
        innerLB[nci] = (DoubleIndex*)malloc(sizeof(DoubleIndex) * nassign[nci]);
        for(i = 0; i < nassign[nci]; i++)
        {
            innerLB[nci][i].id = -1;
            innerLB[nci][i].val = FLOAT_MAX;
        }
    }

    for(nci = 0; nci < ncenter; nci++)
    {// the nci-th centroid
        memcpy(xcenter, centroid+nci*d, sizeof(float)*d);
        for(otheri = 0; otheri < ncenter; otheri++)
        {// choose another centroid
            memcpy(ocenter, centroid+otheri*d, sizeof(float)*d);
            if(otheri != nci)
            {
                for(i = 0; i < nassign[nci]; i++)
                {
                    memcpy(x, ds->data+member[nci][i]*d, sizeof(float)*d);
                    dis = (odistance_square(x, ocenter, d) - odistance_square(x, xcenter, d)) / (2*centroid_dis_map[nci*ncenter+otheri]);
                    if(f_bigger(innerLB[nci][i].val, dis))
                    {// update using smaller distance
                        innerLB[nci][i].val = dis;
                        innerLB[nci][i].id = member[nci][i];          // id is the data point
                    }
                }
            }
        }
        // sort member data points along the innerLB distance in the nci-th cluster
        DI_MergeSort(innerLB[nci], 0, nassign[nci]-1);
    }

    free(centroid_dis_map); centroid_dis_map = NULL;
    free(ocenter); ocenter = NULL;
    free(xcenter); xcenter = NULL;
    free(x); x = NULL;
}
void Clustering::neighbor_cluster_estimation(const fDataSet *ds, int nth)
{
	/// check for necessary data: centroids, basedata
	ASSERTINFO(ds == NULL || centroid == NULL || ds->data == NULL, "IPP");

	/// prepare for necessary variables
	neighbor.resize(ncenter);
	int	i, iclu = -1, ineighbor = -1;
	int	K = ncenter;
	int	n = ds->n;
	int	d = ds->d;
	int	*tmp_assign = ivec_new_set(n * 2, -1);
	float	*tmp_dis = fvec_new_0(n * 2);
	int *neighbor_flag = ivec_new_set(ncenter*ncenter, 0);

	/// find k-nn among all centroids for each base vector: query=basedata, dataset=centroids, k=2 for neighbor cluster
	knn_full_thread (	
				2,		// euclidean distance
				n, K, d, 
				2,		// 2-nn
				centroid, ds->data, NULL, tmp_assign, tmp_dis, nth);

	// extract neighbor clusters for each cluster
	for(i = 0; i < n; i++)
	{
		iclu = tmp_assign[i*2];					// current cluster = current point's 1-NN
		ineighbor = tmp_assign[i*2+1];				// current neighbor cluster = current point's 2-NN
		if(0 == neighbor_flag[iclu*ncenter+ineighbor]){
			neighbor[iclu].push_back(ineighbor);
			neighbor_flag[iclu*ncenter+ineighbor] = 1;
		}

	}
	puts(">>> finished neighbor cluster registration");

	///### display neighbor cluster count
	puts(">>> neighbor cluster");
	int sum_neighbor = 0;
	for(i = 0;  i < K; i++){
		printf("\n%d - %d\t", i, neighbor[i].size());
		sum_neighbor += neighbor[i].size();
	}
	printf("\naveragely %lf neighbors\n", sum_neighbor / (float)K);

	if(K <= 500){
		for(i = 0;  i < K; i++){
			printf("\n%d - %d\t", i, neighbor[i].size());
			int ineighbor;
			for(ineighbor = 0; ineighbor < neighbor[i].size(); ineighbor++){
				printf("%d ", neighbor[i][ineighbor]);
			}
		}
	}
	printf("\naveragely %lf neighbors\n", sum_neighbor / (float)K);

	/// disallocate space
	FREE(tmp_assign);
	FREE(tmp_dis);
}
