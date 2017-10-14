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
#define 	FileCentroid 			".centroid"
#define		FileMember 				".member"
#define 	FileInnerLB 			".innerlb"
#define 	FileNeighborCluster		".neighborcluster"

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
	int 				d;				/// dimension of centroid
	float				*centroid;		/// the centroids
	vector<int> 		assign;		/// the belongingness of data points to clusters
	vector<vector<int> > member;		/// member points of each cluster
	vector<vector<int> > neighbor;		/// neighbor cluster

	

	Clustering(int nc, int _d);
	~Clustering();

	/********************************** common operations ***********************************/
	/** \brief genarate clusters
	 * @param 		niter		number of iterations
	 * @param 		nth 		number of threads
	 * @param 		seed 	the seed of random
	 * @param 		nredo 	number of clustering re-runs
	 */
	void generate_cluster(fDataSet *ds, fDataSet *lds, int niter, int nth, int seed, int nredo);
	/**
	 * \brief Store cluster index structure into file
	 */
	void cluster_into_file(const char *folder);

	/**
  	 *	\brief check if cluster data exists and integrated
	 */
	bool cluster_exists(const char *folder);

	/**
	 * \brief Load cluster index structure from file
	 */
	bool load_cluster(const char *folder);

	
	/// estimate neighbor clusters for each cluster
	void neighbor_cluster_estimation(const fDataSet *ds, int nth);
};


#endif
