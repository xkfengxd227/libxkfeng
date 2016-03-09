/**
 * \file HB.h
 * \brief basic operations of the HB class
 */
#ifndef HB_H
#define HB_H
#include "common.h"
#include "cost.h"
#include "clustering.h"

/**
 * \brief Store HB index structure into file
 * @param	c	the clustering sample
 * @param	folder	the path to store clusters
 * @param	ds	the data point set
 */
void HB_IndexIntoFile(const Clustering *c, const char *folder, const fDataSet *ds);

/** \brief 	check if exists index 
 * @param	folder	folder stores index files
 * @return	check status
 */
bool HB_IndexExists(const char *folder);

/** \brief load clustering index from file
 * @param   c       the Clustering object
 * @param   folder  the index folder
 * @param   d       the dimension
 * @param	return	load status
 */
bool HB_LoadIndex(Clustering *c, const char *folder, const fDataSet *ds);

/**
 * \brief Search nk-NN for all queries in queryset based on hb
 * @param	c	the clustering sample
 * @param 	queryset	the query data set
 * @param	folder	the cluster structure path
 * @param 	nk	number of nns
 * @param	knnset	to store knn
 * @param	cost	the cost performance
 */
void HB_Search(const Clustering *c, const fDataSet *queryset, char *folder, int nk, DoubleIndex **knnset, Cost *cost);

/**
 * \brief Write knnset into file
 * @param	filename	file to store knn results
 * @param	knnset		to store knn
 * @param 	nq	number of queries
 * @param	nk	number of nns
 */
void HB_KnnIntoFile(const char *filename, DoubleIndex **knnset, int nq, int nk);

/********************* basic operations *******************/
/**
 * \brief calculate all the lower bounds between query and all clusters
 * @param	lowerbound	lowerbound of each cluster (DoubleIndex)
 * @param	query		the query point
 * @param	centroid	the centroid vectors
 * @param	innerLB		inner lowerbound in each cluster
 * @param	ncenter		number of cluster
 * @param	d		the dimension
 */
void HB_LowerBound(DoubleIndex *lowerbound, const float *query, const float *centroid, DoubleIndex **innerLB, int ncenter, int d, bool sortflag);

/**
 * \brief Load data points from cluster
 * @param	filename	file storing clusters
 * @param	num		number of data points to load
 * @param	d		the dimension
 * @param	set		store loaded data points
 * @param	set_num		store loaded id of data points
 */
void HB_ClusterFromFile(const char *filename, int num, int d, float *set, int *set_num);

#endif
