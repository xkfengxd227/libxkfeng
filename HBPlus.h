/**
 * \file HBPlus.h
 * \brief Declarations of basic operations for HBPlus
 */
#ifndef HBPLUS_H
#define HBPLUS_H
#include "common.h"
#include "cost.h"
#include "clustering.h"
#include "heap.h"

/**
 * \brief Store the index of HBPlus into file
 * @param	c	the clustering structure
 * @param	indexfolder		the path to store the index
 * @param	ds	the data point set
 */
void HBPlus_IndexIntofile(Clustering *c, char *indexfolder, fDataSet *ds);

/**
 * \brief excute knn search based on HBPlus
 * @param	c	the clustering structure
 * @param	qds the query point set
 * @param	m	
 * @param	alpha	
 * @param	R 
 * @param	r_centroid 
 * @param	indexfolder		
 * @param	nk	number of nns
 * @param	knnset	the knn set
 * @param	cost	the cost
 */
void HBPlus_Search(Clustering *c, fDataSet *qds, int m, float alpha, float *R, float *r_centroid, char *indexfolder, int nk, DoubleIndex **knnset, Cost *cost);

/********************* basic operations *******************/
/**
 * \brief Figure out the LB for each centroid, using R to estimate, and alpha to filter seperate hyperplane bound
 * @param	lowerbound	the lowerbound value (DoubleIndex)
 * @param	query		the query point
 * @param	r_c			the rotated query
 * @param	centroid	the centroids
 * @param	innerLB		the inner lowerbounds
 * @param	ncenter		the number of clusters
 * @param	d			the dimension
 * @param	m			the reduced dimension
 * @param	alpha		a parameter when evaluating lowerbounds
 */
void HBPlus_LowerBound(DoubleIndex *lowerbound, const float *query, const float *r_c, const float *centroid, DoubleIndex **innerLB, int ncenter, int d, int m, float alpha);

/**
 * \brief Load data points from cluster file, with a limitation for LB
 * @param	filename	the cluster file name
 * @param	num			number of data points to load
 * @param	d			dimension
 * @param	set			to store loaded data points
 * @param	set_num		to store loaded id of data points
 * @param	lblimit		another stop flag of lowerbound when load data points
 */
int HBPlus_ClusterFromFile(const char *filename, int num, int d, float *set, int *set_num, double lblimit);

/**
 * \brief Generate the rotate matrc
 * @param	R	the rotated matrix
 * @param	m	the reduced dimension
 * @param	d	the dimension
 */
void HBPlus_GenerateRotate(float *R, int m, int d);

/**
 * \brief  Rotate centroids with R
 * @param	r_c		the rotated centers
 * @param	c		the original centers
 * @param	R		the rotating matrix
 * @param	k		the number of nns
 * @param	d		the dimension
 * @param	m		the reduced dimensions
 */ 
void HBPlus_RotateCentroid(float *r_c, float *c, float *R, int k, int d, int m);
#endif

