/**
 *	\file clustering.h
 *	\brief some basic definitions and declarations of the Clustering class
 */
#ifndef CLUSTERING_H
#define CLUSTERING_H
#include "common.h"

/// \brief the Clustering class
typedef struct
{
    /// offline processing
	int 				ncenter;		/// number of centroids
	int					niter;			/// number of kmeans iterations
	int					nthread;		/// number of processors used
	int 				seed;			/// random seed
	int 				nredo;			/// times of kmeans
	/// offline results
	float 			    *centroid;	    /// the centroids
	int                 *assign;        /// the belongingness of data points to clusters
	int                 *nassign;       /// counts of members in each cluster
	int 				**member;		/// member data points of each cluster

	DoubleIndex         **innerLB;      /// inner distance between member data points and the boundaries in each cluster
} Clustering;

/** \brief initialize some basic parameters of a Clustering object
 * @param		c			the Clustering object
 * @param 		nc		number of clusters
 * @param 		niter		number of iterations
 * @param 		nth 		number of threads
 * @param 		seed 	the seed of random
 * @param 		nredo 	number of clustering re-runs
 */
void C_Init(Clustering *c, int nc, int niter, int nth, int seed, int nredo);
/// release a Clustering object
void C_Delete(Clustering *c);
/// do clustering
void C_Clustering(Clustering *c, fDataSet *ds, fDataSet *lds);
/// figure out the inner distance in each cluster, then sort the
void C_InnerLBDistance(Clustering *c, fDataSet *ds);

#endif
