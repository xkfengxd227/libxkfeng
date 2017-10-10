#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
extern "C"{
  #include <yael/vector.h>
}
#include <assert.h>
#include "hb.h"
#include "common.h"
#include "clustering.h"
#include "heap.h"

HB::~HB(){
	if(innerLB != NULL){
		for(int i = 0; i < ncenter; i++){
			FREE(innerLB[i]);
		}
		free(innerLB);   innerLB = NULL;
	}
}

void HB::inner_lb_distance_OnePerPoint(const fDataSet *ds)
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
			
			/* for each other centroid */
			for(otheri = 0; otheri < ncenter; otheri++)
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

void HB::index_into_file(const char *folder)
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
}

bool HB::index_exists(const char *folder){
	char	filename[255] = {'\0'};
    bool status = true;

    status &= cluster_exists(folder);   /* cluster files */
	sprintf(filename, "%s/%s", folder, FileInnerLB);   /* inner lb file */
	status &= file_exists(filename);

	return status;
}

void HB::load_index(const char *folder){
    ASSERTINFO(folder == NULL || strlen(folder) == 0, "IPP");
	ASSERTINFO(ncenter < 0 || d <= 0, "ncenter or d is invalid");
    ASSERTINFO(!index_exists(folder), "index not exists or not integrated");

	char filename[255] = {'\0'};
	FILE *fp;
	int nci, i, cnt_member;

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
        ASSERTINFO(innerLB[nci] == NULL, "Failed to allocate space for inner LB");

        for(i = 0; i < cnt_member; i++)
        {
            fscanf(fp, " %d %lf", &innerLB[nci][i].id, &innerLB[nci][i].val);
        }
    }
    fclose(fp);
}

void HB::search(const fDataSet *baseset, const fDataSet *queryset, char *folder, int nk, DoubleIndex **knnset, Cost *cost, int lb_type)
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

	char logfile[255];
	FILE *fplb, *fpsch, *fpcost;
	sprintf(logfile, "%s/lb.txt", LogFolder);
	fplb = open_file(logfile, "w");
	sprintf(logfile, "%s/search.txt", LogFolder);
	fpsch = open_file(logfile, "w");
	sprintf(logfile, "%s/cost.txt", LogFolder);
	fpcost = open_file(logfile, "w");


	for(qi = 0; qi < nq; qi++)
	{
		fprintf(fpsch, "q-%d\n", qi);
		fprintf(fplb, "q-%d\n", qi);

		/// initialize the cost recorder
		CostInit(&costi);
		gettimeofday(&tvb, NULL);

		/// the qi-th query
		memcpy(query, queryset->data+qi*d, sizeof(float)*d);
		knnset[qi] = (DoubleIndex*)malloc(sizeof(DoubleIndex)*nk);
		/// calculate and sort the lower bounds between query and all clusters to get the right order
		gettimeofday(&tvb_lb, NULL);

		// switch to different branch according to lb_type
		if(lb_type == (int)Algorithm_Search_TrueLB){
			true_lowerbound(lb, query, baseset);
		}else if(lb_type == (int)Algorithm_Search){
			lowerbound(lb, query, true);
		}else if(lb_type == (int)Algorithm_Search_CrossLB){
			crosspoint_lowerbound(lb, query);
		}

		gettimeofday(&tve_lb, NULL);
		costi.lowerbound = timediff(tvb_lb, tve_lb);

		for(int i = 0; i < ncenter; i++){
			fprintf(fplb, "%d %d %.6f\n", i, lb[i].id, lb[i].val);
		}


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

			fprintf(fpsch, "%d: %d %d [%.6f %.6f]\n", i, cid, set_num, knn_R, lb[i].val);

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

		fprintf(fpcost, "q-%d %.6f\n", qi, (1-costi.point / (float)baseset->n) * 100);
	}
	CostMultiply(cost, 1/(float)nq);

	fclose(fplb);
	fclose(fpsch);
	fclose(fpcost);

	free(set_vector); set_vector = NULL;
	free(query); query = NULL;
	free(lb); lb = NULL;
}

void HB::lowerbound(DoubleIndex *lb, const float *query, bool sortflag)
{
	int i, j, nci, otheri, idi, idj;
	float max_dis, temp_dis;
	float *center = fvec_new(d);
	float *ocenter = fvec_new(d);
	DoubleIndex *sqdis_query_centroid;				// square distance between query and all centroids
	float *centroid_distance_map = fvec_new(ncenter*ncenter);

	/// prepare the query centroid square distance map
	sqdis_query_centroid = (DoubleIndex*)malloc(sizeof(DoubleIndex)*ncenter);
	for(nci = 0; nci < ncenter; nci++)
	{
		sqdis_query_centroid[nci].id = nci;
		memcpy(center, centroid+nci*d, sizeof(float)*d);
		sqdis_query_centroid[nci].val = odistance_square(query, center, d);
	}
	// sort
	DI_MergeSort(sqdis_query_centroid, 0, ncenter-1);

	/// prepare the center distance map
	for(nci = 0; nci < ncenter; nci++)
	{
		memcpy(center, centroid+nci*d, sizeof(float)*d);
		for(otheri = 0; otheri <= nci; otheri++)
		{
			memcpy(ocenter, centroid+otheri*d, sizeof(float)*d);
			centroid_distance_map[nci*ncenter+otheri] = odistance(center, ocenter, d);
			centroid_distance_map[otheri*ncenter+nci] = centroid_distance_map[nci*ncenter+otheri];
		}
	}

	/// calculate the lower bounds
	lb[0].id = sqdis_query_centroid[0].id;
	lb[0].val = FLOAT_ZERO;
	for(i = 1; i < ncenter; i++)
	{
		idi = sqdis_query_centroid[i].id;
		// work out the (i-th id)-th cluster's lower bound
		max_dis = FLOAT_ZERO;
		for(j = 0; j < i; j++)
		{
			idj = sqdis_query_centroid[j].id;
			temp_dis = (sqdis_query_centroid[i].val - sqdis_query_centroid[j].val)
					 / (2*centroid_distance_map[idi*ncenter+idj]);
			if(f_bigger(temp_dis, max_dis))
			{// a larger lower bound distance
				max_dis = temp_dis;
			}
		}
		lb[i].id = idi;
		lb[i].val = max_dis + innerLB[idi][0].val;
	}
	/// sort lower bounds
	if(sortflag){
		DI_MergeSort(lb, 0, ncenter-1);
	}

	free(center); center = NULL;
	free(ocenter); ocenter = NULL;
	free(sqdis_query_centroid); sqdis_query_centroid = NULL;
	free(centroid_distance_map); centroid_distance_map = NULL;
}

void HB::true_lowerbound(DoubleIndex *lb, const float *query, const fDataSet *ds){
	
	int n = ds->n;

	/// distance between q and all vectors
	float *dis = fvec_new(n);
	odis_query_dataset(ds->data, query, d, n, dis);

	/// lb of each cluster
	for(int nci = 0; nci < ncenter; nci++){
		float min_dis = FLOAT_MAX;
		for(int i = 0; i < member[nci].size(); i++){
			int pid = member[nci][i];
			if (f_bigger(min_dis, dis[pid])){
				min_dis = dis[pid];
			}
		}

		lb[nci].id = nci;
		lb[nci].val = min_dis;
	}

	DI_MergeSort(lb, 0, ncenter-1);

	FREE(dis);
}

void HB::crosspoint_lowerbound(DoubleIndex *lb, const float *query){
	/* prepare */
	// distance between q and all clusters: sorted
	DoubleIndex *qcdis_asc = (DoubleIndex*)malloc(sizeof(DoubleIndex) * ncenter);
	for(int i = 0; i < ncenter; i++){
		qcdis_asc[i].val = odistance(query, centroid+i*d, d);
		qcdis_asc[i].id = i;
	}

	// sort all distance between q and C
	DI_MergeSort(qcdis_asc, 0, ncenter-1);

	// calculate cross point lb for each cluster
	lb[0].id = qcdis_asc[0].id;	
	lb[0].val = 0;					// lb of the cluster which q fall into is 0

	for(int ci = 1; ci < ncenter; ci++){	// begins from the 2nd cluster
		int cid = qcdis_asc[ci].id;
		float max_lb = FLOAT_ZERO;
		for(int oi = 0; oi < ci; oi ++){	// separating boundary: [o]ther cluster is nearer to q than ci-th cluster
			/// for another cluster
			int oid = qcdis_asc[oi].id;

			// distance between q and the crosspoint
			float lbcross = crosspoint_distance(query, centroid+d*cid, centroid+d*oid, d, qcdis_asc[ci].val);

			if(f_bigger(lbcross, max_lb)){
				max_lb = lbcross;
			}
		}
		lb[ci].id = cid;
		lb[ci].val = max_lb + innerLB[cid][0].val;
	}

	DI_MergeSort(lb, 0, ncenter-1);

	FREE(qcdis_asc);
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