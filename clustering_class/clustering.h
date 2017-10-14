/**
 *	\file clustering.h
 *	\brief some basic definitions and declarations of the Clustering class
 */
#ifndef CLUSTERING_H
#define CLUSTERING_H
#include "common.h"
#include "dyarray.h"
extern "C"{
#include <yael/vector.h>
#include <vector>
}

///	\brief	cluster type: root, inner cluster, leaf cluster
typedef enum {ClusterType_Root, ClusterType_Inner, ClusterType_Leaf} ClusterType;

#define		HCluster_Postfix		".hcluster"			// file postfix of hierarchical cluster
#define		HCluster_ConfigFile		"_config.hcluster"	// the config file of hierarchical clustering

///	\brief	the Cluster class (the variable order is the store order)
typedef struct{
	/* member */
	int 				npts;			/// count of members
	int 				*idx;			/// point ids of members	
	float 				*data;			/// data point values of members
	
	/* children */
	ClusterType			type;			/// Cluster type
	float				*cents;			/// centroids of all children
	DyArray				children;		/// children's ids
} Cluster;

void Cluster_init(Cluster *clu, int npts);
void Cluster_unset(Cluster *clu);
/**
 *	\brief	get the if of the i-th child
 */
int Cluster_getchild(Cluster *clu, int ichild);

/* ------------------------------------------------- the [adaptive hierarchical clustering] class -------------------------------------------- */

/**
 *	\brief	generate adaptive hierarchical clustering tree for a dataset 
 *	@param 	ahct	the adaptive hierarchical clustering tree
 *	@param 	bf		the branch factor
 *	@param 	rho		minimum count of a cluster
 *	@param 	ds 		the dataset
 */
void ahc_clustering(DyArray *ahct, int bf, int rho, const fDataSet *ds);

/**
 *	\brief	save index into files
 */
void ahc_index_into_file(DyArray *ahct, const char *folder, int d, int bf);

/**
 *	\brief	check out the index
 */
bool ahc_check_index(const char *folder);

/**
 *	\brief	load index from file
 */
bool ahc_index_from_file(DyArray *ahct, const char *folder);

/**
 *	\brief	load a cluster ided cid
 */
Cluster *ahc_load_a_cluster(const char *folder, int cid, int d, int bf);

/**
 *	\brief	count the leaf nodes in ahct
 */
int	ahc_count_leaf(DyArray *ahct);

/**
 *	\brief	for a point v, locate its nearest leaf cluster
 */
int	ahc_quantize(DyArray *ahct, float *v, int d);

/**
 *	\brief	unset the ach tree
 */
void ahc_unset(DyArray *ahct);

/* ------------------------------------------------- the Clustering class: usually for non-hierarchical clustering -------------------------------------------- */
/// \brief the Clustering class (usually for non-hierarchical clustering)
class Clustering
{
public:
	/// clustering results
	int					ncenter;		/// number of centroids
	float				*centroid;		/// the centroids
	int					*assign;		/// the belongingness of data points to clusters

	/// associate with the above data
	int					*nassign;		/// counts of members in each cluster
	int					**member;		/// member data points of each cluster
	
	/// neighbor clusters
	vector<vector<int> > neighbor;		/// neighbor cluster
	DoubleIndex			**innerLB;		/// inner distance between member data points and the boundaries in each cluster


	Clustering(int nc);
	~Clustering();

	/** \brief genarate clusters
	 * @param 		niter		number of iterations
	 * @param 		nth 		number of threads
	 * @param 		seed 	the seed of random
	 * @param 		nredo 	number of clustering re-runs
	 */
	void generate_cluster(fDataSet *ds, fDataSet *lds, int niter, int nth, int seed, int nredo);
	/// figure out the inner distance in each cluster, then sort the
	void inner_lb_distance(fDataSet *ds);
	/// estimate neighbor clusters for each cluster
	void neighbor_cluster_estimation(const fDataSet *ds, int nth);

};


#endif
