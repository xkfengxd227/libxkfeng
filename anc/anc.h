/**
 * \file adaptive-neighbor-cluster(anc).h
 * \brief basic operations of the Adaptive Neighbor Cluster(ANC) class
 */
#ifndef ANC_H
#define ANC_H
#include "common.h"
#include "cost.h"
#include "clustering.h"

class ANC : public Clustering{
public:

	int g;								/// rank range of neighbor clusters
										/// g=3: means the 2nd and the 3rd near cluster is choosed as neighbor cluster
	vector<vector<int> > neighbor;		/// neighbor clusters of each cluster

	/**
	 *	inner lb distance for all points in each cluster, [K*...]
	 * 		stored in an inverted index of [K * ...] size
	 *		in each cluster, the lb sorted in ascending order, therefore
	 *		the first lb of each cluster innerLB[][0] is the lb of the cluster
	 *		which is maintained by HB
	 */
	DoubleIndex			**innerLB;		


	ANC(int _nc, int _g, int _d, int _n) : Clustering(_nc, _d, _n){ 
		g = _g;
		innerLB = NULL;
	}
	~ANC();

	
	/** estimate neighbor clusters for each cluster
	 * return: average neighbor clusters
	 */
	float neighbor_cluster_estimation(const fDataSet *ds, int nth);

	/**
 	 * average neighbor number
	 */
	float average_neighbor();
	
	/// figure out the inner distance for points in each cluster
	void inner_lb_distance_OnePerPoint(const fDataSet *ds);

	/**
	 * \brief Store index structure into file: neighbor cluster, inner lb, cluster(centroid, member)
	 * @param	folder	the path to store clusters
	 */
	void index_into_file(const char *folder);

	/** \brief 	check if exists index 
	 * @param	folder	folder stores index files
	 * @return	check status
	 */
	bool index_exists(const char *folder);

	/** \brief load index from file
	 * @param   folder  the index folder
	 * @param	return	load status
	 */
	void load_index(const char *folder);

	/**
	 * \brief Search k-NN for all queries in queryset based on hb
	 * @param 	queryset	the query data set
	 * @param	folder	the cluster structure path
	 * @param 	nk	number of nns
	 * @param	knnset	to store knn
	 * @param	cost	the cost performance
	 * @param 	lbtype 	lowerbound type
	 */
	void search(const fDataSet *baseset, const fDataSet *queryset, char *folder, int nk, DoubleIndex **knnset, Cost *cost, int lb_type);

	/********************* basic operations *******************/
	/**
	 * \brief calculate all the lower bounds between query and all clusters
	 *		note that: lowerbound = max(dis(q,ci)) + innerLB[ci]
	 *					and max(dis(q,ci)) related to all neighbor clusters
	 */
	void lowerbound(DoubleIndex *lb, const float *query, bool sortflag);

	/**
	 * \brief calculate all the lower bounds between query and all clusters
	 *		note that: lowerbound = max(cross_dis(q,ci)) + innerLB[ci]
	 *					cross_dis(q,ci) is the distance between q and the crosspoint of qc with H
	 */
	void lowerbound_crosspoint(DoubleIndex *lb, const float *query);

	/**
	 * \brief Load data points from cluster on disk
	 * @param	filename	file storing clusters
	 * @param	num		number of data points to load
	 * @param	d		the dimension
	 * @param	set		store loaded data points
	 * @param	set_num		store loaded id of data points
	 */
	/*
	void HB_ClusterFromFile(const char *filename, int num, int d, float *set, int *set_num);
	*/
};

#endif
