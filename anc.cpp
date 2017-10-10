#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <math.h>
extern "C"{
	#include <yael/vector.h>
	#include <yael/machinedeps.h>
	#include <yael/nn.h>
}
#include <assert.h>
#include "anc.h"
#include "common.h"
#include "cost.h"
#include "clustering.h"
#include "heap.h"

ANC::~ANC(){
	if(innerLB != NULL){
		for(int i = 0; i < ncenter; i++){
			FREE(innerLB[i]);
		}
		free(innerLB);   innerLB = NULL;
	}
}

float ANC::neighbor_cluster_estimation(const fDataSet *ds, int nth)
{
	/// check for necessary data: centroids, basedata
	ASSERTINFO(ds == NULL || centroid == NULL || ds->data == NULL, "IPP");

	/// prepare for necessary variables
	neighbor.resize(ncenter);
	int	i, iclu = -1, ineighbor = -1;
	int	K = ncenter;
	int	n = ds->n;
	int	*tmp_assign = ivec_new_set(n * g, -1);
	float	*tmp_dis = fvec_new_0(n * g);
	int *neighbor_flag = ivec_new_set(ncenter*ncenter, 0);

	/// find k-nn among all centroids for each base vector: query=basedata, dataset=centroids, k=2 for neighbor cluster
	knn_full_thread (	
				2,		// euclidean distance
				n, K, d, 
				g,		// g-nn
				centroid, ds->data, NULL, tmp_assign, tmp_dis, nth);

	// extract neighbor clusters for each cluster
	for(i = 0; i < n; i++)
	{
		iclu = tmp_assign[i*g];					// current cluster = current point's 1-NN
		for (int ig = 1; ig < g; ig++){
			ineighbor = tmp_assign[i*g+ig];				// current neighbor cluster = current point's g-thNN
			if(0 == neighbor_flag[iclu*ncenter+ineighbor]){
				neighbor[iclu].push_back(ineighbor);
				neighbor_flag[iclu*ncenter+ineighbor] = 1;
			}
		}
	}

	puts("end neighbor");

	// check number of neighbor cluster
	for(i = 0; i < K; i++){
		ASSERTINFO(neighbor[i].size() == 0, "warning: there is a cluster who has no neighbors");
	}
	puts(">>> finished neighbor cluster registration");

	///### display neighbor cluster count
	puts(">>> neighbor cluster");
	int sum_neighbor = 0;
	for(i = 0;  i < K; i++){
		// printf("\n%d - %d\t", i, neighbor[i].size());
		sum_neighbor += neighbor[i].size();
	}
	float avg_neighbor = sum_neighbor / (float)K;

	/*
	if(K <= 10){
		for(i = 0;  i < K; i++){
			printf("\n%d - %d\t", i, neighbor[i].size());
			for(ineighbor = 0; ineighbor < neighbor[i].size(); ineighbor++){
				printf("%d ", neighbor[i][ineighbor]);
			}
		}
	}*/
	

	/// disallocate space
	FREE(tmp_assign);
	FREE(tmp_dis);

	return avg_neighbor;
}

float ANC::average_neighbor(){
	float avg = 0;
	for(int i = 0; i < ncenter; i++){
		avg += neighbor[i].size();
	}

	return avg / ncenter;
}

void ANC::inner_lb_distance_OnePerPoint(const fDataSet *ds)
{
	int i, j, nci, otheri;
	float dis = 0;
	float *xcenter = fvec_new(d);
	float *ocenter = fvec_new(d);
	float *x = fvec_new(d);
	// distance between each centroid pair
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

	// each point has an innerLB
	// initialize the storing space for inner distance of each member point
	for(nci = 0; nci < ncenter; nci++)
	{
		/// cnt_member_points
		int cnt_member = member[nci].size();
		innerLB[nci] = (DoubleIndex*)malloc(sizeof(DoubleIndex) * cnt_member);
		for(i = 0; i < cnt_member; i++)
		{
			innerLB[nci][i].id = -1;
			innerLB[nci][i].val = FLOAT_MAX;
		}
	}

	for(nci = 0; nci < ncenter; nci++)
	{
		/* in each centroid */
		memcpy(xcenter, centroid+nci*d, sizeof(float)*d);   // the current centroid
		int cnt_member = member[nci].size();	// cnt member points
		
		/* for each member points */
		for(i = 0; i < cnt_member; i++){
			memcpy(x, ds->data+member[nci][i]*d, sizeof(float)*d);
			
			/* for each neighbor cluster */
			int cnt_neighbor = neighbor[nci].size();
			for(otheri = 0; otheri < cnt_neighbor; otheri++)
			{
				if(otheri != nci)
				{
					memcpy(ocenter, centroid+otheri*d, sizeof(float)*d);
					dis = (odistance_square(x, ocenter, d) - odistance_square(x, xcenter, d)) / (2*centroid_dis_map[nci*ncenter+otheri]);
					if(f_bigger(innerLB[nci][i].val, dis))
					{// update using smaller distance
						innerLB[nci][i].val = dis;
						innerLB[nci][i].id = member[nci][i];		  // id is the data point
					}
				}
			}
		}
		// sort member data points along the innerLB distance in the nci-th cluster
		DI_MergeSort(innerLB[nci], 0, cnt_member-1);
	}

	free(centroid_dis_map); centroid_dis_map = NULL;
	free(ocenter); ocenter = NULL;
	free(xcenter); xcenter = NULL;
	free(x); x = NULL;
}

void ANC::index_into_file(const char *folder)
{
	char filename[255] = {'\0'};
	FILE *fp;
	int nci, i;

	/* store clusters */
	cluster_into_file(folder);

	/* inner lower bounds */
	sprintf(filename, "%s/%s", folder, FileInnerLB);
	fp = open_file(filename, "w");
	for(nci = 0; nci < ncenter; nci++)
	{
        // member points of each cluster
		int cnt_member = member[nci].size();
		fprintf(fp, "%d", cnt_member);
		for(i = 0; i < cnt_member; i++)
		{																  // centroids
			fprintf(fp, " %d %lf", innerLB[nci][i].id, innerLB[nci][i].val);
		}
		fputc('\n', fp);
	}
	fclose(fp);

	/* neighbor cluster */
	sprintf(filename, "%s/%s", folder, FileNeighborCluster);
	fp = open_file(filename, "w");
	for(nci = 0; nci < ncenter; nci++)
	{
        // member points of each cluster
		int cnt_neighbor = neighbor[nci].size();
		fprintf(fp, "%d", cnt_neighbor);
		for(i = 0; i < cnt_neighbor; i++)
		{																  // centroids
			fprintf(fp, " %d", neighbor[nci][i]);
		}
		fputc('\n', fp);
	}
	fclose(fp);

}

bool ANC::index_exists(const char *folder){
	char	filename[255] = {'\0'};
    bool status = true;

    status &= cluster_exists(folder);   /* cluster files */
	sprintf(filename, "%s/%s", folder, FileInnerLB);   /* inner lb file */
	status &= file_exists(filename);
	sprintf(filename, "%s/%s", folder, FileNeighborCluster);   /* neighbor cluster file */
	status &= file_exists(filename);

	return status;
}

void ANC::load_index(const char *folder){
    ASSERTINFO(folder == NULL || strlen(folder) == 0, "IPP");
	ASSERTINFO(ncenter < 0 || d <= 0, "ncenter or d is invalid");
    ASSERTINFO(!index_exists(folder), "index not exists or not integrated");

	char filename[255] = {'\0'};
	FILE *fp;
	int nci, i, cnt_member, cnt_neighbor;

	/// load clusters
	load_cluster(folder);

	/// allocate space for data
	innerLB = (DoubleIndex**)malloc(sizeof(DoubleIndex*)*ncenter);
	for(nci = 0; nci < ncenter; nci++){
		innerLB[nci] = NULL;
	}

    /// load inner lower bounds
    sprintf(filename, "%s/%s", folder, FileInnerLB);
    fp = open_file(filename, "r");
    for(nci = 0; nci < ncenter; nci++)
    {
        // member points of each cluster
        fscanf(fp, "%d", &cnt_member);
        ASSERTINFO(cnt_member != member[nci].size(), "count of lb not match to member points, error");
        innerLB[nci] = (DoubleIndex*)malloc(sizeof(DoubleIndex) * cnt_member);
        for(i = 0; i < cnt_member; i++)
        {
            fscanf(fp, " %d %lf", &innerLB[nci][i].id, &innerLB[nci][i].val);
        }
    }
    fclose(fp);

    /// neighbor clusters
    sprintf(filename, "%s/%s", folder, FileNeighborCluster);
    fp = open_file(filename, "r");
    for(nci = 0; nci < ncenter; nci++)
    {
        // member points of each cluster
        fscanf(fp, "%d", &cnt_neighbor);
        vector<int> i_neighbor(cnt_neighbor);
        for(i = 0; i < cnt_neighbor; i++)
        {
            fscanf(fp, " %d", &i_neighbor[i]);
        }

        neighbor.push_back(i_neighbor);
    }
    fclose(fp);

    printf("%s\n", filename);

    ASSERTINFO(neighbor.size() != ncenter, "error: [neighbor] size not match with cluster number");
}

void ANC::search(const fDataSet *baseset, const fDataSet *queryset, char *folder, int nk, DoubleIndex **knnset, Cost *cost, int lb_type)
{
	char filename[256];
	int nq = queryset->n,
		qi, i, set_i;
	int cid;
	float knn_R;
	float *set;
	int *set_id;
	int set_num;
	float *set_vector = NULL;
	float *query = fvec_new(d);
	DoubleIndex candidate;
	DoubleIndex *lb = (DoubleIndex*)malloc(sizeof(DoubleIndex)*ncenter);
																// lower bounds between query and all centers
	
    Cost costi;
	struct timeval tvb, tve, tvb_lb, tve_lb, tvb_io, tve_io;

	for(qi = 0; qi < nq; qi++)
	{
		/// initialize the cost recorder
		CostInit(&costi);
		gettimeofday(&tvb, NULL);

		/// the qi-th query
		memcpy(query, queryset->data+qi*d, sizeof(float)*d);
		knnset[qi] = (DoubleIndex*)malloc(sizeof(DoubleIndex)*nk);
		/// calculate and sort the lower bounds between query and all clusters to get the right order
		gettimeofday(&tvb_lb, NULL);
		if((int)Algorithm_Search_CrossLB == lb_type){
			lowerbound_crosspoint(lb, query);
		}else if((int)Algorithm_Search == lb_type){
			lowerbound(lb, query, true);	
		}
		
		gettimeofday(&tve_lb, NULL);
		costi.lowerbound = timediff(tvb_lb, tve_lb);

		/// search for knn
		set_vector = fvec_new(d);
		knn_R = FLOAT_MAX;
		i = 0;
		Heap heap(nk);
		while(i < ncenter)
		{
			cid = lb[i].id;
			// the i-th cluster
			if(f_bigger(lb[i].val, knn_R))
			{
				break;
			}
			// knn_R > lb[i], means there are candidates in the i-th cluster
			set_num = member[cid].size();
			set = fvec_new(set_num*d);
			set_id = ivec_new(set_num);
			
            /* we do not test the time cost of disk page for speed, we do not really load the data 
            sprintf(filename, "%s/%d.cluster", folder, cid);
			gettimeofday(&tvb_io, NULL);
			HB_ClusterFromFile(filename, set_num, d, set, set_id);
			gettimeofday(&tve_io, NULL);
            costi.io = costi.io + timediff(tvb_io, tve_io);
            */

            /* instead, we extract member points directly from the base set */
            for(int mi = 0; mi < set_num; mi++){
                int pts_id = member[cid][mi];
                set_id[mi] = pts_id;
                memcpy(set+mi*d, baseset->data+pts_id*d, sizeof(float)*d);
            }

            // update cost
			costi.page = costi.page + 1;
			costi.point = costi.point + set_num;

			for(set_i = 0; set_i < set_num; set_i++)
			{// calculate real distance between all candidates and query
				candidate.id = set_id[set_i];
				memcpy(set_vector, set+set_i*d, sizeof(float)*d);
				candidate.val = odistance(query, set_vector, d);
				if(heap.length < heap.MaxNum || f_bigger(heap.elem[0].val, candidate.val))
				{// heap is not full or new value is smaller, insert
					heap.max_insert(&candidate);
				}
			}
			knn_R = heap.elem[0].val;
			i++;
			// free
			free(set); set = NULL;
			free(set_id); set_id = NULL;
		}// end of search loop
		// printf("%d ", i);//
		memcpy(knnset[qi], heap.elem, sizeof(DoubleIndex)*heap.length);

		gettimeofday(&tve, NULL);
		costi.cpu = timediff(tvb, tve);
		costi.search = costi.cpu - costi.lowerbound - costi.io;

		/// sum new cost
		CostCombine(cost, &costi);
	}

	CostMultiply(cost, 1/(float)nq);

	free(set_vector); set_vector = NULL;
	free(query); query = NULL;
	free(lb); lb = NULL;
}


void ANC::lowerbound(DoubleIndex *lb, const float *query, bool sortflag)
{
	int i, j, nci, otheri, ineighbor, id_n;
	float max_dis, temp_dis, sdis_q_c, sdis_q_nc;
	float *center = fvec_new(d);
	float *ocenter = fvec_new(d);
	DoubleIndex *sqdis_query_centroid;				// square distance between query and all centroids


	/// prepare the query centroid square distances
	sqdis_query_centroid = (DoubleIndex*)malloc(sizeof(DoubleIndex)*ncenter);
	for(nci = 0; nci < ncenter; nci++)
	{
		sqdis_query_centroid[nci].id = nci;
		memcpy(center, centroid+nci*d, sizeof(float)*d);
		sqdis_query_centroid[nci].val = odistance_square(query, center, d);
	}
	
	/// figure out lower bounds for each cluster

	for(nci = 0; nci < ncenter; nci++){
		int cnt = 0;

		sdis_q_c = sqdis_query_centroid[nci].val;			// square dis between (q and C)
		memcpy(center, centroid+nci*d, sizeof(float)*d);		// centroid of C
		
		max_dis = FLOAT_ZERO;
		for(i = 0; i < neighbor[nci].size(); i++){			// all neighbor clusters
			id_n = neighbor[nci][i];
			sdis_q_nc = sqdis_query_centroid[id_n].val;			// square dis between (q and neighbor cluster)
			if(f_bigger(sdis_q_c, sdis_q_nc)){				// separating hyperplane
				cnt += 1;

				memcpy(ocenter, centroid+id_n*d, sizeof(float)*d);	// centroid of the neighbor cluster

				temp_dis = (sdis_q_c - sdis_q_nc)
						 / (2*odistance(center, ocenter, d));
				if(f_bigger(temp_dis, max_dis))
				{// a larger lower bound distance
					max_dis = temp_dis;
				}
			}
		}

		lb[nci].id = nci;
		lb[nci].val = max_dis;
	}

	/// sort lower bounds
	if(sortflag){
		DI_MergeSort(lb, 0, ncenter-1);
	}

	/// ### store into files
	FILE *fp = open_file("lowerbound.txt", "w+");
	for(i = 0; i < ncenter; i++){
		fprintf(fp, " %d-%f", lb[i].id, lb[i].val);
	}
	fputc('\n', fp);
	fclose(fp);

	free(center); center = NULL;
	free(ocenter); ocenter = NULL;
	free(sqdis_query_centroid); sqdis_query_centroid = NULL;
}

void ANC::lowerbound_crosspoint(DoubleIndex *lb, const float *query){
	int i, j, nci, otheri, ineighbor, id_n;
	float max_dis, temp_dis, sdis_q_c, sdis_q_nc;
	float *center = fvec_new(d);
	float *ocenter = fvec_new(d);
	DoubleIndex *sqdis_query_centroid;				// square distance between query and all centroids


	/// prepare the query centroid square distances
	sqdis_query_centroid = (DoubleIndex*)malloc(sizeof(DoubleIndex)*ncenter);
	for(nci = 0; nci < ncenter; nci++)
	{
		sqdis_query_centroid[nci].id = nci;
		memcpy(center, centroid+nci*d, sizeof(float)*d);
		sqdis_query_centroid[nci].val = odistance_square(query, center, d);
	}
	
	/// figure out lower bounds for each cluster
	for(nci = 0; nci < ncenter; nci++){
		int cnt = 0;

		sdis_q_c = sqdis_query_centroid[nci].val;			// square dis between (q and C)
		memcpy(center, centroid+nci*d, sizeof(float)*d);		// centroid of C
		
		max_dis = FLOAT_ZERO;
		for(i = 0; i < neighbor[nci].size(); i++){			// all neighbor clusters
			id_n = neighbor[nci][i];
			sdis_q_nc = sqdis_query_centroid[id_n].val;			// square dis between (q and neighbor cluster)
			if(f_bigger(sdis_q_c, sdis_q_nc)){				// separating hyperplane
				cnt += 1;

				memcpy(ocenter, centroid+id_n*d, sizeof(float)*d);	// centroid of the neighbor cluster

				temp_dis = crosspoint_distance(query, center, ocenter, d, sqrt(sdis_q_c));

				if(f_bigger(temp_dis, max_dis))
				{// a larger lower bound distance
					max_dis = temp_dis;
				}
			}
		}

		lb[nci].id = nci;
		lb[nci].val = max_dis;
	}

	/// sort lower bounds
	DI_MergeSort(lb, 0, ncenter-1);

	/// ### store into files
	FILE *fp = open_file("lowerbound.txt", "w+");
	for(i = 0; i < ncenter; i++){
		fprintf(fp, " %d-%f", lb[i].id, lb[i].val);
	}
	fputc('\n', fp);
	fclose(fp);

	free(center); center = NULL;
	free(ocenter); ocenter = NULL;
	free(sqdis_query_centroid); sqdis_query_centroid = NULL;
}

/*
void HB_ClusterFromFile(const char *filename, int num, int d, float *set, int *set_num)
{
	int i;
	FILE *fp = fopen(filename, "rb");
	if(fp == NULL)
	{
		printf("error to open file %s\n", filename);
		exit(-1);
	}
	for(i = 0; i < num; i++)
	{
		fread(set_num+i, sizeof(int), 1, fp);
		fread(set+i*d, sizeof(float), d, fp);
	}
	fclose(fp);
}
*/