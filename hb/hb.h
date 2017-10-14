/**
 * \file hb.h
 * \brief basic operations of the HB class
 */
#ifndef HB_H
#define HB_H
#include "common.h"
#include "cost.h"
#include "clustering.h"

class HB : public Clustering{
public:
	/**
	 *	inner lb distance for all points in each cluster, [K*...]
	 * 		stored in an inverted index of [K * ...] size
	 *		in each cluster, the lb sorted in ascending order, therefore
	 *		the first lb of each cluster innerLB[][0] is the lb of the cluster
	 *		which is maintained by HB
	 */
	DoubleIndex			**innerLB;		


	HB(int _nc, int _d, int _n) : Clustering(_nc, _d, _n){ 
		innerLB = NULL;
	}
	~HB();

	
	
	/// figure out the inner distance for points in each cluster
	void inner_lb_distance_OnePerPoint(const fDataSet *ds);

	/**
	 * \brief Store HB index structure into file
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
	 * @param 	whether use [true lower bound]
	 */
	void search(const fDataSet *baseset, const fDataSet *queryset, char *folder, int nk, DoubleIndex **knnset, Cost *cost, int lb_type);

	/********************* basic operations *******************/
	/**
	 * \brief calculate all the lower bounds between query and all clusters
	 *		note that: lowerbound = max(dis(q,ci)) + innerLB[ci]
	 * @param 	sortflag 	[I do not know what this is.]
	 */
	void lowerbound(DoubleIndex *lb, const float *query, bool sortflag);

	/**
	 * \brief calculate all the true lower bounds between query and all clusters
	 *		note that: true lowerbound is the minimum distance between q and all member points in a cluster
	 * @param	lb			lowerbound of each cluster (DoubleIndex)
	 * @param 	ds 			true lb needs original data points
	 */
	void true_lowerbound(DoubleIndex *lb, const float *query, const fDataSet *ds);

	/**
 	 * 	a kind of pseudo lower bound which is the cross point of qC to H_CC'
	 */
	void crosspoint_lowerbound(DoubleIndex *lb, const float *query);


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
